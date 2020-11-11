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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <glib.h>
#include <dbus/dbus.h>
#include <gdbus.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/uuid.h>

#include "glib-helper.h"
#include "btio.h"

#include "manager.h"
#include "device.h"
#include "voice.h"
#include "log.h"
#include "error.h"
#include "btio.h"
#include "dbus-common.h"
#include "sdp-client.h"
#include "../src/adapter.h"
#include "../src/device.h"

/* #define INCLUDE_FUNCTION_TRACE */

#ifdef INCLUDE_FUNCTION_TRACE
#define TRACE() DBG("%s %s:%d", __func__, __FILE__, __LINE__);
#else
#define TRACE() ((void)0)
#endif

#define BUF_SIZE 1024

#define VOICE_SETTING_INVALID 0xFFFFU
#define VOICE_SETTING_MASK    0x03FFU
#define VOICE_SETTING_TRANS   0x0003U /* transparent eSCO */

static gboolean sco_hci = TRUE;

static GSList *active_devices = NULL;

typedef enum {
	VOICE_ERROR_DISCONNECTED,
	VOICE_ERROR_SUSPENDED,
} voice_error_t;

static char *str_state[] = {
	"VOICE_STATE_DISCONNECTED",
	"VOICE_STATE_CONNECTING",
	"VOICE_STATE_CONNECTED",
	"VOICE_STATE_PLAY_PENDING",
	"VOICE_STATE_PLAYING",
};

struct voice_state_callback {
	voice_state_cb cb;
	void *user_data;
	unsigned int id;
};

struct connect_cb {
	unsigned int id;
	voice_stream_cb_t cb;
	void *cb_data;
};

struct voice_slc {
	char buf[BUF_SIZE];
	int data_start;
	int data_length;
};

struct voice {
	uint32_t handle;

	int rfcomm_ch;

	GIOChannel *rfcomm;
	GIOChannel *sco;
	guint sco_id;
	GSList *callbacks;

	voice_state_t state;

	gboolean search_vox;
	voice_lock_t lock;

	uint16_t stored_voice_setting;

	struct voice_slc *slc;
};

typedef enum {
	VOICE_REPLY_ERROR,
	VOICE_REPLY_OK,
	VOICE_REPLY_ERROR_NOT_SUPPORTED,
} voice_reply_t;

struct event {
	const char *cmd;
	int (*callback) (struct audio_device *device, const char *buf);
};

static GSList *voice_callbacks = NULL;

static int rfcomm_connect(struct audio_device *device, voice_stream_cb_t cb,
				void *user_data, unsigned int *cb_id);
static int get_records(struct audio_device *device);

#define VOICE_ERROR voice_error_quark()

static GQuark voice_error_quark(void)
{
	return g_quark_from_static_string("voice-error-quark");
}

static const char *state2str(voice_state_t state)
{
	switch (state) {
	case VOICE_STATE_DISCONNECTED:
		return "disconnected";
	case VOICE_STATE_CONNECTING:
		return "connecting";
	case VOICE_STATE_CONNECTED:
	case VOICE_STATE_PLAY_PENDING:
		return "connected";
	case VOICE_STATE_PLAYING:
		return "playing";
	}

	return NULL;
}

static int voice_send_valist(struct voice *vox, char *format, va_list ap)
{
	char rsp[BUF_SIZE];
	ssize_t total_written, count;
	int fd;

	TRACE();

	count = vsnprintf(rsp, sizeof(rsp), format, ap);

	if (count < 0)
		return -EINVAL;

	if (!vox->rfcomm) {
		error("voice_send: the voice is not connected");
		return -EIO;
	}

	total_written = 0;
	fd = g_io_channel_unix_get_fd(vox->rfcomm);

	while (total_written < count) {
		ssize_t written;

		written = write(fd, rsp + total_written,
				count - total_written);
		if (written < 0)
			return -errno;

		total_written += written;
	}

	return 0;
}

static int __attribute__((format(printf, 2, 3)))
			voice_send(struct voice *vox, char *format, ...)
{
	va_list ap;
	int ret;

	TRACE();

	va_start(ap, format);
	ret = voice_send_valist(vox, format, ap);
	va_end(ap);

	return ret;
}

