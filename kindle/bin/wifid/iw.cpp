/*
 * iw.cpp
 *
 * Copyright (c) 2009-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * Use is subject to license terms.
 */

/* Headers */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>       /* struct timeval */
#include <sys/socket.h>     /* For AF_INET & struct sockaddr */
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>          /* gethostbyname, getnetbyname */
#include <net/ethernet.h>   /* struct ether_addr */
#include <net/if_arp.h>     /* For ARPHRD_ETHER */
#include <netinet/in.h>     /* For struct sockaddr_in */
#include <netinet/if_ether.h>
#include <memory>
#include <map>
#include <string>

#include "iw.h"
#include "Exception.h"
#include "logging.h"
#include "stringex.h"
#include "ScopedArray.h"
#ifdef ATHEROS
    #include "wmi.h"
    #include "athCountryCode.h"
#endif

using namespace std;

// wpa_supplicant derived declarations

extern "C" {
#include "wpa_ie.h"
#include "priv_netlink.h"
#include "lab126_charset.h"
}

// end wpa_supplicant declarations

#include <linux/wireless.h>

#define module "iw"

/// Atheros Custom Event ID offset (see debug.c)
#define CUSTOM_EVENT_ID_LEN 2
/// Atheros Custom Event Interface ID offset (see debug.c)
#define CUSTOM_INTF_ID_LEN 1
/// Atheros Custom Event Interface WLAN ID
#define CUSTOM_WLAN_INTF_ID 0
/// Minimum number of scan items we should find a scan buffer, if we get E2BIG error
#define MIN_SCAN_ITEMS_ON_ERROR_TOO_BIG 1


/* Disable runtime version warning in iw_get_range_info() */
int iw_ignore_version = 0;

struct AR6102_Sens {
    int rate;
    struct sens {
        int min, typ, max;
    }s;
    unsigned bar;   // simplified signal strength in bars
};


/*
 * These values are tweaked for Shasta
 */
const AR6102_Sens shasta_sens[] = {
{1, {-90,-96,-100}, 1}, /* 1 Mbps*/
{2, {-80,-85,-90 }, 1},  /* 2 Mbps*/
{5, {-75,-77,-80 }, 2},  /* 5.5 Mbps */
{11,{-74,-74,-75 }, 2},  /* 11 Mbps */
{6, {-75,-77,-79 }, 2},  /* 6 Mbps */
{9, {-74,-74,-75 }, 2},  /* 9 Mbps */
{12,{-73,-73,-74 }, 3},  /* 12 Mbps*/
{18,{-71,-72,-73 }, 3},  /* 18 Mbps*/
{24,{-70,-70,-71 }, 3},  /* 24 Mbps*/
{36,{-68,-68,-70 }, 4},  /* 36 Mbps */
{48,{-55,-60,-68 }, 4},  /* 48 Mbps */
{54,{+15,-50,-55 }, 5},   /* 54 Mbps */
{0, {0,0,0},0}
};

const int highest_band = 11,
          lowest_band  = 0;

typedef std::pair<int, char const*> CipherName;
const int MAX_CIPHER_NAMES = 5;

const CipherName cipher_names[MAX_CIPHER_NAMES] = {
    CipherName(WPA_CIPHER_WEP40, "WEP"),
    CipherName(WPA_CIPHER_WEP104, "WEP"),
    CipherName(WPA_CIPHER_TKIP, "TKIP"),
    CipherName(WPA_CIPHER_CCMP, "CCMP"),
    CipherName(WPA_CIPHER_AES_128_CMAC, "AESCMAC")
};

typedef std::map<int, char const*> CipherMap;


/************************ SOCKET SUBROUTINES *************************/

/*
 * Open a socket.
 * The socket allows us to talk to the driver.
 */
void IwAccess::open()
{
    /* Try to open the socket, if success returns it */
    skfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd_ <  0)
        throw SyscallException(errno, "Failed to open IOCTL socket");

    skev_ = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (skev_ < 0)
        throw SyscallException(errno, "Failed to open NETLINK socket");

    struct sockaddr_nl local;
    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_groups = RTMGRP_LINK;
    if (bind(skev_, (struct sockaddr *) &local, sizeof(local)) < 0)
        throw SyscallException(errno, "Failed to bind NETLINK socket");
}

