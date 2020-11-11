/*
 * Copyright (c) 2008-2011 Atheros Communications Inc.
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

/*
 * Bluetooth Filter - WLAN module
 *
 */

#include "abtfilt_int.h"
#ifdef ANDROID
#include "cutils/properties.h"
#endif
#include "nl80211.h"
#include "nl80211_utils.h"

/* Definitions */
#define WLAN_EVENT_SIZE_MAX            1024
#define IW_HEADER_TYPE_POINT           8

/* Function Prototypes */
static void NewLinkEvent(ATH_BT_FILTER_INSTANCE *pInstance,
                         struct nlmsghdr *h, int len);
static void DelLinkEvent(ATH_BT_FILTER_INSTANCE *pInstance,
                         struct nlmsghdr *h, int len);
static void WirelessEvent(ATH_BT_FILTER_INSTANCE *pInstance,
                          char *data, int len);
static A_STATUS WirelessCustomEvent(ATH_BT_FILTER_INSTANCE *pInstance,
                                    char *buf, int len);
static A_STATUS AcquireWlanAdapter(ABF_WLAN_INFO *pAbfWlanInfo);
static void ReleaseWlanAdapter(ABF_WLAN_INFO *pAbfWlanInfo);
static void *WlanEventThread(void *arg);

/* APIs exported to other modules */
A_STATUS
Abf_WlanStackNotificationInit(ATH_BT_FILTER_INSTANCE *pInstance, A_UINT32 flags)
{
    A_STATUS status;
    ATHBT_FILTER_INFO *pInfo;
    ABF_WLAN_INFO *pAbfWlanInfo;

    pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
    if (pInfo->pWlanInfo) {
        return A_OK;
    }

    pAbfWlanInfo = (ABF_WLAN_INFO *)A_MALLOC(sizeof(ABF_WLAN_INFO));
    A_MEMZERO(pAbfWlanInfo,sizeof(ABF_WLAN_INFO));

    A_MUTEX_INIT(&pAbfWlanInfo->hWaitEventLock);
    A_COND_INIT(&pAbfWlanInfo->hWaitEvent);
    A_MEMZERO(pAbfWlanInfo, sizeof(ABF_WLAN_INFO));
    pAbfWlanInfo->pInfo = pInfo;
    pAbfWlanInfo->Loop = TRUE;
    pInfo->pWlanInfo = pAbfWlanInfo;

    /* Spawn a thread which will be used to process events from WLAN */
    status = A_TASK_CREATE(&pInfo->hWlanThread, WlanEventThread, pAbfWlanInfo);
    if (A_FAILED(status)) {
        A_ERR("[%s] Failed to spawn a WLAN thread\n", __FUNCTION__);
        return A_ERROR;
    }

    A_INFO("WLAN Stack Notification init complete\n");

    return A_OK;
}

void
Abf_WlanStackNotificationDeInit(ATH_BT_FILTER_INSTANCE *pInstance)
{
    ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
    ABF_WLAN_INFO *pAbfWlanInfo = pInfo->pWlanInfo;

    if (!pAbfWlanInfo) return;

    /* Terminate and wait for the WLAN Event Handler task to finish */
    A_MUTEX_LOCK(&pAbfWlanInfo->hWaitEventLock);
    if (pAbfWlanInfo->Loop) {
        pAbfWlanInfo->Loop = FALSE;
        A_COND_WAIT(&pAbfWlanInfo->hWaitEvent, &pAbfWlanInfo->hWaitEventLock,
                    WAITFOREVER);
    }
    A_MUTEX_UNLOCK(&pAbfWlanInfo->hWaitEventLock);

    /* Flush all the BT actions from the filter core */
    HandleAdapterEvent(pInfo, ATH_ADAPTER_REMOVED);
    pInfo->pWlanInfo = NULL;
    A_MUTEX_DEINIT(&pAbfWlanInfo->hWaitEventLock);
    A_COND_DEINIT(&pAbfWlanInfo->hWaitEvent);
    A_MEMZERO(pAbfWlanInfo, sizeof(ABF_WLAN_INFO));
    A_FREE(pAbfWlanInfo);

    A_INFO("WLAN Stack Notification de-init complete\n");
}

#define OP_TYPE_SCO	0x01
#define OP_TYPE_A2DP	0x02
#define OP_TYPE_INQUIRY	0x03
#define OP_TYPE_ESCO	0x04

static int get_nl_cmd(int cmd)
{
	int nl_cmd = NL80211_WMI_BT_MAX;
	switch (cmd) {
	case AR6000_XIOCTL_WMI_SET_BT_STATUS:
		nl_cmd = NL80211_WMI_SET_BT_STATUS;
		break;

	case AR6000_XIOCTL_WMI_SET_BT_PARAMS:
		nl_cmd = NL80211_WMI_SET_BT_PARAMS;
		break;

	case AR6000_XIOCTL_WMI_SET_BTCOEX_FE_ANT:
		nl_cmd = NL80211_WMI_SET_BT_FT_ANT;
		break;

	case AR6000_XIOCTL_WMI_SET_BTCOEX_COLOCATED_BT_DEV:
		nl_cmd = NL80211_WMI_SET_COLOCATED_BT_DEV;
		break;

	case AR6000_XIOCTL_WMI_SET_BTCOEX_BTINQUIRY_PAGE_CONFIG:
		nl_cmd = NL80211_WMI_SET_BT_INQUIRY_PAGE_CONFIG;
		break;

	case AR6000_XIOCTL_WMI_SET_BTCOEX_SCO_CONFIG:
		nl_cmd = NL80211_WMI_SET_BT_SCO_CONFIG;
		break;

	case AR6000_XIOCTL_WMI_SET_BTCOEX_A2DP_CONFIG:
		nl_cmd = NL80211_WMI_SET_BT_A2DP_CONFIG;
		break;

	case AR6000_XIOCTL_WMI_SET_BTCOEX_ACLCOEX_CONFIG:
		nl_cmd = NL80211_WMI_SET_BT_ACLCOEX_CONFIG;
		break;

	case AR6000_XIOCTL_WMI_SET_BTCOEX_DEBUG:
		nl_cmd = NL80211_WMI_SET_BT_DEBUG;
		break;

	case AR6000_XIOCTL_WMI_SET_BT_OPERATING_STATUS:
		nl_cmd = NL80211_WMI_SET_BT_OPSTATUS;
		break;

	case AR6000_XIOCTL_WMI_GET_BTCOEX_CONFIG:
		nl_cmd = NL80211_WMI_GET_BT_CONFIG;
		break;

	case AR6000_XIOCTL_WMI_GET_BTCOEX_STATS:
		nl_cmd = NL80211_WMI_GET_BT_STATS;
		break;
	case AR6000_XIOCTL_WMI_SET_BTCOEX_HID_CONFIG:
		nl_cmd = NL80211_WMI_SET_BT_HID_CONFIG;
	}
	return nl_cmd;
}

