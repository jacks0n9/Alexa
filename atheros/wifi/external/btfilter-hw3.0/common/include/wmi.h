/*
 * Copyright (c) 2004-2012 Atheros Communications Inc.
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

#ifndef _WMI_H_
#define _WMI_H_

#ifndef ATH_TARGET
#include "athstartpack.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ATH_MAC_LEN             6               /* length of mac in bytes */

	/*---------------------- BTCOEX RELATED -------------------------------------*/
	/*----------------------COMMON to AR6002 and AR6003 -------------------------*/
	typedef PREPACK struct {
		A_UINT8 streamType;
		A_UINT8 status;
	} POSTPACK WMI_SET_BT_STATUS_CMD;

	/*********************** Applicable to AR6002 ONLY ******************************/

	typedef enum {
		BT_PARAM_SCO = 1,         /* SCO stream parameters */
		BT_PARAM_A2DP ,
		BT_PARAM_ANTENNA_CONFIG,
		BT_PARAM_COLOCATED_BT_DEVICE,
		BT_PARAM_ACLCOEX,
		BT_PARAM_11A_SEPARATE_ANT,
		BT_PARAM_MAX
	} BT_PARAM_TYPE;

	typedef PREPACK struct {
		A_UINT32 numScoCyclesForceTrigger;  /* Number SCO cycles after which
											   force a pspoll. default = 10 */
		A_UINT32 dataResponseTimeout;       /* Timeout Waiting for Downlink pkt
											   in response for ps-poll,
											   default = 10 msecs */
		A_UINT32  stompScoRules;
		A_UINT32 scoOptFlags;               /* SCO Options Flags :
bits:     meaning:
0        Allow Close Range Optimization
1        Force awake during close range
2        If set use host supplied RSSI for OPT
3        If set use host supplied RTS COUNT for OPT
4..7     Unused
8..15    Low Data Rate Min Cnt
16..23   Low Data Rate Max Cnt
											 */

		A_UINT8 stompDutyCyleVal;           /* Sco cycles to limit ps-poll queuing
											   if stomped */
		A_UINT8 stompDutyCyleMaxVal;        /*firm ware increases stomp duty cycle
											  gradually uptill this value on need basis*/
		A_UINT8 psPollLatencyFraction;      /* Fraction of idle
											   period, within which
											   additional ps-polls
											   can be queued */
		A_UINT8 noSCOSlots;                 /* Number of SCO Tx/Rx slots.
											   HVx, EV3, 2EV3 = 2 */
		A_UINT8 noIdleSlots;                /* Number of Bluetooth idle slots between
											   consecutive SCO Tx/Rx slots
											   HVx, EV3 = 4
											   2EV3 = 10 */
		A_UINT8 scoOptOffRssi;/*RSSI value below which we go to ps poll*/
		A_UINT8 scoOptOnRssi; /*RSSI value above which we reenter opt mode*/
		A_UINT8 scoOptRtsCount;
	} POSTPACK BT_PARAMS_SCO;

	typedef PREPACK struct {
		A_UINT32 a2dpWlanUsageLimit; /* MAX time firmware uses the medium for
										wlan, after it identifies the idle time
										default (30 msecs) */
		A_UINT32 a2dpBurstCntMin;   /* Minimum number of bluetooth data frames
									   to replenish Wlan Usage  limit (default 3) */
		A_UINT32 a2dpDataRespTimeout;
		A_UINT32 a2dpOptFlags;      /* A2DP Option flags:
bits:    meaning:
0       Allow Close Range Optimization
1       Force awake during close range
2        If set use host supplied RSSI for OPT
3        If set use host supplied RTS COUNT for OPT
4..7    Unused
8..15   Low Data Rate Min Cnt
16..23  Low Data Rate Max Cnt
									 */
		A_UINT8 isCoLocatedBtRoleMaster;
		A_UINT8 a2dpOptOffRssi;/*RSSI value below which we go to ps poll*/
		A_UINT8 a2dpOptOnRssi; /*RSSI value above which we reenter opt mode*/
		A_UINT8 a2dpOptRtsCount;
	}POSTPACK BT_PARAMS_A2DP;

	/* During BT ftp/ BT OPP or any another data based acl profile on bluetooth
	   (non a2dp).*/
	typedef PREPACK struct {
		A_UINT32 aclWlanMediumUsageTime;  /* Wlan usage time during Acl (non-a2dp)
											 coexistence (default 30 msecs) */
		A_UINT32 aclBtMediumUsageTime;   /* Bt usage time during acl coexistence
											(default 30 msecs)*/
		A_UINT32 aclDataRespTimeout;
		A_UINT32 aclDetectTimeout;      /* ACL coexistence enabled if we get
										   10 Pkts in X msec(default 100 msecs) */
		A_UINT32 aclmaxPktCnt;          /* No of ACL pkts to receive before
										   enabling ACL coex */

	}POSTPACK BT_PARAMS_ACLCOEX;

	typedef PREPACK struct {
		PREPACK union {
			BT_PARAMS_SCO scoParams;
			BT_PARAMS_A2DP a2dpParams;
			BT_PARAMS_ACLCOEX  aclCoexParams;
			A_UINT8 antType;         /* 0 -Disabled (default)
										1 - BT_ANT_TYPE_DUAL
										2 - BT_ANT_TYPE_SPLITTER
										3 - BT_ANT_TYPE_SWITCH */
			A_UINT8 coLocatedBtDev;  /* 0 - BT_COLOCATED_DEV_BTS4020 (default)
										1 - BT_COLCATED_DEV_CSR
										2 - BT_COLOCATED_DEV_VALKYRIe
									  */
		} POSTPACK info;
		A_UINT8 paramType ;
	} POSTPACK WMI_SET_BT_PARAMS_CMD;

	/************************ END AR6002 BTCOEX *******************************/
	/*-----------------------AR6003 BTCOEX -----------------------------------*/

	/*  ---------------WMI_SET_BTCOEX_FE_ANT_CMDID --------------------------*/
	/* Indicates front end antenna configuration. This command needs to be issued
	 * right after initialization and after WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMDID.
	 * AR6003 enables coexistence and antenna switching based on the configuration.
	 */
	typedef enum {
		WMI_BTCOEX_NOT_ENABLED = 0,
		WMI_BTCOEX_FE_ANT_SINGLE =1,
		WMI_BTCOEX_FE_ANT_DUAL=2,
		WMI_BTCOEX_FE_ANT_DUAL_HIGH_ISO=3,
		WMI_BTCOEX_FE_ANT_DUAL_SH_BT_LOW_ISO = 4,
		WMI_BTCOEX_FE_ANT_DUAL_SH_BT_HIGH_ISO = 5,
		WMI_BTCOEX_FE_ANT_TRIPLE=6,
		WMI_BTCOEX_FE_ANT_TYPE_MAX
	}WMI_BTCOEX_FE_ANT_TYPE;

	typedef PREPACK struct {
		A_UINT8 btcoexFeAntType; /* 1 - WMI_BTCOEX_FE_ANT_SINGLE for single antenna front end
									2 - WMI_BTCOEX_FE_ANT_DUAL for dual antenna front end
									(for isolations less 35dB, for higher isolation there
									is not need to pass this command).
									(not implemented)
								  */
	}POSTPACK WMI_SET_BTCOEX_FE_ANT_CMD;

	/* -------------WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMDID ----------------*/
	/* Indicate the bluetooth chip to the firmware. Firmware can have different algorithm based
	 * bluetooth chip type.Based on bluetooth device, different coexistence protocol would be used.
	 */
	typedef PREPACK struct {
		A_UINT8 btcoexCoLocatedBTdev; /*1 - Qcom BT (3 -wire PTA)
										2 - CSR BT  (3 wire PTA)
										3 - Atheros 3001 BT (3 wire PTA)
										4 - STE bluetooth (4-wire ePTA)
										5 - Atheros 3002 BT (4-wire MCI)
										defaults= 3 (Atheros 3001 BT )
									   */
	}POSTPACK WMI_SET_BTCOEX_COLOCATED_BT_DEV_CMD;

	/*---------------------WMI_SET_BTCOEX_SCO_CONFIG_CMDID ---------------*/
	/* Configure  SCO parameters. These parameters would be used whenever firmware is indicated
	 * of (e)SCO profile on bluetooth ( via WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMDID).
	 * Configration of BTCOEX_SCO_CONFIG data structure are common configuration and applies
	 * ps-poll mode and opt mode.
	 * Ps-poll Mode - Station is in power-save and retrieves downlink data between sco gaps.
	 * Opt Mode - station is in awake state and access point can send data to station any time.
	 * BTCOEX_PSPOLLMODE_SCO_CONFIG - Configuration applied only during ps-poll mode.
	 * BTCOEX_OPTMODE_SCO_CONFIG - Configuration applied only during opt mode.
	 */
