/***
  This file is part of PulseAudio.

  Copyright (c) 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include <errno.h>

#include <pulsecore/config.h>
#include <pulse/xmalloc.h>
#include <pulsecore/macro.h>
#include <pulsecore/hashmap.h>
#include <pulsecore/hook-list.h>
#include <pulsecore/core.h>
#include <pulsecore/core-util.h>
#include <pulsecore/sink-input.h>
#include <pulsecore/modargs.h>
#include <pulsecore/thread.h>
#include <pulsecore/mutex.h>

#include <shmqueue.h>
#include "module-lab126-audiod-symdefs.h"
#include "module-lab126-audiod-interface.h"

PA_MODULE_AUTHOR("Amazon.com, Inc.");
PA_MODULE_DESCRIPTION("Handle volume controls");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(TRUE);
PA_MODULE_USAGE("fileCmd=<Shm Q name>");

/// Default stream names for music and microphone
#define AUDIOD_STREAM_TYPE_MUSIC        "Music"
#define AUDIOD_STREAM_TYPE_MUSIC_BT     "Music-BT"
#define AUDIOD_STREAM_TYPE_MIC          "MIC"

/// Dictionary definitions
/// Maximum 10 different types of stream
/// Each type name must be 30 char or shorter
#define AUDIOD_DIC_MAX_NUM_ENTRIES      10
#define AUDIOD_DIC_MAX_NAME_SIZE        30

/// Maximum command size from shared memory
#define AUDIOD_MAX_COMMAND_SIZE         200

/// Define for 100%, 400%
#define _100_PERCENT                    100
#define _400_PERCENT                    400

/// Ramp duration for PA volume changes
#define VOLUME_RAMP_MS                  250

/// Ramp duration for PA volume changes when the speaker volume changes
#define VOLUME_SHORT_RAMP_MS            50

/// Decimal Base
#define BASE10                          10

/// Fixed gain applied to some streams
// The audio coming from BT is 5dB lower, this gain compensate for it
#define AUDIOD_GAIN_MUSIC_BT            ((float) 1.18)


static const char* const valid_modargs[] = {
    "fileCmd",
    NULL
};

/// Structure representing a volume state
typedef struct _volumeState {
    unsigned int volume;
    unsigned int volSpeaker;
    bool useVolSpeaker;
} volumeState;

/* A NOTE ABOUT THREADING:
 *
 * Everything in this file must execute on the `main' thread (also
 * called the `control' thread). This is different from the other
 * modules that spawn a thread for doing their I/O functions. The
 * reason is that this module must access the `core' data structures
 * and also set volumes on sinks... and these tasks may only be
 * performed from the `main' thread.
 *
 * The ShmQPollHandle spawns a thread to wait for events on the ShmQ,
 * and gets the audiod_event_cb to fire in the main loop using
 * shm_io_event.
 *
 * See also:
 * http://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/Developer/Threading/
 * http://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/Developer/CoreAPI/
 * http://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/Developer/MainLoop/
 */
struct userdata {
    pa_core *       core;
    pa_hook_slot *  sink_input_fixate_hook_slot;
    char *          filename;       // Shared memory file name
    ShmQHandle      hShmQ;          // Shared memory handle
    ShmQPollHandle  hShmQPoll;      // Signal that data is waiting
    pa_io_event *   shm_io_event;
    volumeState     dicVolume[AUDIOD_DIC_MAX_NUM_ENTRIES];
    char            dicName[AUDIOD_DIC_MAX_NUM_ENTRIES][AUDIOD_DIC_MAX_NAME_SIZE];
    unsigned int    prevSpeakVol;
};

