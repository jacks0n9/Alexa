/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2014       Amazon.com, Inc.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define AUDIO_VOICE_INTERFACE "org.bluez.Voice"

#define DEFAULT_VOICE_UPLOAD_CHANNEL 17

typedef enum {
	VOICE_STATE_DISCONNECTED,
	VOICE_STATE_CONNECTING,
	VOICE_STATE_CONNECTED,
	VOICE_STATE_PLAY_PENDING, /* Internal-only state */
	VOICE_STATE_PLAYING
} voice_state_t;

typedef enum {
	VOICE_LOCK_READ =  1 << 0,
	VOICE_LOCK_WRITE = 1 << 1,
} voice_lock_t;

uint32_t voice_config_init(GKeyFile *config);
void voice_connect_cb(GIOChannel *chan, GError *err, gpointer user_data);

struct voice *voice_init(struct audio_device *dev, const char *uuidstr);
void voice_unregister(struct audio_device *dev);
void voice_update(struct audio_device *dev, const char *uuidstr);

typedef void (*voice_state_cb) (struct audio_device *dev,
				voice_state_t old_state,
				voice_state_t new_state,
				void *user_data);
unsigned int voice_add_state_cb(voice_state_cb cb, void *user_data);
gboolean voice_remove_state_cb(unsigned int id);

void voice_set_state(struct audio_device *dev, voice_state_t new_state);
voice_state_t voice_get_state(struct audio_device *dev);
gboolean voice_is_active(struct audio_device *dev);
gboolean voice_is_connected(struct audio_device *dev);
int voice_connect_rfcomm(struct audio_device *dev, GIOChannel *io);
int voice_connect_sco(struct audio_device *dev, GIOChannel *chan);
GIOChannel *voice_get_rfcomm(struct audio_device *dev);

int voice_get_sco_fd(struct audio_device *dev);
gboolean voice_lock(struct audio_device *dev, voice_lock_t lock);
gboolean voice_unlock(struct audio_device *dev, voice_lock_t lock);

typedef void (*voice_stream_cb_t) (struct audio_device *dev, GError *err,
				   void *user_data);
unsigned int voice_request_stream(struct audio_device *dev,
				  voice_stream_cb_t cb, void *user_data);
gboolean voice_cancel_stream(struct audio_device *dev, unsigned int id);
void voice_suspend_stream(struct audio_device *dev);