#define WMI_SCO_CONFIG_FLAG_ALLOW_OPTIMIZATION   (1 << 0)
#define WMI_SCO_CONFIG_FLAG_IS_EDR_CAPABLE       (1 << 1)
#define WMI_SCO_CONFIG_FLAG_IS_BT_MASTER         (1 << 2)
#define WMI_SCO_CONFIG_FLAG_FW_DETECT_OF_PER     (1 << 3)
	typedef PREPACK struct {
		A_UINT32 scoSlots;                  /* Number of SCO Tx/Rx slots.
											   HVx, EV3, 2EV3 = 2 */
		A_UINT32 scoIdleSlots;              /* Number of Bluetooth idle slots between
											   consecutive SCO Tx/Rx slots
											   HVx, EV3 = 4
											   2EV3 = 10
											 */
		A_UINT32 scoFlags;                 /* SCO Options Flags :
bits:    meaning:
0   Allow Close Range Optimization
1   Is EDR capable or Not
2   IS Co-located Bt role Master
3   Firmware determines the periodicity of SCO.
											*/

		A_UINT32 linkId;                      /* applicable to STE-BT - not used */
	}POSTPACK BTCOEX_SCO_CONFIG;

	typedef PREPACK struct {
		A_UINT32  scoCyclesForceTrigger;    /* Number SCO cycles after which
											   force a pspoll. default = 10 */
		A_UINT32 scoDataResponseTimeout;     /* Timeout Waiting for Downlink pkt
												in response for ps-poll,
												default = 20 msecs */

		A_UINT32 scoStompDutyCyleVal;        /* not implemented */

		A_UINT32 scoStompDutyCyleMaxVal;     /*Not implemented */

		A_UINT32 scoPsPollLatencyFraction;   /* Fraction of idle
												period, within which
												additional ps-polls can be queued
												1 - 1/4 of idle duration
												2 - 1/2 of idle duration
												3 - 3/4 of idle duration
												default =2 (1/2)
											  */
	}POSTPACK BTCOEX_PSPOLLMODE_SCO_CONFIG;

	typedef PREPACK struct {
		A_UINT32 scoStompCntIn100ms;/*max number of SCO stomp in 100ms allowed in
									  opt mode. If exceeds the configured value,
									  switch to ps-poll mode
									  default = 3 */

		A_UINT32 scoContStompMax;   /* max number of continous stomp allowed in opt mode.
									   if excedded switch to pspoll mode
									   default = 3 */

		A_UINT32 scoMinlowRateMbps; /* Low rate threshold */

		A_UINT32 scoLowRateCnt;     /* number of low rate pkts (< scoMinlowRateMbps) allowed in 100 ms.
									   If exceeded switch/stay to ps-poll mode, lower stay in opt mode.
									   default = 36
									 */

		A_UINT32 scoHighPktRatio;   /*(Total Rx pkts in 100 ms + 1)/
									  ((Total tx pkts in 100 ms - No of high rate pkts in 100 ms) + 1) in 100 ms,
									  if exceeded switch/stay in opt mode and if lower switch/stay in  pspoll mode.
									  default = 5 (80% of high rates)
									 */

		A_UINT32 scoMaxAggrSize;    /* Max number of Rx subframes allowed in this mode. (Firmware re-negogiates
									   max number of aggregates if it was negogiated to higher value
									   default = 1
									   Recommended value Basic rate headsets = 1, EDR (2-EV3)  =4.
									 */
	}POSTPACK BTCOEX_OPTMODE_SCO_CONFIG;

	typedef PREPACK struct {
		A_UINT32 scanInterval;
		A_UINT32 maxScanStompCnt;
	}POSTPACK BTCOEX_WLANSCAN_SCO_CONFIG;

	typedef PREPACK struct {
		BTCOEX_SCO_CONFIG scoConfig;
		BTCOEX_PSPOLLMODE_SCO_CONFIG scoPspollConfig;
		BTCOEX_OPTMODE_SCO_CONFIG scoOptModeConfig;
		BTCOEX_WLANSCAN_SCO_CONFIG scoWlanScanConfig;
	}POSTPACK WMI_SET_BTCOEX_SCO_CONFIG_CMD;

        typedef PREPACK struct {
                A_UINT32 scoStompCntIn100ms;/*max number of SCO stomp in 100ms allowed in
                                                                          opt mode. If exceeds the configured value,
                                                                          switch to ps-poll mode
                                                                          default = 3 */

                A_UINT32 scoContStompMax;   /* max number of continous stomp allowed in opt mode.
                                                                           if excedded switch to pspoll mode
                                                                           default = 3 */

                A_UINT32 scoMinlowRateMbps; /* Low rate threshold */

                A_UINT32 scoLowRateCnt;     /* number of low rate pkts (< scoMinlowRateMbps) allowed in 100 ms.
                                                                           If exceeded switch/stay to ps-poll mode, lower stay in opt mode.
                                                                           default = 36
                                                                         */

                A_UINT32 scoHighPktRatio;   /*(Total Rx pkts in 100 ms + 1)/
                                                                          ((Total tx pkts in 100 ms - No of high rate pkts in 100 ms) + 1) in 100 ms,
                                                                          if exceeded switch/stay in opt mode and if lower switch/stay in  pspoll mode.
                                                                          default = 5 (80% of high rates)
                                                                         */

                A_UINT32 scoMaxAggrSize;    /* Max number of Rx subframes allowed in this mode. (Firmware re-negogiates
                                                                           max number of aggregates if it was negogiated to higher value
                                                                           default = 1
                                                                           Recommended value Basic rate headsets = 1, EDR (2-EV3)  =4.
                                                                         */
                A_UINT32 NullBackoff;       /* Number of us the Null frame should go out before the next SCO slot */
	}POSTPACK BTCOEX_OPTMODE_SCO_CONFIG_EXT;

        typedef PREPACK struct {
                BTCOEX_SCO_CONFIG scoConfig;
                BTCOEX_PSPOLLMODE_SCO_CONFIG scoPspollConfig;
		BTCOEX_OPTMODE_SCO_CONFIG_EXT scoOptModeConfig;
                BTCOEX_WLANSCAN_SCO_CONFIG scoWlanScanConfig;
	}POSTPACK WMI_SET_BTCOEX_SCO_CONFIG_EXT_CMD;

	/* ------------------WMI_SET_BTCOEX_A2DP_CONFIG_CMDID -------------------*/
	/* Configure A2DP profile parameters. These parameters would be used whenver firmware is indicated
	 * of A2DP profile on bluetooth ( via WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMDID).
	 * Configuration of BTCOEX_A2DP_CONFIG data structure are common configuration and applies to
	 * ps-poll mode and opt mode.
	 * Ps-poll Mode - Station is in power-save and retrieves downlink data between a2dp data bursts.
	 * Opt Mode - station is in power save during a2dp bursts and awake in the gaps.
	 * BTCOEX_PSPOLLMODE_A2DP_CONFIG - Configuration applied only during ps-poll mode.
	 * BTCOEX_OPTMODE_A2DP_CONFIG - Configuration applied only during opt mode.
	 */