int send_btcoex_wmi_cmd(ATHBT_FILTER_INFO *pInfo, void *data, int len)
{
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	ABF_WLAN_INFO *pAbfWlanInf = pInfo->pWlanInfo;
	int cmd = *(int *)data;
	int nl_cmd;

	nl_cmd = get_nl_cmd(cmd);
	/* check for valid BTCOEX commands */
	if (nl_cmd == NL80211_WMI_BT_MAX)
		return 0;
	if (nl_cmd == NL80211_WMI_SET_BT_FT_ANT) {
		if (pInfo->Flags & ABF_FE_ANT_IS_SA) {
			A_INFO("Issue FE antenna configuration as single\n");
		}
		else if (pInfo->Flags & ABF_FE_ANT_IS_DA_SB_LI) {
			A_INFO("Issue FE antenna configuration as dual antenna shared BT\n");
		}
		else if (pInfo->Flags & ABF_FE_ANT_IS_3A) {
			A_INFO("Issue FE antenna configuration as 2x2 WLAN and non-shared BT\n");
		}
		else {
			A_INFO("Issue FE antenna configuration as dual\n");
		}
	}
	/*
	 * copy nl command in the place of ioctl so that
	 * driver can understand.
	 */
	*(int *)data = nl_cmd;

	return nl80211_send_btcoex_cmd(pInstance->nlstate,
			pAbfWlanInf->IfIndex, (char *)data, len);
}