static int voice_generic_rsp(struct audio_device *device, voice_reply_t err)
{
	struct voice *vox = device->voice;

	TRACE();

	switch (err) {
	case VOICE_REPLY_OK:
		return voice_send(vox, "OK\r\n");
	default:
		return voice_send(vox, "ERROR\r\n");
	}
}

uint32_t voice_config_init(GKeyFile *config)
{
	GError *err = NULL;
	char *str;

	TRACE();

	/* Use the default values if there is no config file */
	if (config == NULL)
		return 0;

	str = g_key_file_get_string(config, "General", "SCORouting",
					&err);
	if (err) {
		DBG("audio.conf: %s", err->message);
		g_clear_error(&err);
	} else {
		if (strcmp(str, "PCM") == 0)
			sco_hci = FALSE;
		else if (strcmp(str, "HCI") == 0)
			sco_hci = TRUE;
		else
			error("Invalid Voice Routing value: %s", str);
		g_free(str);
	}

	return 0;
}

gboolean voice_is_connected(struct audio_device *dev)
{
	struct voice *vox = dev->voice;

	TRACE();

	if (vox->state != VOICE_STATE_DISCONNECTED)
		return TRUE;

	return FALSE;
}

gboolean voice_is_active(struct audio_device *dev)
{
	struct voice *vox = dev->voice;

	TRACE();

	if (vox->state != VOICE_STATE_DISCONNECTED)
		return TRUE;

	return FALSE;
}

static void save_voice_setting(struct audio_device *dev)
{
	struct voice *vox = dev->voice;
	struct btd_device *btd = dev->btd_dev;
	struct btd_adapter *adapter = device_get_adapter(btd);

	if (btd_adapter_read_voice_setting(adapter, &vox->stored_voice_setting) < 0) {
		error("Could not read/save the voice setting");
		vox->stored_voice_setting = VOICE_SETTING_INVALID;
	}
}

static void restore_voice_setting(struct audio_device *dev)
{
	struct voice *vox = dev->voice;
	struct btd_device *btd = dev->btd_dev;
	struct btd_adapter *adapter = device_get_adapter(btd);

	TRACE();

	if (vox->stored_voice_setting == VOICE_SETTING_INVALID) {
		return;
	}

	if (btd_adapter_write_voice_setting(adapter, vox->stored_voice_setting) < 0) {
		error("Could not restore the voice setting");
		return;
	}

	vox->stored_voice_setting = VOICE_SETTING_INVALID;
}

static int initiate_voice(struct audio_device *dev, const char *buf)
{
	struct btd_device *btd = dev->btd_dev;
	struct btd_adapter *adapter = device_get_adapter(btd);
	struct voice *vox = dev->voice;

	TRACE();

	if (vox->state < VOICE_STATE_CONNECTED) {
		error("Attempt to start voice stream before connecting");
		voice_generic_rsp(dev, VOICE_REPLY_ERROR);
		return -1;
	} else if (vox->state > VOICE_STATE_PLAY_PENDING) {
		error("Attempt to start voice stream while one in progress");
		error("Stopping previous sco stream to allow future ones to proceed");
		voice_set_state(dev, VOICE_STATE_CONNECTED);
		voice_generic_rsp(dev, VOICE_REPLY_ERROR);
		return -1;
	}

	save_voice_setting(dev);

	if (btd_adapter_write_voice_setting(adapter, VOICE_SETTING_TRANS) < 0) {
		error("Could not write voice setting");
		voice_generic_rsp(dev, VOICE_REPLY_ERROR);
		return -1;
	}

	voice_set_state(dev, VOICE_STATE_PLAY_PENDING);

	return voice_generic_rsp(dev, VOICE_REPLY_OK);
}

static struct event event_callbacks[] = {
	{ "ATD:V1", initiate_voice },
	{ 0 }
};

static int handle_event(struct audio_device *device, const char *buf)
{
	struct event *ev;

	TRACE();

	DBG("Received %s", buf);

	for (ev = event_callbacks ; ev->cmd ; ++ev) {
		if (!strncmp(buf, ev->cmd, strlen(ev->cmd)))
			return ev->callback(device, buf);
	}

	return -EINVAL;
}

static unsigned int connect_cb_new(struct voice *vox,
				   voice_stream_cb_t func,
				   void *user_data)
{
	struct connect_cb *cb;
	static unsigned int free_cb_id = 1;

	TRACE();

	if (!func)
		return 0;

	cb = g_new(struct connect_cb, 1);

	cb->cb = func;
	cb->cb_data = user_data;
	cb->id = free_cb_id++;

	vox->callbacks = g_slist_append(vox->callbacks, cb);

	return cb->id;
}

