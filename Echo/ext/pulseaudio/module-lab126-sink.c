/************************************************************************
 * module-lab126-sink.c
 *
 * Copyright (c) 2011-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * Use is subject to license terms.
 ****************************************************************/

#include <pulsecore/config.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <poll.h>

#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/xmalloc.h>

#include <pulsecore/i18n.h>
#include <pulsecore/macro.h>
#include <pulsecore/sink.h>
#include <pulsecore/module.h>
#include <pulsecore/core-util.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>
#include <pulsecore/thread.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/rtpoll.h>

#include <shmbuffer.h>
#include <shmqueue.h>
#include <llog.h>

#include "module-lab126-sink-symdef.h"

/* define LAB126_SINK_DEBUG */

PA_MODULE_AUTHOR("Amazon.com, Inc.");
PA_MODULE_DESCRIPTION("Lab126 shared memory queue sink");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(FALSE);
PA_MODULE_USAGE(
        "sink_name=<name of sink> "
        "sink_properties=<properties for the sink> "
        "file=<name of the shared memory queue started with slash > "
        "powerfile=<name of the shared memory buffer started with slash > "
        "format=<sample format> "
        "rate=<sample rate> "
        "channels=<number of channels> "
        "channel_map=<channel map> "
        "fragment_usecs=<ideal write size> "
        "minimum_full_fragments=<minimum fragments in buffer>");

#define DEFAULT_SINK_NAME "lab126-sink"

/* Default values for module arguments. May be overwritten
 * in configuration files in /etc/pulse/
 */
#define DEFAULT_FILE_NAME "/QSpeakerIn.shm"
#define DEFAULT_FRAGMENT_USECS 16000
#define DEFAULT_MINIMUM_FULL_FRAGMENTS 4
/* When audioproxyd reads the data from the shm, it takes about .136 sec to
 * reach the speaker. This was estimated using some loopback tests.
 */
#define DEFAULT_STATIC_OUTPUT_LATENCY_USECS 136000

/* DYNAMIC LATENCY MANAGEMENT
 *
 * PA has a dynamic latency management scheme that's a big part of
 * it's low-power/low-cpu strategy.  We enable it as long as we have a
 * sufficiently large buffer. Otherwise we use "low latency"
 * logic. For this module the internal strategy is as follows:
 *
 * Consider the "downstream" ShmQ Buffer:
 *
 *                -->|  |<-- minimum fragment (shmq_fragment_bytes)
 *                   |  |
 * |--|--|--|--|--|--|--|--|--|--|--|--|--|--|
 * A           A  A        A           A     A
 * |           |  |        |           |     +-- End of buffer (shmq_size_bytes)
 * |           |  |        |           |
 * |           |  |        |           +-- Max fill (buffer_always_free_bytes)
 * |           |  |        |
 * |           |  |        +-- Wake-up threshold (buffer_free_bytes_thresh)
 * |           |  |
 * |           |  +-- Minimum fill threshold (buffer_minimum_full_bytes)
 * |           |
 * |           +-- Minimum required threshold (shmq_minimum_full_bytes)
 * |
 * +-- Downstream read pointer
 *
 * The buffer is divided into fragments that match the read chunk size
 * of the downstream application. As much as possible, we always
 * read/write a single fragment.
 *
 * Minimum required threshold: The buffer must be at least this full
 * in order to prevent underruns for the downstream process.
 *
 * Minimum fill threshold: The buffer must be at least this full in
 * order to exit from the process_render() function. If we request a
 * fragment and get back a short fragment, we give up if this
 * threshold is met.
 *
 * Wake-up threshold: After processing some audio, we sleep until the
 * buffer falls below the Wake-up threshold.
 *
 * Max fill: We do not fill the buffer any more than this. This size
 * is set based on the latency that PA configured for this module.
 *
 */

struct userdata {
    pa_core *core;
    pa_module *module;
    pa_sink *sink;
    pa_sink_flags_t sink_flags;

    pa_thread *thread;
    pa_thread_mq thread_mq;
    pa_rtpoll *rtpoll;

    /* DYNAMIC LATENCY MANAGEMENT */
    unsigned buffer_free_bytes_thresh;
    unsigned buffer_always_free_bytes;
    unsigned buffer_minimum_full_bytes;
    unsigned buffer_max_request;
    unsigned fragment_bytes;

    char *filename;
    char *powerfilename;


    /* SHMQ BUFFER PROPERTIES */
    ShmQHandle hShmQ;
    ShmQPollHandle hShmQPoll;
    pa_rtpoll_item *shmq_poll_item;
    unsigned shmq_size_bytes;
    unsigned shmq_fragment_bytes;
    unsigned shmq_minimum_full_bytes;

