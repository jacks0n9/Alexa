/*
 * wpa_supplicant - WPA/RSN IE and KDE processing
 * Copyright (c) 2003-2008, Jouni Malinen <j@w1.fi>
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

#include "ieee80211.h"
#include "wpa_ie.h"

#include <llog.h>
#include <memory.h>


#define wpa_printf(level, fmt,...)  LLOGS_INFO("wpaie", #level, fmt, "", ##__VA_ARGS__)


static int wpa_selector_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104;
	return 0;
}


static int wpa_key_mgmt_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_NONE)
		return WPA_KEY_MGMT_WPA_NONE;
	return 0;
}


static int wpa_parse_wpa_ie_wpa(const u8 *wpa_ie, size_t wpa_ie_len,
				struct wpa_ie_data *data)
{
	const struct wpa_ie_hdr *hdr;
	const u8 *pos;
	int left;
	int i, count;

	memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_WPA;
	data->pairwise_cipher = WPA_CIPHER_TKIP;
	data->group_cipher = WPA_CIPHER_TKIP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
	data->mgmt_group_cipher = 0;

	if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}

	if (wpa_ie_len < sizeof(struct wpa_ie_hdr)) {
		wpa_printf(MSG_DEBUG, "%s: ie len too short %lu",
			   __func__, (unsigned long) wpa_ie_len);
		return -1;
	}

	hdr = (const struct wpa_ie_hdr *) wpa_ie;

	if (hdr->elem_id != WLAN_EID_VENDOR_SPECIFIC ||
	    hdr->len != wpa_ie_len - 2 ||
	    RSN_SELECTOR_GET(hdr->oui) != WPA_OUI_TYPE ||
	    WPA_GET_LE16(hdr->version) != WPA_VERSION) {
		wpa_printf(MSG_DEBUG, "%s: malformed ie or unknown version",
			   __func__);
		return -1;
	}

	pos = (const u8 *) (hdr + 1);
	left = wpa_ie_len - sizeof(*hdr);

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie length mismatch, %u too much",
			   __func__, left);
		return -1;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (pairwise), "
				   "count %u left %u", __func__, count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for key mgmt)",
			   __func__);
		return -1;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (key mgmt), "
				   "count %u left %u", __func__, count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for capabilities)",
			   __func__);
		return -1;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie has %u trailing bytes - ignored",
			   __func__, left);
	}

	return 0;
}

#define WAPI_OUI                ((int)0x00721400)

typedef enum _WAPI_AKM
{
    WAPI_AKML_RESV = 0,
    WAPI_AKML_CERT,
    WAPI_AKML_PSK
}WAPI_AKM;


#define WAPI_AKM_PSK            ((WAPI_AKML_PSK << 24)  | WAPI_OUI)
#define WAPI_AKM_CERT           ((WAPI_AKML_CERT << 24) | WAPI_OUI)


#define WAPIIE_ELEMENT_ID_LEN 1
#define WAPIIE_LENGHTH_LEN    1
#define WAPIIE_VERSION_LEN    2
#define WAPIIE_AKM_CNT_LEN    2
#define WAPIIE_AKM_SUIT_LEN   4

#define WAPIIE_AKM_SUIT_PSK     0x00147202
#define WAPIIE_AKM_SUIT_CERT    0x00147201
#define MY_GET32(p) ((((*p) << 24) & 0xff000000) | (((*(p+1)) << 16) & 0xff0000) | (((*(p+2)) << 8)& 0xff00) | ((*(p+3)) & 0xff))



/**
 * wpa_parse_wapi_ie - Parse WAPI_IE
 * @wpa_ie: Pointer to WPA or RSN IE
 * @wpa_ie_len: Length of the WPA/RSN IE
 * @data: Pointer to data area for parsing results
 * Returns: 0 on success, -1 on failure
 *
 * Parse the contents of WPA or RSN IE and write the parsed data into data.
 */
int wpa_parse_wapi_ie(const u8 *wapi_ie, size_t wapi_ie_len,
		     struct wpa_ie_data *data)
{
	int akm_suit_cnt = 0, i;
	u8 *ie_hdr = (u8 *)wapi_ie, *p_akm_auit_cnt, *p_akm;



//	if (!(wapi_ie_len >= 1 && wapi_ie[0] == WLAN_EID_WAPI))
//		return -1 ;

	memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_WAPI ;
	data->pairwise_cipher = WPA_CIPHER_SMS4;
	data->group_cipher = WPA_CIPHER_SMS4;
	data->key_mgmt = WPA_KEY_MGMT_WAPI_PSK;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
	data->mgmt_group_cipher = 0;