void IwAccess::close()
{
    ::close(skev_);
    ::close(skfd_);
}

/*
 * Wrapper to push some Wireless Parameter in the driver
 */
int IwAccess::iw_set_ext(int request, struct iwreq * pwrq)       /* Fixed part of the request */
{
    /* Set device name */
    strncpy(pwrq->ifr_name, ifname_, IFNAMSIZ - 1);
    /* Do the request */
    return(ioctl(skfd_, request, pwrq));
}

/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
int IwAccess::iw_get_ext(int request,  struct iwreq * pwrq)       /* Fixed part of the request */
{
    /* Set device name */
    strncpy(pwrq->ifr_name, ifname_, IFNAMSIZ - 1);
    /* Do the request */
    return(ioctl(skfd_, request, pwrq));
}

int IwAccess::ifflags()
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname_, IFNAMSIZ - 1);
    if (ioctl(skfd_, SIOCGIFFLAGS, (caddr_t) &ifr) < 0)
        throw SyscallException(errno, "ioctl[SIOCGIFFLAGS]");

    return ifr.ifr_flags & 0xffff;
}

/*
 * Class Public API
 */
std::string  IwAccess::macAddr()
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname_, IFNAMSIZ - 1);
    if(ioctl(skfd_, SIOCGIFHWADDR, (caddr_t) &ifr) < 0)
        throw SyscallException(errno, "ioctl[SIOCGIFHWADDR]");

    return mac2str(ifr.ifr_hwaddr.sa_data, ETH_ALEN);
}

bool IwAccess::ifup()
{
    int flags = ifflags();
    LLOGS_DEBUG(module, "if", "ifflags=%04x", "", flags);
    return (flags & IFF_UP);
}


void IwAccess::get_stats(iwstats * stats)
{
    struct iwreq  wrq;
    wrq.u.data.pointer = (caddr_t) stats;
    wrq.u.data.length = sizeof(struct iw_statistics);
    wrq.u.data.flags = 1;     /* Clear updated flag */
    if(iw_get_ext(SIOCGIWSTATS, &wrq) < 0)
        throw SyscallException(errno, "WE ioctl failed");
}

void IwAccess::signalStrength(SignalQ & q)
{
    signal_level(q);

    signalStrength(q, q.signal_level_dbm, "signalStrength");
}

void IwAccess::wifiPowerSave(bool on)
{
    struct iwreq wrq;

    wrq.u.power.disabled = (on == true)? 0 : 1;

    LLOGS_DEBUG0(module, "WiFi Power Save: %s", (on == true)? "Enable" : "Disable");
    if (iw_set_ext(SIOCSIWPOWER, &wrq) < 0)
    {
        throw SyscallException(errno, "WE SIOCSIWPOWER ioctl failed");
    }
}

/*
 * NOTE: the following comment is of historical interest only
 *
 * 6102 reported SNR in dB is assigned
 * to iq->qual and iq->level = 161 + iq->qual.
 *
 * APs in US usually have output power of 30 dbm. Over air, you will
 * surely loose at least 30 dB. So, at 6102, you will never get beyond
 * 0 dBm, which means SNR is about max 96 dB.
 *
 * So iq->level will vary from 161 (for 0 dB SNR which is equal to -96
 * dbm) to 161 + 96 = 257.
 *
 * A zero bar is returned when the rssi is out of range
 */
void IwAccess::signalStrength(SignalQ & q, int rssi, const char * label)
{
    if(!label)
        label = "";

    q.bar = 0;
    q.signal_level_dbm = rssi;

    for(const AR6102_Sens * sens = shasta_sens; sens->rate != 0; sens++)
        if(rssi <= sens->s.min && rssi > sens->s.max)
        {
            q.bar = sens->bar;
            break;
        }

    if (q.bar == 0)
    {
        LLOGS_INFO(module, "rssi", "bar=0, rssi=%d", "invalid rssi", rssi);
    }

    LLOGS_DEBUG0(module, "%s: rssi=%d dB %d/%d", label, rssi, q.bar, q.max_bar);
}