int parse_coex_wmi_cmd(ATHBT_FILTER_INFO *pInfo, void *data, int len)
{
	ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
	ABF_WLAN_INFO *pAbfWlanInf = pInfo->pWlanInfo;
	A_UINT32 id = ((A_UINT32 *)data)[0];
	void *ptr;
	int err = 0;
	unsigned char esco = 0;
	enum nl80211_btcoex_antenna_config antenna;
	enum nl80211_btcoex_vendor_list bt_vendor = NL80211_BTCOEX_VENDOR_DEFAULT;

	if (pInfo->SCOConnectInfo.LinkType == BT_LINK_TYPE_ESCO) {
		A_INFO("ESCO...\n");
		esco = 1;
	} else {
		A_INFO("SCO...\n");
	}
	A_INFO("Chip is %s ID %d\n", (pInfo->Flags & ABF_WIFI_CHIP_IS_VENUS)?"Venus":"Other", id);
	ptr = data + sizeof(A_UINT32);
	switch(id) {
	case AR6000_XIOCTL_WMI_SET_BTCOEX_FE_ANT:

		if (pInfo->Flags & ABF_BT_CHIP_IS_QCOM) {
			/* Send  BT vendor */
			bt_vendor = NL80211_BTCOEX_VENDOR_QCOM;
		}
		nl80211_send_bt_vendor(pInstance->nlstate, pAbfWlanInf->IfIndex, bt_vendor);

	if(pInfo->Flags & ABF_FE_ANT_IS_SA) {
		/* Indicate front end antenna configuration as single antenna  */
		A_INFO("FLAGS = %x, Issue FE antenna configuration as single \n", pInfo->Flags);
		antenna = NL80211_BTCOEX_ANTENNA_SA;
	}
	else if (pInfo->Flags & ABF_FE_ANT_IS_DA_SB_LI) {
		A_INFO("FLAGS = %x, Issue FE antenna configuration as dual antenna shared BT \n", pInfo->Flags);
		antenna = NL80211_BTCOEX_ANTENNA_DASB_LI;
	}
	else if (pInfo->Flags & ABF_FE_ANT_IS_3A) {
		A_INFO("FLAGS = %x, Issue FE antenna configuration as 2x2 WLAN and non-shared BT \n", pInfo->Flags);
		antenna = NL80211_BTCOEX_ANTENNA_3A;
	}
	else {
		A_INFO("FLAGS = %x, Issue FE antenna configuration as dual \n", pInfo->Flags);
		antenna = NL80211_BTCOEX_ANTENNA_DA;
	}
		nl80211_send_antenna(pInstance->nlstate, pAbfWlanInf->IfIndex, antenna);
	break;
	case AR6000_XIOCTL_WMI_SET_BTCOEX_A2DP_CONFIG:
		/* 
		 * TBD: Right now no A2DP related information is sent
		 * to the Wifi Chip other than A2DP up/down.
		 */
	break;
	case AR6000_XIOCTL_WMI_SET_BTCOEX_SCO_CONFIG:
		if (pInfo->Flags & ABF_WIFI_CHIP_IS_VENUS) {
			A_INFO("SCO Started %d Buffer size \n", sizeof(WMI_SET_BTCOEX_SCO_CONFIG_CMD));
			A_INFO("Num Tx Interval %d\n", pInfo->SCOConnectInfo.TransmissionInterval);
			A_INFO("Num Tx Pkt Len %d\n", pInfo->SCOConnectInfo.TxPacketLength);
			nl80211_send_sco(pInstance->nlstate, pAbfWlanInf->IfIndex,
					 1, esco, pInfo->SCOConnectInfo.TransmissionInterval,
					pInfo->SCOConnectInfo.TxPacketLength);
		} else {
			err = -1;
		}
	break;
	case AR6000_XIOCTL_WMI_SET_BT_OPERATING_STATUS:
		A_INFO("Operation WMI_SET_BT_OPERATING_STATUS \n");
		if (pInfo->Flags & ABF_WIFI_CHIP_IS_VENUS) {
			WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMD *op =
				(WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMD*)ptr;
			unsigned char status;
			A_INFO("Profile Type Type %d\n", op->btProfileType);
			switch(op->btProfileType) {
			case OP_TYPE_INQUIRY:
				if (op->btOperatingStatus == 2)
					status = 0;
				else
					status = 1;
				nl80211_send_scan(pInstance->nlstate,
						  pAbfWlanInf->IfIndex, status);
				A_INFO("Inquiry Status %d\n", op->btOperatingStatus);
			break;
			case OP_TYPE_SCO:
				A_INFO("SCO Status %d\n", op->btOperatingStatus);
				if (op->btOperatingStatus == 2)
					nl80211_send_sco(pInstance->nlstate,
							 pAbfWlanInf->IfIndex, 0, esco, 0, 0);
			break;
			case OP_TYPE_ESCO:
				A_INFO("ESCO Status %d\n", op->btOperatingStatus);
			break;
			case OP_TYPE_A2DP:
				A_INFO("A2DP Status %d\n", op->btOperatingStatus);
				if (op->btOperatingStatus == 2) {
					nl80211_send_a2dp(pInstance->nlstate,
							  pAbfWlanInf->IfIndex, 0);
				} else {
					enum nl80211_btcoex_acl_role role =
						NL80211_BTCOEX_ACL_ROLE_UNKNOWN;

					if (!pInfo->A2DPConnection_Role)
						role = NL80211_BTCOEX_ACL_ROLE_MASTER;
					else
						role = NL80211_BTCOEX_ACL_ROLE_SLAVE;
					A_INFO("Role %d LMP ver %d\n", role,
						pInfo->A2DPConnection_LMPVersion);
					nl80211_send_acl_info(pInstance->nlstate,
							  pAbfWlanInf->IfIndex, role,
							  pInfo->A2DPConnection_LMPVersion);

					nl80211_send_a2dp(pInstance->nlstate,
							  pAbfWlanInf->IfIndex, 1);
				}
			break;
			}
		} else {
			err = -1;
		}
	break;
	case AR6000_XIOCTL_WMI_SET_BT_STATUS:
		A_INFO("Operation WMI_SET_BT_STATUS_CMD \n");
		if (pInfo->Flags & ABF_WIFI_CHIP_IS_VENUS) {
		} else {
			WMI_SET_BT_STATUS_CMD  *op = 
				(WMI_SET_BT_STATUS_CMD*)ptr;
			A_INFO("Chip Is Not Venus %d\n", sizeof(WMI_SET_BT_PARAMS_CMD));
			A_INFO("\tStream Type %d\n", op->streamType);
			A_INFO("\t\tStatus %d\n", op->status);
			switch(op->streamType) {
			case OP_TYPE_INQUIRY:
				A_INFO("Operation Inquiry status %d\n",op->status);
				if (op->status == 2)
					nl80211_send_scan(pInstance->nlstate,
							  pAbfWlanInf->IfIndex, 0);
				else
					nl80211_send_scan(pInstance->nlstate,
							  pAbfWlanInf->IfIndex, 1);

			break;
			case OP_TYPE_A2DP:
				A_INFO("Operation A2DP status %d\n",op->status);
				if (op->status == 2)
					nl80211_send_a2dp(pInstance->nlstate,
							  pAbfWlanInf->IfIndex, 0);
				else {
					enum nl80211_btcoex_acl_role role =
						NL80211_BTCOEX_ACL_ROLE_UNKNOWN;
					if (!pInfo->A2DPConnection_Role)
						role = NL80211_BTCOEX_ACL_ROLE_MASTER;
					else
						role = NL80211_BTCOEX_ACL_ROLE_SLAVE;
					A_INFO("Role %d LMP ver %d\n", role,
						pInfo->A2DPConnection_LMPVersion);
					nl80211_send_acl_info(pInstance->nlstate,
						pAbfWlanInf->IfIndex, role,
						pInfo->A2DPConnection_LMPVersion);

					nl80211_send_a2dp(pInstance->nlstate,
							  pAbfWlanInf->IfIndex, 1);
				}
			break;
			case OP_TYPE_SCO:
				if (op->status == 2)
					nl80211_send_sco(pInstance->nlstate,
							 pAbfWlanInf->IfIndex, 0, esco, 0, 0);
				else
					nl80211_send_sco(pInstance->nlstate,
							 pAbfWlanInf->IfIndex, 1, esco, 0, 0);
				A_INFO("Operation SCO status %d\n",op->status);
			break;
			}
		}
	break;
	}

	return err;
}