	p_akm_auit_cnt = ie_hdr + (WAPIIE_ELEMENT_ID_LEN + WAPIIE_LENGHTH_LEN + WAPIIE_VERSION_LEN);
	akm_suit_cnt = ((*p_akm_auit_cnt) | ((*(p_akm_auit_cnt + 1)) << 8 & 0xff00)) & 0xffff;
	p_akm = (p_akm_auit_cnt + WAPIIE_AKM_CNT_LEN);
	for (i = 0; i < akm_suit_cnt; i++) {
		LLOGS_INFO("WPAIE", "PARSE_WAPI_IE", "", "Validate WAPI IE length change present");
		if (p_akm > wapi_ie + wapi_ie_len - WAPIIE_AKM_SUIT_LEN) {
			wpa_printf(MSG_ERROR, "%s: malformed ie", __func__);
			break;
		}
		if (WAPIIE_AKM_SUIT_PSK == MY_GET32(p_akm)) {
			data->key_mgmt |= WPA_KEY_MGMT_WAPI_PSK;
            LLOGS_INFO("WPAIE", "PARSE_WAPI_IE", "PSK=%X", "", data->key_mgmt);
		} else if (WAPIIE_AKM_SUIT_CERT == MY_GET32(p_akm)) {
			data->key_mgmt |= WPA_KEY_MGMT_WAPI_CERT;
		    LLOGS_INFO("WPAIE", "PARSE_WAPI_IE", "CERT=%X supported", "", data->key_mgmt);
		}
		p_akm += WAPIIE_AKM_SUIT_LEN;
	}
    LLOGS_INFO("WPAIE", "PARSE_WAPI_IE", "KEY_MGMT=%X auit=%x suit=%d", "", data->key_mgmt, *p_akm_auit_cnt, akm_suit_cnt);
	return 0;

}


/**
 * wpa_parse_wpa_ie - Parse WPA/RSN IE
 * @wpa_ie: Pointer to WPA or RSN IE
 * @wpa_ie_len: Length of the WPA/RSN IE
 * @data: Pointer to data area for parsing results
 * Returns: 0 on success, -1 on failure
 *
 * Parse the contents of WPA or RSN IE and write the parsed data into data.
 */
int wpa_parse_wpa_ie(const u8 *wpa_ie, size_t wpa_ie_len,
		     struct wpa_ie_data *data)
{
	if (wpa_ie_len >= 1 && wpa_ie[0] == WLAN_EID_RSN)
		return wpa_parse_wpa_ie_rsn(wpa_ie, wpa_ie_len, data);
	else
		return wpa_parse_wpa_ie_wpa(wpa_ie, wpa_ie_len, data);
}

static int rsn_selector_to_bitfield(const u8 *s)
{
    if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_NONE)
        return WPA_CIPHER_NONE;
    if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP40)
        return WPA_CIPHER_WEP40;
    if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_TKIP)
        return WPA_CIPHER_TKIP;
    if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_CCMP)
        return WPA_CIPHER_CCMP;
    if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP104)
        return WPA_CIPHER_WEP104;
#ifdef CONFIG_IEEE80211W
    if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_AES_128_CMAC)
        return WPA_CIPHER_AES_128_CMAC;
#endif /* CONFIG_IEEE80211W */
    return 0;
}


static int rsn_key_mgmt_to_bitfield(const u8 *s)
{
    if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_UNSPEC_802_1X)
        return WPA_KEY_MGMT_IEEE8021X;
    if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X)
        return WPA_KEY_MGMT_PSK;
#ifdef CONFIG_IEEE80211R
    if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_FT_802_1X)
        return WPA_KEY_MGMT_FT_IEEE8021X;
    if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_FT_PSK)
        return WPA_KEY_MGMT_FT_PSK;
#endif /* CONFIG_IEEE80211R */
#ifdef CONFIG_IEEE80211W
    if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_802_1X_SHA256)
        return WPA_KEY_MGMT_IEEE8021X_SHA256;
    if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_PSK_SHA256)
        return WPA_KEY_MGMT_PSK_SHA256;
#endif /* CONFIG_IEEE80211W */
    return 0;
}

/**
 * wpa_parse_wpa_ie_rsn - Parse RSN IE
 * @rsn_ie: Buffer containing RSN IE
 * @rsn_ie_len: RSN IE buffer length (including IE number and length octets)
 * @data: Pointer to structure that will be filled in with parsed data
 * Returns: 0 on success, <0 on failure
 */
int wpa_parse_wpa_ie_rsn(const u8 *rsn_ie, size_t rsn_ie_len,
             struct wpa_ie_data *data)
{
#ifndef CONFIG_NO_WPA2
    const struct rsn_ie_hdr *hdr;
    const u8 *pos;
    int left;
    int i, count;

    memset(data, 0, sizeof(*data));
    data->proto = WPA_PROTO_RSN;
    data->pairwise_cipher = WPA_CIPHER_CCMP;
    data->group_cipher = WPA_CIPHER_CCMP;
    data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
    data->capabilities = 0;
    data->pmkid = NULL;
    data->num_pmkid = 0;
#ifdef CONFIG_IEEE80211W
    data->mgmt_group_cipher = WPA_CIPHER_AES_128_CMAC;
#else /* CONFIG_IEEE80211W */
    data->mgmt_group_cipher = 0;
#endif /* CONFIG_IEEE80211W */

    if (rsn_ie_len == 0) {
        /* No RSN IE - fail silently */
        return -1;
    }