/// Mapping from Percent to Absolute Volumes. These values follow
/// the doppler DAC Digital Volume vs volume (%) graph. PA volumes
//  calculated using ratio of these volumes lead to the desired
//  audible volume performance at different levels.
const float volumeMap[] = {5,
     5.853200,  6.526400,  7.199600,  7.872800,  8.546000,  9.219200,  9.892400, 10.565600, 11.238800, 11.912000,
    12.585200, 13.258400, 13.931600, 14.604800, 15.2, 16.4, 17.6, 18.8, 20, 21.2,
    22.4, 23.6, 24.8, 26, 27.2, 28.4, 29.6, 30.8, 32, 33.2,
    34.4, 35.6, 36.8, 38, 39.2, 40.4, 41.6, 42.8, 44, 45.2,
    46.4, 47.6, 48.8, 50, 51.2, 52.4, 53.6, 54.8, 56, 57.2,
    58.4, 59.6, 60.8, 62, 63.2, 64.4, 65.6, 66.8, 68, 69.2,
    70.4, 71.6, 72.8, 74, 75.2, 76.4, 77.6, 78.8, 80, 81.2,
    82.4, 83.6, 84.8, 86, 87.2, 88.4, 89.6, 90.8, 92, 93.2,
    94.4, 95.6, 96.8, 98, 99.2, 100.4, 101.6, 102.8, 104, 105.2,
    106.4, 107.6, 108.8, 110, 111.2, 112.4, 113.6, 114.8, 116, 117.2
};

static volumeState getVolume(struct userdata *u, const char * pStr);

/*********************************************************
 * Convert volume from percent to PA value
 * @param   volState state containing volume values in percent
 * @return  volume for PA
 ********************************************************/
static unsigned int percentToPaVolume(volumeState volState)
{
    bool relative = false;
    float paVolumeRatio = 1.0f;
    unsigned int volume;
    unsigned int volStream = (volState.volume * volState.volSpeaker) /_100_PERCENT;

    // If the volume is zero, there is no normalization to perform, so just return 0.
    if (volState.volume == 0) {
        return 0;
    }

    if (volState.useVolSpeaker && volStream < _100_PERCENT
            && volState.volSpeaker < _100_PERCENT) {
        paVolumeRatio = volumeMap[volStream] / volumeMap[volState.volSpeaker];
        if (paVolumeRatio < 1) {
            relative = true;
        }
    }

    if (relative) {
        // PA Volume adjustment is relative to the current speaker volume
        volume = (PA_VOLUME_NORM - PA_VOLUME_MUTED) * paVolumeRatio;
    } else {
        // Simple conversion from volume in percent to PA volume
        volume = ((PA_VOLUME_NORM - PA_VOLUME_MUTED) * volState.volume) /_100_PERCENT;
    }

    pa_log_debug( "AUDIOD: %s Relative %d, Stream Volume(%%=%d,abs=%d,mapped=%f), "
            "Speaker Volume(abs=%d,mapped=%f), Volume Ratio = %f, PA Volume = %d",
            __FUNCTION__, relative, volState.volume, volStream, volumeMap[volStream],
            volState.volSpeaker, volumeMap[volState.volSpeaker], paVolumeRatio, volume);

    // Limit volume to valid range
    PA_CLAMP_VOLUME(volume);

    return volume;
}


/*********************************************************
 * Applies a gain on the volume based on the stream name
 * @param   name of the stream
 * @param   Current pa volume
 * @return  pa volume after gain
 ********************************************************/
static unsigned int applyStreamGain( const char * role, unsigned int volume)
{
    unsigned int volumeOut = volume;
    if(role) {
        // init gain with no gain
        float gain = 1.0;
        if( !strcmp(role, AUDIOD_STREAM_TYPE_MUSIC_BT)) {
            gain = AUDIOD_GAIN_MUSIC_BT;
        }

        if(gain != 1.0) {
            volumeOut = (unsigned int) (gain * volume);
            PA_CLAMP_VOLUME(volumeOut);
            pa_log("Stream gain %s vol %d gain %f out %d", role, volume, gain, volumeOut);
        }
    }
    return volumeOut;
}

/*********************************************************
 * Set the volume of a type of stream to the given value
 * @param   user data
 * @param   type of stream to set value
 * @param   volState state containing volume values in percent
 * @param   length_in_ms, ramp length in milliseconds
 * @return  0 is OK
 ********************************************************/