/*
 * If we are using atheros then we obtained the average beacon rssi from the atheros stats
 * otherwise, we get an instant value from standard iw stats.
 */
void IwAccess::signal_level(SignalQ &q)
{
#ifdef _NOTDEFINED
    int rssi;
    int noise;
    TARGET_STATS athStats;

    get_athStats(&athStats);

    rssi = athStats.cs_aveBeacon_rssi;
    if (!rssi)
    {
        rssi = NOISE_FLOOR
    }

    noise = athStats.noise_floor_calib;
    if (!noise)
    {
        noise = NOISE_FLOOR;
    }

    q.signal_level_dbm = rssi;
    q.noise_level_dbm = noise;

#else
    iwstats stat;
    get_stats(&stat);
    iwqual  * qual = &stat.qual;
    int dblevel = NOISE_FLOOR_DBM, dbnoise = NOISE_FLOOR_DBM;

    LLOGS_DEBUG1(module": updated=0x%x, level=0x%x, noise=0x%x", "", qual->updated, qual->level, qual->noise);

     /* Deal with signal level in dBm  (absolute power measurement) */
    if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
    {
        dblevel = qual->level;
        /* Implement a range for dBm [-192; 63] */
        if (qual->level >= 64)
            dblevel -= 0x100;
    }

    /* Update our DB with the signal level */
    q.signal_level_dbm = dblevel;

    /* Deal with noise level in dBm (absolute power measurement) */
    if (!(qual->updated & IW_QUAL_NOISE_INVALID))
    {
        dbnoise = qual->noise;
        /* Implement a range for dBm [-192; 63] */
        if (qual->noise >= 64)
            dbnoise -= 0x100;
    }

    /* Update our DB with the noise level */
    q.noise_level_dbm = dbnoise;

#endif /* ATHEROS */
}

std::string & IwAccess::countryCode(std::string & cCode)
{
#ifdef ATHEROS
    cCode = ath_toisostring( get_athCountryCode() );
#else
    cCode = "<unknown>";
#endif
    return cCode;
}

#ifdef ATHEROS
void IwAccess::get_athStats(TARGET_STATS *athStats)
{
    struct ifreq ifr;
    TARGET_STATS_CMD athStatsCmd;

    memset(&athStatsCmd, 0, sizeof(athStatsCmd));

    ifr.ifr_data = (void *)&athStatsCmd;

    if (iw_get_ext(AR6000_IOCTL_WMI_GET_TARGET_STATS, (struct iwreq *)&ifr) < 0)
    {
        throw SyscallException(errno, "AR6000_IOCTL_WMI_GET_TARGET_STATS ioctl failed");
    }

    memcpy(athStats, &athStatsCmd.targetStats, sizeof(*athStats));
}

/*
 * Returns country code
 * 3-17-2011: In atheros 3.0.x SW ccode is at offset 4
 */
int IwAccess::get_athCountryCode()

{
    struct ifreq ifr;
    union {
        char    raw[16];
        int16_t w16[8];
        int32_t w32[4];
    } buf;

    memset(buf.raw, 0, sizeof(buf));
    buf.w32[0] = AR6000_XIOCTL_WMI_GET_RD;

    ifr.ifr_data = (void *)buf.raw;
    if (iw_get_ext(AR6000_IOCTL_EXTENDED, (struct iwreq *)&ifr) < 0)
    {
        throw SyscallException(errno, "AR6000_XIOCTL_WMI_GET_RD ioctl failed");
    }

    return buf.w16[2];
}

/*
 * Convert Atheros' numeric country code into ISO-3166 two character string
 */
const std::string IwAccess::ath_toisostring(int code)
{
    for (size_t i = 0; i < sizeof(allCountries)/sizeof(COUNTRY_CODE_MAP); i++)
    {
        if (allCountries[i].countryCode == REGCODE_GET_CODE(code))
            return allCountries[i].isoName;
    }
    return string("<invalid>");
}