A_STATUS
Abf_WlanDispatchIO_new(ATHBT_FILTER_INFO *pInfo, unsigned long int req,
                   void *data, int size)
{
    struct ifreq ifr;
    char ifname[IFNAMSIZ], *ethIf;
        int i;
    ABF_WLAN_INFO *pAbfWlanInfo = pInfo->pWlanInfo;
    ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

    if (!pAbfWlanInfo->Handle) {
        /* No adapter to issue ioctl on */
        return A_DEVICE_NOT_FOUND;
    }

    /* Get the adpater name from command line if specified */
    if (pInstance->pWlanAdapterName != NULL) {
        ethIf = pInstance->pWlanAdapterName;
    } else {
        if ((ethIf = getenv("NETIF")) == NULL) {
            ethIf = pAbfWlanInfo->IfName;
        }
    }
    /* Frame and issue the requested ioctl to the WLAN adapter */
    A_MEMZERO(ifname, IFNAMSIZ);
    strcpy(ifname, ethIf);
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    ifr.ifr_data = (void *)data;

    A_INFO("WMI Cmd: %d Len: %d\n", ((int*)data)[0], size);
        for (i = 0; i < size; i++) {
                printf("%x ", ((unsigned char *)data)[i]);
        }
        printf("\n");

    if (send_btcoex_wmi_cmd(pInfo, data, size) < 0) {
        A_ERR("[%s] send WMI command (cmd:0x%X, size:%d) call failed!\n",
                        __FUNCTION__, ((int*)data)[0], size);
        return A_ERROR;
    }
    return A_OK;
}

A_STATUS
Abf_WlanDispatchIO(ATHBT_FILTER_INFO *pInfo, unsigned long int req,
                   void *data, int size)
{
    struct ifreq ifr;
    char ifname[IFNAMSIZ], *ethIf;
	int i;
    ABF_WLAN_INFO *pAbfWlanInfo = pInfo->pWlanInfo;
    ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;

    if (!pAbfWlanInfo->Handle) {
        /* No adapter to issue ioctl on */
        return A_DEVICE_NOT_FOUND;
    }

    /* Get the adpater name from command line if specified */
    if (pInstance->pWlanAdapterName != NULL) {
        ethIf = pInstance->pWlanAdapterName;
    } else {
        if ((ethIf = getenv("NETIF")) == NULL) {
            ethIf = pAbfWlanInfo->IfName;
        }
    }
    /* Frame and issue the requested ioctl to the WLAN adapter */
    A_MEMZERO(ifname, IFNAMSIZ);
    strcpy(ifname, ethIf);
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    ifr.ifr_data = (void *)data;

#if 1
    A_INFO("WMI Cmd: %d Len: %d\n", ((int*)data)[0], size);
	for (i = 0; i < size; i++) {
		printf("%x ", ((unsigned char *)data)[i]);
	}
	printf("\n");
#endif

//    if (send_btcoex_wmi_cmd(pInfo, data, size) < 0) {
    if (parse_coex_wmi_cmd(pInfo, data, size) < 0) {
        A_ERR("[%s] send WMI command (cmd:0x%X, size:%d) call failed!\n",
			__FUNCTION__, ((int*)data)[0], size);
        return A_ERROR;
    }
    return A_OK;
}

/* Internal functions */
static void *
WlanEventThread(void *arg)
{
    int left, ret, sd;
    struct timeval tv;
    socklen_t fromlen;
    struct nlmsghdr *h;
    fd_set readfds, tempfds;
    char buf[WLAN_EVENT_SIZE_MAX];
    struct sockaddr_nl from, local;
    ABF_WLAN_INFO *pAbfWlanInfo = (ABF_WLAN_INFO *)arg;
    ATHBT_FILTER_INFO *pInfo = pAbfWlanInfo->pInfo;
    ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
    A_STATUS status;

    A_INFO("Starting the WLAN Event Handler task\n");

    A_INFO("Checking WLAN adapter on startup .. \n");

	if (!pInstance->pWlanAdapterName) {
		Abf_WlanCheckSettings(pAbfWlanInfo->IfName);
		if (pAbfWlanInfo->IfName[0])
			pAbfWlanInfo->IfIndex = if_nametoindex(
							pAbfWlanInfo->IfName);
	}
    status = AcquireWlanAdapter(pAbfWlanInfo);

    if (A_FAILED(status)) {
        A_INFO("No WLAN adapter on startup (OKAY) \n");
    }else {
        /* Communicate this to the Filter task */
        HandleAdapterEvent(pInfo, ATH_ADAPTER_ARRIVED);
        A_INFO("WLAN Adapter Added\n");
    }


    do {
        sd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        if (sd < 0) {
            A_ERR("[%s] socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE): %d\n",
                  __FUNCTION__, sd);
            break;
        }

        A_MEMZERO(&local, sizeof(struct sockaddr_nl));
        local.nl_family = AF_NETLINK;
        local.nl_groups = RTMGRP_LINK;
        if ((ret = bind(sd, (struct sockaddr *) &local, sizeof(local))) < 0) {
            A_ERR("[%s] bind(netlink): %d\n", __FUNCTION__, ret);
            close(sd);
            break;
        }

        FD_ZERO(&readfds);
        FD_SET(sd, &readfds);
        while (pAbfWlanInfo->Loop) {
            A_MEMCPY(&tempfds, &readfds, sizeof(fd_set));
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            ret = select(sd+1, &tempfds, NULL, NULL, &tv);
            if ((ret < 0) && (errno != EINTR)) {
                A_ERR("[%s] select failed: %d\n", __FUNCTION__, ret);
                break;
            } else if ((ret > 0) && (FD_ISSET(sd, &tempfds))) {
                fromlen = sizeof(from);
                do {
                    left = recvfrom(sd, buf, sizeof(buf), 0,
                                    (struct sockaddr *) &from, &fromlen);
                } while (left == -1 && errno == EINTR);

                if (left < 0) {
                    A_ERR("[%s] recvfrom(netlink)\n", __FUNCTION__);
                    continue;
                 //   break;
                }

                h = (struct nlmsghdr *) buf;
                while (left >= (int)sizeof(*h)) {
                    int len, plen;

                    len = h->nlmsg_len;
                    plen = len - sizeof(*h);
                    if (len > left || plen < 0) {
                        A_ERR("[%s] malformed netlink message\n", __FUNCTION__);
                        continue;
                    }

                    //A_DEBUG("RTM Message Type: %s\n",
                    //        ((h->nlmsg_type == RTM_NEWLINK) ?
                    //         "RTM_NEWLINK" : ((h->nlmsg_type == RTM_DELLINK) ?
                    //         "RTM_DELLINK" : "RTM_OTHER")));
                    switch (h->nlmsg_type) {
                    case RTM_NEWLINK:
                        NewLinkEvent(pInstance, h, plen);
                        break;
                    case RTM_DELLINK:
                        DelLinkEvent(pInstance, h, plen);
                        break;
                    default:
                        break;
                    }

                    len = NLMSG_ALIGN(len);
                    left -= len;
                    h = (struct nlmsghdr *) ((char *) h + len);
                }
            }
        }

        close(sd);
    } while (FALSE);

    /* Clean up the resources allocated in this task */
    A_INFO("Terminating the WLAN Event Handler task\n");
    A_MUTEX_LOCK(&pAbfWlanInfo->hWaitEventLock);
    pAbfWlanInfo->Loop = FALSE;
    A_COND_SIGNAL(&pAbfWlanInfo->hWaitEvent);
    A_MUTEX_UNLOCK(&pAbfWlanInfo->hWaitEventLock);

    return NULL;
}