static int setVolumeRamp(  struct userdata *u, const char *role_to_apply,
        volumeState volState, int length_in_ms)
{
    pa_sink *s;
    pa_sink_input *j;
    uint32_t idx, idx_sink;

    pa_log_debug( "AUDIOD: %s >>> Set Volume %s to %d in %dms",
                  __FUNCTION__, role_to_apply, volState.volume, length_in_ms);
    pa_assert(u);

    // loop over all of the sinks, we should have limited sinks
    PA_IDXSET_FOREACH(s, u->core->sinks, idx_sink) {
        pa_sink_assert_ref(s);
        // loop over all the sink-inputs of the sink
        PA_IDXSET_FOREACH(j, s->inputs, idx) {
            const char *role;

            if( !(role = pa_proplist_gets(j->proplist, PA_PROP_MEDIA_ROLE))){
                role = AUDIOD_STREAM_TYPE_MUSIC;
            }

            if( !pa_streq(role, role_to_apply)) {
                continue;
            }
            {
                pa_sink_input_state_t state;
                // Apply fixed gain on some streams
                unsigned int paVolume  = percentToPaVolume(volState);
                paVolume = applyStreamGain( role, paVolume);


                // Change the volume for a sink-input stream only when
                // it is in Linked State. It is possible for the stream
                // to be in INIT state or Gstreamer might have moved it
                // to an UNLINKED state. In both these cases changing
                // the volume is an invalid action in pulseaudio which
                // will cause an assert.
                state = pa_sink_input_get_state(j);
                if(PA_SINK_INPUT_IS_LINKED(state))
                {
                    pa_cvolume_ramp vol_ramp;
                    int len = length_in_ms;
                    if(state == PA_SINK_INPUT_CORKED) {
                        // No ramp when the stream is paused
                        len = 0;
                    }
                    pa_cvolume_ramp_set(&vol_ramp, 1, PA_VOLUME_RAMP_TYPE_LINEAR, len, paVolume);
                    //Two TRUEs below, one for save_volume, one for absolute.
                    pa_sink_input_set_volume_ramp(j, &vol_ramp, TRUE, TRUE);
                    pa_log("AUDIOD: Stream '%s' new volume %d in %d ms ramp", role_to_apply, paVolume, len);
                }
                else
                {
                    pa_log("AUDIOD: %s stream not in Linked State. Cannot change its volume.",
                            role_to_apply);
                }
            } // loop over sink-inputs of the sink
        }
    }// loop over the sink

    return 0;
}

/*********************************************************
 * Set the volume of a type of stream to the given value
 * @param   user data
 * @param   type of stream to set value
 * @param   volState state containing volume values in percent
 * @return  0 is OK
 ********************************************************/
static int setVolume( struct userdata *u, const char *role_to_apply, volumeState volState)
{
    // Only apply ramp when the speaker volume does not change
    unsigned int newSpeakVol = volState.volSpeaker;
    if( volState.useVolSpeaker && u->prevSpeakVol != newSpeakVol) {
        u->prevSpeakVol = newSpeakVol;
        return setVolumeRamp(u, role_to_apply, volState, VOLUME_SHORT_RAMP_MS);
    }
    else {
        return setVolumeRamp(u, role_to_apply, volState, VOLUME_RAMP_MS);
    }
}


/*********************************************************
 * Callback hook when a new stream connects
 * sink_input_fixate_hook is called early (e.g. before sink_put() is called)
 * @param   PA Core struct
 * @param   PA new data struct
 * @param   user data
 * @return  PA OK
 ********************************************************/
static pa_hook_result_t sink_input_fixate_hook_callback(pa_core *c, pa_sink_input_new_data *new_data, struct userdata *u)
{
    const char null_name[] = "(Null)";
    const char *name;

    pa_assert(c);
    pa_assert(new_data);
    pa_assert(u);

    pa_log_debug( "AUDIOD: %s >>>", __FUNCTION__);
    if (!new_data->volume_writable) {
        // some sink input'v volume is not changeable
        pa_log("AUDIOD: Not restoring volume for sink input, its volume can't be changed.");
        // for doppler, should assert(0) because of the role check above
    }
    else if (new_data->volume_is_set) {
        pa_log("AUDIOD: Not restoring volume for sink input, already set.");
        // for doppler, should assert(0) because of the role check above
    }
    else {
        pa_cvolume v;
        pa_channel_map map;
        volumeState volState;

        name = pa_proplist_gets(new_data->proplist, PA_PROP_MEDIA_ROLE);
        // Get volume for that Media Role
        // Handles null name
        volState = getVolume(u, name);
        v.channels = 1;
        v.values[0] = percentToPaVolume(volState);

        // call a function here to apply gain if Music-BT
        v.values[0] = applyStreamGain( name, v.values[0]);

        pa_channel_map_init_mono(&map);
        pa_cvolume_remap(&v, &map, &new_data->channel_map);

        pa_sink_input_new_data_set_volume(new_data, &v);

        new_data->volume_is_absolute = FALSE;
        new_data->save_volume = TRUE;

        if(!name) {
            name = null_name;
        }
        pa_log("AUDIOD: Hooked %s volume %d", name, volState.volume);
    }
    return PA_HOOK_OK;
}