    pa_memchunk memchunk;

    unsigned int static_output_latency_usecs;

    ShmbufHandle hPowerBufferShmHandle;
};

unsigned int LLOG_G_LOG_MASK;

static const char* const valid_modargs[] = {
    "sink_name",
    "sink_properties",
    "file",
    "powerfile",
    "format",
    "rate",
    "channels",
    "channel_map",
    "fragment_usecs",
    "minimum_full_fragments",
    "static_output_latency_usecs",
    NULL
};

static int sink_process_msg(
        pa_msgobject *o,
        int code,
        void *data,
        int64_t offset,
        pa_memchunk *chunk) {

    struct userdata *u = PA_SINK(o)->userdata;

    switch (code) {
        case PA_SINK_MESSAGE_SET_STATE:

            if (PA_PTR_TO_UINT(data) == PA_SINK_RUNNING)
            {
                //nothing to do here at the moment
            }

            break;

        case PA_SINK_MESSAGE_GET_LATENCY: {

            int samples = ShmQDataAvail(u->hShmQ);

            pa_usec_t latency = pa_bytes_to_usec(samples, &u->sink->sample_spec);
            latency += u->static_output_latency_usecs;

            *((pa_usec_t*) data) = latency;

#if LAB126_SINK_DEBUG
            pa_log_debug("lab126sink:latency=%lu", (unsigned long)latency);
#endif

            return 0;
        }

        case PA_SINK_MESSAGE_PLAYBACK_AVERAGE_POWER:
            // Write the power value into shared memory buffer
            ShmbufWrite(u->hPowerBufferShmHandle, (unsigned char*)(&offset), 1);
            return 0;
    }

    return pa_sink_process_msg(o, code, data, offset, chunk);
}

static void sink_update_requested_latency_cb(pa_sink *s) {
    struct userdata *u;
    pa_usec_t requested_latency_raw;
    pa_usec_t requested_latency;
    pa_usec_t buffer_usec;
    pa_usec_t max_latency_usec;
    size_t requested_latency_bytes;

    pa_log_info("m-l126-sink: sink_update_requested_latency_cb");

    pa_sink_assert_ref(s);
    u = s->userdata;
    pa_assert_se(u);

    requested_latency_raw = pa_sink_get_requested_latency_within_thread(s);
    requested_latency = requested_latency_raw;
    buffer_usec = pa_bytes_to_usec(u->shmq_size_bytes, &s->sample_spec);
    max_latency_usec = buffer_usec + u->static_output_latency_usecs;

    if (requested_latency == (pa_usec_t) -1) {
        requested_latency = s->thread_info.max_latency;
    }

    if (requested_latency > max_latency_usec) {
        requested_latency = max_latency_usec;
    }

    if (requested_latency < s->thread_info.min_latency) {
        requested_latency = s->thread_info.min_latency;
    }

    pa_assert(requested_latency > u->static_output_latency_usecs);
    requested_latency -= u->static_output_latency_usecs;
    requested_latency_bytes = pa_usec_to_bytes(requested_latency, &s->sample_spec);

    /* A "fragment" is our quantum unit
     *
     * There must always be 2 fragments in buffer for audioproxyd
     *
     * The remaining is divided by 2 for our write size
     *
     * The following truths must hold:
     *
     *     buffer_nfrags = latency_nfrags + always_empty_nfrags;
     */
    size_t latency_nfrags;
    size_t min_nfrags;
    size_t buffer_nfrags;
    size_t always_empty_nfrags;

    latency_nfrags = lrintf((float)requested_latency_bytes / u->shmq_fragment_bytes);
    min_nfrags = lrintf((float)u->shmq_minimum_full_bytes / u->shmq_fragment_bytes);
    buffer_nfrags = u->shmq_size_bytes / u->shmq_fragment_bytes;

    /* PA has requested someing below s->thread_info.min_latency */
    pa_assert(latency_nfrags > min_nfrags);

    if (latency_nfrags > buffer_nfrags) {
        latency_nfrags = buffer_nfrags;
    }

    always_empty_nfrags = buffer_nfrags - latency_nfrags;

    /* latency_nfrags = downstream_nfrags + variable_nfrags
     */
    pa_assert(u->shmq_minimum_full_bytes % u->shmq_fragment_bytes == 0);
    size_t downstream_nfrags = u->shmq_minimum_full_bytes / u->shmq_fragment_bytes;
    size_t variable_nfrags = latency_nfrags - downstream_nfrags;

    /* thresh_nfrags = variable_nfrags / 2
     */

    size_t thresh_nfrags = variable_nfrags / 2;

    if (thresh_nfrags < 4) {
        thresh_nfrags = variable_nfrags - 1;
    }

    size_t fragment_nfrags = thresh_nfrags;

    u->buffer_free_bytes_thresh = (thresh_nfrags + always_empty_nfrags) * u->shmq_fragment_bytes;
    ShmQPollSetThreshold(u->hShmQPoll, u->buffer_free_bytes_thresh);
    u->buffer_always_free_bytes = u->shmq_size_bytes - latency_nfrags * u->shmq_fragment_bytes;
    u->buffer_minimum_full_bytes = (1 + downstream_nfrags) * u->shmq_fragment_bytes;
    u->buffer_max_request = latency_nfrags * u->shmq_fragment_bytes;
    u->fragment_bytes = fragment_nfrags * u->shmq_fragment_bytes;

    pa_sink_set_max_request_within_thread(s, u->buffer_max_request);

    pa_log_info("requested_latency_raw=%lu requested_latency=%lu buffer_nfrags=%lu latency_nfrags=%lu variable_nfrags=%lu fragment_nfrags=%lu always_empty_nfrags=%lu",
                (unsigned long)requested_latency_raw,
                (unsigned long)requested_latency,
                (unsigned long)buffer_nfrags,
                (unsigned long)latency_nfrags,
                (unsigned long)variable_nfrags,
                (unsigned long)fragment_nfrags,
                (unsigned long)always_empty_nfrags);
}