A_STATUS
Abf_WlanIssueFrontEndConfig(ATHBT_FILTER_INFO * pInfo)
{
    WMI_SET_BTCOEX_FE_ANT_CMD btcoexFeAntCmd;
    WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMD btcoexCoLocatedBtCmd;
    A_UINT32  buf_fe_ant_cmd[sizeof(A_UINT32) + sizeof(WMI_SET_BTCOEX_FE_ANT_CMD)];
    A_UINT32  buf_co_located_bt_cmd[sizeof(A_UINT32) + sizeof(WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMD)];
    A_STATUS status;

    /* Set co-located bt type to 1, generic for any PTA based bluetooth */
    buf_co_located_bt_cmd[0] = AR6000_XIOCTL_WMI_SET_BTCOEX_COLOCATED_BT_DEV;

    if (pInfo->Flags & ABF_BT_CHIP_IS_QCOM) {
        btcoexCoLocatedBtCmd.btcoexCoLocatedBTdev = 2;
    } else {
        btcoexCoLocatedBtCmd.btcoexCoLocatedBTdev = 1;
    }

    A_MEMCPY(&buf_co_located_bt_cmd[1], (void *)&btcoexCoLocatedBtCmd,
             sizeof(WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMD));

	A_INFO("Front End config\n");
    status = Abf_WlanDispatchIO(pInfo, AR6000_IOCTL_EXTENDED,
                                (void *)buf_co_located_bt_cmd,
                                (sizeof(WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMD) + sizeof(A_UINT32)));

    if (A_FAILED(status)) {
        A_ERR("[%s] Failed to issue Co-located BT configuration\n", __FUNCTION__);
        return A_ERROR;
    }

    if(pInfo->Flags & ABF_FE_ANT_IS_SA) {
        /* Indicate front end antenna configuration as single antenna  */
        A_INFO("FLAGS = %x, Issue FE antenna configuration as single \n", pInfo->Flags);
        btcoexFeAntCmd.btcoexFeAntType = WMI_BTCOEX_FE_ANT_SINGLE;
    }
    else if (pInfo->Flags & ABF_FE_ANT_IS_DA_SB_LI) {
        A_INFO("FLAGS = %x, Issue FE antenna configuration as dual antenna shared BT \n", pInfo->Flags);
        btcoexFeAntCmd.btcoexFeAntType = WMI_BTCOEX_FE_ANT_DUAL_SH_BT_LOW_ISO;
    } else if (pInfo->Flags & ABF_FE_ANT_IS_3A) {
        A_INFO("FLAGS = %x, Issue FE antenna configuration as 2x2 WLAN and non-shared BT\n", pInfo->Flags);
        btcoexFeAntCmd.btcoexFeAntType = WMI_BTCOEX_FE_ANT_TRIPLE;
    }else {
        A_INFO("FLAGS = %x, Issue FE antenna configuration as dual \n", pInfo->Flags);
        btcoexFeAntCmd.btcoexFeAntType = WMI_BTCOEX_FE_ANT_DUAL;
    }

    buf_fe_ant_cmd[0] = AR6000_XIOCTL_WMI_SET_BTCOEX_FE_ANT;

    A_MEMCPY(&buf_fe_ant_cmd[1], (void *)&btcoexFeAntCmd, sizeof(WMI_SET_BTCOEX_FE_ANT_CMD));


    status = Abf_WlanDispatchIO(pInfo, AR6000_IOCTL_EXTENDED,
                                (void *)buf_fe_ant_cmd,
                                (sizeof(WMI_SET_BTCOEX_FE_ANT_CMD) + sizeof(A_UINT32)));

    if (A_FAILED(status)) {
        A_ERR("[%s] Failed to issue FE ant configuration\n", __FUNCTION__);
        return A_ERROR;
    }

    return A_OK;

}