#endif /* ATHEROS */

void IwAccess::scan(const char * ssid)
{
    struct iwreq iwr;
    struct iw_scan_req req;
    const size_t ssid_len = (ssid ? strlen(ssid) : 0);

    if (ssid_len > IW_ESSID_MAX_SIZE)
        throw  Exception("iwscan: SSID too long");

    memset(&iwr, 0, sizeof(iwr));
    memset(&req, 0, sizeof(req));
    if (ssid && ssid_len > 0)
    {
        req.essid_len = ssid_len;
        req.bssid.sa_family = ARPHRD_ETHER;
        memset(req.bssid.sa_data, 0xff, ETH_ALEN);
        memcpy(req.essid, ssid, ssid_len);

        iwr.u.data.pointer = (caddr_t) &req;
        iwr.u.data.length = sizeof(req);
        iwr.u.data.flags = IW_SCAN_THIS_ESSID;
    }

    if (iw_set_ext(SIOCSIWSCAN, &iwr) < 0)
    {
        throw SyscallException(errno, "WE SIOCSIWSCAN ioctl failed");
    }
}

/*
 * Returns: true if there is pending data; false otherwise
 */
bool IwAccess::poll()
{
    const int maxfd = skev_ + 1;
    struct timeval tv = { 0, 0 }; // no wait
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(skev_, &rset);

    int nready = ::select(maxfd, &rset, 0, 0, &tv);
    return nready > 0;
}

void IwAccess::receiveEvent(const IwEventCallbacks * iwevcb)
{
        char buf[8192];
        int left;
        struct sockaddr_nl from;
        socklen_t fromlen;
        struct nlmsghdr *h;

        fromlen = sizeof(from);
        left = recvfrom(skev_, buf, sizeof(buf), MSG_DONTWAIT,
                        (struct sockaddr *) &from, &fromlen);
        if (left < 0)
        {
            if (errno != EINTR && errno != EAGAIN)
                LLOGS_ERROR(module, "wevent", "errno=%s", "", strerror(errno));
            return;
        }
        iecb_ = iwevcb;

        h = (struct nlmsghdr *) buf;
        while (left >= (int) sizeof(*h))
        {
                int len, plen;

                len = h->nlmsg_len;
                plen = len - sizeof(*h);
                if (len > left || plen < 0) {
                        LLOGS_ERROR(module, "wevent", "Malformed netlink message: "
                                   "len=%d left=%d plen=%d", "",
                                   len, left, plen);
                        break;
                }

                switch (h->nlmsg_type)
                {
                    case RTM_NEWLINK:
                        parseRtmLinkEvent(h, plen);
                            break;
                    case RTM_DELLINK:
                            //wpa_driver_wext_event_rtm_dellink(eloop_ctx, sock_ctx,
                            //                                  h, plen);
                            break;
                }

                len = NLMSG_ALIGN(len);
                left -= len;
                h = (struct nlmsghdr *) ((char *) h + len);
        }

        if (left > 0) {
                LLOGS_INFO(module, "wevent", "%d extra bytes in the end of netlink message", "", left);
        }
}

void IwAccess::parseRtmLinkEvent(struct nlmsghdr *h,  size_t len)
{
    struct ifinfomsg *ifi;
    int attrlen, nlmsg_len, rta_len;
    struct rtattr * attr;

    if (len < sizeof(*ifi))
        return;

    ifi = (struct ifinfomsg*)NLMSG_DATA(h);

    nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

    attrlen = h->nlmsg_len - nlmsg_len;
    if (attrlen < 0)
        return;

    attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

    rta_len = RTA_ALIGN(sizeof(struct rtattr));
    while (RTA_OK(attr, attrlen))
    {
        if (attr->rta_type == IFLA_WIRELESS)
        {
            parseWlanEvent(((char *) attr) + rta_len, attr->rta_len - rta_len);
        }
        else if (attr->rta_type == IFLA_IFNAME)
        {;} // link up/down events
        attr = RTA_NEXT(attr, attrlen);
    }
}