static void process_rewind(struct userdata *u) {
    size_t rewind_request_bytes;
    int actual_rewind;

    pa_assert(u);

    if (!(u->sink_flags & PA_SINK_DYNAMIC_LATENCY)) {
        pa_log_info("REWIND ignored");
        pa_sink_process_rewind(u->sink, 0);
        return;
    }

    rewind_request_bytes = u->sink->thread_info.rewind_nbytes;
    u->sink->thread_info.rewind_nbytes = 0;

    actual_rewind = ShmQWriteRewind(u->hShmQ, rewind_request_bytes, u->shmq_minimum_full_bytes);
    if (actual_rewind > 0) {
        pa_sink_process_rewind(u->sink, actual_rewind);
    } else {
        pa_sink_process_rewind(u->sink, 0);
    }

    pa_log_info("REWIND: requested=%u,actual=%d,left=%d", rewind_request_bytes, actual_rewind, ShmQDataAvail(u->hShmQ));

    return;
}

static void process_render(struct userdata *u) {
    int avail;
    int want;
    ssize_t written;

    pa_assert(u);

    avail = ShmQSpaceAvail(u->hShmQ);

    if (avail < u->buffer_free_bytes_thresh) {
        return;
    }

    want = avail - u->buffer_always_free_bytes;
    if (want > u->buffer_max_request) {
        want = u->buffer_max_request;
    }

    for ( ; want > 0 ; want -= written)
    {
        void *p;
        int fragment;

        written = 0;

        // heavy-lifting here, i.e. get data from all sink-inputs,
        // resampling, channel-mapping, mixing, etc.
        fragment = (want > u->shmq_fragment_bytes) ? u->shmq_fragment_bytes : want;
        pa_sink_render(u->sink, fragment, &u->memchunk);

#ifdef LAB126_SINK_DEBUG
        pa_log_debug(
            "Received Lab126 Data:CurShmData:%d,SpaceAvail: %d FrameLen:%d",
            ShmQDataAvail(u->hShmQ), ShmQSpaceAvail(u->hShmQ),
            u->memchunk.length);
#endif

        pa_assert(u->memchunk.length>0);
        pa_assert(!(u->memchunk.length&1));

        p = pa_memblock_acquire(u->memchunk.memblock);
        written = ShmQWrite(u->hShmQ, p + u->memchunk.index, u->memchunk.length);
        pa_memblock_release(u->memchunk.memblock);

        pa_assert(written == u->memchunk.length);

        if (written != u->memchunk.length)
        {
            pa_log("ShmQWrite:requested=%d,ret=%d", u->memchunk.length, written);
            pa_memblock_unref(u->memchunk.memblock);
            pa_memchunk_reset(&u->memchunk);
            break;
        }

        pa_memblock_unref(u->memchunk.memblock);
        pa_memchunk_reset(&u->memchunk);
    }
}

