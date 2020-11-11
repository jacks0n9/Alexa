 /***
 * module-lab126-source.c
 *
 * Copyright (c) 2011-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * Use is subject to license terms.
 **/

#include <pulsecore/config.h>


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/xmalloc.h>

#include <pulsecore/core-util.h>
#include <pulsecore/log.h>
#include <pulsecore/macro.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/source.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/thread.h>

#include <shmbuffer.h>
#include <poll.h>

#include "module-lab126-source-symdef.h"

PA_MODULE_AUTHOR("Amazon.com, Inc.");
PA_MODULE_DESCRIPTION("Lab126 shared memory buffer source");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(FALSE);
PA_MODULE_USAGE(
        "format=<sample format> "
        "channels=<number of channels> "
        "file=<name of the shared memory buffer started with slash > "
        "rate=<sample rate> "
        "source_name=<name of source> "
        "channel_map=<channel map> "
        "description=<description for the source> "
        "latency_time=<latency time in ms>");

// This default name can and shall be overwritten in configuration files
// in /etc/pulse
#define DEFAULT_SOURCE_NAME "lab126-source"

// This default name can and shall be overwritten in configuration files
// in /etc/pulse
#define DEFAULT_FILE_NAME "/BMicsOut.shm"

#define DEFAULT_LATENCY_TIME_USEC 20000
#define MAX_LATENCY_USEC 50000
// The pulseaudio based mics currently have various latency issues.  The max
// latency has been reduced to 50ms to minimize some of the long delays (2+
// seconds) we have seen.

struct userdata {
    pa_core *core;
    pa_module *module;
    pa_source *source;

    pa_thread *thread;
    pa_thread_mq thread_mq;
    pa_rtpoll *rtpoll;

    size_t block_size;

    pa_usec_t block_usec;

    pa_usec_t latency_time;

    char *filename;     //shared memory buffer name
    ShmbufHandle hShmB; //handle to Shared Memory Buffer
    ShmbufPollHandle hShmBPoll;
    pa_rtpoll_item *shmbuf_poll_item;

    pa_memchunk memchunk; //buffer to store data read grom Shared Memory Buffer
};

static const char* const valid_modargs[] = {
    "rate",
    "format",
    "channels",
    "source_name",
    "channel_map",
    "description",
    "latency_time",
    "file",
    NULL
};

static int source_process_msg(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk) {
    struct userdata *u = PA_SOURCE(o)->userdata;

    switch (code) {
        case PA_SOURCE_MESSAGE_SET_STATE: {
            int msg_size = ShmbufGetMsgSize(u->hShmB);

            if (PA_PTR_TO_UINT(data) == PA_SOURCE_RUNNING)
            {
                // adjust the initial latency
                int nbytes = pa_usec_to_bytes(u->latency_time, &u->source->sample_spec);
                nbytes /= msg_size;

                // or blindly set to 0 for minimal delay? as we know this
                // is for bluetooth only.
                ShmbufReadStart(u->hShmB, nbytes);
                pa_log_debug("PA_SOURCE_RUNNING set, latencytme=%lu, block_usec=%lu(0x%lx), latency=%lusamples",
                (unsigned long)(u->latency_time), (unsigned long)(u->block_usec),
                (unsigned long)(u->block_usec), (unsigned long)nbytes);
            }

            break;
        }

        case PA_SOURCE_MESSAGE_GET_LATENCY: {
            int msg_size = ShmbufGetMsgSize(u->hShmB);
            int samples = ShmbufDataAvail(u->hShmB);

            samples *= msg_size;
            pa_usec_t latency = pa_bytes_to_usec(samples, &u->source->sample_spec);

            *((pa_usec_t*) data) = latency;

            pa_log_debug("lab126src:latency=%lu", (unsigned long)latency);

            return 0;
        }
    }

    return pa_source_process_msg(o, code, data, offset, chunk);
}

static void source_update_requested_latency_cb(pa_source *s) {
    struct userdata *u;

    pa_source_assert_ref(s);
    u = s->userdata;
    pa_assert(u);

    u->latency_time = pa_source_get_requested_latency_within_thread(s);

    if (u->latency_time == (pa_usec_t) -1)
        u->latency_time = DEFAULT_LATENCY_TIME_USEC;

    pa_log_debug("requested: latency=%lu", (unsigned long)(u->latency_time));
}

