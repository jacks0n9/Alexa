/*
 * IEEE 802.11 Frame type definitions
 *
 * Copyright (c) 2010 Amazon Technologies, Inc.  All rights reserved.
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * Use is subject to license terms.
 */


#ifndef IEEE802_11_H
#define IEEE802_11_H

#ifdef __linux__
#include <endian.h>
#include <byteswap.h>
#endif /* __linux__ */

#include <stddef.h>

#ifndef WPA_TYPES_DEFINED
#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;
#define WPA_TYPES_DEFINED
#endif

#undef BIT
#define BIT(n)  (1 << n)

#ifdef __GNUC__
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define STRUCT_PACKED
#endif

/* IEEE 802.11 defines */

#define WLAN_FC_PVER        0x0003
#define WLAN_FC_TODS        0x0100
#define WLAN_FC_FROMDS      0x0200
#define WLAN_FC_MOREFRAG    0x0400
#define WLAN_FC_RETRY       0x0800
#define WLAN_FC_PWRMGT      0x1000
#define WLAN_FC_MOREDATA    0x2000
#define WLAN_FC_ISWEP       0x4000
#define WLAN_FC_ORDER       0x8000

#define WLAN_FC_GET_TYPE(fc)    (((fc) & 0x000c) >> 2)
#define WLAN_FC_GET_STYPE(fc)   (((fc) & 0x00f0) >> 4)

#define WLAN_GET_SEQ_FRAG(seq) ((seq) & (BIT(3) | BIT(2) | BIT(1) | BIT(0)))
#define WLAN_GET_SEQ_SEQ(seq) \
    (((seq) & (~(BIT(3) | BIT(2) | BIT(1) | BIT(0)))) >> 4)

#define WLAN_FC_TYPE_MGMT       0
#define WLAN_FC_TYPE_CTRL       1
#define WLAN_FC_TYPE_DATA       2

/* management */
#define WLAN_FC_STYPE_ASSOC_REQ     0
#define WLAN_FC_STYPE_ASSOC_RESP    1
#define WLAN_FC_STYPE_REASSOC_REQ   2
#define WLAN_FC_STYPE_REASSOC_RESP  3
#define WLAN_FC_STYPE_PROBE_REQ     4
#define WLAN_FC_STYPE_PROBE_RESP    5
#define WLAN_FC_STYPE_BEACON        8
#define WLAN_FC_STYPE_ATIM      9
#define WLAN_FC_STYPE_DISASSOC      10
#define WLAN_FC_STYPE_AUTH      11
#define WLAN_FC_STYPE_DEAUTH        12
#define WLAN_FC_STYPE_ACTION        13

/* control */
#define WLAN_FC_STYPE_PSPOLL        10
#define WLAN_FC_STYPE_RTS       11
#define WLAN_FC_STYPE_CTS       12
#define WLAN_FC_STYPE_ACK       13
#define WLAN_FC_STYPE_CFEND     14
#define WLAN_FC_STYPE_CFENDACK      15

/* data */
#define WLAN_FC_STYPE_DATA      0
#define WLAN_FC_STYPE_DATA_CFACK    1
#define WLAN_FC_STYPE_DATA_CFPOLL   2
#define WLAN_FC_STYPE_DATA_CFACKPOLL    3
#define WLAN_FC_STYPE_NULLFUNC      4
#define WLAN_FC_STYPE_CFACK     5
#define WLAN_FC_STYPE_CFPOLL        6
#define WLAN_FC_STYPE_CFACKPOLL     7
#define WLAN_FC_STYPE_QOS_DATA      8

/* Authentication algorithms */
#define WLAN_AUTH_OPEN          0
#define WLAN_AUTH_SHARED_KEY        1
#define WLAN_AUTH_FT            2
#define WLAN_AUTH_LEAP          128

#define WLAN_AUTH_CHALLENGE_LEN 128

#define WLAN_CAPABILITY_ESS BIT(0)
#define WLAN_CAPABILITY_IBSS BIT(1)
#define WLAN_CAPABILITY_CF_POLLABLE BIT(2)
#define WLAN_CAPABILITY_CF_POLL_REQUEST BIT(3)
#define WLAN_CAPABILITY_PRIVACY BIT(4)
#define WLAN_CAPABILITY_SHORT_PREAMBLE BIT(5)
#define WLAN_CAPABILITY_PBCC BIT(6)
#define WLAN_CAPABILITY_CHANNEL_AGILITY BIT(7)
#define WLAN_CAPABILITY_SPECTRUM_MGMT BIT(8)
#define WLAN_CAPABILITY_SHORT_SLOT_TIME BIT(10)
#define WLAN_CAPABILITY_DSSS_OFDM BIT(13)