static void run_connect_cb(struct audio_device *dev, GError *err)
{
	struct voice *vox = dev->voice;
	GSList *l;

	for (l = vox->callbacks ; l != NULL ; l = l->next) {
		struct connect_cb *cb = l->data;
		cb->cb(dev, err, cb->cb_data);
	}

	g_slist_free_full(vox->callbacks, g_free);
	vox->callbacks = NULL;
}

static gboolean sco_io_cb(GIOChannel *chan, GIOCondition cond,
			  struct audio_device *dev)
{
	struct voice *vox = dev->voice;

	TRACE();

	if (cond & G_IO_NVAL)
		return FALSE;

	DBG("sco connection is released");
	g_io_channel_shutdown(vox->sco, TRUE, NULL);
	g_io_channel_unref(vox->sco);
	vox->sco = NULL;
	voice_set_state(dev, VOICE_STATE_CONNECTED);

	return FALSE;
}

static void sco_connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct audio_device *dev = user_data;
	struct voice *vox = dev->voice;

	TRACE();

	vox->sco = g_io_channel_ref(chan);

	if (err) {
		error("sco_connect_cb(): %s", err->message);
		voice_suspend_stream(dev);
		return;
	}

	g_io_add_watch(vox->sco, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
		       (GIOFunc) sco_io_cb, dev);

	voice_set_state(dev, VOICE_STATE_PLAYING);
	run_connect_cb(dev, NULL);
}

static gboolean rfcomm_io_cb(GIOChannel *chan, GIOCondition cond,
				struct audio_device *device)
{
	struct voice *vox;
	struct voice_slc *slc;
	unsigned char buf[BUF_SIZE];
	ssize_t bytes_read;
	size_t free_space;
	int fd;

	TRACE();

	if (cond & G_IO_NVAL)
		return FALSE;

	vox = device->voice;
	slc = vox->slc;

	if (cond & (G_IO_ERR | G_IO_HUP)) {
		DBG("ERR or HUP on RFCOMM socket");
		goto failed;
	}

	fd = g_io_channel_unix_get_fd(chan);

	bytes_read = read(fd, buf, sizeof(buf) - 1);
	if (bytes_read < 0)
		return TRUE;

	free_space = sizeof(slc->buf) - slc->data_start -
			slc->data_length - 1;

	if (free_space < (size_t) bytes_read) {
		/* Very likely that the VOX is sending us garbage so
		 * just ignore the data and disconnect */
		error("Too much data to fit incoming buffer");
		goto failed;
	}

	memcpy(&slc->buf[slc->data_start], buf, bytes_read);
	slc->data_length += bytes_read;

	/* Make sure the data is null terminated so we can use string
	 * functions */
	slc->buf[slc->data_start + slc->data_length] = '\0';

	while (slc->data_length > 0) {
		char *cr;
		int err;
		off_t cmd_len;

		cr = strchr(&slc->buf[slc->data_start], '\r');
		if (!cr)
			break;

		cmd_len = 1 + (off_t) cr - (off_t) &slc->buf[slc->data_start];
		*cr = '\0';

		if (cmd_len > 1)
			err = handle_event(device, &slc->buf[slc->data_start]);
		else
			/* Silently skip empty commands */
			err = 0;

		if (err == -EINVAL) {
			error("Badly formated or unrecognized command: %s",
					&slc->buf[slc->data_start]);
			err = voice_generic_rsp(device,
						VOICE_REPLY_ERROR_NOT_SUPPORTED);
			if (err < 0)
				goto failed;
		} else if (err < 0)
			error("Error handling command %s: %s (%d)",
						&slc->buf[slc->data_start],
						strerror(-err), -err);

		slc->data_start += cmd_len;
		slc->data_length -= cmd_len;

		if (!slc->data_length)
			slc->data_start = 0;
	}

	return TRUE;

failed:
	voice_set_state(device, VOICE_STATE_DISCONNECTED);

	return FALSE;
}

static gboolean sco_cb(GIOChannel *chan, GIOCondition cond,
			struct audio_device *device)
{
	TRACE();

