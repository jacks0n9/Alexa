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
#ifndef __LAB126_PULSEAUDIO_ALSA_COMMON_H__
#define __LAB126_PULSEAUDIO_ALSA_COMMON_H__

#include <alsa/asoundlib.h>
#include <pulsecore/rtpoll.h>

/**
 * Convenience structure for dealing with the ALSA PCM API
 *
 * This structure and its related functions are convenience helpers
 * for dealing with the ALSA PCM API. It provides helper methods for
 * opening a and closing a PCM port, setting the HW and SW parameters,
 * querying the status, and installing the polling file descriptors
 * into a PulseAudio rtpoll object.  Much of the code required for
 * these tasks is boilerplate and common for both capture and
 * playback.
 *
 * However, it still makes the raw PCM handles available to the user
 * of this API (especially the snd_pcm_t object) so that you can make
 * direct ALSA PCM API calls.  For example, for playback you must
 * directly call snd_pcm_writei() to write data to the device.
 */
typedef struct alsa_pcm_data {
    /** @defgroup alsa_pcm_data_user_supplied_inputs User-supplied inputs
     *
     * These must be fully filled out and valid before calling
     * `alsa_open_port'.  The values aren't fully documented here.
     * The terminology matches the ALSA documentation, so refer to the
     * ALSA PCM API documentation for details.
     * http://www.alsa-project.org/alsa-doc/alsa-lib/group___p_c_m.html
     *
     * {
     */
    const char *device;         ///< Device ID string (e.g. "hw:0,0")
    snd_pcm_stream_t direction; ///< ALSA SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURES
    snd_pcm_format_t format;    ///< ALSA SND_PCM_FORMAT_*
    unsigned sample_rate;       ///< Hardware audio sampling rate
    unsigned channels;          ///< Number of interleaved channels to send to the hardware
    unsigned period_size;       ///< Number of audio frames (1sample x Nchannels) per write
    unsigned period_count;      ///< Number of periods to use for hardware buffer
    unsigned start_threshold;   ///< If 0, will be set to 1/2 of buffer size
    unsigned stop_threshold;    ///< If 0, will be set to the buffer size
    unsigned avail_min;         ///< If 0, will be set to 1
    /* }
     */

    /** Handle to alsa pcm object */
    snd_pcm_t *pcm;

    /** Handle to alsa pcm statu object */
    snd_pcm_status_t *status;

    /** cached PCM state from `status' */
    snd_pcm_state_t pcm_state;

    /** cached avail audio frames from `status'. For a playback port,
     * this is the amount of space in the buffer available for writing
     * audio.  For a capture port, this is the number of bytes
     * available for reading.
     */
    snd_pcm_uframes_t avail;
} alsa_pcm_data;

/**
 * Open up an ALSA PCM port with the properties specified.
 *
 * @param alsa pointer to structure that has all the "user supplied
 * inputs" filled out (device, direction, format, etc.)
 *
 * @return 0 if port opened correctly and all of the "user-supplied
 * inputs" will be updated with the actual values that the hardware
 * granted.  If port fails to open, a negative return code will be
 * returned and the `pcm' and `status' pointers will be NULL.
 */
int alsa_open_port(alsa_pcm_data *alsa);

/**
 * Close an ALSA PCM port that was opened by alsa_open_port().
 *
 * Note: it is safe to call this even if alsa_open_port() returns an
 * error.
 *
 * @param alsa pointer to structure with alsa data.
 */
void alsa_close_port(alsa_pcm_data *alsa);

/**
 * convenience funtion to refresh the state with the hardware
 *
 * Updates the `state', `pcm_state', and `avail' members with the
 * current state of the hardware.
 *
 * @param alsa pointer to an alsa_pcm_data stucture that has been
 * opened.
 *
 * @return 0 on success, a negative number if it fails.
 */
int alsa_refresh_state(alsa_pcm_data *alsa);

/**
 * Updates the rtpoll_item with the correct polling file descriptors
 * from alsa
 *
 * @param alsa pointer to an open alsa_pcm_data structure.
 * @param rtpoll_item pointer to a pa_rtpoll_item structure where the
 * file descriptors belong.
 *
 * @return 0 if the rtpoll_item is successfully updated.  A negative
 * value if there is an error.
 */
int alsa_refresh_polling_fd(alsa_pcm_data *alsa, pa_rtpoll_item *rtpoll_item);

/**
 * Creates a pa_rtpoll_item for use with polling the PCM port
 *
 * @param alsa pointer to an open alsa_pcm_data structure.
 * @param rtpoll pointer to a pa_rtpoll structure that will be used
 * for polling the PCM port.
 *
 * @return If successful, a pointer to a pa_rtpoll_item that is
 * associated with rtpoll and to be used for the PCM port.  The file
 * descriptors will be initialized.  On failure, returns NULL.
 */
pa_rtpoll_item* alsa_set_up_pa_polling(alsa_pcm_data *alsa, pa_rtpoll *rtpoll);

/**
 * De-allocates the rtpoll_item created by alsa_set_up_pa_polling()
 *
 * @param alsa pointer to an open alsa_pcm_data structure.
 * @param i pointer to pa_rtpoll_item returned by alsa_set_up_pa_polling()
 */
void alsa_tear_down_pa_polling(alsa_pcm_data *alsa, pa_rtpoll_item *i);


#endif /* __LAB126_PULSEAUDIO_ALSA_COMMON_H__ */