/* Status codes (IEEE 802.11-2007, 7.3.1.9, Table 7-23) */
#define WLAN_STATUS_SUCCESS 0
#define WLAN_STATUS_UNSPECIFIED_FAILURE 1
#define WLAN_STATUS_CAPS_UNSUPPORTED 10
#define WLAN_STATUS_REASSOC_NO_ASSOC 11
#define WLAN_STATUS_ASSOC_DENIED_UNSPEC 12
#define WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG 13
#define WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION 14
#define WLAN_STATUS_CHALLENGE_FAIL 15
#define WLAN_STATUS_AUTH_TIMEOUT 16
#define WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA 17
#define WLAN_STATUS_ASSOC_DENIED_RATES 18
/* IEEE 802.11b */
#define WLAN_STATUS_ASSOC_DENIED_NOSHORT 19
#define WLAN_STATUS_ASSOC_DENIED_NOPBCC 20
#define WLAN_STATUS_ASSOC_DENIED_NOAGILITY 21
/* IEEE 802.11h */
#define WLAN_STATUS_SPEC_MGMT_REQUIRED 22
#define WLAN_STATUS_PWR_CAPABILITY_NOT_VALID 23
#define WLAN_STATUS_SUPPORTED_CHANNEL_NOT_VALID 24
/* IEEE 802.11g */
#define WLAN_STATUS_ASSOC_DENIED_NO_SHORT_SLOT_TIME 25
#define WLAN_STATUS_ASSOC_DENIED_NO_ER_PBCC 26
#define WLAN_STATUS_ASSOC_DENIED_NO_DSSS_OFDM 27
/* IEEE 802.11w */
#define WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY 30
#define WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION 31
/* IEEE 802.11i */
#define WLAN_STATUS_INVALID_IE 40
#define WLAN_STATUS_GROUP_CIPHER_NOT_VALID 41
#define WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID 42
#define WLAN_STATUS_AKMP_NOT_VALID 43
#define WLAN_STATUS_UNSUPPORTED_RSN_IE_VERSION 44
#define WLAN_STATUS_INVALID_RSN_IE_CAPAB 45
#define WLAN_STATUS_CIPHER_REJECTED_PER_POLICY 46
#define WLAN_STATUS_TS_NOT_CREATED 47
#define WLAN_STATUS_DIRECT_LINK_NOT_ALLOWED 48
#define WLAN_STATUS_DEST_STA_NOT_PRESENT 49
#define WLAN_STATUS_DEST_STA_NOT_QOS_STA 50
#define WLAN_STATUS_ASSOC_DENIED_LISTEN_INT_TOO_LARGE 51
/* IEEE 802.11r */
#define WLAN_STATUS_INVALID_FT_ACTION_FRAME_COUNT 52
#define WLAN_STATUS_INVALID_PMKID 53
#define WLAN_STATUS_INVALID_MDIE 54
#define WLAN_STATUS_INVALID_FTIE 55

/* Reason codes (IEEE 802.11-2007, 7.3.1.7, Table 7-22) */
#define WLAN_REASON_UNSPECIFIED 1
#define WLAN_REASON_PREV_AUTH_NOT_VALID 2
#define WLAN_REASON_DEAUTH_LEAVING 3
#define WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY 4
#define WLAN_REASON_DISASSOC_AP_BUSY 5
#define WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA 6
#define WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA 7
#define WLAN_REASON_DISASSOC_STA_HAS_LEFT 8
#define WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH 9
/* IEEE 802.11h */
#define WLAN_REASON_PWR_CAPABILITY_NOT_VALID 10
#define WLAN_REASON_SUPPORTED_CHANNEL_NOT_VALID 11
/* IEEE 802.11i */
#define WLAN_REASON_INVALID_IE 13
#define WLAN_REASON_MICHAEL_MIC_FAILURE 14
#define WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT 15
#define WLAN_REASON_GROUP_KEY_UPDATE_TIMEOUT 16
#define WLAN_REASON_IE_IN_4WAY_DIFFERS 17
#define WLAN_REASON_GROUP_CIPHER_NOT_VALID 18
#define WLAN_REASON_PAIRWISE_CIPHER_NOT_VALID 19
#define WLAN_REASON_AKMP_NOT_VALID 20
#define WLAN_REASON_UNSUPPORTED_RSN_IE_VERSION 21
#define WLAN_REASON_INVALID_RSN_IE_CAPAB 22
#define WLAN_REASON_IEEE_802_1X_AUTH_FAILED 23
#define WLAN_REASON_CIPHER_SUITE_REJECTED 24


/* Information Element IDs */
#define WLAN_EID_SSID 0
#define WLAN_EID_SUPP_RATES 1
#define WLAN_EID_FH_PARAMS 2
#define WLAN_EID_DS_PARAMS 3
#define WLAN_EID_CF_PARAMS 4
#define WLAN_EID_TIM 5
#define WLAN_EID_IBSS_PARAMS 6
#define WLAN_EID_COUNTRY 7
#define WLAN_EID_CHALLENGE 16
/* EIDs defined by IEEE 802.11h - START */
#define WLAN_EID_PWR_CONSTRAINT 32
#define WLAN_EID_PWR_CAPABILITY 33
#define WLAN_EID_TPC_REQUEST 34
#define WLAN_EID_TPC_REPORT 35
#define WLAN_EID_SUPPORTED_CHANNELS 36
#define WLAN_EID_CHANNEL_SWITCH 37
#define WLAN_EID_MEASURE_REQUEST 38
#define WLAN_EID_MEASURE_REPORT 39
#define WLAN_EID_QUITE 40
#define WLAN_EID_IBSS_DFS 41
/* EIDs defined by IEEE 802.11h - END */
#define WLAN_EID_ERP_INFO 42
#define WLAN_EID_HT_CAP 45
#define WLAN_EID_RSN 48
#define WLAN_EID_EXT_SUPP_RATES 50
#define WLAN_EID_MOBILITY_DOMAIN 54
#define WLAN_EID_FAST_BSS_TRANSITION 55
#define WLAN_EID_TIMEOUT_INTERVAL 56
#define WLAN_EID_RIC_DATA 57
#define WLAN_EID_HT_OPERATION 61
#define WLAN_EID_SECONDARY_CHANNEL_OFFSET 62
#define WLAN_EID_20_40_BSS_COEXISTENCE 72
#define WLAN_EID_20_40_BSS_INTOLERANT 73
#define WLAN_EID_OVERLAPPING_BSS_SCAN_PARAMS 74
#define WLAN_EID_MMIE 76
#define WLAN_EID_VENDOR_SPECIFIC 221
#define WLAN_EID_WAPI 68
#define GENERIC_INFO_ELEM 0xdd
#define RSN_INFO_ELEM 0x30