#define WMI_A2DP_CONFIG_FLAG_ALLOW_OPTIMIZATION    (1 << 0)
#define WMI_A2DP_CONFIG_FLAG_IS_EDR_CAPABLE        (1 << 1)
#define WMI_A2DP_CONFIG_FLAG_IS_BT_ROLE_MASTER     (1 << 2)
#define WMI_A2DP_CONFIG_FLAG_IS_A2DP_HIGH_PRI      (1 << 3)
#define WMI_A2DP_CONFIG_FLAG_FIND_BT_ROLE          (1 << 4)

	typedef PREPACK struct {
		A_UINT32 a2dpFlags;      /* A2DP Option flags:
bits:    meaning:
0       Allow Close Range Optimization
1       IS EDR capable
2       IS Co-located Bt role Master
3       a2dp traffic is high priority
4       Fw detect the role of bluetooth.
								  */
		A_UINT32 linkId;         /* Applicable only to STE-BT - not used */

	}POSTPACK BTCOEX_A2DP_CONFIG;

	typedef PREPACK struct {
		A_UINT32 a2dpWlanMaxDur; /* MAX time firmware uses the medium for
									wlan, after it identifies the idle time
									default (30 msecs) */

		A_UINT32 a2dpMinBurstCnt;   /* Minimum number of bluetooth data frames
									   to replenish Wlan Usage  limit (default 3) */

		A_UINT32 a2dpDataRespTimeout; /* Max duration firmware waits for downlink
										 by stomping on  bluetooth
										 after ps-poll is acknowledged.
										 default = 20 ms
									   */
	}POSTPACK BTCOEX_PSPOLLMODE_A2DP_CONFIG;

	typedef PREPACK struct {
		A_UINT32 a2dpMinlowRateMbps;  /* Low rate threshold */

		A_UINT32 a2dpLowRateCnt;    /* number of low rate pkts (< a2dpMinlowRateMbps) allowed in 100 ms.
									   If exceeded switch/stay to ps-poll mode, lower stay in opt mode.
									   default = 36
									 */

		A_UINT32 a2dpHighPktRatio;   /*(Total Rx pkts in 100 ms + 1)/
									   ((Total tx pkts in 100 ms - No of high rate pkts in 100 ms) + 1) in 100 ms,
									   if exceeded switch/stay in opt mode and if lower switch/stay in  pspoll mode.
									   default = 5 (80% of high rates)
									  */

		A_UINT32 a2dpMaxAggrSize;    /* Max number of Rx subframes allowed in this mode. (Firmware re-negogiates
										max number of aggregates if it was negogiated to higher value
										default = 1
										Recommended value Basic rate headsets = 1, EDR (2-EV3)  =8.
									  */
		A_UINT32 a2dpPktStompCnt;    /*number of a2dp pkts that can be stomped per burst.
									   default = 6*/

	}POSTPACK BTCOEX_OPTMODE_A2DP_CONFIG;

	typedef PREPACK struct {
		BTCOEX_A2DP_CONFIG a2dpConfig;
		BTCOEX_PSPOLLMODE_A2DP_CONFIG a2dppspollConfig;
		BTCOEX_OPTMODE_A2DP_CONFIG a2dpOptConfig;
	}POSTPACK WMI_SET_BTCOEX_A2DP_CONFIG_CMD;


	/* ------------------WMI_SET_BTCOEX_HID_CONFIG_CMDID -------------------*/
	/* Configure HID profile parameters. These parameters would be used whenver firmware is indicated
	 * of HID profile on bluetooth ( via WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMDID).
	 * Configuration of BTCOEX_HID_CONFIG data structure are common configuration and applies to
	 */