A_STATUS
Abf_WlanGetSleepState(ATHBT_FILTER_INFO * pInfo)
{
    /* setup ioctl cmd */
    A_UINT32 cmd = AR6000_XIOCTL_GET_WLAN_SLEEP_STATE;
	A_INFO("Get Sleep state\n");
    A_STATUS status = Abf_WlanDispatchIO(pInfo, AR6000_IOCTL_EXTENDED,
                                         (void *)&cmd,
                                         sizeof(A_UINT32));

    if (A_FAILED(status)) {
        A_ERR("[%s] Failed to issue get WLAN sleep state\n", __FUNCTION__);
        return A_ERROR;
    }

    return A_OK;
}

A_STATUS
Abf_WlanGetCurrentWlanOperatingFreq( ATHBT_FILTER_INFO * pInfo)
{
    A_STATUS status;
    struct iwreq wrq;
    char ifname[IFNAMSIZ], *ethIf;
    ATH_BT_FILTER_INSTANCE *pInstance = pInfo->pInstance;
    ABF_WLAN_INFO *pAbfWlanInfo = pInfo->pWlanInfo;

    /* Get the adpater name from command line if specified */
    if (pInstance->pWlanAdapterName != NULL) {
        ethIf = pInstance->pWlanAdapterName;
    } else {
        if ((ethIf = getenv("NETIF")) == NULL) {
            ethIf = pAbfWlanInfo->IfName;
        }
    }
    A_MEMZERO(ifname, IFNAMSIZ);
    strcpy(ifname, ethIf);
    strncpy(wrq.ifr_name, ifname, sizeof(wrq.ifr_name));
    if ((status = ioctl(pAbfWlanInfo->Handle, SIOCGIWFREQ, &wrq)) < 0) {
        return A_ERROR;
    }
    /*Freq is in Hz, converted into to MhZ */
    pAbfWlanInfo->Channel = (wrq.u.freq.m/100000);

    IndicateCurrentWLANOperatingChannel(pInfo, pAbfWlanInfo->Channel);
    return status;
}

static void
NewLinkEvent(ATH_BT_FILTER_INSTANCE *pInstance, struct nlmsghdr *h, int len)
{
    struct ifinfomsg *ifi;
    struct rtattr * attr;
    int attrlen, nlmsg_len, rta_len;
    ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
    ABF_WLAN_INFO *pAbfWlanInfo = (ABF_WLAN_INFO *)pInfo->pWlanInfo;

    if (len < (int)sizeof(*ifi)) {
        A_DEBUG("packet too short\n");
        return;
    }

    ifi = NLMSG_DATA(h);

    nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

    attrlen = h->nlmsg_len - nlmsg_len;
    if (attrlen < 0) {
        A_DEBUG("bad attrlen\n");
        return;
    }

    attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

    rta_len = RTA_ALIGN(sizeof(struct rtattr));
    while (RTA_OK(attr, attrlen)) {
        if (attr->rta_type == IFLA_WIRELESS) {
            /*
             * We need to ensure that the event is from the WLAN instance
             * that we are interested in TODO
             */
            WirelessEvent(pInstance, ((char*)attr) + rta_len,
                          attr->rta_len - rta_len);
        } else if (attr->rta_type == IFLA_IFNAME) {
            /*
             * Shall be used to get the socket descriptor. Also we should do
             * it only until we get the adapter we are interested in
             */
            if (!pAbfWlanInfo->Handle) {
                A_DEBUG("WLAN Adapter Interface: %s, Len: %d\n",
                        (((char *)attr) + rta_len), attr->rta_len - rta_len);
                A_MEMCPY(pAbfWlanInfo->IfName, ((char *)attr + rta_len),
                         attr->rta_len - rta_len);
                pAbfWlanInfo->IfIndex = if_nametoindex(pAbfWlanInfo->IfName);
            } else if (ifi->ifi_change && pAbfWlanInfo->IfIndex == ifi->ifi_index) {
                A_CHAR ifName[IFNAMSIZ];
                A_MEMCPY(ifName, ((char *)attr + rta_len), attr->rta_len - rta_len);
                if (A_MEMCMP(pAbfWlanInfo->IfName, ifName, sizeof(ifName))!=0) {
                    A_MEMCPY(pAbfWlanInfo->IfName, ifName, sizeof(ifName));
                }
            }
        }
        attr = RTA_NEXT(attr, attrlen);
    }
}

static void
DelLinkEvent(ATH_BT_FILTER_INSTANCE *pInstance, struct nlmsghdr *h, int len)
{
    A_BOOL found;
    struct ifinfomsg *ifi;
    struct rtattr * attr;
    int attrlen, nlmsg_len, rta_len;
    ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
    ABF_WLAN_INFO *pAbfWlanInfo = (ABF_WLAN_INFO *)pInfo->pWlanInfo;

    if (!pAbfWlanInfo->Handle) return;

    if (len < (int)sizeof(*ifi)) {
        A_DEBUG("packet too short\n");
        return;
    }

    ifi = NLMSG_DATA(h);

    nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

    attrlen = h->nlmsg_len - nlmsg_len;
    if (attrlen < 0) {
        A_DEBUG("bad attrlen\n");
        return;
    }

    attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

    rta_len = RTA_ALIGN(sizeof(struct rtattr));
    found = FALSE;
    while (RTA_OK(attr, attrlen)) {
        if (attr->rta_type == IFLA_IFNAME) {
            /*
             * Shall be used to get the socket descriptor. Also we should do
             * it only until we get the adapter we are interested in
             */
            if (!(strcmp(pAbfWlanInfo->IfName, ((char *)attr + rta_len)))) {
                found = TRUE;
            }
        }

        attr = RTA_NEXT(attr, attrlen);
    }

    if (!found) return;

    /* Flush all the BT actions from the filter core */
    HandleAdapterEvent(pInfo, ATH_ADAPTER_REMOVED);

    ReleaseWlanAdapter(pAbfWlanInfo);

    /* Reset the WLAN adapter specific info */
    A_MEMZERO(pAbfWlanInfo->AdapterName, WLAN_ADAPTER_NAME_SIZE_MAX);
    pAbfWlanInfo->PhyCapability = 0;

    A_INFO("WLAN Adapter Removed\n");
}