#define SSID_MAX_WPA_IE_LEN 40
#define SSID_MAX_WAPI_IE_LEN 100

#define SSID_MAX_WPS_IE_LEN 0x0800

/* Action frame categories (IEEE 802.11-2007, 7.3.1.11, Table 7-24) */
#define WLAN_ACTION_SPECTRUM_MGMT 0
#define WLAN_ACTION_QOS 1
#define WLAN_ACTION_DLS 2
#define WLAN_ACTION_BLOCK_ACK 3
#define WLAN_ACTION_RADIO_MEASUREMENT 5
#define WLAN_ACTION_FT 6
#define WLAN_ACTION_SA_QUERY 8
#define WLAN_ACTION_WMM 17

/* SA Query Action frame (IEEE 802.11w/D7.0, 7.4.9) */
#define WLAN_SA_QUERY_REQUEST 0
#define WLAN_SA_QUERY_RESPONSE 1

#define WLAN_SA_QUERY_TR_ID_LEN 16

/* Timeout Interval Type */
#define WLAN_TIMEOUT_REASSOC_DEADLINE 1
#define WLAN_TIMEOUT_KEY_LIFETIME 2
#define WLAN_TIMEOUT_ASSOC_COMEBACK 3



#define ERP_INFO_NON_ERP_PRESENT BIT(0)
#define ERP_INFO_USE_PROTECTION BIT(1)
#define ERP_INFO_BARKER_PREAMBLE_MODE BIT(2)


/* HT Capability element */

enum {
    MAX_RX_AMPDU_FACTOR_8KB = 0,
    MAX_RX_AMPDU_FACTOR_16KB,
    MAX_RX_AMPDU_FACTOR_32KB,
    MAX_RX_AMPDU_FACTOR_64KB
};

enum {
    CALIBRATION_NOT_SUPPORTED = 0,
    CALIBRATION_CANNOT_INIT,
    CALIBRATION_CAN_INIT,
    CALIBRATION_FULL_SUPPORT
};

enum {
    MCS_FEEDBACK_NOT_PROVIDED = 0,
    MCS_FEEDBACK_UNSOLICITED,
    MCS_FEEDBACK_MRQ_RESPONSE
};


struct ieee80211_ht_capability {
    u16 capabilities_info;
    u8 mac_ht_params_info;
    u8 supported_mcs_set[16];
    u16 extended_ht_capability_info;
    u32 tx_BF_capability_info;
    u8 antenna_selection_info;
} STRUCT_PACKED;


struct ieee80211_ht_operation {
    u8 control_chan;
    u8 ht_param;
    u16 operation_mode;
    u16 stbc_param;
    u8 basic_set[16];
} STRUCT_PACKED;

/* auxiliary bit manipulation macros FIXME: move it to common later... */
#define SET_2BIT_U8(_ptr_, _shift_, _val_)              \
    ((*(_ptr_) &= ~(3 << (_shift_))),               \
     (*(_ptr_) |= (*(_ptr_) & (((u8)3) << (_shift_))) |     \
              (((u8)(_val_) & 3) << _shift_)))

#define GET_2BIT_U8(_var_, _shift_) \
    (((_var_) & (((u8)3) << (_shift_))) >> (_shift_))

#define SET_2BIT_LE16(_u16ptr_, _shift_, _val_)             \
    ((*(_u16ptr_) &= ~(3 << (_shift_))),                \
     (*(_u16ptr_) |=                        \
        (((*(_u16ptr_)) & (((u16)3) << ((u16)_shift_))) |   \
        (((u16)(_val_) & (u16)3) << (u16)(_shift_)))))

#define GET_2BIT_LE16(_var_, _shift_)   \
    (((_var_) & (((u16)3) << (_shift_))) >> (_shift_))

#define SET_2BIT_LE32(_u32ptr_, _shift_, _val_)             \
    ((*(_u32ptr_) &= ~(3 << (_shift_))),                \
     (*(_u32ptr_) |= (((*(_u32ptr_)) & (((u32)3) << (_shift_))) |   \
            (((u32)(_val_) & 3) << _shift_))))

#define GET_2BIT_LE32(_var_, _shift_)   \
    (((_var_) & (((u32)3) << (_shift_))) >> (_shift_))

#define SET_3BIT_LE16(_u16ptr_, _shift_, _val_)             \
    ((*(_u16ptr_) &= ~(7 << (_shift_))),                \
    (*(_u16ptr_) |= (((*(_u16ptr_)) & (((u16)7) << (_shift_))) |    \
            (((u16)(_val_) & 7) << _shift_))))

#define GET_3BIT_LE16(_var_, _shift_)   \
    (((_var_) & (((u16)7) << (_shift_))) >> (_shift_))

#define SET_3BIT_LE32(_u32ptr_, _shift_, _val_)             \
    ((*(_u32ptr_) &= ~(7 << (_shift_))),                \
     (*(_u32ptr_) |= (((*(_u32ptr_)) & (((u32)7) << (_shift_))) |   \
            (((u32)(_val_) & 7) << _shift_))))

#define GET_3BIT_LE32(_var_, _shift_)   \
    (((_var_) & (((u32)7) << (_shift_))) >> (_shift_))