	if (cond & G_IO_NVAL)
		return FALSE;

	error("Audio connection got disconnected");

	voice_set_state(device, VOICE_STATE_CONNECTED);

	return FALSE;
}

void voice_connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct audio_device *dev = user_data;
	struct voice *vox = dev->voice;
	char vox_address[18], obf_vox_addr[18];

	TRACE();

	if (err) {
		error("%s", err->message);
		goto failed;
	}

	if (vox->rfcomm == NULL)
		vox->rfcomm = g_io_channel_ref(chan);

	ba2str(&dev->dst, vox_address);
	str2obfstr(vox_address, obf_vox_addr);

	g_io_add_watch(chan, G_IO_IN | G_IO_ERR | G_IO_HUP| G_IO_NVAL,
			(GIOFunc) rfcomm_io_cb, dev);

	DBG("Connected to %s", obf_vox_addr);

	vox->slc = g_new0(struct voice_slc, 1);

	voice_set_state(dev, VOICE_STATE_CONNECTED);

	return;

failed:
	if (vox->rfcomm)
		voice_set_state(dev, VOICE_STATE_CONNECTED);
	else
		voice_set_state(dev, VOICE_STATE_DISCONNECTED);
}

int voice_connect_rfcomm(struct audio_device *dev, GIOChannel *io)
{
	struct voice *vox = dev->voice;

	TRACE();

	if (!io)
		return -EINVAL;

	vox->rfcomm = g_io_channel_ref(io);

	voice_set_state(dev, VOICE_STATE_CONNECTING);

	return 0;
}

GIOChannel *voice_get_rfcomm(struct audio_device *dev)
{
	struct voice *vox = dev->voice;

	TRACE();

	return vox->rfcomm;
}

int voice_connect_sco(struct audio_device *dev, GIOChannel *io)
{
	struct voice *vox = dev->voice;

	TRACE();

	if (vox->state < VOICE_STATE_PLAY_PENDING)
		return -EINVAL;

	if (vox->sco)
		return -EISCONN;

	if (vox->state != VOICE_STATE_PLAY_PENDING) {
		warn("Receiving voice connection in state %d, expected %d",
		     (int)vox->state, (int)VOICE_STATE_PLAY_PENDING);
	}

	vox->sco = g_io_channel_ref(io);

	voice_set_state(dev, VOICE_STATE_PLAYING);

	return 0;
}

static void close_sco(struct audio_device *device)
{
	struct voice *vox = device->voice;

	TRACE();

	if (vox->sco) {
		int sock = g_io_channel_unix_get_fd(vox->sco);
		shutdown(sock, SHUT_RDWR);
		g_io_channel_shutdown(vox->sco, TRUE, NULL);
		g_io_channel_unref(vox->sco);
		vox->sco = NULL;
	}

	if (vox->sco_id) {
		g_source_remove(vox->sco_id);
		vox->sco_id = 0;
	}

	restore_voice_setting(device);
}

static int voice_close_rfcomm(struct audio_device *dev)
{
	struct voice *vox = dev->voice;
	GIOChannel *rfcomm = vox->rfcomm;

	TRACE();

	if (rfcomm) {
		g_io_channel_shutdown(rfcomm, TRUE, NULL);
		g_io_channel_unref(rfcomm);
		vox->rfcomm = NULL;
	}

	g_free(vox->slc);
	vox->slc = NULL;

	return 0;
}

static void voice_free(struct audio_device *dev)
{
	struct voice *vox = dev->voice;

	TRACE();

	close_sco(dev);
	voice_close_rfcomm(dev);

	g_free(vox);
	dev->voice = NULL;
}

static void voice_shutdown(struct audio_device *dev)
{
	TRACE();

	voice_set_state(dev, VOICE_STATE_DISCONNECTED);
}

int voice_get_sco_fd(struct audio_device *dev)
{
	struct voice *vox = dev->voice;

	TRACE();

	if (!vox || !vox->sco)
		return -1;

	return g_io_channel_unix_get_fd(vox->sco);
}

gboolean voice_lock(struct audio_device *dev, voice_lock_t lock)
{
	struct voice *vox = dev->voice;
	TRACE();

	if (vox->lock & lock)
		return FALSE;

	vox->lock |= lock;

	return TRUE;
}