#define WMI_HID_CONFIG_FLAG_ALLOW_OPTIMIZATION    (1 << 0)
#define WMI_HID_CONFIG_FLAG_IS_EDR_CAPABLE        (1 << 1)
#define WMI_HID_CONFIG_FLAG_IS_BT_ROLE_MASTER     (1 << 2)
#define WMI_HID_CONFIG_FLAG_IS_A2DP_HIGH_PRI      (1 << 3)


	typedef PREPACK struct {
		A_UINT32 hidFlags;		/* HID Option flags:
						bits:    meaning:
						0       Allow Close Range Optimization
					*/
		A_UINT32 hiddevices;         /*current   device number of hid */
		A_UINT32 maxStompSlot;       /* max stomped */
		A_UINT32 aclPktCntLowerLimit;       /*acl dectet when hid on */
		A_UINT32 dummy[2];           /**/

	}POSTPACK BTCOEX_HID_CONFIG;

	typedef PREPACK struct {
		A_UINT32 hidWlanMaxDur; /*not used */

		A_UINT32 hidMinBurstCnt;   /* not used */

		A_UINT32 hidDataRespTimeout; /* not used */
	}POSTPACK BTCOEX_PSPOLLMODE_HID_CONFIG;

	typedef PREPACK struct {
		A_UINT32 hidMinlowRateMbps;  /* not used  */

		A_UINT32 hidLowRateCnt;    /* not used */

		A_UINT32 hidHighPktRatio;   /*not used  */

		A_UINT32 hidMaxAggrSize;    /* not used  */
		A_UINT32 hidPktStompCnt;    /*not used */

	}POSTPACK BTCOEX_OPTMODE_HID_CONFIG;

	typedef PREPACK struct {
		BTCOEX_HID_CONFIG hidConfig;
		BTCOEX_PSPOLLMODE_HID_CONFIG hidpspollConfig;
		BTCOEX_OPTMODE_HID_CONFIG hidOptConfig;
	}POSTPACK WMI_SET_BTCOEX_HID_CONFIG_CMD;


	/*------------ WMI_SET_BTCOEX_ACLCOEX_CONFIG_CMDID---------------------*/
	/* Configure non-A2dp ACL profile parameters.The starts of ACL profile can either be
	 * indicated via WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMDID orenabled via firmware detection
	 *  which is configured via "aclCoexFlags".
	 * Configration of BTCOEX_ACLCOEX_CONFIG data structure are common configuration and applies
	 * ps-poll mode and opt mode.
	 * Ps-poll Mode - Station is in power-save and retrieves downlink data during wlan medium.
	 * Opt Mode - station is in power save during bluetooth medium time and awake during wlan duration.
	 *             (Not implemented yet)
	 *
	 * BTCOEX_PSPOLLMODE_ACLCOEX_CONFIG - Configuration applied only during ps-poll mode.
	 * BTCOEX_OPTMODE_ACLCOEX_CONFIG - Configuration applied only during opt mode.
	 */