#define HT_CAP_INFO_LDPC_CODING_CAP     ((u16) BIT(0))
#define HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET  ((u16) BIT(1))
#define HT_CAP_INFO_SMPS_MASK           ((u16) (BIT(2) | BIT(3)))
#define HT_CAP_INFO_SMPS_STATIC         ((u16) 0)
#define HT_CAP_INFO_SMPS_DYNAMIC        ((u16) BIT(2))
#define HT_CAP_INFO_SMPS_DISABLED       ((u16) (BIT(2) | BIT(3)))
#define HT_CAP_INFO_GREEN_FIELD         ((u16) BIT(4))
#define HT_CAP_INFO_SHORT_GI20MHZ       ((u16) BIT(5))
#define HT_CAP_INFO_SHORT_GI40MHZ       ((u16) BIT(6))
#define HT_CAP_INFO_TX_STBC         ((u16) BIT(7))
#define HT_CAP_INFO_RX_STBC_MASK        ((u16) (BIT(8) | BIT(9)))
#define HT_CAP_INFO_RX_STBC_1           ((u16) BIT(8))
#define HT_CAP_INFO_RX_STBC_12          ((u16) BIT(9))
#define HT_CAP_INFO_RX_STBC_123         ((u16) (BIT(8) | BIT(9)))
#define HT_CAP_INFO_DELAYED_BA          ((u16) BIT(10))
#define HT_CAP_INFO_MAX_AMSDU_SIZE      ((u16) BIT(11))
#define HT_CAP_INFO_DSSS_CCK40MHZ       ((u16) BIT(12))
#define HT_CAP_INFO_PSMP_SUPP           ((u16) BIT(13))
#define HT_CAP_INFO_40MHZ_INTOLERANT        ((u16) BIT(14))
#define HT_CAP_INFO_LSIG_TXOP_PROTECT_SUPPORT   ((u16) BIT(15))


#define MAC_HT_PARAM_INFO_MAX_RX_AMPDU_FACTOR_OFFSET    0
#define MAC_HT_PARAM_INFO_MAX_MPDU_DENSITY_OFFSET   2

#define EXT_HT_CAP_INFO_PCO         ((u16) BIT(0))
#define EXT_HT_CAP_INFO_TRANS_TIME_OFFSET   1
#define EXT_HT_CAP_INFO_MCS_FEEDBACK_OFFSET 8
#define EXT_HT_CAP_INFO_HTC_SUPPORTED       ((u16) BIT(10))
#define EXT_HT_CAP_INFO_RD_RESPONDER        ((u16) BIT(11))


#define TX_BEAMFORM_CAP_TXBF_CAP ((u32) BIT(0))
#define TX_BEAMFORM_CAP_RX_STAGGERED_SOUNDING_CAP ((u32) BIT(1))
#define TX_BEAMFORM_CAP_TX_STAGGERED_SOUNDING_CAP ((u32) BIT(2))
#define TX_BEAMFORM_CAP_RX_ZLF_CAP ((u32) BIT(3))
#define TX_BEAMFORM_CAP_TX_ZLF_CAP ((u32) BIT(4))
#define TX_BEAMFORM_CAP_IMPLICIT_ZLF_CAP ((u32) BIT(5))
#define TX_BEAMFORM_CAP_CALIB_OFFSET 6
#define TX_BEAMFORM_CAP_EXPLICIT_CSI_TXBF_CAP ((u32) BIT(8))
#define TX_BEAMFORM_CAP_EXPLICIT_UNCOMPR_STEERING_MATRIX_CAP ((u32) BIT(9))
#define TX_BEAMFORM_CAP_EXPLICIT_BF_CSI_FEEDBACK_CAP ((u32) BIT(10))
#define TX_BEAMFORM_CAP_EXPLICIT_BF_CSI_FEEDBACK_OFFSET 11
#define TX_BEAMFORM_CAP_EXPLICIT_UNCOMPR_STEERING_MATRIX_FEEDBACK_OFFSET 13
#define TX_BEAMFORM_CAP_EXPLICIT_COMPRESSED_STEERING_MATRIX_FEEDBACK_OFFSET 15
#define TX_BEAMFORM_CAP_MINIMAL_GROUPING_OFFSET 17
#define TX_BEAMFORM_CAP_CSI_NUM_BEAMFORMER_ANT_OFFSET 19
#define TX_BEAMFORM_CAP_UNCOMPRESSED_STEERING_MATRIX_BEAMFORMER_ANT_OFFSET 21
#define TX_BEAMFORM_CAP_COMPRESSED_STEERING_MATRIX_BEAMFORMER_ANT_OFFSET 23
#define TX_BEAMFORM_CAP_SCI_MAX_OF_ROWS_BEANFORMER_SUPPORTED_OFFSET 25


#define ASEL_CAPABILITY_ASEL_CAPABLE ((u8) BIT(0))
#define ASEL_CAPABILITY_EXPLICIT_CSI_FEEDBACK_BASED_TX_AS_CAP ((u8) BIT(1))
#define ASEL_CAPABILITY_ANT_INDICES_FEEDBACK_BASED_TX_AS_CAP ((u8) BIT(2))
#define ASEL_CAPABILITY_EXPLICIT_CSI_FEEDBACK_CAP ((u8) BIT(3))
#define ASEL_CAPABILITY_ANT_INDICES_FEEDBACK_CAP ((u8) BIT(4))
#define ASEL_CAPABILITY_RX_AS_CAP ((u8) BIT(5))
#define ASEL_CAPABILITY_TX_SOUND_PPDUS_CAP ((u8) BIT(6))


struct ht_cap_ie {
    u8 id;
    u8 length;
    struct ieee80211_ht_capability data;
} STRUCT_PACKED;


#define REC_TRANS_CHNL_WIDTH_20     0
#define REC_TRANS_CHNL_WIDTH_ANY    1