gboolean voice_unlock(struct audio_device *dev, voice_lock_t lock)
{
	struct voice *vox = dev->voice;
	TRACE();

	if (!(vox->lock & lock))
		return FALSE;

	vox->lock &= ~lock;

	if (vox->lock)
		return TRUE;

	if (vox->state == VOICE_STATE_PLAYING)
		voice_suspend_stream(dev);

	return TRUE;
}

static gboolean request_stream_cb(gpointer data)
{
	run_connect_cb(data, NULL);
	return FALSE;
}

unsigned int voice_request_stream(struct audio_device *dev,
				  voice_stream_cb_t cb, void *user_data)
{
	struct voice *vox = dev->voice;
	GError *err = NULL;
	GIOChannel *io;

	TRACE();

	if (!vox->rfcomm) {
		get_records(dev);
	} else if (!vox->sco) {
		io = bt_io_connect(BT_IO_SCO, sco_connect_cb, dev, NULL, &err,
				   BT_IO_OPT_SOURCE_BDADDR, &dev->src,
				   BT_IO_OPT_DEST_BDADDR, &dev->dst,
				   BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				   BT_IO_OPT_INVALID);
		if (!io) {
			error("%s", err->message);
			g_error_free(err);
			return 0;
		}
	} else {
		g_idle_add(request_stream_cb, dev);
	}

	return connect_cb_new(vox, cb, user_data);
}

gboolean voice_cancel_stream(struct audio_device *dev, unsigned int id)
{
	struct voice *vox = dev->voice;
	GSList *l;
	struct connect_cb *cb = NULL;

	TRACE();

	for (l = vox->callbacks ; l != NULL ; l = l->next) {
		struct connect_cb *tmp = l->data;

		if (tmp->id == id) {
			cb = tmp;
			break;
		}
	}

	if (!cb)
		return FALSE;

	vox->callbacks = g_slist_remove(vox->callbacks, cb);
	g_free(cb);

	voice_suspend_stream(dev);

	return TRUE;
}

void voice_suspend_stream(struct audio_device *dev)
{
	GError *gerr = NULL;
	struct voice *vox = dev->voice;

	TRACE();

	if (!vox || !vox->sco)
		return;

	g_io_channel_shutdown(vox->sco, TRUE, NULL);
	g_io_channel_unref(vox->sco);
	vox->sco = NULL;
	g_set_error(&gerr, VOICE_ERROR, VOICE_ERROR_SUSPENDED, "Suspended");
	run_connect_cb(dev, gerr);
	g_error_free(gerr);
	voice_set_state(dev, VOICE_STATE_CONNECTED);
}

voice_state_t voice_get_state(struct audio_device *dev)
{
	struct voice *vox = dev->voice;

	return vox->state;
}

