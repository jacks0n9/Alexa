/***
  Copyright (c) 2013 Amazon.com, Inc. or its affiliates. All Rights Reserved.

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

#include <alsa/asoundlib.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/log.h>

#include "alsa-common.h"

/* #define TRACE_LOGS */
#ifdef TRACE_LOGS
#define pa_log_trace(...) pa_log_debug(##__VA_ARGS__)
#else
#define pa_log_trace(...) ((void)0)
#endif

static int setup_hw_params(alsa_pcm_data *alsa)
{
    snd_pcm_hw_params_t *hw_params = NULL;
    snd_pcm_format_t tmp_format;
    unsigned tmp_sample_rate;
    unsigned tmp_channels;
    snd_pcm_uframes_t tmp_period_size;
    unsigned tmp_period_count;
    int err;

    err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0) {
        pa_log_error("Could not allocate hw params: %s", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params_any(alsa->pcm, hw_params);
    if (err < 0) {
        pa_log_error("Could not initialize hw_params: %s", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params_set_access(alsa->pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        pa_log_error("Could not set access type to %d: %s", SND_PCM_ACCESS_RW_INTERLEAVED,
               snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params_set_channels(alsa->pcm, hw_params, alsa->channels);
    if (err < 0) {
        pa_log_error("Could not set channels to %u: %s", alsa->channels, snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params_set_format(alsa->pcm, hw_params, alsa->format);
    if (err < 0) {
        pa_log_error("Could not set audio format to %u: %s", alsa->format, snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params_set_rate(alsa->pcm, hw_params, alsa->sample_rate, 0);
    if (err < 0) {
        pa_log_error("Could not set sample rate to %u: %s", alsa->sample_rate, snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params_set_period_size(alsa->pcm, hw_params, alsa->period_size, 0);
    if (err < 0) {
        pa_log_error("Could not set period size to <=512: %s", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params_set_periods(alsa->pcm, hw_params, alsa->period_count, 0);
    if (err < 0) {
        pa_log_error("Could not set period count to <=4: %s", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_hw_params(alsa->pcm, hw_params);
    if (err < 0) {
        pa_log_error("Could not set hw_params: %s", snd_strerror(err));
        goto fail;
    }

    /* Query hw_params for the final values awarded */
    err = snd_pcm_hw_params_get_format(hw_params, &tmp_format);
    if (err < 0) {
        pa_log_error("Could not get format after setting hw_params: %s", snd_strerror(err));
        goto fail;
    }
    err = snd_pcm_hw_params_get_rate(hw_params, &tmp_sample_rate, 0);
    if (err < 0) {
        pa_log_error("Could not get sample rate after setting hw_params: %s", snd_strerror(err));
        goto fail;
    }
    err = snd_pcm_hw_params_get_channels(hw_params, &tmp_channels);
    if (err < 0) {
        pa_log_error("Could not get channel count after setting hw_params: %s", snd_strerror(err));
        goto fail;
    }
    err = snd_pcm_hw_params_get_period_size(hw_params, &tmp_period_size, 0);
    if (err < 0) {
        pa_log_error("Could not get period size after setting hw_params: %s", snd_strerror(err));
        goto fail;
    }
    err = snd_pcm_hw_params_get_periods(hw_params, &tmp_period_count, 0);
    if (err < 0) {
        pa_log_error("Could not get period count after setting hw_params: %s", snd_strerror(err));
        goto fail;
    }

    if (tmp_format != alsa->format) {
        pa_log_warn("ALSA changed format from %u to %u",
                    alsa->format, tmp_format);
        alsa->format = tmp_format;
    }

    if (tmp_sample_rate != alsa->sample_rate) {
        pa_log_warn("ALSA changed sample rate from %u to %u",
                    alsa->sample_rate, tmp_sample_rate);
        alsa->sample_rate = tmp_sample_rate;
    }

    if (tmp_channels != alsa->channels) {
        pa_log_warn("ALSA changed channel count from %u to %u", alsa->channels,
                    tmp_channels);
        alsa->channels = tmp_channels;
    }

    if (tmp_period_size != alsa->period_size) {
        pa_log_warn("ALSA changed period size from %u to %u", alsa->period_size,
                    tmp_period_size);
        alsa->period_size = tmp_period_size;
    }

    if (tmp_period_count != alsa->period_count) {
        pa_log_warn("ALSA changed period count from %u to %u",
                    alsa->period_count, tmp_period_count);
        alsa->period_count = tmp_period_count;
    }

    snd_pcm_hw_params_free(hw_params);
    hw_params = NULL;

    return 0;

fail:
    if (hw_params)
        snd_pcm_hw_params_free(hw_params);

    return -1;
}

static int setup_sw_params(alsa_pcm_data *alsa)
{
    snd_pcm_sw_params_t *sw_params = NULL;
    snd_pcm_uframes_t tmp_start_thresh;
    snd_pcm_uframes_t tmp_stop_thresh;
    snd_pcm_uframes_t tmp_avail_min;
    int err;

    err = snd_pcm_sw_params_malloc(&sw_params);
    if (err < 0) {
        pa_log_error("Could not allocate sw_params (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params_current(alsa->pcm, sw_params);
    if (err < 0) {
        pa_log_error("Couldn't get the default sw_params (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params_set_avail_min(alsa->pcm, sw_params, alsa->avail_min);
    if (err < 0) {
        pa_log_error("Couldn't set the sw_params avail min (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params_set_start_threshold(alsa->pcm, sw_params, alsa->start_threshold);
    if (err < 0) {
        pa_log_error("Couldn't set the sw_params start thresh (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params_set_stop_threshold(alsa->pcm, sw_params, alsa->stop_threshold);
    if (err < 0) {
        pa_log_error("Couldn't set the sw_params stop thresh (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params(alsa->pcm, sw_params);
    if (err < 0) {
        pa_log_error("Couldn't set the sw_params (%s)", snd_strerror(err));
        goto fail;
    }

    /* Query and update with current values */
    err = snd_pcm_sw_params_current(alsa->pcm, sw_params);
    if (err < 0) {
        pa_log_error("Couldn't get the default sw_params (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params_get_avail_min(sw_params, &tmp_avail_min);
    if (err < 0) {
        pa_log_error("couldn't retrieve the sw_params avail_min (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params_get_start_threshold(sw_params, &tmp_start_thresh);
    if (err < 0) {
        pa_log_error("couldn't retreive the sw_params start_thresh (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_sw_params_get_stop_threshold(sw_params, &tmp_stop_thresh);
    if (err < 0) {
        pa_log_error("couldn't retrieve the sw_params stop_thresh (%s)", snd_strerror(err));
        goto fail;
    }

    if (alsa->avail_min != tmp_avail_min) {
        pa_log_warn("for sw_params, requested avail_min %u but got %u",
                    alsa->avail_min, tmp_avail_min);
        alsa->avail_min = tmp_avail_min;
    }

    if (alsa->start_threshold != tmp_start_thresh) {
        pa_log_warn("for sw_params, requested start_thresh %u but got %u",
                    alsa->start_threshold, tmp_start_thresh);
        alsa->start_threshold = tmp_start_thresh;
    }

    if (alsa->stop_threshold != tmp_stop_thresh) {
        pa_log_warn("for sw_params, requested stop_thresh %u but got %u",
                    alsa->stop_threshold, tmp_stop_thresh);
        alsa->stop_threshold = tmp_stop_thresh;
    }

    snd_pcm_sw_params_free(sw_params);

    return 0;

fail:
    if (sw_params)
        snd_pcm_sw_params_free(sw_params);

    return -1;
}

static int setup_status(alsa_pcm_data *alsa)
{
    int err;

    err = snd_pcm_status_malloc(&alsa->status);
    if (err < 0) {
        pa_log_error("could not allocate snd_pcm_status_t (%s)", snd_strerror(err));
        goto fail;
    }

    err = snd_pcm_status(alsa->pcm, alsa->status);
    if (err < 0) {
        pa_log_error("could not get initial state of alsa port (%s)", snd_strerror(err));
        goto fail;
    }

    return 0;

fail:
    if (alsa->status)
        snd_pcm_status_free(alsa->status);

    alsa->status = NULL;

    return -1;
}

int alsa_open_port(alsa_pcm_data *alsa)
{
    int err;

    /* Check inputs */

    if (!alsa) {
        pa_log_error("alsa struct is null");
        goto fail;
    }

    if (alsa->pcm) {
        pa_log_error("pcm pointer is not null");
        goto fail;
    }

    if (alsa->status) {
        pa_log_error("status pointer is not null");
        goto fail;
    }

    if (!alsa->device) {
        pa_log_error("device string is null");
        goto fail;
    }

    /* Set default values */
    if (!alsa->start_threshold)
        alsa->start_threshold = alsa->period_size * (alsa->period_count / 2);

    if (!alsa->stop_threshold)
        alsa->stop_threshold = alsa->period_size * alsa->period_count;

    if (!alsa->avail_min)
        alsa->avail_min = 1;

    if (alsa->start_threshold >= alsa->stop_threshold) {
        pa_log_warn("start_threshold (%ul) >= stop_threshold (%ul) is probably a mistake",
                    (unsigned long)alsa->start_threshold,
                    (unsigned long)alsa->stop_threshold);
    }

    err = snd_pcm_open(&alsa->pcm, alsa->device, alsa->direction, 0 /* mode */);
    if (err < 0) {
        pa_log_error("Could not open PCM %s: %s", alsa->device, snd_strerror(err));
        goto fail_clean;
    }

    err = snd_pcm_nonblock(alsa->pcm, 0);
    if (err < 0) {
        pa_log_error("Could not set blocking mode: %s", snd_strerror(err));
        goto fail_clean;
    }

    err = setup_hw_params(alsa);
    if (err < 0)
        goto fail_clean;

    err = setup_sw_params(alsa);
    if (err < 0)
        goto fail_clean;


    err = setup_status(alsa);
    if (err < 0)
        goto fail_clean;

    err = snd_pcm_prepare(alsa->pcm);
    if (err < 0) {
        pa_log_error("could not call PREPARE on pcm: %s", snd_strerror(err));
        goto fail_clean;
    }

    return 0;

fail_clean:

    alsa_close_port(alsa);

fail:
    return -1;
}

void alsa_close_port(alsa_pcm_data *alsa)
{
    if (alsa->status)
        snd_pcm_status_free(alsa->status);

    if (alsa->pcm)
        snd_pcm_close(alsa->pcm);

    alsa->status = NULL;
    alsa->pcm = NULL;
}

int alsa_refresh_state(alsa_pcm_data *alsa)
{
    int err;

    err = snd_pcm_status(alsa->pcm, alsa->status);
    if (err < 0) {
        pa_log_error("couldn't get status of PCM port: %s", snd_strerror(err));
        return err;
    }

    alsa->pcm_state = snd_pcm_status_get_state(alsa->status);
    alsa->avail = snd_pcm_status_get_avail(alsa->status);

    return 0;
}

int alsa_refresh_polling_fd(alsa_pcm_data *alsa, pa_rtpoll_item *rtpoll_item)
{
    struct pollfd *pfd;
    unsigned nfd;
    int err;

    pfd = pa_rtpoll_item_get_pollfd(rtpoll_item, &nfd);
    if (!pfd) {
        pa_log_error("pa_rtpoll_item_get_pollfd didn't return a valid pointer");
        return -1;
    }

    err = snd_pcm_poll_descriptors(alsa->pcm, pfd, nfd);
    if (err < 0) {
        pa_log_error("snd_pcm_poll_descriptors faild: %s", snd_strerror(errno));
        return err;
    }

    return 0;
}

/* Return >0 to skip poll and continue with next iteration */
static int before_polling_callback(pa_rtpoll_item *i)
{
    struct alsa_pcm_data *alsa = NULL;
    int err;

    alsa = pa_rtpoll_item_get_userdata(i);

    err = alsa_refresh_state(alsa);
    if (err < 0)
        return 1;

    switch (alsa->pcm_state) {
    case SND_PCM_STATE_XRUN:
    case SND_PCM_STATE_OPEN:
    case SND_PCM_STATE_SETUP:
    case SND_PCM_STATE_PREPARED:
    case SND_PCM_STATE_PAUSED:
        pa_log_trace("before_polling_callback(): nothing to do");
        return 0;
        break;
    case SND_PCM_STATE_RUNNING:
    case SND_PCM_STATE_DRAINING:
        break;
    default:
        pa_log_error("Warning: PCM port in unexpected state 0x%x", alsa->pcm_state);
    };

    if (alsa->avail >= alsa->period_size)
        return alsa->avail;

    pa_log_trace("before_polling_callback(): nothing to do");

    return 0;
}

pa_rtpoll_item* alsa_set_up_pa_polling(alsa_pcm_data *alsa, pa_rtpoll *rtpoll)
{
    pa_rtpoll_item *i = NULL;
    struct pollfd *pfd;
    int n;
    unsigned m;
    int err;

    n = snd_pcm_poll_descriptors_count(alsa->pcm);
    if (n < 0) {
        pa_log_error("could not get polling descriptor count: %s", snd_strerror(n));
        goto fail;
    }

    i = pa_rtpoll_item_new(rtpoll, PA_RTPOLL_NORMAL, (unsigned)n);
    if (!i) {
        pa_log_error("could not create a new rtpoll item");
        goto fail;
    }

    pfd = pa_rtpoll_item_get_pollfd(i, &m);
    if (!pfd) {
        pa_log_error("could not get pointer to pollfd");
        goto fail;
    }

    err = alsa_refresh_polling_fd(alsa, i);
    if (err < 0)
        goto fail;

    pa_rtpoll_item_set_userdata(i, alsa);
    pa_rtpoll_item_set_before_callback(i, before_polling_callback);

    return i;

fail:
    if (i)
        pa_rtpoll_item_free(i);

    return NULL;
}

void alsa_tear_down_pa_polling(alsa_pcm_data *data, pa_rtpoll_item *i)
{
    pa_rtpoll_item_free(i);
}