void IwAccess::parseWlanEvent(char * data,  size_t len)
{
    struct iw_event iwe_buf, *iwe = &iwe_buf;
    char *pos, *end, *custom;

    pos = data;
    end = data + len;

    if(!pos)
    {
        LLOGS_ERROR(module, "parseWlanEvent", "InvalidDataPointer", "");
        return;
    }

    LLOGS_DEBUG1(module, "raw event: hex=[%s]",hexlify(data, min(len, 16u)).c_str());
    while (pos + IW_EV_LCP_LEN <= end)
    {
        /* Event data may be unaligned, so make a local, aligned copy
                     * before processing. */
        memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
        if (iwe->cmd != IWEVGENIE) {
            LLOGS_DEBUG1(module, "Wireless event: cmd=0x%x len=%d",iwe->cmd, iwe->len);
        }
        if (iwe->len <= IW_EV_LCP_LEN)
            return;

        custom = pos + IW_EV_POINT_LEN;
        if (WE_VERSION > 18 &&
                (iwe->cmd == IWEVMICHAELMICFAILURE ||
                 iwe->cmd == IWEVCUSTOM ||
                 iwe->cmd == IWEVASSOCREQIE ||
                 iwe->cmd == IWEVASSOCRESPIE ||
                 iwe->cmd == SIOCGIWSCAN ||
                 iwe->cmd == IWEVPMKIDCAND)) {
            /* WE-19 removed the pointer from struct iw_point */
            char *dpos = (char *) &iwe_buf.u.data.length;
            int dlen = dpos - (char *) &iwe_buf;
            memcpy(dpos, pos + IW_EV_LCP_LEN,
                   sizeof(struct iw_event) - dlen);
        } else {
            memcpy(&iwe_buf, pos, sizeof(struct iw_event));
            custom += IW_EV_POINT_OFF;
        }

        switch (iwe->cmd)
        {
        case SIOCGIWAP:
            if(memcmp(iwe->u.ap_addr.sa_data, "\x00\x00\x00\x00\x00\x00", ETH_ALEN) != 0)
            {
                string mac(mac2str(iwe->u.ap_addr.sa_data, ETH_ALEN));
                LLOGS_INFO(module, "wevent", "new-AP=%s", "", mac_obfuscate(mac).c_str());
                if(iecb_)
                    iecb_->assocEvent(iecb_->user, mac);
            }
            break;
        case SIOCGIWSCAN:
#ifdef ATHEROS
            if(iwe->u.data.flags == 0)
#endif
            {
                try {
                    ScanResultList sr = scan_result();
                    LLOGS_DEBUG(module, "scan-result", "", "");
                    for(ScanResultList::const_iterator i = sr.begin(); i != sr.end(); ++i)
                        LLOGS_DEBUG(module, "\t", "%s", "", i->str().c_str());
                    if(iecb_)
                        iecb_->scanResult(iecb_->user, SUCCESS, sr);
                }
                catch(SyscallException & e)
                {
                    ScanResultList stub;
                    LLOGS_ERROR(module, "scan-result", "error=%d", "", e.getErrno());
                    if(iecb_)
                        iecb_->scanResult(iecb_->user, FAILED, stub);
                }
            }
#ifdef ATHEROS
            else{
                ScanResultList sr;
                LLOGS_ERROR(module, "scan-result", "error=%d", "", iwe->u.data.flags);
                if(iecb_)
                    iecb_->scanResult(iecb_->user, iwe->u.data.flags, sr);
            }
#endif
            break;
        case IWEVCUSTOM:
#ifdef ATHEROS
        {
            uint16_t wmiEvent = *((uint16_t*)custom);
            if(wmiEvent == WMI_READY_EVENTID && iecb_)
                iecb_->driverReady(iecb_->user);
            else if(wmiEvent == WMI_DISCONNECT_EVENTID)
            {
                // Get the disconnect event from the custom event, avoid alignment issues
                WMI_DISCONNECT_EVENT event;
                memset(&event, 0x0, sizeof(WMI_DISCONNECT_EVENT));
                uint8_t * ev = (uint8_t*)(custom + CUSTOM_EVENT_ID_LEN + CUSTOM_INTF_ID_LEN);
                memcpy(&event, ev, sizeof(WMI_DISCONNECT_EVENT));

                /* Log the disconnect event from the driver but perform disconnect only on the
                    wpa_supplicant disconnect event */

                LLOGS_INFO(module, "wevent", "ev=disconnect, bssid=%s, reason=%d, prot-reason=%d", "",
                            mac_obfuscate(mac2str(event.bssid, ETH_ALEN)).c_str(), event.disconnectReason, event.protocolReasonStatus);
                if (iecb_)
                {
                    iecb_->discEvent(iecb_->user, mac2str(event.bssid, ETH_ALEN),
                        event.disconnectReason, event.protocolReasonStatus);
                }
            }
            else if(wmiEvent == WMI_ERROR_REPORT_EVENTID)
            {
                WMI_TARGET_ERROR_REPORT_EVENT * ev = (WMI_TARGET_ERROR_REPORT_EVENT*)(custom+2);
                LLOGS_WARN(module, "wevent", "ev=target-error, reason=%xh", "", ev->errorVal);
                if(iecb_)
                {
                    iecb_->targetError(iecb_->user);
                }
            }
        }
#endif
            break;
        }

        pos += iwe->len;
    }
}