void voice_set_state(struct audio_device *dev, voice_state_t state)
{
	struct voice *vox = dev->voice;
	gboolean value;
	const char *state_str;
	voice_state_t old_state = vox->state;
	GSList *l;
	char vox_address[18], obf_vox_addr[18];

	TRACE();

	if (old_state == state)
		return;

	state_str = state2str(state);

	switch (state) {
	case VOICE_STATE_DISCONNECTED:
		value = FALSE;
		close_sco(dev);
		voice_close_rfcomm(dev);
		emit_property_changed(dev->conn, dev->path,
					AUDIO_VOICE_INTERFACE, "State",
					DBUS_TYPE_STRING, &state_str);
		g_dbus_emit_signal(dev->conn, dev->path,
					AUDIO_VOICE_INTERFACE,
					"Disconnected",
					DBUS_TYPE_INVALID);
		if (vox->state > VOICE_STATE_CONNECTING) {
			emit_property_changed(dev->conn, dev->path,
					AUDIO_VOICE_INTERFACE, "Connected",
					DBUS_TYPE_BOOLEAN, &value);
		}
		active_devices = g_slist_remove(active_devices, dev);
		break;
	case VOICE_STATE_CONNECTING:
		emit_property_changed(dev->conn, dev->path,
					AUDIO_VOICE_INTERFACE, "State",
					DBUS_TYPE_STRING, &state_str);
		break;
	case VOICE_STATE_CONNECTED:
		close_sco(dev);
		emit_property_changed(dev->conn, dev->path,
				      AUDIO_VOICE_INTERFACE, "State",
				      DBUS_TYPE_STRING, &state_str);
		if (vox->state < state) {
			g_dbus_emit_signal(dev->conn, dev->path,
						AUDIO_VOICE_INTERFACE,
						"Connected",
						DBUS_TYPE_INVALID);
			value = TRUE;
			emit_property_changed(dev->conn, dev->path,
						AUDIO_VOICE_INTERFACE,
						"Connected",
						DBUS_TYPE_BOOLEAN, &value);
			active_devices = g_slist_append(active_devices, dev);
		} else if (vox->state == VOICE_STATE_PLAYING) {
			value = FALSE;
			g_dbus_emit_signal(dev->conn, dev->path,
						AUDIO_VOICE_INTERFACE,
						"Stopped",
						DBUS_TYPE_INVALID);
			emit_property_changed(dev->conn, dev->path,
						AUDIO_VOICE_INTERFACE,
						"Playing",
						DBUS_TYPE_BOOLEAN, &value);
		}
		break;
	case VOICE_STATE_PLAY_PENDING:
		break;
	case VOICE_STATE_PLAYING:
		value = TRUE;
		emit_property_changed(dev->conn, dev->path,
					AUDIO_VOICE_INTERFACE, "State",
					DBUS_TYPE_STRING, &state_str);

		/* Do not watch HUP since we need to know when the link is
		   really disconnected */
		vox->sco_id = g_io_add_watch(vox->sco,
					G_IO_ERR | G_IO_NVAL,
					(GIOFunc) sco_cb, dev);

		g_dbus_emit_signal(dev->conn, dev->path,
					AUDIO_VOICE_INTERFACE, "Playing",
					DBUS_TYPE_INVALID);
		emit_property_changed(dev->conn, dev->path,
					AUDIO_VOICE_INTERFACE, "Playing",
					DBUS_TYPE_BOOLEAN, &value);
		break;
	}

	vox->state = state;

	ba2str(&dev->dst, vox_address);
	str2obfstr(vox_address, obf_vox_addr);
	DBG("State changed %s: %s -> %s", obf_vox_addr, str_state[old_state],
		str_state[state]);

	for (l = voice_callbacks; l != NULL; l = l->next) {
		struct voice_state_callback *cb = l->data;
		cb->cb(dev, old_state, state, cb->user_data);
	}
}

static void path_unregister(void *data)
{
	struct audio_device *dev = data;
	struct voice *vox = dev->voice;
	char addr[18], obf_addr[18];

	TRACE();

	if (vox->state > VOICE_STATE_DISCONNECTED) {
		DBG("Voice unregistered while device was connected!");
		voice_shutdown(dev);
	}

	ba2str(&dev->dst, addr);
	str2obfstr(addr, obf_addr);
	DBG("Unregistered interface %s for device %s",
	    AUDIO_VOICE_INTERFACE, obf_addr);

	voice_free(dev);
}

static int voice_set_channel(struct voice *vox,
			     const sdp_record_t *record)
{
	int ch;
	sdp_list_t *protos;

	TRACE();

	if (sdp_get_access_protos(record, &protos) < 0) {
		error("Unable to get access protos from voice record");
		return -1;
	}

	ch = sdp_get_proto_port(protos, RFCOMM_UUID);

	sdp_list_foreach(protos, (sdp_list_func_t) sdp_list_free, NULL);
	sdp_list_free(protos, NULL);

	if (ch <= 0) {
		error("Unable to get RFCOMM channel from voice record");
		return -1;
	}

	vox->rfcomm_ch = ch;
	vox->handle = record->handle;

	return 0;
}

static void get_record_cb(sdp_list_t *recs, int err, gpointer user_data)
{
	struct audio_device *dev = user_data;
	struct voice *vox = dev->voice;
	sdp_record_t *record = NULL;
	sdp_list_t *r;
	uuid_t uuid;

	TRACE();

	if (err < 0) {
		error("Unable to get service record: %s (%d)",
							strerror(-err), -err);
		goto failed;
	}

	if (!recs || !recs->data) {
		error("No records found");
		goto failed;
	}

	bt_string2uuid(&uuid, VOICE_UPLOAD_UUID);

	for (r = recs; r != NULL; r = r->next) {
		sdp_list_t *classes;
		uuid_t class;

		record = r->data;

		if (sdp_get_service_classes(record, &classes) < 0) {
			error("Unable to get service classes from record");
			continue;
		}

		memcpy(&class, classes->data, sizeof(uuid));

		sdp_list_free(classes, free);

		if (sdp_uuid_cmp(&class, &uuid) == 0)
			break;
	}

	if (r == NULL) {
		error("No record found with UUID %s", VOICE_UPLOAD_UUID);
		goto failed;
	}

	if (voice_set_channel(vox, record) < 0) {
		error("Unable to extract RFCOMM channel from service record");
		goto failed;
	}

	err = rfcomm_connect(dev, NULL, NULL, NULL);
	if (err < 0) {
		error("Unable to connect: %s (%d)", strerror(-err), -err);
		goto failed;
	}

	return;

failed:
	voice_set_state(dev, VOICE_STATE_DISCONNECTED);
}

