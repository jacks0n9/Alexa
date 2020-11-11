/* ========================================================================== */
/**
 *  @file   ti/mas/afe/afe.h
 *
 *  @brief  The interface definitions for Audio Front End
 *
 *  ============================================================================
 */
/* --COPYRIGHT--,BSD
 * Copyright (c) 2008, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/** @defgroup AFE */

#ifndef __AFE_H__
#define __AFE_H__

/* -- RTSC Types Header -- */
#include <xdc/std.h>

#include "ti/mas/devnode/devnode.h"

#include "ti/mas/eco/eco.h"
#include "ti/mas/inc/ctrlMsg.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef FIX_COMPIL_WARNING
#define FIX_COMPIL_WARNING(a) (a) = (a)
#endif


/** @ingroup AFE */
/* @{ */
#define AFE_NUM_TX_CHANNELS  (2)  /* = Number of Tx slots to be used                 */
#define AFE_NUM_MIC_CH       (8)  /* = Number of Rx slots to be used                 */
#define AFE_NUM_MAX_CHANNELS (8)  /* = max(AFE_NUM_TX_CHANNELS, AFE_NUM_MIC_CH) */

/******************************************************************************
 * AFE_HwConfig
 *****************************************************************************/
/**
 *  @brief     Message structure to configure AFE startup parameters.
 *             Message ID in the header must be set to AFE_MSG_HW_CONFIG
 *             This control message mustbe called after AFE is created
 *             and before any other control message is sent to AFE.
 *             NOTE: this message must be called only once
 */
typedef struct AFE_HwConfig {
  UInt16  num_rx_timeslots;       /**< Number of timeslots per frame sync */
  UInt16  num_tx_timeslots;       /**< Number of timeslots per frame sync */
  UInt32  rx_sampling_frequency;  /**< Sampling frequency  */
  UInt32  tx_sampling_frequency;  /**< Sampling frequency  */
#define  AFE_SAMP_FREQ_8KHZ     8000
#define  AFE_SAMP_FREQ_16KHZ    16000
#define  AFE_SAMP_FREQ_32KHZ    32000
#define  AFE_SAMP_FREQ_441KHZ   44100
#define  AFE_SAMP_FREQ_48KHZ    48000
} AFE_HwConfig;

/***********************************************************************
 * Name: AFE_EcoSlotConfig
 ***********************************************************************/
/**
 *  @brief     Message structure to configure ECO algo (AEU/APU etc.) instances in AFE Stream
 *             Max Number of ECO algos are based upon internal support in AFE.
 *             Each slots can have one ECO algo placed. See user guide for
 *             more information of slotId and algId.
 */
#define AFE_MAX_ECO_SLOTS (1)

typedef struct AFE_EcoSlotConfig {
#define AFE_INVALID_ECO_HANDLE    0x0000
   ECO_Handle  ecoHandle;

   UInt16      algId;        /* This algo ID is usually valid for AEU type ECOs
                                which has many independent algos inside one ECO instance.
                                0x0100 is reserved for APU */
#define AFE_ALGID_APU         0x0100

} AFE_EcoSlotConfig;


#define AFE_MAX_PERF_STREAM_SIZE  (64)

typedef struct AFE_SlotPerfDesc {
   UInt16      algValid;     /* 0: No Perf Info Needed, 1: Perf Info Needed */

   UInt16      perfSize;     /* size of perf for this slot algo in bytes if algValid is 1 */

} AFE_SlotPerfDesc;

typedef struct AFE_PerfInfo {
   AFE_SlotPerfDesc  perfDesc[AFE_MAX_ECO_SLOTS];
   Int8              perfData[AFE_MAX_PERF_STREAM_SIZE];
} AFE_PerfInfo;

/***********************************************************************
 * Name: AFE_TdmChanConfig
 ***********************************************************************/
/**
 *  @brief     Message structure to configure TDM channels in an AFE Stream
 *             Each TDM channel has one playout and one capture timeslot.
 */

#define AFE_MAX_TDMCHAN_PER_STREAM  (AFE_NUM_MAX_CHANNELS)

typedef struct AFE_TdmChanConfig {
  UInt16 tdmChanId; /**< TDM Chan Id in the AFE, 0 based */
  Int16  captureTs; /**< TDM Timeslot number for capture, 0 based, -1 is invalid TS */
  Int16  playoutTs; /**< TDM Timeslot number for playout, 0 based, -1 os invalid TS */
} AFE_TdmChanConfig;