/*********************************************************
 * Save volume of a type of stream
 * @param   user data
 * @param   type of stream to save value
 * @param   volState state containing volume values in percent
 * @return  0 is OK
 ********************************************************/
static int saveVolume( struct userdata *u, const char * pStr, volumeState volState)
{
    int status = -1;
    int i;

    for(i = 0; i < AUDIOD_DIC_MAX_NUM_ENTRIES; i++){
        char * pName = u->dicName[i];
        if(*pName == 0){
            // Empty entry - save the volume here
            strncpy( pName, pStr, AUDIOD_DIC_MAX_NAME_SIZE);
            u->dicVolume[i] = volState;
            status = 0;
            break;
        }
        else if( !strcmp(pName, pStr)){
            u->dicVolume[i] = volState;
            status = 0;
            break;
        }
    }

    return status;
}

/*********************************************************
 * Get volume of a type of stream
 * Default return value is 100 if the stream is not in our known list.
 * @param   user data
 * @param   type of stream to save value
 * @return  volume state containing volume values in percent
 ********************************************************/
static volumeState getVolume(struct userdata *u, const char * pStr)
{
    int i;
    volumeState volState;
    volState.volume = 100;
    volState.volSpeaker = 100;
    volState.useVolSpeaker = true;

    // If the volume type isn't set, default to music.
    if(!pStr) {
        pStr = AUDIOD_STREAM_TYPE_MUSIC;
    }

    for(i = 0; i < AUDIOD_DIC_MAX_NUM_ENTRIES; i++){
        char * pName = u->dicName[i];
        if(*pName == 0){
            break;
        }
        else if( !strcmp(pName, pStr)){
            volState = u->dicVolume[i];
            break;
        }
    }

    pa_log_debug( "AUDIOD: %s Stream Type %s, Volume %d, Speaker Volume %d, (%s)",
            __FUNCTION__, pStr, volState.volume, volState.volSpeaker,
            volState.useVolSpeaker? "Use" : "Don't Use");

    return volState;
}

/*********************************************************
 * Callback to handle commands from Audiod
 * Runs in the main thread
 *********************************************************
 */
void audiod_event_cb(pa_mainloop_api *ea, pa_io_event *e, int fd, pa_io_event_flags_t events, void *userdata)
{
    struct userdata *u = userdata;

    pa_assert(u);

    int status;
    tAudiodApiCmd cmd;
    unsigned cmds_to_handle;

    ShmQPollAckFd(u->hShmQPoll);
    cmds_to_handle = ShmQDataAvail(u->hShmQ) / sizeof(cmd);

    while (cmds_to_handle--) {
        status = ShmQRead(u->hShmQ, &cmd, sizeof(cmd), sizeof(cmd));
        if (status == 0) {
            continue;
        }
        else if( status != sizeof(cmd) || cmd.sync != AUDIOD_API_SYNC) {
            pa_log("AUDIOD: Error reading shmQ %d %d %X", status, sizeof(cmd), cmd.sync);
            // If for some reason the handle is null
            if(!u->hShmQ) {
                pa_log("AUDIOD: Error reading shmQ NULL Handle!!");
                // Non recoverable error
                u->core->mainloop->io_enable(u->shm_io_event, PA_IO_EVENT_NULL);
                ShmQPollDestroy(u->hShmQPoll);
            }
            else {
                // Error handling - queue in bad state - discard the data and try again
                ShmQDiscardData(u->hShmQ);
            }
            continue;
        }
        else {
            // Process the command
            volumeState volState;
            // Force terminate the string
            cmd.type[AUDIOD_API_TYPE_LEN-1] = 0;
            pa_log_debug("AUDIOD: Rx cmd %s %d %d", cmd.type, cmd.volStream, cmd.volSpeak);
            volState.volume = cmd.volStream;
            volState.volSpeaker = cmd.volSpeak;
            volState.useVolSpeaker = true;

            status = saveVolume( u, (const char *) cmd.type, volState);
            if( status){
                pa_log("AUDIOD: Could not save volume %d for %s", volState.volume, cmd.type);
            }
            status = setVolume( u, (const char *) cmd.type, volState);
            if( status){
                pa_log("AUDIOD: Could not set volume %d for %s", volState.volume, cmd.type);
            }
        }
    }
}