/**
 *
 */
std::vector<ScanResultItem> IwAccess::scan_result()
{
    struct iwreq iwr;
    size_t ap_num = 0;
    int first, maxrate;

    struct iw_event iwe_buf, *iwe = &iwe_buf;
    char *pos, *end, *custom, *genie, *gpos, *gend;
    struct iw_param p;
    int blen = 0;
    const int max_buf_len = 0xffff;
    const int min_buf_len = 8192;

    std::vector<ScanResultItem> result;
    ScanResultItem wap;

    memset(&iwr, 0, sizeof(iwr));

    ScopedArray<char*> buffer(new char [max_buf_len]);
    iwr.u.data.pointer = buffer.get();

    for(blen = min_buf_len; blen <= max_buf_len+1;  blen = blen*2)
    {
        iwr.u.data.length = uint16_t( std::min(max_buf_len, blen) );
        if (iw_set_ext(SIOCGIWSCAN, &iwr) < 0)
        {
            if(errno != E2BIG && blen > max_buf_len)
                throw SyscallException(errno, "WE SIOCGIWSCAN ioctl failed");
            LLOGS_WARN(module, "scan", "blen=%d,err=%d", "insufficient buffer", blen, errno);
            errno = 0;
        }
        else
        {
            LLOGS_INFO(module, "scan", "blen=%d,err=%d",
                       "Obtained complete scan buffer", blen, errno);
            errno = 0;
            break;  // success
        }
    }

    LLOGS_INFO(module, "scan", "blen=%d,err=%d", "processing buffer", blen, errno);

    size_t clen,
            len = iwr.u.data.length,
           caps = 0;
    ap_num = 0;
    first = 1;

    pos = (char *) iwr.u.data.pointer;
    end = pos + len;

    while (pos + IW_EV_LCP_LEN <= end)
    {
        int ssid_len;
        /* Event data may be unaligned, so make a local, aligned copy
         * before processing. */
        memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
        if (iwe->len <= IW_EV_LCP_LEN)
            break;

        custom = pos + IW_EV_POINT_LEN;
        if (WE_VERSION > 18 &&
            (iwe->cmd == SIOCGIWESSID ||
             iwe->cmd == SIOCGIWENCODE ||
             iwe->cmd == IWEVGENIE ||
             iwe->cmd == IWEVASSOCREQIE ||
             iwe->cmd == IWEVCUSTOM)) {
            /* WE-19 removed the pointer from struct iw_point */
            char *dpos = (char *) &iwe_buf.u.data.length;
            int dlen = dpos - (char *) &iwe_buf;
            memcpy(dpos, pos + IW_EV_LCP_LEN,
                  sizeof(struct iw_event) - dlen);
        } else {
            memcpy(&iwe_buf, pos, sizeof(struct iw_event));
            custom += IW_EV_POINT_OFF;
        }

        switch (iwe->cmd)
        {
        case SIOCGIWAP:
            if (!first)
            {
                if((caps & WLAN_CAPABILITY_PRIVACY) && wap.key_mgmt.empty())
                    wap.key_mgmt.push_back("WEP");
                result.push_back(wap);
                wap = ScanResultItem();
                caps = 0;
            }
            first = 0;
            wap.bssid = mac2str(iwe->u.ap_addr.sa_data, ETH_ALEN);
            break;
        case SIOCGIWMODE:
            wap.adhoc = (iwe->u.mode == IW_MODE_ADHOC);
            break;
        case SIOCGIWESSID:
            ssid_len = iwe->u.essid.length;
            if (custom + ssid_len > end)
            {
#ifdef LAB126
                // Mark any malformed data as hidden
                wap.hidden = true;
                LLOGS_ERROR(module, "scan", "MalformedESSIDCmd", "ssid_len=%d", ssid_len);
#endif
                break;
            }
            if (iwe->u.essid.flags && ssid_len > 0 && ssid_len
                    <= IW_ESSID_MAX_SIZE)
            {
#ifdef LAB126
                /*
                 * Special case for hidden networks
                 * Many APs advertise a hidden network with valid length
                 * but zeroes as the SSID.
                 */
                if (*custom == 0)
                {
                    ssid_len = 0;
                    wap.hidden = true;
                }
#endif /* LAB126 */
                char utf8[256];
                encode_lab126_charset(custom, ssid_len, utf8);
                wap.essid.assign(utf8);
            }
#ifdef LAB126
            else if(0 == ssid_len)
            {
                // Zero length ssid is a hidden network or probe request
                wap.hidden = true;
            }
#endif
            break;
        case SIOCGIWFREQ:
            if (true)
            {
                int divi = 1000000, i;
                if (iwe->u.freq.e == 0) {
                    /*
                     * Some drivers do not report
                     * frequency, but a channel. Try to map
                     * this to frequency by assuming they
                     * are using IEEE 802.11b/g.
                     */
                    if (iwe->u.freq.m >= 1 &&
                        iwe->u.freq.m <= 13) {
                        wap.freq =
                            2407 +
                            5 * iwe->u.freq.m;
                        break;
                    } else if (iwe->u.freq.m == 14) {
                        wap.freq = 2484;
                        break;
                    }
                }
                if (iwe->u.freq.e > 6) {
                    break;
                }
                for (i = 0; i < iwe->u.freq.e; i++)
                    divi /= 10;
                wap.freq = iwe->u.freq.m / divi;
            }
            break;
        case IWEVQUAL:
            /*
             * Proper sign widening of signed int8 stored in unsigned int8
             * into int32. Compiler needs help because a signed value is
             * stored as unsigned.
             */
            wap.ss = int(iwe->u.qual.level) - 0x100;
            break;
        case SIOCGIWENCODE:
            if ( !(iwe->u.data.flags & IW_ENCODE_DISABLED))
                caps |= WLAN_CAPABILITY_PRIVACY;
            break;
        case SIOCGIWRATE:
            custom = pos + IW_EV_LCP_LEN;
            clen = iwe->len;
            if (custom + clen > end)
                break;
            maxrate = 0;
            while (((ssize_t) clen) >=
                   (ssize_t) sizeof(struct iw_param)) {
                /* Note: may be misaligned, make a local,
                 * aligned copy */
                memcpy(&p, custom, sizeof(struct iw_param));
                if (p.value > maxrate)
                    maxrate = p.value;
                clen -= sizeof(struct iw_param);
                custom += sizeof(struct iw_param);
            }
            break;
        case IWEVGENIE:
            gpos = genie = custom;
            gend = genie + iwe->u.data.length;
            if (gend > end) {
                LLOGS_ERROR(module,"scan-result", "IWEVGENIE overflow", "");
                // We failed to parse the scan list. Throw an exception/report failure,
                // so that we may trigger a new scan
                throw SyscallException(errno, "IWEVGENIE overflow");
            }
            while (gpos + 1 < gend &&
                   gpos + 2 + (u8) gpos[1] <= gend) {
                u16 ie = gpos[0], ielen = gpos[1] + 2;

                switch (ie)
                {
                case GENERIC_INFO_ELEM:
                    if (ielen >= 2 + 4 && memcmp(&gpos[2],
                            "\x00\x50\xf2\x01", 4) == 0 && ielen
                            <= SSID_MAX_WPA_IE_LEN)
                    {
                        struct wpa_ie_data data;
                        int res = wpa_parse_wpa_ie((const u8*)gpos, ielen, &data);
                        if(res == 0)
                            formatKmCipher(data, wap);
                    }
                    else if (ielen >= 2 + 4 && memcmp(&gpos[2],
                            "\x00\x50\xf2\x04", 4) == 0  &&
                     ielen <= SSID_MAX_WPS_IE_LEN )
                    {
                        LLOGS_DEBUG0(module, "SSID with WPS: %s", wap.essid.c_str());
                        //memcpy(results[ap_num].wps_ie, gpos, ielen);
                        //results[ap_num].wps_ie_len = ielen;
                        wap.wps = true;
                    }
                    break;
                case RSN_INFO_ELEM:
                    if (ielen <= SSID_MAX_WPA_IE_LEN)
                    {
                        struct wpa_ie_data data;
                        int res = wpa_parse_wpa_ie((const u8*)gpos, ielen, &data);
                        if(res == 0)
                            formatKmCipher(data, wap);
                    }
                    break;
                }
                gpos += ielen;
            }
            break;
        case IWEVCUSTOM:
        case IWEVASSOCREQIE:    /* HACK in driver to get around IWEVCUSTOM size limit */
            clen = iwe->u.data.length;
            if (custom + clen > end)
                break;

            if (clen > 7 && strncmp(custom, "wsc_ie=", 7) == 0)
            {
                char *spos;
                int bytes;
                spos = custom + 7;
                bytes = custom + clen - spos;

                if (bytes > SSID_MAX_WPS_IE_LEN) {
                    LLOGS_ERROR(module, "scan-result", "Too long WPS IE",  "(%d)", bytes);
                    break;
                }
                //memcpy(results[ap_num].wps_ie, spos, bytes);
                //results[ap_num].wps_ie_len = bytes;
                wap.wps =  true;
            }
            break;
        }

        pos += iwe->len;
    }

    if(!first)
    {
        if((caps & WLAN_CAPABILITY_PRIVACY) && wap.key_mgmt.empty())
            wap.key_mgmt.push_back("WEP");
        result.push_back(wap);
    }

    if ((blen > max_buf_len) && (result.size() <= MIN_SCAN_ITEMS_ON_ERROR_TOO_BIG))
    {
        // We exceeded the max buffer len, so we should have many items
        // If we are unable to get any useful scan items >MIN_SCAN_ITEMS_ON_ERROR_TOO_BIG
        // Throw an exception, so we can re-scan
        throw SyscallException(errno, "scan overflow no items");
    }

    return result;
}

void IwAccess::formatKmCipher(struct wpa_ie_data & data, ScanResultItem & sr)
{
    CipherMap cipherMap(cipher_names, cipher_names+MAX_CIPHER_NAMES);
    char smethod[64];
    snprintf(smethod, 64, "WPA%s-%s-", (data.proto == WPA_PROTO_RSN ? "2" : ""),
                                       (data.key_mgmt == WPA_KEY_MGMT_IEEE8021X ? "EAP" : "PSK"));
    string cires, gcires;
    for(CipherMap::iterator ci = cipherMap.begin(); ci != cipherMap.end(); ++ci)
    {
        const int cipher = ci->first;
        const string ciname(ci->second);
        if( (cipher & data.pairwise_cipher) == cipher)
        {
            cires += (cires.empty() ? ciname : "+" + ciname);
        }
        if( (cipher & data.group_cipher) == cipher)
        {
            gcires += (gcires.empty() ? ciname : "+" + ciname);
        }
    }
    sr.key_mgmt.push_back(smethod+cires);
    sr.group.push_back(gcires);
}