/***********************************************************************
 * Name: AFE_StreamConfig
 ***********************************************************************/
/**
 *  @brief     Message structure to configure parameters for an AFE
 *             stream including enable/disable.
 *             This control message must be
 *             sent after AFE_MSG_HW_CONFIG message is sent
 */
typedef struct AFE_StreamConfig {
   /**< Valid Stream ID */
   UInt16             streamId;

   /**< indicates which parameters are specified in this message*/
   UInt16             valid_params;
#define AFE_STREAM_CONFIG_VALID_ECO_CONFIG        0x0001   /**< ECO Config valid */
#define AFE_STREAM_CONFIG_VALID_TDMCHAN_CONFIG    0x0002   /**< TDM Chan Config valid */

   /**< Valid if valid_params & AFE_STREAM_CONFIG_VALID_ECO_CONFIG is true */
   AFE_EcoSlotConfig  ecoConfig[AFE_MAX_ECO_SLOTS];
   /**< Note: Slot 0 is reserved for APU */
   /*
     Mic     (Tx)  >---5---0---1---2--->
                                         Network Side
     Speaker (Rx)  <---6---0---4---3---<
   */

   /**< Valid if valid_params & AFE_STREAM_CONFIG_VALID_TDMCHAN_CONFIG is true */
   UInt16             tsActive; /**< If Inactive then the timeslots in tdmConfig[] array
                                     need to be closed, if active then they need
                                     to be opened for data transfer
                                     Note: Make sure that the requested TDM Chans and
                                     Timeslots are not used by other streams */
#define AFE_STREAM_CONFIG_TS_INACTIVE            0x0000
#define AFE_STREAM_CONFIG_TS_ACTIVE              0x0001
   UInt16             numTdmChans; /**< Number of TDM Channels valid in the tdmChanConfig array */
   AFE_TdmChanConfig  tdmChanConfig[AFE_MAX_TDMCHAN_PER_STREAM];
} AFE_StreamConfig;

/** Returned by AFE in for control or process call */
typedef enum
{
 AFE_STATUS_OK=0,                          /**< Control message status OK */
 AFE_STATUS_INVALID_MSG=256,               /**< Invalid AFE control message ID */
 AFE_STATUS_OPERATION_FAILED,              /**< Requested operation failed */
 AFE_STATUS_NOT_SUPPORTED,                 /**< Requested feature not supported */
 AFE_STATUS_INVALID_HW_CONFIG,             /**< Bad param(s) in HW_CONFIG msg */
 AFE_STATUS_MAX_STREAM_EXCEEDED,           /**< Too many channels requested */
 AFE_STATUS_INVALID_TDM_CONFIG,            /**< Bad param(s) in TDM_CONFIG msg */
 AFE_STATUS_TS_ALREADY_ENABLED,            /**< Timeslot already enabled */
 AFE_STATUS_NO_FREE_TS,                    /**< No free timeslots available */
 AFE_STATUS_INVALID_TS,                    /**< Invalid timeslot specified */
 AFE_STATUS_SAMPLING_RATE_UNSUPPORTED,     /**< Unsupported sampling rate */
 AFE_STATUS_AIC_INIT_FAILED,               /**< AIC init failed */
 AFE_STATUS_TDM_INIT_FAILED,               /**< TDM init failed */
 AFE_STATUS_TDM_START_FAILED,              /**< TDM start up failed */
 AFE_STATUS_INVALID_STREAM,                /**< Invalid channel ID */
 AFE_STATUS_INACTIVE_STREAM,               /**< Message sent to inactive channel */
 AFE_STATUS_INVALID_NUM_AEU_ALGOS,         /**< Number of Algos exceeds AFE_MAX_AEU_ALGOS */
 AFE_STATUS_PERFARRAY_SIZE_ERROR,          /**< Size of the Perf Stats buffer is zero */
 AFE_STATUS_NUM_TDMCHAN_ERROR,             /**< TDM Chan Number specified in Stream Config is invalid */
 AFE_STATUS_TDM_STOP_FAILED                /**< TDM stop failed */
} AFE_Return;

#define AFE_ECO_CONTROL_ERROR_BASE 0x1000  /**< The ECO_control call error has this base value */

/**
 *  @brief This params is used to get resource required to create an
 *         AFE instance and associated algorithms
 */