#define OP_MODE_PURE                    0
#define OP_MODE_MAY_BE_LEGACY_STAS      1
#define OP_MODE_20MHZ_HT_STA_ASSOCED    2
#define OP_MODE_MIXED                   3

#define HT_INFO_HT_PARAM_SECONDARY_CHNL_OFF_MASK    ((u8) BIT(0) | BIT(1))
#define HT_INFO_HT_PARAM_SECONDARY_CHNL_ABOVE       ((u8) BIT(0))
#define HT_INFO_HT_PARAM_SECONDARY_CHNL_BELOW       ((u8) BIT(0) | BIT(1))
#define HT_INFO_HT_PARAM_REC_TRANS_CHNL_WIDTH       ((u8) BIT(2))
#define HT_INFO_HT_PARAM_RIFS_MODE          ((u8) BIT(3))
#define HT_INFO_HT_PARAM_CTRL_ACCESS_ONLY       ((u8) BIT(4))
#define HT_INFO_HT_PARAM_SRV_INTERVAL_GRANULARITY   ((u8) BIT(5))

#define HT_INFO_OPERATION_MODE_OP_MODE_MASK \
        ((le16) (0x0001 | 0x0002))
#define HT_INFO_OPERATION_MODE_OP_MODE_OFFSET       0
#define HT_INFO_OPERATION_MODE_NON_GF_DEVS_PRESENT  ((u8) BIT(2))
#define HT_INFO_OPERATION_MODE_TRANSMIT_BURST_LIMIT ((u8) BIT(3))
#define HT_INFO_OPERATION_MODE_NON_HT_STA_PRESENT   ((u8) BIT(4))

#define HT_INFO_STBC_PARAM_DUAL_BEACON          ((u16) BIT(6))
#define HT_INFO_STBC_PARAM_DUAL_STBC_PROTECT        ((u16) BIT(7))
#define HT_INFO_STBC_PARAM_SECONDARY_BCN        ((u16) BIT(8))
#define HT_INFO_STBC_PARAM_LSIG_TXOP_PROTECT_ALLOWED    ((u16) BIT(9))
#define HT_INFO_STBC_PARAM_PCO_ACTIVE           ((u16) BIT(10))
#define HT_INFO_STBC_PARAM_PCO_PHASE            ((u16) BIT(11))


/* Secondary channel offset element */
#define SECONDARY_CHANNEL_OFFSET_NONE   0
#define SECONDARY_CHANNEL_OFFSET_ABOVE  1
#define SECONDARY_CHANNEL_OFFSET_BELOW  3
struct secondary_channel_offset_ie {
    u8 id;
    u8 length;
    u8 secondary_offset_offset;
} STRUCT_PACKED;


/* body of Recommended Transmit Channel Width action frame */
#define CHANNEL_WIDTH_20    0
#define CHANNEL_WIDTH_ANY   1
struct recommended_tx_channel_width_action {
    u8 category;
    u8 action;
    u8 channel_width;
} STRUCT_PACKED;

/* body of MIMO Power Save action frame */
#define PWR_SAVE_MODE_STATIC    0
#define PWR_SAVE_MODE_DYNAMIC   1
struct mimo_pwr_save_action {
    u8 category;
    u8 action;
    u8 enable;
    u8 mode;
} STRUCT_PACKED;


#define WME_OUI_TYPE 2
#define WME_OUI_SUBTYPE_INFORMATION_ELEMENT 0
#define WME_OUI_SUBTYPE_PARAMETER_ELEMENT 1
#define WME_OUI_SUBTYPE_TSPEC_ELEMENT 2
#define WME_VERSION 1

#define WME_ACTION_CODE_SETUP_REQUEST 0
#define WME_ACTION_CODE_SETUP_RESPONSE 1
#define WME_ACTION_CODE_TEARDOWN 2

#define WME_SETUP_RESPONSE_STATUS_ADMISSION_ACCEPTED 0
#define WME_SETUP_RESPONSE_STATUS_INVALID_PARAMETERS 1
#define WME_SETUP_RESPONSE_STATUS_REFUSED 3

#define WME_TSPEC_DIRECTION_UPLINK 0
#define WME_TSPEC_DIRECTION_DOWNLINK 1
#define WME_TSPEC_DIRECTION_BI_DIRECTIONAL 3


#define VENDOR_HT_CAPAB_OUI_TYPE 0x33 /* 00-90-4c:0x33 */

#define WPA_CIPHER_NONE BIT(0)
#define WPA_CIPHER_WEP40 BIT(1)
#define WPA_CIPHER_WEP104 BIT(2)
#define WPA_CIPHER_TKIP BIT(3)
#define WPA_CIPHER_CCMP BIT(4)
#define WPA_CIPHER_AES_128_CMAC BIT(5)
#define WPA_CIPHER_SMS4 BIT(6)


#define WPA_KEY_MGMT_IEEE8021X BIT(0)
#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_NONE BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA BIT(3)
#define WPA_KEY_MGMT_WPA_NONE BIT(4)
#define WPA_KEY_MGMT_FT_IEEE8021X BIT(5)
#define WPA_KEY_MGMT_FT_PSK BIT(6)
#define WPA_KEY_MGMT_IEEE8021X_SHA256 BIT(7)
#define WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define WPA_KEY_MGMT_WPS BIT(9)
#define WPA_KEY_MGMT_WAPI_PSK   BIT(10)
#define	WPA_KEY_MGMT_WAPI_CERT  BIT(11)
	