static void thread_func(void *userdata) {
    struct userdata *u = userdata;

    pa_assert(u);

    pa_log_debug("Thread starting up");

    if (u->core->realtime_scheduling)
        pa_make_realtime(u->core->realtime_priority);

    pa_thread_mq_install(&u->thread_mq);
    pa_rtpoll_set_timer_disabled(u->rtpoll);

    for (;;) {
        int ret;

        ShmQPollAckFd(u->hShmQPoll);

        /* Render some data and drop it immediately */
        if (PA_SINK_IS_OPENED(u->sink->thread_info.state))
        {

            if (u->sink->thread_info.rewind_requested)
            {
                if (u->sink->thread_info.rewind_nbytes > 0)
                {
                    process_rewind(u);
                }
                else
                {
                    pa_sink_process_rewind(u->sink, 0);
                }
            }

            process_render(u);
        }

        /* Hmm, nothing to do. Let's sleep */
        if ((ret = pa_rtpoll_run(u->rtpoll, TRUE)) < 0)
            goto fail;

        if (ret == 0)
            goto finish;
    }

fail:
    /* If this was no regular exit from the loop we have to continue
     * processing messages until we received PA_MESSAGE_SHUTDOWN */
    pa_asyncmsgq_post(u->thread_mq.outq, PA_MSGOBJECT(u->core), PA_CORE_MESSAGE_UNLOAD_MODULE, u->module, 0, NULL, NULL);
    pa_asyncmsgq_wait_for(u->thread_mq.inq, PA_MESSAGE_SHUTDOWN);

finish:
    pa_log_debug("Thread shutting down");
}