#define WMI_ACLCOEX_FLAGS_ALLOW_OPTIMIZATION   (1 << 0)
#define WMI_ACLCOEX_FLAGS_DISABLE_FW_DETECTION (1 << 1)

	typedef PREPACK struct {
		A_UINT32 aclWlanMediumDur;      /* Wlan usage time during Acl (non-a2dp)
										   coexistence (default 30 msecs)
										 */

		A_UINT32 aclBtMediumDur;       /* Bt usage time during acl coexistence
										  (default 30 msecs)
										*/

		A_UINT32 aclDetectTimeout;     /* BT activity observation time limit.
										  In this time duration, number of bt pkts are counted.
										  If the Cnt reaches "aclPktCntLowerLimit" value
										  for "aclIterToEnableCoex" iteration continuously,
										  firmware gets into ACL coexistence mode.
										  Similarly, if bt traffic count during ACL coexistence
										  has not reached "aclPktCntLowerLimit" continuously
										  for "aclIterToEnableCoex", then ACL coexistence is
										  disabled.
										  -default 100 msecs
										*/

		A_UINT32 aclPktCntLowerLimit;   /* Acl Pkt Cnt to be received in duration of
										   "aclDetectTimeout" for
										   "aclIterForEnDis" times to enabling ACL coex.
										   Similar logic is used to disable acl coexistence.
										   (If "aclPktCntLowerLimit"  cnt of acl pkts
										   are not seen by the for "aclIterForEnDis"
										   then acl coexistence is disabled).
										   default = 10
										 */

		A_UINT32 aclIterForEnDis;      /* number of Iteration of "aclPktCntLowerLimit" for Enabling and
										  Disabling Acl Coexistence.
										  default = 3
										*/

		A_UINT32 aclPktCntUpperLimit; /* This is upperBound limit, if there is more than
										 "aclPktCntUpperLimit" seen in "aclDetectTimeout",
										 ACL coexistence is enabled right away.
										 - default 15*/

		A_UINT32 aclCoexFlags;          /* A2DP Option flags:
bits:    meaning:
0       Allow Close Range Optimization
1       disable Firmware detection
(Currently supported configuration is aclCoexFlags =0)
										 */

		A_UINT32 linkId;                /* Applicable only for STE-BT - not used */

	}POSTPACK BTCOEX_ACLCOEX_CONFIG;

	typedef PREPACK struct {
		A_UINT32 aclDataRespTimeout;   /* Max duration firmware waits for downlink
										  by stomping on  bluetooth
										  after ps-poll is acknowledged.
										  default = 20 ms */

	}POSTPACK BTCOEX_PSPOLLMODE_ACLCOEX_CONFIG;


	/* Not implemented yet*/
	typedef PREPACK struct {
		A_UINT32 aclCoexMinlowRateMbps;
		A_UINT32 aclCoexLowRateCnt;
		A_UINT32 aclCoexHighPktRatio;
		A_UINT32 aclCoexMaxAggrSize;
		A_UINT32 aclPktStompCnt;
	}POSTPACK BTCOEX_OPTMODE_ACLCOEX_CONFIG;

	typedef PREPACK struct {
		BTCOEX_ACLCOEX_CONFIG aclCoexConfig;
		BTCOEX_PSPOLLMODE_ACLCOEX_CONFIG aclCoexPspollConfig;
		BTCOEX_OPTMODE_ACLCOEX_CONFIG aclCoexOptConfig;
	}POSTPACK WMI_SET_BTCOEX_ACLCOEX_CONFIG_CMD;

	/* -----------WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMDID ------------------*/
	typedef PREPACK struct {
		A_UINT32 btProfileType;
		A_UINT32 btOperatingStatus;
		A_UINT32 btLinkId;
	}WMI_SET_BTCOEX_BT_OPERATING_STATUS_CMD;

	/*--------------------------END OF BTCOEX -------------------------------------*/

	typedef PREPACK struct {
		A_UINT32 sleepState;
	}WMI_REPORT_SLEEP_STATE_EVENT;

	typedef enum {
		WMI_REPORT_SLEEP_STATUS_IS_DEEP_SLEEP =0,
		WMI_REPORT_SLEEP_STATUS_IS_AWAKE
	} WMI_REPORT_SLEEP_STATUS;

	/*
	 * List of Events (target to host)
	 */
	typedef enum {
		WMI_READY_EVENTID = 0x1001,
		WMI_CONNECT_EVENTID,
		WMI_DISCONNECT_EVENTID,
		WMI_BSSINFO_EVENTID,
		WMI_CMDERROR_EVENTID,
		WMI_REGDOMAIN_EVENTID,
		WMI_PSTREAM_TIMEOUT_EVENTID,
		WMI_NEIGHBOR_REPORT_EVENTID,
		WMI_TKIP_MICERR_EVENTID,
		WMI_SCAN_COMPLETE_EVENTID,	/* 0x100a */
		WMI_REPORT_STATISTICS_EVENTID,
		WMI_RSSI_THRESHOLD_EVENTID,
		WMI_ERROR_REPORT_EVENTID,
		WMI_OPT_RX_FRAME_EVENTID,
		WMI_REPORT_ROAM_TBL_EVENTID,
		WMI_EXTENSION_EVENTID,
		WMI_CAC_EVENTID,
		WMI_SNR_THRESHOLD_EVENTID,
		WMI_LQ_THRESHOLD_EVENTID,
		WMI_TX_RETRY_ERR_EVENTID,	/* 0x1014 */
		WMI_REPORT_ROAM_DATA_EVENTID,
		WMI_TEST_EVENTID,
		WMI_APLIST_EVENTID,
		WMI_GET_WOW_LIST_EVENTID,
		WMI_GET_PMKID_LIST_EVENTID,
		WMI_CHANNEL_CHANGE_EVENTID,
		WMI_PEER_NODE_EVENTID,
		WMI_PSPOLL_EVENTID,
		WMI_DTIMEXPIRY_EVENTID,
		WMI_WLAN_VERSION_EVENTID,
		WMI_SET_PARAMS_REPLY_EVENTID,
		WMI_ADDBA_REQ_EVENTID,		/*0x1020 */
		WMI_ADDBA_RESP_EVENTID,
		WMI_DELBA_REQ_EVENTID,
		WMI_TX_COMPLETE_EVENTID,
		WMI_HCI_EVENT_EVENTID,
		WMI_ACL_DATA_EVENTID,
		WMI_REPORT_SLEEP_STATE_EVENTID,
		WMI_WAPI_REKEY_EVENTID,
		WMI_REPORT_BTCOEX_STATS_EVENTID,
		WMI_REPORT_BTCOEX_CONFIG_EVENTID,
		WMI_GET_PMK_EVENTID,

		/* DFS Events */
		WMI_DFS_HOST_ATTACH_EVENTID,	/* 102B */
		WMI_DFS_HOST_INIT_EVENTID,
		WMI_DFS_RESET_DELAYLINES_EVENTID,
		WMI_DFS_RESET_RADARQ_EVENTID,
		WMI_DFS_RESET_AR_EVENTID,
		WMI_DFS_RESET_ARQ_EVENTID,
		WMI_DFS_SET_DUR_MULTIPLIER_EVENTID,
		WMI_DFS_SET_BANGRADAR_EVENTID,
		WMI_DFS_SET_DEBUGLEVEL_EVENTID,
		WMI_DFS_PHYERR_EVENTID,
		/* CCX Evants */
		WMI_CCX_RM_STATUS_EVENTID,	/* 1035 */

		/* P2P Events */
		WMI_P2P_GO_NEG_RESULT_EVENTID,	/* 1036 */

		WMI_WAC_SCAN_DONE_EVENTID,
		WMI_WAC_REPORT_BSS_EVENTID,
		WMI_WAC_START_WPS_EVENTID,
		WMI_WAC_CTRL_REQ_REPLY_EVENTID,
		WMI_REPORT_WMM_PARAMS_EVENTID,
		WMI_WAC_REJECT_WPS_EVENTID,

		/* More P2P Events */
		WMI_P2P_GO_NEG_REQ_EVENTID,
		WMI_P2P_INVITE_REQ_EVENTID,
		WMI_P2P_INVITE_RCVD_RESULT_EVENTID,
		WMI_P2P_INVITE_SENT_RESULT_EVENTID,
		WMI_P2P_PROV_DISC_RESP_EVENTID,
		WMI_P2P_PROV_DISC_REQ_EVENTID,

		/* RFKILL Events */
		WMI_RFKILL_STATE_CHANGE_EVENTID,
		WMI_RFKILL_GET_MODE_CMD_EVENTID,

		WMI_P2P_START_SDPD_EVENTID,
		WMI_P2P_SDPD_RX_EVENTID,

		/* Special event used to notify host that AR6003
		 * has processed sleep command (needed to prevent
		 * a late incoming credit report from crashing
		 * the system)
		 */
		WMI_SET_HOST_SLEEP_MODE_CMD_PROCESSED_EVENTID,

		WMI_THIN_RESERVED_START_EVENTID = 0x8000,
		/* Events in this range are reserved for thinmode
		 * See wmi_thin.h for actual definitions */
		WMI_THIN_RESERVED_END_EVENTID = 0x8fff,

		WMI_SET_CHANNEL_EVENTID,
		WMI_ASSOC_REQ_EVENTID,

		/* generic ACS event */
		WMI_ACS_EVENTID,
		WMI_STORERECALL_STORE_EVENTID,
		WMI_WOW_EXT_WAKE_EVENTID,
		WMI_GTK_OFFLOAD_STATUS_EVENTID,
		WMI_NETWORK_LIST_OFFLOAD_EVENTID,
		WMI_REMAIN_ON_CHNL_EVENTID,
		WMI_CANCEL_REMAIN_ON_CHNL_EVENTID,
		WMI_TX_STATUS_EVENTID,
		WMI_RX_PROBE_REQ_EVENTID,
		WMI_P2P_CAPABILITIES_EVENTID,
		WMI_RX_ACTION_EVENTID,
		WMI_P2P_INFO_EVENTID,
		/* WPS Events */
		WMI_WPS_GET_STATUS_EVENTID,
		WMI_WPS_PROFILE_EVENTID,

		/* more P2P events */
		WMI_NOA_INFO_EVENTID,
		WMI_OPPPS_INFO_EVENTID,
		WMI_PORT_STATUS_EVENTID,

		/* 802.11w */
		WMI_GET_RSN_CAP_EVENTID,

		WMI_FLOWCTRL_IND_EVENTID,
		WMI_FLOWCTRL_UAPSD_FRAME_DILIVERED_EVENTID,

		/*Socket Translation Events*/
		WMI_SOCKET_RESPONSE_EVENTID,

		WMI_LOG_FRAME_EVENTID,
		WMI_QUERY_PHY_INFO_EVENTID,
		WMI_CCX_ROAMING_EVENTID,

		WMI_P2P_NODE_LIST_EVENTID,
		WMI_P2P_REQ_TO_AUTH_EVENTID,
		WMI_DIAGNOSTIC_EVENTID,	/* diagnostic */
		WMI_DISC_PEER_EVENTID,	/* wifi discovery */
		WMI_BSS_RSSI_INFO_EVENTID,
	} WMI_EVENT_ID;

	typedef PREPACK struct {
		A_UINT8     macaddr[ATH_MAC_LEN];
		A_UINT8     phyCapability;              /* WMI_PHY_CAPABILITY */
	} POSTPACK WMI_READY_EVENT_1;

	typedef PREPACK struct {
		A_UINT32    sw_version;
		A_UINT32    abi_version;
		A_UINT8     macaddr[ATH_MAC_LEN];
		A_UINT8     phyCapability;              /* WMI_PHY_CAPABILITY */
	} POSTPACK WMI_READY_EVENT_2;