static inline int wpa_key_mgmt_wpa_ieee8021x(int akm)
{
    return akm == WPA_KEY_MGMT_IEEE8021X ||
        akm == WPA_KEY_MGMT_FT_IEEE8021X ||
        akm == WPA_KEY_MGMT_IEEE8021X_SHA256;
}

static inline int wpa_key_mgmt_wpa_psk(int akm)
{
    return akm == WPA_KEY_MGMT_PSK ||
        akm == WPA_KEY_MGMT_FT_PSK ||
        akm == WPA_KEY_MGMT_PSK_SHA256;
}

static inline int wpa_key_mgmt_ft(int akm)
{
    return akm == WPA_KEY_MGMT_FT_PSK ||
        akm == WPA_KEY_MGMT_FT_IEEE8021X;
}

static inline int wpa_key_mgmt_sha256(int akm)
{
    return akm == WPA_KEY_MGMT_PSK_SHA256 ||
        akm == WPA_KEY_MGMT_IEEE8021X_SHA256;
}


#define WPA_PROTO_WPA BIT(0)
#define WPA_PROTO_RSN BIT(1)
#define WPA_PROTO_WAPI BIT(2)

#define WPA_AUTH_ALG_OPEN BIT(0)
#define WPA_AUTH_ALG_SHARED BIT(1)
#define WPA_AUTH_ALG_LEAP BIT(2)


typedef enum { WPA_ALG_NONE, WPA_ALG_WEP, WPA_ALG_TKIP, WPA_ALG_CCMP,
           WPA_ALG_IGTK, WPA_ALG_PMK
			   ,WPA_ALG_SMS4
         } wpa_alg;
typedef enum { CIPHER_NONE, CIPHER_WEP40, CIPHER_TKIP, CIPHER_CCMP,
           CIPHER_WEP104 
			   ,CIPHER_SMS4
         } wpa_cipher;
typedef enum { KEY_MGMT_802_1X, KEY_MGMT_PSK, KEY_MGMT_NONE,
           KEY_MGMT_802_1X_NO_WPA, KEY_MGMT_WPA_NONE,
           KEY_MGMT_FT_802_1X, KEY_MGMT_FT_PSK,
           KEY_MGMT_802_1X_SHA256, KEY_MGMT_PSK_SHA256,
           KEY_MGMT_WPS,
           KEY_MGMT_WAPI
} wpa_key_mgmt;

#define WPA_MAX_SSID_LEN 32

/* IEEE 802.11i */
#define PMKID_LEN 16
#define PMK_LEN 32
#define WPA_REPLAY_COUNTER_LEN 8
#define WPA_NONCE_LEN 32
#define WPA_KEY_RSC_LEN 8
#define WPA_GMK_LEN 32
#define WPA_GTK_MAX_LEN 32

#define WPA_SELECTOR_LEN 4
#define WPA_VERSION 1
#define RSN_SELECTOR_LEN 4
#define RSN_VERSION 1

#define RSN_SELECTOR(a, b, c, d) \
    ((((u32) (a)) << 24) | (((u32) (b)) << 16) | (((u32) (c)) << 8) | \
     (u32) (d))

#define WPA_AUTH_KEY_MGMT_NONE RSN_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_AUTH_KEY_MGMT_UNSPEC_802_1X RSN_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X RSN_SELECTOR(0x00, 0x50, 0xf2, 2)
#define WPA_CIPHER_SUITE_NONE RSN_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_CIPHER_SUITE_WEP40 RSN_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_CIPHER_SUITE_TKIP RSN_SELECTOR(0x00, 0x50, 0xf2, 2)
#if 0
#define WPA_CIPHER_SUITE_WRAP RSN_SELECTOR(0x00, 0x50, 0xf2, 3)
#endif
#define WPA_CIPHER_SUITE_CCMP RSN_SELECTOR(0x00, 0x50, 0xf2, 4)
#define WPA_CIPHER_SUITE_WEP104 RSN_SELECTOR(0x00, 0x50, 0xf2, 5)


#define RSN_AUTH_KEY_MGMT_UNSPEC_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 2)
#define RSN_AUTH_KEY_MGMT_FT_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 3)
#define RSN_AUTH_KEY_MGMT_FT_PSK RSN_SELECTOR(0x00, 0x0f, 0xac, 4)
#define RSN_AUTH_KEY_MGMT_802_1X_SHA256 RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_AUTH_KEY_MGMT_PSK_SHA256 RSN_SELECTOR(0x00, 0x0f, 0xac, 6)

#define RSN_CIPHER_SUITE_NONE RSN_SELECTOR(0x00, 0x0f, 0xac, 0)
#define RSN_CIPHER_SUITE_WEP40 RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_CIPHER_SUITE_TKIP RSN_SELECTOR(0x00, 0x0f, 0xac, 2)
#if 0
#define RSN_CIPHER_SUITE_WRAP RSN_SELECTOR(0x00, 0x0f, 0xac, 3)
#endif
#define RSN_CIPHER_SUITE_CCMP RSN_SELECTOR(0x00, 0x0f, 0xac, 4)
#define RSN_CIPHER_SUITE_WEP104 RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_CIPHER_SUITE_AES_128_CMAC RSN_SELECTOR(0x00, 0x0f, 0xac, 6)

/* EAPOL-Key Key Data Encapsulation
 * GroupKey and PeerKey require encryption, otherwise, encryption is optional.
 */
#define RSN_KEY_DATA_GROUPKEY RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#if 0
#define RSN_KEY_DATA_STAKEY RSN_SELECTOR(0x00, 0x0f, 0xac, 2)
#endif
#define RSN_KEY_DATA_MAC_ADDR RSN_SELECTOR(0x00, 0x0f, 0xac, 3)
#define RSN_KEY_DATA_PMKID RSN_SELECTOR(0x00, 0x0f, 0xac, 4)