static int get_records(struct audio_device *dev)
{
	uuid_t uuid;

	TRACE();

	voice_set_state(dev, VOICE_STATE_CONNECTING);
	bt_string2uuid(&uuid, VOICE_UPLOAD_UUID);
	return bt_search_service(&dev->src, &dev->dst, &uuid,
				 get_record_cb, dev, NULL);

}

static int rfcomm_connect(struct audio_device *dev, voice_stream_cb_t cb,
				void *user_data, unsigned int *cb_id)
{
	struct voice *vox = dev->voice;
	char address[18], obf_addr[18];
	GError *err = NULL;

	TRACE();

	if (!manager_allow_voice_connection(dev))
		return -ECONNREFUSED;

	if (vox->rfcomm_ch < 0)
		return get_records(dev);

	ba2str(&dev->dst, address);
	str2obfstr(address, obf_addr);
	DBG("Connecting to %s channel %d", obf_addr,
		vox->rfcomm_ch);

	vox->rfcomm = bt_io_connect(BT_IO_RFCOMM, voice_connect_cb, dev,
				    NULL, &err,
				    BT_IO_OPT_SOURCE_BDADDR, &dev->src,
				    BT_IO_OPT_DEST_BDADDR, &dev->dst,
				    BT_IO_OPT_CHANNEL, vox->rfcomm_ch,
				    BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				    BT_IO_OPT_INVALID);

	vox->rfcomm_ch = -1;

	if (!vox->rfcomm) {
		error("%s", err->message);
		g_error_free(err);
		return -EIO;
	}

	voice_set_state(dev, VOICE_STATE_CONNECTING);

	if (cb) {
		unsigned int id = connect_cb_new(vox, cb, user_data);
		if (cb_id)
			*cb_id = id;
	}

	return 0;
}

static DBusMessage *voice_connect(DBusConnection *conn, DBusMessage *msg,
				  void *data)
{
	struct audio_device *device = data;
	struct voice *vox = device->voice;
	int err;

	TRACE();

	if (vox->state == VOICE_STATE_CONNECTING)
		return btd_error_in_progress(msg);
	else if (vox->state > VOICE_STATE_CONNECTING)
		return btd_error_already_connected(msg);

	device->auto_connect = FALSE;

	err = rfcomm_connect(device, NULL, NULL, NULL);
	if (err < 0)
		return btd_error_failed(msg, strerror(-err));

	return NULL;
}

static DBusMessage *voice_disconnect(DBusConnection *conn, DBusMessage *msg,
				  void *data)
{
	struct audio_device *device = data;
	struct voice *vox = device->voice;
	char vox_address[18], obf_vox_addr[18];

	TRACE();

	if (vox->state == VOICE_STATE_CONNECTED)
		return btd_error_not_connected(msg);

	voice_shutdown(device);
	ba2str(&device->dst, vox_address);
	str2obfstr(vox_address, obf_vox_addr);
	info("Disconnected from %s", obf_vox_addr);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *voice_stop(DBusConnection *conn, DBusMessage *msg,
				  void *data)
{
	struct audio_device *device = data;
	struct voice *vox = device->voice;
	DBusMessage *reply = NULL;

	TRACE();

	if (vox->state < VOICE_STATE_PLAYING)
		return btd_error_not_connected(msg);

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return NULL;

	voice_set_state(device, VOICE_STATE_CONNECTED);

	return reply;
}

static DBusMessage *voice_get_properties(DBusConnection *conn,
					DBusMessage *msg, void *data)
{
	struct audio_device *device = data;
	struct voice *vox = device->voice;
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusMessageIter dict;
	gboolean value;
	const char *state;

	TRACE();

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return NULL;

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);


	/* Playing */
	value = (vox->state == VOICE_STATE_PLAYING);
	dict_append_entry(&dict, "Playing", DBUS_TYPE_BOOLEAN, &value);

	/* State */
	state = state2str(vox->state);
	if (state)
		dict_append_entry(&dict, "State", DBUS_TYPE_STRING, &state);

	/* Connected */
	value = (vox->state >= VOICE_STATE_CONNECTED);
	dict_append_entry(&dict, "Connected", DBUS_TYPE_BOOLEAN, &value);

	if (!value)
		goto done;

done:
	dbus_message_iter_close_container(&iter, &dict);

	return reply;
}