static void
WirelessEvent(ATH_BT_FILTER_INSTANCE *pInstance, char *data, int len)
{
    A_STATUS status = A_OK;
    struct iw_event iwe_buf, *iwe = &iwe_buf;
    char *pos, *end, *custom, *buf;

    pos = data;
    end = data + len;

    while ((pos + IW_EV_LCP_PK_LEN <= end) && (status == A_OK)) {
        /* Event data may be unaligned, so make a local, aligned copy
         * before processing. */
        A_MEMCPY(&iwe_buf, pos, IW_EV_LCP_LEN);
        if (iwe->len <= IW_EV_LCP_LEN) {
            status = A_ERROR;
            break;
        }

        custom = pos + IW_EV_POINT_LEN;
        if (WIRELESS_EXT > 18 &&
            (iwe->cmd == IWEVMICHAELMICFAILURE ||
             iwe->cmd == IWEVCUSTOM ||
             iwe->cmd == IWEVASSOCREQIE ||
             iwe->cmd == IWEVASSOCRESPIE ||
             iwe->cmd == IWEVPMKIDCAND ||
             iwe->cmd == IWEVGENIE)) {
            /* WE-19 removed the pointer from struct iw_point */
            char *dpos = (char *) &iwe_buf.u.data.length;
            int dlen = dpos - (char *) &iwe_buf;
            A_MEMCPY(dpos, pos + IW_EV_LCP_LEN,
                   sizeof(struct iw_event) - dlen);
        } else {
            A_MEMCPY(&iwe_buf, pos, sizeof(struct iw_event));
            custom += IW_EV_POINT_OFF;
        }

        switch (iwe->cmd) {
        case SIOCGIWAP:
            break;
        case IWEVCUSTOM:
            if (custom + iwe->u.data.length > end) {
                A_ERR("[%s:%d] Check Failed\n", __FUNCTION__, __LINE__);
                status = A_ERROR;
                break;
            }
            buf = A_MALLOC(iwe->u.data.length + 1);
            if (buf == NULL) {
                A_ERR("[%s:%d] Check Failed\n", __FUNCTION__, __LINE__);
                status = A_ERROR;
                break;
            }
            A_MEMCPY(buf, custom, iwe->u.data.length);
            status = WirelessCustomEvent(pInstance, buf, iwe->u.data.length);
            A_FREE(buf);
            break;
        case SIOCGIWSCAN:
            break;
        case SIOCSIWESSID:
            break;
        case IWEVGENIE:
            if (custom + iwe->u.data.length > end || (iwe->u.data.length < 2)) {
                A_ERR("event = IWEVGENIE with wrong length %d remain %d\n",
                                      iwe->u.data.length, (end-custom));
                status = A_ERROR;
                break;
            }
            buf = A_MALLOC(iwe->u.data.length + 1);
            if (buf == NULL) {
                A_ERR("[%s:%d] Check Failed\n", __FUNCTION__, __LINE__);
                status = A_ERROR;
                break;
            }
            A_MEMCPY(buf, custom, iwe->u.data.length);
            status = WirelessCustomEvent(pInstance, buf, iwe->u.data.length);
            A_FREE(buf);
            break;
        default:
            break;
        }

        pos += iwe->len;
    }
}

