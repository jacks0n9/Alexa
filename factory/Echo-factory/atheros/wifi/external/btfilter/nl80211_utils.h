/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __NL80211_UTILS_H
#define __NL80211_UTILS_H

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

#include "nl80211.h"

#ifndef CONFIG_LIBNL20
#define nl_sock nl_handle
#endif

struct nl80211_state {
	struct nl_sock *nl_sock;
	struct nl_cache *nl_cache;
	struct genl_family *nl80211;
};

int nl80211_init(struct nl80211_state **state);
void nl80211_cleanup(struct nl80211_state *state);
int nl80211_get_info(struct nl80211_state *state, char *name);
int nl80211_send_scan(struct nl80211_state *state, int devidx, unsigned char scan);
int nl80211_send_sco(struct nl80211_state *state, int devidx,
		     unsigned char stat, unsigned char esco, int tx_interval,
		     int tx_max_pkt_len);
int nl80211_send_a2dp(struct nl80211_state *state, int devidx,
		      unsigned char stat);
int nl80211_send_acl_info(struct nl80211_state *state, int devidx,
			  enum nl80211_btcoex_acl_role role, int lmp_ver);
int nl80211_send_antenna(struct nl80211_state *state, int devidx,
			 unsigned char stat);
int nl80211_send_bt_vendor(struct nl80211_state *state, int devidx,
			 unsigned char stat);
int nl80211_send_btcoex_cmd(struct nl80211_state *state, int devidx,
			 char *data, int len);
#endif /* __NL80211_UTILS_H */