#define RSN_KEY_DATA_SMK RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_KEY_DATA_NONCE RSN_SELECTOR(0x00, 0x0f, 0xac, 6)
#define RSN_KEY_DATA_LIFETIME RSN_SELECTOR(0x00, 0x0f, 0xac, 7)
#define RSN_KEY_DATA_ERROR RSN_SELECTOR(0x00, 0x0f, 0xac, 8)

#define RSN_KEY_DATA_IGTK RSN_SELECTOR(0x00, 0x0f, 0xac, 9)

#define WPA_OUI_TYPE RSN_SELECTOR(0x00, 0x50, 0xf2, 1)

#define RSN_SELECTOR_PUT(a, val) WPA_PUT_BE32((u8 *) (a), (val))
#define RSN_SELECTOR_GET(a) WPA_GET_BE32((const u8 *) (a))

#define RSN_NUM_REPLAY_COUNTERS_1 0
#define RSN_NUM_REPLAY_COUNTERS_2 1
#define RSN_NUM_REPLAY_COUNTERS_4 2
#define RSN_NUM_REPLAY_COUNTERS_16 3


#define WPA_IGTK_LEN 16 /* CONFIG_IEEE80211W */


/* IEEE 802.11, 7.3.2.25.3 RSN Capabilities */
#define WPA_CAPABILITY_PREAUTH BIT(0)
#define WPA_CAPABILITY_NO_PAIRWISE BIT(1)
/* B2-B3: PTKSA Replay Counter */
/* B4-B5: GTKSA Replay Counter */
#define WPA_CAPABILITY_MFPR BIT(6)
#define WPA_CAPABILITY_MFPC BIT(7)
#define WPA_CAPABILITY_PEERKEY_ENABLED BIT(9)


/* IEEE 802.11r */
#define MOBILITY_DOMAIN_ID_LEN 2
#define FT_R0KH_ID_MAX_LEN 48
#define FT_R1KH_ID_LEN 6
#define WPA_PMK_NAME_LEN 16


/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
#define WPA_KEY_INFO_TYPE_MASK ((u16) (BIT(0) | BIT(1) | BIT(2)))
#define WPA_KEY_INFO_TYPE_HMAC_MD5_RC4 BIT(0)
#define WPA_KEY_INFO_TYPE_HMAC_SHA1_AES BIT(1)
#define WPA_KEY_INFO_TYPE_AES_128_CMAC 3
#define WPA_KEY_INFO_KEY_TYPE BIT(3) /* 1 = Pairwise, 0 = Group key */
/* bit4..5 is used in WPA, but is reserved in IEEE 802.11i/RSN */
#define WPA_KEY_INFO_KEY_INDEX_MASK (BIT(4) | BIT(5))
#define WPA_KEY_INFO_KEY_INDEX_SHIFT 4
#define WPA_KEY_INFO_INSTALL BIT(6) /* pairwise */
#define WPA_KEY_INFO_TXRX BIT(6) /* group */
#define WPA_KEY_INFO_ACK BIT(7)
#define WPA_KEY_INFO_MIC BIT(8)
#define WPA_KEY_INFO_SECURE BIT(9)
#define WPA_KEY_INFO_ERROR BIT(10)
#define WPA_KEY_INFO_REQUEST BIT(11)
#define WPA_KEY_INFO_ENCR_KEY_DATA BIT(12) /* IEEE 802.11i/RSN only */
#define WPA_KEY_INFO_SMK_MESSAGE BIT(13)



/* WPA IE version 1
 * 00-50-f2:1 (OUI:OUI type)
 * 0x01 0x00 (version; little endian)
 * (all following fields are optional:)
 * Group Suite Selector (4 octets) (default: TKIP)
 * Pairwise Suite Count (2 octets, little endian) (default: 1)
 * Pairwise Suite List (4 * n octets) (default: TKIP)
 * Authenticated Key Management Suite Count (2 octets, little endian)
 *    (default: 1)
 * Authenticated Key Management Suite List (4 * n octets)
 *    (default: unspec 802.1X)
 * WPA Capabilities (2 octets, little endian) (default: 0)
 */

struct wpa_ie_hdr {
    u8 elem_id;
    u8 len;
    u8 oui[4]; /* 24-bit OUI followed by 8-bit OUI type */
    u8 version[2]; /* little endian */
} STRUCT_PACKED;


/* 1/4: PMKID
 * 2/4: RSN IE
 * 3/4: one or two RSN IEs + GTK IE (encrypted)
 * 4/4: empty
 * 1/2: GTK IE (encrypted)
 * 2/2: empty
 */

/* RSN IE version 1
 * 0x01 0x00 (version; little endian)
 * (all following fields are optional:)
 * Group Suite Selector (4 octets) (default: CCMP)
 * Pairwise Suite Count (2 octets, little endian) (default: 1)
 * Pairwise Suite List (4 * n octets) (default: CCMP)
 * Authenticated Key Management Suite Count (2 octets, little endian)
 *    (default: 1)
 * Authenticated Key Management Suite List (4 * n octets)
 *    (default: unspec 802.1X)
 * RSN Capabilities (2 octets, little endian) (default: 0)
 * PMKID Count (2 octets) (default: 0)
 * PMKID List (16 * n octets)
 * Management Group Cipher Suite (4 octets) (default: AES-128-CMAC)
 */

struct rsn_ie_hdr {
    u8 elem_id; /* WLAN_EID_RSN */
    u8 len;
    u8 version[2]; /* little endian */
} STRUCT_PACKED;