#if defined(ATH_TARGET)
#ifdef AR6002_REV2
#define WMI_READY_EVENT WMI_READY_EVENT_1  /* AR6002_REV2 target code */
#else
#define WMI_READY_EVENT WMI_READY_EVENT_2  /* AR6001, AR6002_REV4, AR6002_REV6 */
#endif
#else
#define WMI_READY_EVENT WMI_READY_EVENT_2 /* host code */
#endif



	/*
	 * Connect Event
	 */
	typedef PREPACK struct {
		PREPACK union {
			struct {
				A_UINT16    channel;
				A_UINT8     bssid[ATH_MAC_LEN];
				A_UINT16    listenInterval;
				A_UINT16    beaconInterval;
				A_UINT32    networkType;
			} infra_ibss_bss;
			struct {
				A_UINT8     phymode;
				A_UINT8     aid;
				A_UINT8     mac_addr[ATH_MAC_LEN];
				A_UINT8     auth;
				A_UINT8     keymgmt;
				A_UINT16    cipher;
				A_UINT8     apsd_info;
				A_UINT8     unused[3];
			} ap_sta;
			struct {
				A_UINT16    channel;
				A_UINT8     bssid[ATH_MAC_LEN];
				A_UINT8     unused[8];
			} ap_bss;
		} POSTPACK u;
		A_UINT8     beaconIeLen;
		A_UINT8     assocReqLen;
		A_UINT8     assocRespLen;
		A_UINT8     assocInfo[1];
	} POSTPACK WMI_CONNECT_EVENT;

	/*
	 * Disconnect Event
	 */
	typedef enum {
		NO_NETWORK_AVAIL   = 0x01,
		LOST_LINK          = 0x02,     /* bmiss */
		DISCONNECT_CMD     = 0x03,
		BSS_DISCONNECTED   = 0x04,
		AUTH_FAILED        = 0x05,
		ASSOC_FAILED       = 0x06,
		NO_RESOURCES_AVAIL = 0x07,
		CSERV_DISCONNECT   = 0x08,
		INVALID_PROFILE    = 0x0a,
		DOT11H_CHANNEL_SWITCH = 0x0b,
		PROFILE_MISMATCH   = 0x0c,
		CONNECTION_EVICTED = 0x0d,
		IBSS_MERGE         = 0x0e,
		EXCESS_TX_RETRY    = 0x0f,     /* TX frames failed after excessive retries */
		SEC_HS_TO_RECV_M1  = 0x10,     /* Security 4-way handshake timed out waiting for M1 */
		SEC_HS_TO_RECV_M3  = 0x11,     /* Security 4-way handshake timed out waiting for M3 */
		TKIP_COUNTERMEASURES = 0x12,
	} WMI_DISCONNECT_REASON;

	typedef PREPACK struct {
		A_UINT16    protocolReasonStatus;  /* reason code, see 802.11 spec. */
		A_UINT8     bssid[ATH_MAC_LEN];    /* set if known */
		A_UINT8     disconnectReason ;      /* see WMI_DISCONNECT_REASON */
		A_UINT8     assocRespLen;
		A_UINT8     assocInfo[1];
	} POSTPACK WMI_DISCONNECT_EVENT;

	/*
	 * Command Error Event
	 */
	typedef enum {
		INVALID_PARAM  = 0x01,
		ILLEGAL_STATE  = 0x02,
		INTERNAL_ERROR = 0x03,
	} WMI_ERROR_CODE;

	typedef PREPACK struct {
		A_UINT16    commandId;
		A_UINT8     errorCode;
	} POSTPACK WMI_CMD_ERROR_EVENT;

	/*
	 *  WMI_ERROR_REPORT_EVENTID
	 */
	typedef enum{
		WMI_TARGET_PM_ERR_FAIL      = 0x00000001,
		WMI_TARGET_KEY_NOT_FOUND    = 0x00000002,
		WMI_TARGET_DECRYPTION_ERR   = 0x00000004,
		WMI_TARGET_BMISS            = 0x00000008,
		WMI_PSDISABLE_NODE_JOIN     = 0x00000010,
		WMI_TARGET_COM_ERR          = 0x00000020,
		WMI_TARGET_FATAL_ERR        = 0x00000040,
		WMI_TARGET_BCN_FOUND        = 0x00000080
	} WMI_TARGET_ERROR_VAL;

	typedef PREPACK struct {
		A_UINT32 errorVal;
	}POSTPACK  WMI_TARGET_ERROR_REPORT_EVENT;

	typedef PREPACK struct {
		A_UINT8 retrys;
	}POSTPACK  WMI_TX_RETRY_ERR_EVENT;

#ifndef ATH_TARGET
#include "athendpack.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _WMI_H_ */
