/***
  Copyright 2004-2008 Lennart Poettering
  Copyright (c) 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/
#include <pulsecore/config.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

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

#include <alsa/asoundlib.h>

#include "module-lab126-bt-sink-symdef.h"
#include "alsa-common.h"

/* #define TRACE_LOGS */
#ifdef TRACE_LOGS
#define pa_log_trace(...) pa_log_debug(##__VA_ARGS__)
#else
#define pa_log_trace(...) ((void)0)
#endif

PA_MODULE_AUTHOR("Amazon, Inc.");
PA_MODULE_DESCRIPTION("Lab126 Bluetooth Sink");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(FALSE);
PA_MODULE_USAGE(
        "sink_name=<name of sink> "
        "sink_properties=<properties for the sink> ");

#define DEFAULT_SINK_NAME "lab126-bt-sink"

#define DEFAULT_FORMAT PA_SAMPLE_S16LE
#define DEFAULT_ALSA_DEVICE "hw:0,2"
#define DEFAULT_SAMPLE_RATE 8000
#define DEFAULT_CHANNEL_COUNT 1
#define DEFAULT_PERIOD_SIZE 128
#define DEFAULT_PERIOD_COUNT 4


#define BLOCK_USEC (PA_USEC_PER_SEC * 2)
/* Never rewind closer than `REWIND_LIMIT_USECS' to the hw_ptr. */
#define REWIND_LIMIT_USECS 1375

struct userdata {
    pa_core *core;
    pa_module *module;
    pa_sink *sink;

    pa_thread *thread;
    pa_thread_mq thread_mq;
    pa_rtpoll *rtpoll;
    pa_rtpoll_item *rtpoll_item; /* for ALSA fd's */

    pa_usec_t block_usec;

    alsa_pcm_data *alsa;
};

static const char* const valid_modargs[] = {
    "sink_name",
    "sink_properties",
    NULL
};

static int sink_process_msg(
        pa_msgobject *o,
        int code,
        void *data,
        int64_t offset,
        pa_memchunk *chunk)
{

    struct userdata *u = PA_SINK(o)->userdata;

    switch (code) {
        case PA_SINK_MESSAGE_SET_STATE:

            break;
        case PA_SINK_MESSAGE_GET_LATENCY: {
            snd_pcm_uframes_t written;
            alsa_pcm_data *alsa = u->alsa;
            pa_usec_t delay;

            alsa_refresh_state(alsa);
            written = alsa->period_size * alsa->period_count - alsa->avail;

            delay = 1000000 * written / alsa->sample_rate;

            *((pa_usec_t*) data) = delay;

            return 0;
        }   break;
    }

    return pa_sink_process_msg(o, code, data, offset, chunk);
}

static void sink_update_requested_latency_cb(pa_sink *s)
{
    struct userdata *u;
    size_t nbytes;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    u->block_usec = pa_sink_get_requested_latency_within_thread(s);

    if (u->block_usec == (pa_usec_t) -1)
        u->block_usec = s->thread_info.max_latency;

    nbytes = pa_usec_to_bytes(u->block_usec, &s->sample_spec);
    pa_sink_set_max_rewind_within_thread(s, nbytes);
    pa_sink_set_max_request_within_thread(s, nbytes);
}

static void process_rewind(struct userdata *u)
{
    alsa_pcm_data *alsa;
    snd_pcm_sframes_t rewindable;
    snd_pcm_sframes_t rewind_buffer;
    size_t rewind_nbytes;

    pa_assert(u);

    alsa = u->alsa;

    rewind_nbytes = u->sink->thread_info.rewind_nbytes;

    if (!PA_SINK_IS_OPENED(u->sink->thread_info.state) || rewind_nbytes <= 0)
        goto do_nothing;

    pa_log_debug("Requested to rewind %lu bytes.", (unsigned long) rewind_nbytes);

    rewindable = snd_pcm_rewindable(alsa->pcm);
    if (rewindable < 0) {
        pa_log_error("Can not rewind PCM (%s)", snd_strerror(rewindable));
        goto do_nothing;
    }

    rewind_buffer = alsa->sample_rate * REWIND_LIMIT_USECS / 1000000;

    if (rewindable <= rewind_buffer) {
        rewindable = 0;
    } else {
        rewindable -= rewind_buffer;
    }

    if (!rewindable)
        goto do_nothing;

    rewindable = snd_pcm_rewind(alsa->pcm, rewindable);
    if (rewindable < 0) {
        pa_log_warn("Rewinding failed (%s)", snd_strerror(rewindable));
        goto do_nothing;
    }

    rewind_nbytes = (size_t)snd_pcm_frames_to_bytes(alsa->pcm, rewindable);
    pa_sink_process_rewind(u->sink, rewind_nbytes);

    return;

do_nothing:

    pa_sink_process_rewind(u->sink, 0);
}