static DBusMessage *voice_set_property(DBusConnection *conn,
				       DBusMessage *msg, void *data)
{
	/* We do not currently support any writeable properties */
	TRACE();

	return btd_error_invalid_args(msg);
}

static const GDBusMethodTable voice_methods[] = {
	{ GDBUS_ASYNC_METHOD("Connect", NULL, NULL, voice_connect) },
	{ GDBUS_METHOD("Disconnect", NULL, NULL, voice_disconnect) },
	{ GDBUS_METHOD("Stop", NULL, NULL, voice_stop) },
	{ GDBUS_METHOD("GetProperties",
			NULL, GDBUS_ARGS({ "properties", "a{sv}" }),
			voice_get_properties) },
	{ GDBUS_METHOD("SetProperty",
			GDBUS_ARGS({ "name", "s" }, { "value", "v" }), NULL,
			voice_set_property) },
	{ }
};

static const GDBusSignalTable voice_signals[] = {
	{ GDBUS_SIGNAL("PropertyChanged",
			GDBUS_ARGS({ "name", "s" }, { "value", "v" })) },
	{ }
};

struct voice *voice_init(struct audio_device *dev, const char *uuidstr)
{
	struct voice *vox;
	const sdp_record_t *record;
	char addr[18], obf_addr[18];

	TRACE();

	vox = g_new0(struct voice, 1);
	if (!vox) {
		error("Could not allocate memory for Voice interface");
		return NULL;
	}

	vox->rfcomm_ch = -1;
	vox->search_vox = server_is_enabled_uuid128(&dev->src, uuidstr);
	vox->stored_voice_setting = VOICE_SETTING_INVALID;

	record = btd_device_get_record(dev->btd_dev, uuidstr);

	if (record)
		vox->handle = record->handle;

	if (!g_dbus_register_interface(dev->conn, dev->path,
				       AUDIO_VOICE_INTERFACE,
				       voice_methods, voice_signals, NULL,
				       dev, path_unregister)) {
		g_free(vox);
		return NULL;
	}

	ba2str(&dev->dst, addr);
	str2obfstr(addr, obf_addr);
	DBG("Registered interface %s for device %s",
	    AUDIO_VOICE_INTERFACE, obf_addr);

	return vox;
}

void voice_unregister(struct audio_device *dev)
{
	g_dbus_unregister_interface(dev->conn, dev->path,
				    AUDIO_VOICE_INTERFACE);
}

void voice_update(struct audio_device *dev, const char *uuidstr)
{
	struct voice *vox = dev->voice;
	const sdp_record_t *record;

	TRACE();

	record = btd_device_get_record(dev->btd_dev, uuidstr);
	if (!record)
		return;

	vox->handle = record->handle;
}

unsigned int voice_add_state_cb(voice_state_cb cb, void *user_data)
{
	struct voice_state_callback *state_cb;
	static unsigned int id = 0;

	state_cb = g_new(struct voice_state_callback, 1);
	state_cb->cb = cb;
	state_cb->user_data = user_data;
	state_cb->id = ++id;

	voice_callbacks = g_slist_append(voice_callbacks, state_cb);

	return state_cb->id;
}

gboolean voice_remove_state_cb(unsigned int id)
{
	GSList *l;

	for (l = voice_callbacks; l != NULL; l = l->next) {
		struct voice_state_callback *cb = l->data;
		if (cb && cb->id == id) {
			voice_callbacks = g_slist_remove(voice_callbacks, cb);
			g_free(cb);
			return TRUE;
		}
	}

	return FALSE;
}