static A_STATUS
WirelessCustomEvent(ATH_BT_FILTER_INSTANCE *pInstance, char *buf, int len)
{
    char *ptr;
	int length;
	A_UINT16 eventid;
    WMI_READY_EVENT *ev1;
    WMI_CONNECT_EVENT *ev2;
    WMI_REPORT_SLEEP_STATE_EVENT * ev3;
    A_STATUS status = A_OK;
    ATHBT_FILTER_INFO *pInfo = (ATHBT_FILTER_INFO *)pInstance->pContext;
    ABF_WLAN_INFO *pAbfWlanInfo = pInfo->pWlanInfo;

    do {
        eventid = *((A_UINT16 *)buf);
        ptr = buf + 2; //Skip the event id
        length = len - 2;
        switch (eventid) {
        case (WMI_READY_EVENTID):
            if (length < (int)sizeof(WMI_READY_EVENT)) {
                A_ERR("[%s:%d] Check Failed\n", __FUNCTION__, __LINE__);
                status = A_ERROR;
                break;
            }
            ev1 = (WMI_READY_EVENT *)ptr;
            A_MEMCPY(pAbfWlanInfo->AdapterName, ev1->macaddr, ATH_MAC_LEN);
            pAbfWlanInfo->PhyCapability = ev1->phyCapability;
            A_DEBUG("WMI READY: Capability: %d, Address: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
                    pAbfWlanInfo->PhyCapability,
                    (pAbfWlanInfo->AdapterName[0]),
                    (pAbfWlanInfo->AdapterName[1]),
                    (pAbfWlanInfo->AdapterName[2]),
                    (pAbfWlanInfo->AdapterName[3]),
                    (pAbfWlanInfo->AdapterName[4]),
                    (pAbfWlanInfo->AdapterName[5]));

            /*
             * Open a handle for the ioctls that will be issued later
             * Try 10 times because the driver may not yet be ready to receive
             * IOCTLs, so we give the driver time to get ready by looping here
             */
                status = AcquireWlanAdapter(pAbfWlanInfo);

            if (A_FAILED(status)) {
                A_ERR("[%s] Failed to acquire WLAN adapter\n", __FUNCTION__);
                break;
            }

            /* Communicate this to the Filter task */
            HandleAdapterEvent(pInfo, ATH_ADAPTER_ARRIVED);
            A_INFO("WLAN Adapter Added\n");
            break;
        case (WMI_CONNECT_EVENTID):
            if (length < (int)sizeof(WMI_CONNECT_EVENT)) {
                A_ERR("[%s:%d] Check Failed\n", __FUNCTION__, __LINE__);
                status = A_ERROR;
                break;
            }
            ev2 = (WMI_CONNECT_EVENT *)ptr;
            pAbfWlanInfo->Channel = ev2->u.infra_ibss_bss.channel;
            A_DEBUG("WMI CONNECT: Channel: %d\n", ev2->u.infra_ibss_bss.channel);
            IndicateCurrentWLANOperatingChannel(pInfo, pAbfWlanInfo->Channel);
            break;
        case (WMI_DISCONNECT_EVENTID):
            A_DEBUG("WMI DISCONNECT: %d\n", len);
            IndicateCurrentWLANOperatingChannel(pInfo, 0);
            break;
        case (WMI_ERROR_REPORT_EVENTID):
            A_DEBUG("WMI ERROR REPORT: %d\n", len);
            break;
        case (WMI_SCAN_COMPLETE_EVENTID):
            A_DEBUG("WMI SCAN COMPLETE: %d\n", len);
            break;
        case (WMI_REPORT_SLEEP_STATE_EVENTID):
            A_DEBUG("WMI_REPORT_SLEEP_STATE_EVENTID: %d\n", len);
            if(length < (int)sizeof(WMI_REPORT_SLEEP_STATE_EVENT)) {
                A_ERR("[%s]Incorrect length passed - length = %d, len =%d\n", __FUNCTION__, length, len);
            }
            ev3 = (WMI_REPORT_SLEEP_STATE_EVENT *)ptr;
            switch(ev3->sleepState) {
                case  WMI_REPORT_SLEEP_STATUS_IS_DEEP_SLEEP:
                    HandleAdapterEvent(pInfo, ATH_ADAPTER_REMOVED);
                    break;
                case WMI_REPORT_SLEEP_STATUS_IS_AWAKE:
                    Abf_WlanIssueFrontEndConfig( pInfo);
                    HandleAdapterEvent(pInfo, ATH_ADAPTER_ARRIVED);
                    break;
            }
            break;
        default:
            //A_DEBUG("Event: 0x%x, Not Handled\n", eventid);
            break;
        }
    } while (FALSE);

    return status;
}

static A_STATUS
AcquireWlanAdapter(ABF_WLAN_INFO *pAbfWlanInfo)
{
    int sd;
	int index = 0;
    ATHBT_FILTER_INFO *pInfo = pAbfWlanInfo->pInfo;
	ATH_BT_FILTER_INSTANCE *pInstance = NULL;

	if (pInfo)
		pInstance = pInfo->pInstance;

	if (!pInfo && !pInstance)
		return A_ERROR;

    if (pAbfWlanInfo->Handle != 0) {
        return A_OK;
    }

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        A_ERR("[%s] Error creating socket: %d\n", __FUNCTION__, sd);
        return A_ERROR;
    }

    pAbfWlanInfo->Handle = sd;
    /* Try to get Device info to determinate that wlan is enabled */
	A_INFO("Trying To get Device Info\n");
	index = nl80211_get_info(pInstance->nlstate, pInstance->pWlanAdapterName);
    if (index < 0) {
        A_INFO("No device info present.\n");
        return A_ERROR;
    }
	pAbfWlanInfo->IfIndex = index;
    return A_OK;
}

static void
ReleaseWlanAdapter(ABF_WLAN_INFO *pAbfWlanInfo)
{
    close(pAbfWlanInfo->Handle);
    pAbfWlanInfo->Handle = 0;
    pAbfWlanInfo->HostVersion = 0;
    pAbfWlanInfo->TargetVersion = 0;
}

int Abf_wait_for_wlan(void)
{
#ifdef ANDROID
	A_CHAR wlan_status[IFNAMSIZ] = "";

	/* sleep until WLAN comes up */
	while (strcmp(wlan_status, "ok") != 0) {
		sleep(4);
		property_get("wlan.driver.status", wlan_status, NULL);
	}
#endif

	A_INFO("WLAN has Arrived\n");
	return 0;
}

void Abf_WlanCheckSettings(A_CHAR *wifname)
{
    A_CHAR ifname[IFNAMSIZ];
    ifname[0] = '\0';

#ifdef ANDROID 
    char ifprop[PROPERTY_VALUE_MAX];
    if (wifname[0] == '\0' && property_get("wifi.interface", ifprop, NULL)) {
        strcpy(wifname, ifprop);
    }
#else
    {
        A_BOOL found = FALSE;
        A_CHAR linebuf[1024];
        FILE *f = fopen("/proc/net/wireless", "r");
        if (f) {
            while(fgets(linebuf, sizeof(linebuf)-1, f)) {
                if (strchr(linebuf, ':')) {
                    char *dest = ifname;
                    char *p = linebuf;
                    while(*p && isspace(*p)) ++p;
                    while (*p && *p != ':')
                        *dest++ = *p++;
                    *dest = '\0';
                    if (strcmp(wifname, ifname)==0) {
                        found = TRUE;
                        break;
                    }
                }
            }
            if (!found && ifname[0]!='\0') {
                strcpy(wifname, ifname);
            }
            fclose(f);
        }
    }
#endif

    A_DEBUG("%s : wlan: %s\n", __FUNCTION__, wifname);
}