enum {
    STK_MUI_4WAY_STA_AP = 1,
    STK_MUI_4WAY_STAT_STA = 2,
    STK_MUI_GTK = 3,
    STK_MUI_SMK = 4
};

enum {
    STK_ERR_STA_NR = 1,
    STK_ERR_STA_NRSN = 2,
    STK_ERR_CPHR_NS = 3,
    STK_ERR_NO_STSL = 4
};

struct rsn_error_kde {
    u16 mui;
    u16 error_type;
} STRUCT_PACKED;

struct wpa_igtk_kde {
    u8 keyid[2];
    u8 pn[6];
    u8 igtk[WPA_IGTK_LEN];
} STRUCT_PACKED;

// CONFIG_IEEE80211R
struct rsn_mdie {
    u8 mobility_domain[MOBILITY_DOMAIN_ID_LEN];
    u8 ft_capab;
} STRUCT_PACKED;

#define RSN_FT_CAPAB_FT_OVER_DS BIT(0)
#define RSN_FT_CAPAB_FT_RESOURCE_REQ_SUPP BIT(1)

struct rsn_ftie {
    u8 mic_control[2];
    u8 mic[16];
    u8 anonce[WPA_NONCE_LEN];
    u8 snonce[WPA_NONCE_LEN];
    /* followed by optional parameters */
} STRUCT_PACKED;

#define FTIE_SUBELEM_R1KH_ID 1
#define FTIE_SUBELEM_GTK 2
#define FTIE_SUBELEM_R0KH_ID 3
#define FTIE_SUBELEM_IGTK 4

/* CONFIG_IEEE80211R */

struct wpa_ie_data {
    int proto;
    int pairwise_cipher;
    int group_cipher;
    int key_mgmt;
    int capabilities;
    size_t num_pmkid;
    const u8 *pmkid;
    int mgmt_group_cipher;
};

/* Macros for handling unaligned memory accesses */

#define WPA_GET_BE16(a) ((u16) (((a)[0] << 8) | (a)[1]))
#define WPA_PUT_BE16(a, val)            \
    do {                    \
        (a)[0] = ((u16) (val)) >> 8;    \
        (a)[1] = ((u16) (val)) & 0xff;  \
    } while (0)

#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WPA_PUT_LE16(a, val)            \
    do {                    \
        (a)[1] = ((u16) (val)) >> 8;    \
        (a)[0] = ((u16) (val)) & 0xff;  \
    } while (0)

#define WPA_GET_BE24(a) ((((u32) (a)[0]) << 16) | (((u32) (a)[1]) << 8) | \
             ((u32) (a)[2]))
#define WPA_PUT_BE24(a, val)                    \
    do {                            \
        (a)[0] = (u8) ((((u32) (val)) >> 16) & 0xff);   \
        (a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);    \
        (a)[2] = (u8) (((u32) (val)) & 0xff);       \
    } while (0)

#define WPA_GET_BE32(a) ((((u32) (a)[0]) << 24) | (((u32) (a)[1]) << 16) | \
             (((u32) (a)[2]) << 8) | ((u32) (a)[3]))
#define WPA_PUT_BE32(a, val)                    \
    do {                            \
        (a)[0] = (u8) ((((u32) (val)) >> 24) & 0xff);   \
        (a)[1] = (u8) ((((u32) (val)) >> 16) & 0xff);   \
        (a)[2] = (u8) ((((u32) (val)) >> 8) & 0xff);    \
        (a)[3] = (u8) (((u32) (val)) & 0xff);       \
    } while (0)

#define WPA_GET_LE32(a) ((((u32) (a)[3]) << 24) | (((u32) (a)[2]) << 16) | \
             (((u32) (a)[1]) << 8) | ((u32) (a)[0]))
#define WPA_PUT_LE32(a, val)                    \
    do {                            \
        (a)[3] = (u8) ((((u32) (val)) >> 24) & 0xff);   \
        (a)[2] = (u8) ((((u32) (val)) >> 16) & 0xff);   \
        (a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);    \
        (a)[0] = (u8) (((u32) (val)) & 0xff);       \
    } while (0)

#define WPA_GET_BE64(a) ((((u64) (a)[0]) << 56) | (((u64) (a)[1]) << 48) | \
             (((u64) (a)[2]) << 40) | (((u64) (a)[3]) << 32) | \
             (((u64) (a)[4]) << 24) | (((u64) (a)[5]) << 16) | \
             (((u64) (a)[6]) << 8) | ((u64) (a)[7]))
#define WPA_PUT_BE64(a, val)                \
    do {                        \
        (a)[0] = (u8) (((u64) (val)) >> 56);    \
        (a)[1] = (u8) (((u64) (val)) >> 48);    \
        (a)[2] = (u8) (((u64) (val)) >> 40);    \
        (a)[3] = (u8) (((u64) (val)) >> 32);    \
        (a)[4] = (u8) (((u64) (val)) >> 24);    \
        (a)[5] = (u8) (((u64) (val)) >> 16);    \
        (a)[6] = (u8) (((u64) (val)) >> 8); \
        (a)[7] = (u8) (((u64) (val)) & 0xff);   \
    } while (0)

#define WPA_GET_LE64(a) ((((u64) (a)[7]) << 56) | (((u64) (a)[6]) << 48) | \
             (((u64) (a)[5]) << 40) | (((u64) (a)[4]) << 32) | \
             (((u64) (a)[3]) << 24) | (((u64) (a)[2]) << 16) | \
             (((u64) (a)[1]) << 8) | ((u64) (a)[0]))


#endif /* IEEE802_11_H */
