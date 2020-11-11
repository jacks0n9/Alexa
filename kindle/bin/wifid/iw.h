/*
 * iw.h
 *
 * Copyright (c) 2009-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * Use is subject to license terms.
 */


#include <netdb.h>          /* gethostbyname, getnetbyname */
#include <net/ethernet.h>   /* struct ether_addr */
#include <net/if_arp.h>     /* For ARPHRD_ETHER */
#include <netinet/in.h>     /* For struct sockaddr_in */
#include <netinet/if_ether.h>
#include <linux/if.h>
#include <string.h>

#include "Wifinet.h"

#ifdef ATHEROS

#include <athdefs.h>
#include <a_osapi.h>
#include <a_types.h>
#include <athdrv_linux.h>

#endif /* ATHEROS */

/****************************** DEBUG ******************************/

//#define DEBUG 1

/************************ CONSTANTS & MACROS ************************/

/* Various versions information */
/* Recommended Wireless Extension version */
#define WE_VERSION	21
/* Maximum forward compatibility built in this version of WT */
#define WE_MAX_VERSION	22
/* Version of Wireless Tools */
#define WT_VERSION	29

/* Paths */
#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_NET_DEV		"/proc/net/dev"

/* Some usefull constants */
#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9
/* For doing log10/exp10 without libm */
#define LOG10_MAGIC	1.25892541179

/* Ambient noise floor */
#define NOISE_FLOOR_DBM (-104)

/* Backward compatibility for network headers */
#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801		/* IEEE 802.11			*/
#endif /* ARPHRD_IEEE80211 */

/****************************** TYPES ******************************/

/* Shortcuts */
typedef struct iw_statistics	iwstats;
typedef struct iw_range		iwrange;
typedef struct iw_param		iwparam;
typedef struct iw_freq		iwfreq;
typedef struct iw_quality	iwqual;
typedef struct iw_priv_args	iwprivargs;
typedef struct sockaddr		sockaddr;

typedef  void (*ScanResultCallback)(void*, int, ScanResultList & sr);
typedef  void (*DriverReadyCallback)(void*);
typedef  void (*TargetErrorCallback)(void*);
typedef  void (*DisconnectCallback)(void*,const std::string&, int,int);
typedef  void (*AssocCallback)(void*,const std::string&);


struct IwEventCallbacks {
    void * user;
    ScanResultCallback  scanResult;
    DriverReadyCallback driverReady;
    TargetErrorCallback targetError;
    DisconnectCallback  discEvent;
    AssocCallback       assocEvent;
};

class IwAccess {
    int  skfd_; // socket
    int  skev_; // event socket
    char ifname_[IFNAMSIZ+1];
    const IwEventCallbacks *iecb_;

    int  iw_get_ext(int request, struct iwreq *   pwrq);
    int  iw_set_ext(int request, struct iwreq *   pwrq);
    void get_stats (iwstats * stats);
    int  ifflags();

#ifdef ATHEROS
    void get_athStats(TARGET_STATS *athStats);
    int get_athCountryCode();
    const std::string ath_toisostring(int code);
#endif /* ATHEROS */
    void signal_level(SignalQ &q);
    void formatKmCipher(struct wpa_ie_data & data, ScanResultItem & sr);

public:
    enum ScanReason {
        SUCCESS   = 0,
        FAILED    = 1,  // not defined by atheros
        CANCELLED = 16,
        BUSY      = 13
    };

    IwAccess(const char * ifn){ strncpy(ifname_, ifn, IFNAMSIZ); skfd_ = skev_ = -1; iecb_ = 0; }

    void open();
    void close();

    void signalStrength(SignalQ & q);
    void signalStrength(SignalQ & q, int level, const char * label="");
    bool ifup();
    std::string  macAddr();
    void wifiPowerSave(bool on);
    std::string & countryCode(std::string & cCode);
    bool poll();
    void scan(const char * ssid);
    std::vector<ScanResultItem> scan_result();
    void parseRtmLinkEvent(struct nlmsghdr *h,  size_t len);
    void parseWlanEvent(char * data,  size_t len);
    void receiveEvent(const IwEventCallbacks*);
};