int pa__init(pa_module*m) {
    struct userdata *u = NULL;
    pa_sample_spec ss;
    pa_channel_map map;
    pa_modargs *ma = NULL;
    pa_sink_new_data data;
    uint32_t fragment_usecs = DEFAULT_FRAGMENT_USECS;
    uint32_t minimum_full_fragments = DEFAULT_MINIMUM_FULL_FRAGMENTS;
    uint32_t static_output_latency_usecs = DEFAULT_STATIC_OUTPUT_LATENCY_USECS;
    struct pollfd *pollfd;

    pa_assert(m);

    /* for shmbuffer logging */
    LLOG_INIT("module-lab126-sink", LLOG_FACILITY_DEFAULT);

    if (!(ma = pa_modargs_new(m->argument, valid_modargs))) {
        pa_log("Failed to parse module arguments.");
        goto fail;
    }

    ss = m->core->default_sample_spec;
    map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        pa_log("Invalid sample format specification or channel map");
        goto fail;
    }

    if (pa_modargs_get_value_u32(ma, "fragment_usecs", &fragment_usecs)) {
        pa_log("Invalid fragment_usecs specification");
        goto fail;
    }

    if (pa_modargs_get_value_u32(ma, "minimum_full_fragments", &minimum_full_fragments)) {
        pa_log("Invalid minimum_full_usecs specification");
        goto fail;
    }

    if (pa_modargs_get_value_u32(ma, "static_output_latency_usecs", &static_output_latency_usecs)) {
        pa_log("Invalid static_output_latency_usecs specification");
        goto fail;
    }

    m->userdata = u = pa_xnew0(struct userdata, 1);
    u->core = m->core;
    u->module = m;
    pa_memchunk_reset(&u->memchunk);
    u->rtpoll = pa_rtpoll_new();
    pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll);

    // Open shared memory for playback power write operation
    u->powerfilename = pa_runtime_path(pa_modargs_get_value(ma, "powerfile", ""));

    if (pa_endswith(u->powerfilename, ".shm")) {
        u->hPowerBufferShmHandle = ShmbufOpen(u->powerfilename, SHUMBUF_WRITE);

        if ( !u->hPowerBufferShmHandle ) {
            pa_log("ShmbufOpen(%s) error", u->powerfilename);
            goto fail;
        }
    }

    u->filename = pa_runtime_path(pa_modargs_get_value(ma, "file", DEFAULT_FILE_NAME));

    u->hShmQ = ShmQOpen(u->filename);
    if ( !u->hShmQ )
    {
        pa_log("ShmQOpen(%s) error", u->filename);
        goto fail;
    }

    u->hShmQPoll = ShmQPollCreate(u->hShmQ, 0, u->core->realtime_priority);
    if (!u->hShmQPoll) {
        pa_log("ShmQPollCreate() failed");
        goto fail;
    }

    u->shmq_poll_item = pa_rtpoll_item_new(u->rtpoll, PA_RTPOLL_NEVER, 1);
    pollfd = pa_rtpoll_item_get_pollfd(u->shmq_poll_item, NULL);
    pollfd->fd = ShmQPollGetFd(u->hShmQPoll);
    pollfd->events = POLLIN;

    pa_sink_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;
    pa_sink_new_data_set_name(&data, pa_modargs_get_value(ma, "sink_name", DEFAULT_SINK_NAME));

    pa_sink_new_data_set_sample_spec(&data, &ss);
    pa_sink_new_data_set_channel_map(&data, &map);
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_DESCRIPTION, pa_modargs_get_value(ma, "description", _("Lab126 sink")));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_CLASS, "abstract");
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING, u->filename);

    if (pa_modargs_get_proplist(ma, "sink_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        pa_log("Invalid properties");
        pa_sink_new_data_done(&data);
        goto fail;
    }

    u->shmq_size_bytes = ShmQCapacity(u->hShmQ);
    pa_assert(u->shmq_size_bytes > 0);
    u->shmq_fragment_bytes = pa_usec_to_bytes(fragment_usecs, &ss);
    u->shmq_minimum_full_bytes = minimum_full_fragments * u->shmq_fragment_bytes;
    u->static_output_latency_usecs = static_output_latency_usecs;

    u->buffer_free_bytes_thresh = u->shmq_fragment_bytes;
    u->buffer_minimum_full_bytes = u->shmq_size_bytes;
    u->buffer_always_free_bytes = 0;
    u->buffer_max_request = u->shmq_size_bytes;
    u->fragment_bytes = u->shmq_fragment_bytes;

    u->sink_flags = PA_SINK_LATENCY;

    if (u->shmq_size_bytes > (u->shmq_minimum_full_bytes + u->shmq_fragment_bytes)) {
        u->sink_flags |= PA_SINK_DYNAMIC_LATENCY;
    }

    u->sink = pa_sink_new(m->core, &data, u->sink_flags);
    pa_sink_new_data_done(&data);

    if (!u->sink) {
        pa_log("Failed to create sink object.");
        goto fail;
    }

    u->sink->parent.process_msg = sink_process_msg;
    u->sink->update_requested_latency = sink_update_requested_latency_cb;
    u->sink->userdata = u;

    pa_sink_set_asyncmsgq(u->sink, u->thread_mq.inq);
    pa_sink_set_rtpoll(u->sink, u->rtpoll);

    if (u->sink_flags & PA_SINK_DYNAMIC_LATENCY) {
        pa_usec_t buffer_usec = pa_bytes_to_usec(u->shmq_size_bytes, &u->sink->sample_spec);
        pa_usec_t min_latency_usec = pa_bytes_to_usec(
            u->shmq_minimum_full_bytes + u->shmq_fragment_bytes, &u->sink->sample_spec);
        min_latency_usec += u->static_output_latency_usecs;
        pa_sink_set_max_rewind(u->sink, u->shmq_size_bytes - u->shmq_minimum_full_bytes);
        pa_sink_set_latency_range(u->sink, min_latency_usec,
                                  buffer_usec + u->static_output_latency_usecs);
    } else {
        pa_sink_set_max_rewind(u->sink, 0);
    }
    pa_sink_set_max_request(u->sink, u->buffer_max_request);


    if (!(u->thread = pa_thread_new("pa-126-sink", thread_func, u))) {
        pa_log("Failed to create thread.");
        goto fail;
    }

    pa_sink_put(u->sink);

    pa_modargs_free(ma);

    return 0;

fail:
    if (ma)
        pa_modargs_free(ma);

    pa__done(m);

    return -1;
}

int pa__get_n_used(pa_module *m) {
    struct userdata *u;

    pa_assert(m);
    u = m->userdata;
    pa_assert_se(u);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module*m) {
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata))
    {
        return;
    }

    if (u->sink)
    {
        pa_sink_unlink(u->sink);
    }

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->sink)
    {
        pa_sink_unref(u->sink);
    }

    if (u->filename) {
        pa_xfree(u->filename);
    }

    if (u->shmq_poll_item) {
        pa_rtpoll_item_free(u->shmq_poll_item);
    }

    if (u->hShmQPoll) {
        ShmQPollDestroy(u->hShmQPoll);
    }

    if (u->hShmQ)
    {
        ShmQClose(u->hShmQ);
    }

    if (u->rtpoll)
    {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->powerfilename) {
        pa_xfree(u->powerfilename);
    }

    if (u->hPowerBufferShmHandle) {
        ShmbufClose(u->hPowerBufferShmHandle);
    }

    pa_xfree(u);
}