/*********************************************************
 * Init called when module is loaded
 * @param   PA module struct
 * @return  0 for OK
 ********************************************************/
int pa__init(pa_module *m) {
    pa_modargs *ma = NULL;
    struct userdata *u;

    pa_assert(m);

    ma = pa_modargs_new(m->argument, valid_modargs);
    if( !ma) {
        pa_log("AUDIOD: Failed to parse module arguments");
        goto fail;
    }

    m->userdata = u = pa_xnew0(struct userdata, 1);

    u->core = m->core;

    // Open shared memory for command read operation
    u->filename =  pa_runtime_path(pa_modargs_get_value(ma, "fileCmd", ""));
    u->hShmQ = ShmQOpen(u->filename);

    if( !u->hShmQ) {
        pa_log("AUDIOD: ShmQOpen(%s) error", u->filename);
        goto fail;
    }
    pa_log_debug("AUDIOD: Shared mem %s blck", u->filename);

    u->hShmQPoll = ShmQPollCreate(u->hShmQ, 1, u->core->realtime_priority);
    if( !u->hShmQPoll) {
        pa_log("AUDIOD: ShmQPollCreate() error");
        goto fail;
    }

    u->shm_io_event = u->core->mainloop->io_new(u->core->mainloop,
                                                ShmQPollGetFd(u->hShmQPoll),
                                                PA_IO_EVENT_NULL,
                                                audiod_event_cb,
                                                u);
    if (!u->shm_io_event) {
        pa_log("AUDIOD: unable to create shm_io_event");
        goto fail;
    }

    // Boost microphone output by 400% (12dB) in BT-HFP mode
    volumeState volState;
    volState.volume = _400_PERCENT;
    volState.volSpeaker = 0;
    volState.useVolSpeaker = false;
    saveVolume( u, AUDIOD_STREAM_TYPE_MIC, volState);

    u->prevSpeakVol = 0;

    u->sink_input_fixate_hook_slot = pa_hook_connect(&m->core->hooks[PA_CORE_HOOK_SINK_INPUT_FIXATE],
            PA_HOOK_EARLY, (pa_hook_cb_t) sink_input_fixate_hook_callback, u);

    pa_modargs_free(ma);

    /* Ready to roll... */
    u->core->mainloop->io_enable(u->shm_io_event, PA_IO_EVENT_INPUT);

    return 0;

fail:
    pa__done(m);

    if( ma) {
        pa_modargs_free(ma);
    }

    return -1;
}

/*********************************************************
 * De-Init called when module is unloaded
 * @param   PA module struct
 * @return  None
 ********************************************************/
void pa__done(pa_module *m) {
    struct userdata* u;
    pa_log("AUDIOD: PA_done!!");

    pa_assert(m);

    u =  m->userdata;
    if( !u) {
        return;
    }

    if( u->sink_input_fixate_hook_slot) {
        pa_hook_slot_free(u->sink_input_fixate_hook_slot);
    }

    if( u->shm_io_event) {
        u->core->mainloop->io_free(u->shm_io_event);
    }

    if( u->hShmQPoll) {
        ShmQPollDestroy(u->hShmQPoll);
    }

    if( u->hShmQ) {
        ShmQHangUp(u->hShmQ, SHMQUEUE_HANGUP_READER);
        ShmQClose(u->hShmQ);
    }
    if( u->filename) {
        pa_xfree(u->filename);
    }

    pa_xfree(u);
}