static void process_render(struct userdata *u)
{
    alsa_pcm_data *alsa = NULL;
    snd_pcm_uframes_t avail_write;

    pa_assert(u);

    alsa = u->alsa;

    if (!alsa->pcm)
        return;

    /* thread_func() calls alsa_refresh_state() */

    switch (alsa->pcm_state) {
    case SND_PCM_STATE_XRUN:
        pa_log_error("sink: XRUN");
        /* fall through */
    case SND_PCM_STATE_OPEN:
    case SND_PCM_STATE_SETUP:
        snd_pcm_prepare(alsa->pcm);
        break;
    case SND_PCM_STATE_PREPARED:
    case SND_PCM_STATE_RUNNING:
    case SND_PCM_STATE_DRAINING:
    case SND_PCM_STATE_PAUSED:
        break;
    default:
        pa_log_error("Warning: PCM port in unexpected state 0x%x", alsa->pcm_state);
    }

    avail_write = alsa->avail;

    if (avail_write > alsa->period_size)
        avail_write = alsa->period_size;

    if (avail_write < alsa->period_size)
        return;

    /* Fill the buffer up the latency size */
    while (avail_write > 0) {
        pa_memchunk chunk;
        void *p;
        ssize_t bytes;
        snd_pcm_sframes_t frames;

        bytes = snd_pcm_frames_to_bytes(alsa->pcm, avail_write);

        /* TODO: Handle if this assert fails */
        pa_assert(bytes <= u->sink->thread_info.max_request);

        pa_memchunk_reset(&chunk);
        pa_sink_render(u->sink, bytes, &chunk);
        p = pa_memblock_acquire(chunk.memblock);

        frames = snd_pcm_bytes_to_frames(alsa->pcm, chunk.length);
        snd_pcm_writei(alsa->pcm, p + chunk.index, frames);
        avail_write -= frames;

        pa_memblock_release(chunk.memblock);
        pa_memblock_unref(chunk.memblock);
    }

}