#define SAMPLES_PER_TRANSFER    2048
#define POLL_INTERVALE  8000
static void thread_func(void *userdata) {
    struct userdata *u = userdata;
    int l;
    void *p;

    pa_assert(u);

    pa_log_debug("Thread starting up");

    if (u->core->realtime_scheduling)
        pa_make_realtime(u->core->realtime_priority);

    pa_thread_mq_install(&u->thread_mq);

    int msg_size = ShmbufGetMsgSize(u->hShmB);

    // Initialize start location for reading shared memory buffer
    // the return value is the seqnum number, which we don't care
    ShmbufReadStart(u->hShmB, 0);
    pa_rtpoll_set_timer_disabled(u->rtpoll);

    for (;;) {
        int ret;

        ShmbufPollAckFd(u->hShmBPoll);

        if (PA_SOURCE_IS_OPENED(u->source->thread_info.state))
        {
            int ret;

            // Read the shared memory buffer if there is enough data
            // By design we should never get blocked when we read the
            // shared memory buffer
            while ( (ret=ShmbufDataAvail(u->hShmB)) > SAMPLES_PER_TRANSFER)
            {
               // Allocate memory to store the audio samples read from shared memory buffer
                if (!u->memchunk.memblock) {
                    u->memchunk.memblock = pa_memblock_new(u->core->mempool,
                        SAMPLES_PER_TRANSFER*msg_size);
                    u->memchunk.index = u->memchunk.length = 0;
                }

                p = pa_memblock_acquire(u->memchunk.memblock);

                // Read shared memory buffer
                do
                {
                    l=ShmbufReadWait(u->hShmB, (uint8_t*) p, SAMPLES_PER_TRANSFER, 0);
                    if (l==E_SHMBUF_OVERWRITTEN)
                    {
                        pa_log("shmbuffer overrun for %s", u->filename);
                        //maybe should use the configured latency instead of 0?
                        ShmbufReadStart(u->hShmB, 0);
                    }
                    else if (l<0)
                    {
                        pa_memblock_release(u->memchunk.memblock);
                        pa_memblock_unref(u->memchunk.memblock);
                        pa_memchunk_reset(&u->memchunk);
                        goto fail;
                    }
                } while (l==E_SHMBUF_OVERWRITTEN);

                l *= msg_size;

                pa_memblock_release(u->memchunk.memblock);

                u->memchunk.length = (size_t) l;
                pa_source_post(u->source, &u->memchunk);
                u->memchunk.index += (size_t) l;

                pa_memblock_unref(u->memchunk.memblock);
                pa_memchunk_reset(&u->memchunk);

            }

            // check return value of ShmbufDataAvail()
            if (ret==E_SHMBUF_OVERWRITTEN)
            {
                 pa_log("shmbuffer overrun 2 for %s", u->filename);

                 //maybe should use the configured latency instead of 0?
                 ShmbufReadStart(u->hShmB, 0);
            }
            else if (ret<0)
            {
                 goto fail;
            }
        }

        /* Hmm, nothing to do. Let's sleep */
        if ((ret = pa_rtpoll_run(u->rtpoll, TRUE)) < 0)
        {
            goto fail;
        }

        if (ret == 0)
        {
            goto finish;
        }
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
    pa_source_new_data data;
    uint32_t latency_time = DEFAULT_LATENCY_TIME_USEC;
    struct pollfd *pollfd;

    pa_assert(m);

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

    m->userdata = u = pa_xnew0(struct userdata, 1);
    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();
    pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll);

    // Open shared memory buffer for read operation
    u->filename =  pa_runtime_path(pa_modargs_get_value(ma, "file", DEFAULT_FILE_NAME));
    u->hShmB = ShmbufOpen(u->filename, SHUMBUF_READ);

    if ( !u->hShmB )
    {
        pa_log("ShmbufOpen(%s) error", u->filename);
        goto fail;
    }

    u->hShmBPoll = ShmbufPollCreate(u->hShmB, u->core->realtime_priority);
    if (!u->hShmBPoll) {
        pa_log("ShmbufPollCreate() failed");
        goto fail;
    }

    u->shmbuf_poll_item = pa_rtpoll_item_new(u->rtpoll, PA_RTPOLL_NEVER, 1);
    pollfd = pa_rtpoll_item_get_pollfd(u->shmbuf_poll_item, NULL);
    pollfd->fd = ShmbufPollGetFd(u->hShmBPoll);
    pollfd->events = POLLIN;

    pa_source_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;
    pa_source_new_data_set_name(&data, pa_modargs_get_value(ma, "source_name", DEFAULT_SOURCE_NAME));
    pa_source_new_data_set_sample_spec(&data, &ss);
    pa_source_new_data_set_channel_map(&data, &map);
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_DESCRIPTION, pa_modargs_get_value(ma, "description", "lab126 source"));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_CLASS, "abstract");

    u->source = pa_source_new(m->core, &data, PA_SOURCE_LATENCY | PA_SOURCE_DYNAMIC_LATENCY);
    pa_source_new_data_done(&data);

    if (!u->source) {
        pa_log("Failed to create source object.");
        goto fail;
    }

    u->latency_time = DEFAULT_LATENCY_TIME_USEC;
    if (pa_modargs_get_value_u32(ma, "latency_time", &latency_time) < 0) {
        pa_log("Failed to parse latency_time value.");
        goto fail;
    }
    u->latency_time = latency_time*1000;    //converted to usec

    u->source->parent.process_msg = source_process_msg;
    u->source->update_requested_latency = source_update_requested_latency_cb;
    u->source->userdata = u;

    pa_source_set_asyncmsgq(u->source, u->thread_mq.inq);
    pa_source_set_rtpoll(u->source, u->rtpoll);

    pa_source_set_latency_range(u->source, 0, MAX_LATENCY_USEC);
    u->block_usec = u->source->thread_info.max_latency;
    pa_log("block_usec=%llu", u->block_usec);

    u->source->thread_info.max_rewind =
        pa_usec_to_bytes(u->block_usec, &u->source->sample_spec);

    if (!(u->thread = pa_thread_new("pa-126-src", thread_func, u))) {
        pa_log("Failed to create thread.");
        goto fail;
    }

    pa_source_put(u->source);

    pa_modargs_free(ma);

    return 0;

fail:
    if (ma)
        pa_modargs_free(ma);

    pa__done(m);

    return -1;
}

void pa__done(pa_module*m) {
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata))
    {
        return;
    }

    if (u->source)
    {
        pa_source_unlink(u->source);
    }

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->source)
    {
        pa_source_unref(u->source);
    }

    if (u->shmbuf_poll_item) {
        pa_rtpoll_item_free(u->shmbuf_poll_item);
    }

    if (u->rtpoll)
    {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->filename)
    {
        pa_xfree(u->filename);
    }

    if (u->hShmBPoll) {
        ShmbufPollDestroy(u->hShmBPoll);
    }

    // Close shared memory buffer
    if (u->hShmB)
    {
        ShmbufClose(u->hShmB);
    }

    pa_xfree(u);
}