typedef struct AFE_CreateParams {
  Uint32       rx_sampling_rate; /**< Rx sampling rate handled by this AFE instance */
  Uint32       tx_sampling_rate; /**< Tx sampling rate handled by this AFE instance */
  Uint16       num_rx_tdmchans;  /**< Number of rx TDM Channels supported */
  Uint16       num_tx_tdmchans;  /**< Number of tx TDM Channels supported */
  Uint16       frame_size_ms;    /**< Frame size handled in ms */
  Uint16       num_chans;        /**< Number of channels supported */
  Uint16       num_streams;      /**< Number of streams supported */
} AFE_CreateParams;

/**
 * @brief  APU module's Parameters needed during instance creation
 */
typedef struct AFE_Params {
    IDEVNODE_Params      params; /**< First Element of the derived class */
    AFE_CreateParams     createParams; /**< This AFE specific create params */
} AFE_Params;

/******************************************************************************
 * AFE_ControlParams
 *****************************************************************************/
/**
 * @brief  AFE module's Control Parameters
 */
typedef struct AFE_ControlParams {
    Uint16                controlId;
/**< AFE control IDs */
#define AFE_MSG_HW_CONFIG                        ( 0)
#define AFE_MSG_STREAM_CONFIG                    ( 1)
#define AFE_MSG_SHUTDOWN                         ( 3)
#define AFE_MSG_LAB                              ( 0x1000)
    union
    {
      AFE_HwConfig       hwConfig;
      AFE_StreamConfig   streamConfig;
      tCtrlMsg           lab;
    } ctrl;
} AFE_ControlParams;

/******************************************************************************
 * AFE_Stats
 *****************************************************************************/
/**
 *  @brief     Message structure to return statistics of the AFE instance
 *
 */
typedef struct AFE_Stats {
  Uint32  frames;          /* Number of frames serviced */
  Uint32  lastIsr;         /* time at which last ISR occured */
  Uint32  lastdtIsr;       /* delta time after which the last ISR occured wrt its previous ISR */
  Uint32  dtIsrMax;        /* max delta time between 2 ISRs */
  Uint32  dtIsrMin;        /* min delta time between 2 ISRs */
  Uint32  dtIsrErrP;       /* Number of times delta ISR Time was above +10% variation */
  Uint32  dtIsrErrN;       /* Number of times delta ISR Time was below -10% variation */
} AFE_Stats;

/**
 * @brief  Union of APU module's Response Parameters, such as status.
 */
typedef struct AFE_ResponseParams {
    Uint16                responseId;
    union
    {
      AFE_Stats       stats;
      tCtrlMsg        lab;
    } resp;
} AFE_ResponseParams;

/**
 *  @brief     Enumeration type defining various stream usage modes
 */
typedef enum
{
  AFE_STREAM_USAGE_MONO = 0 /**< Stream usage scenario with a mono playout and a mono capture channel
                                  and multiple channels  of debug data */
} AFE_StreamUsage;


/************************************************************************
 * Definitions for stream usage scenario: MONO
 ************************************************************************/

/**<
               APU TAP Points
               -------------------
      Tx In >-|                   |-> Mono Capture
Mic           |        TX         |
              |                   |
               -------------------
TDM Side                               Network Side
               -------------------
     Rx Out <-|                   |-< Mono Playout
Speaker       |        RX         |
              |                   |
               -------------------
*/

/**
 *  @brief   This enumeration defines the channel buffer map for the MONO stream
 *           usage mode
 *
 */
typedef enum
{
  AFE_STREAM_USAGE_MONO_PLAYOUT = 0,   /**< Playout PCM data to speaker.*/
  AFE_STREAM_USAGE_MONO_CAPTURE,       /**< Captured PCM data from microphone. */

  AFE_STREAM_USAGE_MONO_NUM_CHANNELS   /**< Number of voice/debug data channels supported */

} AFE_STREAM_USAGE_MONO_CHANNEL_MAP;

#define PCM_BUFSIZE(sample_freq, frame_size)   ((((2*frame_size)*(sample_freq))/1000))      /*in bytes */

/* @} */ /* ingroup */


/**********************************************************************
 *      MESSAGE   FROM   DSP   DEFINITION   AND   HANDLER
 *********************************************************************/
#ifdef AFE_H_NEED_MSGQ
typedef struct {
    MSGQ_MsgHeader  header;
    tCtrlMsg        labmsg;
} tMsgFromDsp;

/* Application message size */
#define APPMSGSIZE (DSPLINK_ALIGN (sizeof (tMsgFromDsp), DSPLINK_BUF_ALIGN))
#define APPMSGSIGNATURE 0x1372

#endif




#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* __AFE_H__ */