    if (rsn_ie_len < sizeof(struct rsn_ie_hdr)) {
        wpa_printf(MSG_DEBUG, "%s: ie len too short %lu",
               __func__, (unsigned long) rsn_ie_len);
        return -1;
    }

    hdr = (const struct rsn_ie_hdr *) rsn_ie;

    if (hdr->elem_id != WLAN_EID_RSN ||
        hdr->len != rsn_ie_len - 2 ||
        WPA_GET_LE16(hdr->version) != RSN_VERSION) {
        wpa_printf(MSG_DEBUG, "%s: malformed ie or unknown version",
               __func__);
        return -2;
    }

    pos = (const u8 *) (hdr + 1);
    left = rsn_ie_len - sizeof(*hdr);

    if (left >= RSN_SELECTOR_LEN) {
        data->group_cipher = rsn_selector_to_bitfield(pos);
#ifdef CONFIG_IEEE80211W
        if (data->group_cipher == WPA_CIPHER_AES_128_CMAC) {
            wpa_printf(MSG_DEBUG, "%s: AES-128-CMAC used as group "
                   "cipher", __func__);
            return -1;
        }
#endif /* CONFIG_IEEE80211W */
        pos += RSN_SELECTOR_LEN;
        left -= RSN_SELECTOR_LEN;
    } else if (left > 0) {
        wpa_printf(MSG_DEBUG, "%s: ie length mismatch, %u too much",
               __func__, left);
        return -3;
    }

    if (left >= 2) {
        data->pairwise_cipher = 0;
        count = WPA_GET_LE16(pos);
        pos += 2;
        left -= 2;
        if (count == 0 || left < count * RSN_SELECTOR_LEN) {
            wpa_printf(MSG_DEBUG, "%s: ie count botch (pairwise), "
                   "count %u left %u", __func__, count, left);
            return -4;
        }
        for (i = 0; i < count; i++) {
            data->pairwise_cipher |= rsn_selector_to_bitfield(pos);
            pos += RSN_SELECTOR_LEN;
            left -= RSN_SELECTOR_LEN;
        }
#ifdef CONFIG_IEEE80211W
        if (data->pairwise_cipher & WPA_CIPHER_AES_128_CMAC) {
            wpa_printf(MSG_DEBUG, "%s: AES-128-CMAC used as "
                   "pairwise cipher", __func__);
            return -1;
        }
#endif /* CONFIG_IEEE80211W */
    } else if (left == 1) {
        wpa_printf(MSG_DEBUG, "%s: ie too short (for key mgmt)",
               __func__);
        return -5;
    }

    if (left >= 2) {
        data->key_mgmt = 0;
        count = WPA_GET_LE16(pos);
        pos += 2;
        left -= 2;
        if (count == 0 || left < count * RSN_SELECTOR_LEN) {
            wpa_printf(MSG_DEBUG, "%s: ie count botch (key mgmt), "
                   "count %u left %u", __func__, count, left);
            return -6;
        }
        for (i = 0; i < count; i++) {
            data->key_mgmt |= rsn_key_mgmt_to_bitfield(pos);
            pos += RSN_SELECTOR_LEN;
            left -= RSN_SELECTOR_LEN;
        }
    } else if (left == 1) {
        wpa_printf(MSG_DEBUG, "%s: ie too short (for capabilities)",
               __func__);
        return -7;
    }

    if (left >= 2) {
        data->capabilities = WPA_GET_LE16(pos);
        pos += 2;
        left -= 2;
    }

    if (left >= 2) {
        data->num_pmkid = WPA_GET_LE16(pos);
        pos += 2;
        left -= 2;
        if (left < (int) data->num_pmkid * PMKID_LEN) {
            wpa_printf(MSG_DEBUG, "%s: PMKID underflow "
                   "(num_pmkid=%lu left=%d)",
                   __func__, (unsigned long) data->num_pmkid,
                   left);
            data->num_pmkid = 0;
            return -9;
        } else {
            data->pmkid = pos;
            pos += data->num_pmkid * PMKID_LEN;
            left -= data->num_pmkid * PMKID_LEN;
        }
    }

#ifdef CONFIG_IEEE80211W
    if (left >= 4) {
        data->mgmt_group_cipher = rsn_selector_to_bitfield(pos);
        if (data->mgmt_group_cipher != WPA_CIPHER_AES_128_CMAC) {
            wpa_printf(MSG_DEBUG, "%s: Unsupported management "
                   "group cipher 0x%x", __func__,
                   data->mgmt_group_cipher);
            return -10;
        }
        pos += RSN_SELECTOR_LEN;
        left -= RSN_SELECTOR_LEN;
    }
#endif /* CONFIG_IEEE80211W */

    if (left > 0) {
        wpa_printf(MSG_DEBUG, "%s: ie has %u trailing bytes - ignored",
               __func__, left);
    }

    return 0;
#else /* CONFIG_NO_WPA2 */
    return -1;
#endif /* CONFIG_NO_WPA2 */
}

