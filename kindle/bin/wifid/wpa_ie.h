/*
 * wpa_supplicant - WPA/RSN IE and KDE definitions
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef WPA_IE_H
#define WPA_IE_H

#include "ieee80211.h"

int wpa_parse_wpa_ie(const u8 *wpa_ie, size_t wpa_ie_len,
             struct wpa_ie_data *data);

int wpa_parse_wpa_ie_rsn(const u8 *rsn_ie, size_t rsn_ie_len,
             struct wpa_ie_data *data);

int wpa_parse_wapi_ie(const u8 *wapi_ie, size_t rsn_ie_len,
             struct wpa_ie_data *data);

#endif /* WPA_IE_H */