static void thread_func(void *userdata)
{
    struct userdata *u = userdata;

    pa_assert(u);

    /* TODO: replace with the sink name that was passed as an arg to module */
    pa_log_debug(DEFAULT_SINK_NAME " I/O thread starting up");

    if (u->core->realtime_scheduling)
        pa_make_realtime(u->core->realtime_priority);

    pa_thread_mq_install(&u->thread_mq);

    for (;;) {
        int ret;

        pa_log_trace(DEFAULT_SINK_NAME " inner loop");

        if (u->alsa->pcm)
            alsa_refresh_state(u->alsa);

        if (PA_UNLIKELY(u->sink->thread_info.rewind_requested))
            process_rewind(u);

        if (PA_SINK_IS_OPENED(u->sink->thread_info.state))
            process_render(u);

        pa_rtpoll_set_timer_disabled(u->rtpoll);
        if (alsa_refresh_polling_fd(u->alsa, u->rtpoll_item) < 0)
            goto fail;

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

static int open_alsa_pcm(struct userdata *u)
{
    u->alsa->direction = SND_PCM_STREAM_PLAYBACK;

    if (alsa_open_port(u->alsa)) {
        pa_log_error("could not open PCM port");
        return -1;
    }

    return 0;
}

static int set_up_alsa_polling(struct userdata *u)
{
    pa_rtpoll_item *item;

    pa_assert(u);
    pa_assert(u->alsa);

    item = alsa_set_up_pa_polling(u->alsa, u->rtpoll);
    if (!item)
        return -1;

    u->rtpoll_item = item;

    return 0;
}

int pa__init(pa_module*m)
{
    struct userdata *u = NULL;
    pa_sample_spec ss;
    pa_channel_map map;
    pa_modargs *ma = NULL;
    pa_sink_new_data data;
    size_t nbytes;

    pa_assert(m);

    if (!(ma = pa_modargs_new(m->argument, valid_modargs))) {
        pa_log_error("Failed to parse module arguments.");
        goto fail;
    }

    ss.format = PA_SAMPLE_S16LE;
    ss.rate = 8000;
    ss.channels = 1;
    pa_channel_map_init_mono(&map);

    m->userdata = u = pa_xnew0(struct userdata, 1);
    if (!u) {
        pa_log_error("allocating userdata struct failed");
        goto fail;
    }

    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();
    if (!u->rtpoll) {
        pa_log_error("allocating rtpoll failed");
        goto fail;
    }

    u->alsa = pa_xnew0(struct alsa_pcm_data, 1);
    if (!u->alsa) {
        pa_log_error("allocating alsa_pcm_data failed");
        goto fail;
    }

    pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll);

    u->alsa->device = DEFAULT_ALSA_DEVICE;
    switch (ss.format) {
    case PA_SAMPLE_S16LE:
        u->alsa->format = SND_PCM_FORMAT_S16_LE;
        break;
    default:
        pa_assert(0 == "Unsupported audio format for module");
    }
    u->alsa->sample_rate = ss.rate;
    u->alsa->channels = DEFAULT_CHANNEL_COUNT;
    u->alsa->period_size = DEFAULT_PERIOD_SIZE;
    u->alsa->period_count = DEFAULT_PERIOD_COUNT;

    pa_sink_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;
    pa_sink_new_data_set_name(&data, pa_modargs_get_value(ma, "sink_name", DEFAULT_SINK_NAME));
    pa_sink_new_data_set_sample_spec(&data, &ss);
    pa_sink_new_data_set_channel_map(&data, &map);
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_DESCRIPTION, "Lab126 Bluetooth Output");
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_CLASS, "sound");

    if (pa_modargs_get_proplist(ma, "sink_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        pa_log_error("Invalid properties");
        pa_sink_new_data_done(&data);
        goto fail;
    }

    if (open_alsa_pcm(u)) {
        pa_log_error("Could not open ALSA PCM device");
        goto fail;
    }

    if (set_up_alsa_polling(u)) {
        pa_log_error("Could not set up ALSA PCM polling descriptors");
        goto fail;
    }

    u->sink = pa_sink_new(m->core, &data, PA_SINK_LATENCY|PA_SINK_DYNAMIC_LATENCY);
    pa_sink_new_data_done(&data);

    if (!u->sink) {
        pa_log_error("Failed to create sink object.");
        goto fail;
    }

    u->sink->parent.process_msg = sink_process_msg;
    u->sink->update_requested_latency = sink_update_requested_latency_cb;
    u->sink->userdata = u;

    pa_sink_set_asyncmsgq(u->sink, u->thread_mq.inq);
    pa_sink_set_rtpoll(u->sink, u->rtpoll);

    u->block_usec = BLOCK_USEC;
    nbytes = pa_usec_to_bytes(u->block_usec, &u->sink->sample_spec);
    pa_sink_set_max_rewind(u->sink, nbytes);
    pa_sink_set_max_request(u->sink, nbytes);

    if (!(u->thread = pa_thread_new("pa-126-btsink", thread_func, u))) {
        pa_log_error("Failed to create thread.");
        goto fail;
    }

    pa_sink_set_latency_range(u->sink, 0, BLOCK_USEC);

    pa_proplist_sets(u->sink->proplist, PA_PROP_DEVICE_BUS, "bluetooth");
    if (!pa_proplist_gets(u->sink->proplist, "bluetooth.protocol"))
        pa_proplist_sets(u->sink->proplist, "bluetooth.protocol", "hfgw");

    pa_sink_put(u->sink);

    pa_modargs_free(ma);

    pa_log_debug("%s exited successfully", __func__);

    return 0;

fail:
    if (u) {
        if (u->alsa) {
            alsa_close_port(u->alsa);
            pa_xfree(u->alsa);
        }

        if (u->rtpoll)
            pa_rtpoll_free(u->rtpoll);

        pa_xfree(u);
    }

    if (ma)
        pa_modargs_free(ma);

    pa__done(m);

    return -1;
}

int pa__get_n_used(pa_module *m)
{
    struct userdata *u;

    pa_assert(m);
    u = m->userdata;
    pa_assert_se(u);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module*m)
{
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata))
        return;

    if (u->sink)
        pa_sink_unlink(u->sink);

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->sink)
        pa_sink_unref(u->sink);

    if (u->rtpoll)
        pa_rtpoll_free(u->rtpoll);

    if (u->alsa)
        alsa_close_port(u->alsa);

    pa_xfree(u);
}
