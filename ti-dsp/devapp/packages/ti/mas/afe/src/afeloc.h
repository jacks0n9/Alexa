/* ========================================================================== */
/**
 *  @file   afeloc.h
 *
 *  path   /dsps_gtmas/ti/mas/afe/src/afeloc.h
 *
 *  @brief  AFE local definitions
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __AFELOC_H__
#define __AFELOC_H__

/* -- Standard C Headers -- */

/* -- DSP/BIOS Headers -- */
#include <std.h>
#include <sem.h>
#include <log.h>

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- Headers for other packages -- */
#include <ti/mas/devnode/idevnode.h>
#include <ti/mas/aeu/aeu.h>

/* -- AFE Local Headers -- */
#include "afe.h"
#include "src/hal.h"
#include "src/afeutl.h"
#include "frame.h"
#include "evtLog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @var    typedef int AFE_Error
 *
 *  @brief  AFE error code
 *
 *          Every AFE API returns error code of type @c AFE_Error @c. Return
 *          value of zero indicate success, positive value indicate error.
 *
 */
typedef int AFE_Error;

/**
 *  @def  AFE_SUCCESS
 *        AFE error code for successful execution
 */
#define AFE_SUCCESS                 (0)

/**
 *  @def  AFE_FAILURE
 *        AFE error code for general AFE error condition
 */
#define AFE_FAILURE                 (1)

/**
 *  @def  AFE_OUTOFMEMORY
 *        AFE error code for memory allocation failure
 */
#define AFE_OUTOFMEMORY             (2)

/**
 *  @def  AFE_NOTSUPPORTED
 *        AFE error code for not supported feature
 */
#define AFE_NOTSUPPORTED            (4)

/**
 *  @def  AFE_INVALIDPARAMS
 *        AFE error code for invalid supplied parameters
 */
#define AFE_INVALIDPARAMS           (5)

/**
 *  @def  AFE_MAX_TDM_PORTS
 *        Maximum number of TDM ports supported
 */
#define AFE_MAX_TDM_PORTS         (1)

/**
 *  @def  AFE_TDMWORDLENGTH
 *        Number of bits per each TDM timeslot word
 */
#define AFE_TDMWORDLENGTH       (16)

/**
 *  @def  AFE_TDMISRINTERVALMS
 *        The time interval in ms between two consecutive TDM ISR
 */
#define AFE_TDMISRINTERVALMS    (FRAME_LEN_IN_MS)

/**
 *  @var    typedef void  *AFE_Handle
 *
 *  @brief  Pointer to AFE Object
 *
 *          AFE_Handle represent the instance or object of AFE module
 *          AFE_Handle is the first parameter to all AFE APIs.
 *
 */
typedef void  *AFE_Handle;

/* ============ trace ============= */
/**
 *  @name   trace
 *
 *  @brief  trace LOG_Obj used to do LOG_printf
 *
 */
extern LOG_Obj trace;

/* Useful macros */

/**
 *  @def  AFE_FIRSTCHAN
 *        Logical channel number's always starts from 1
 */
#define AFE_FIRSTCHAN     (1)

/**
 *  @def  AFE_CHIDMASK
 *        Channel number within ID. Lower 8-bits are reserved for channel number
 */
#define AFE_CHIDMASK      (0xFF)

/**
 *  @def  AFE_MAKEID
 *        Creates ID out of module ID and channel number
 */
#define AFE_MAKEID(mid, chnum)    ( (tuint)(mid)<<8 | (tuint)(chnum) )

/**
 *  @def  AFE_EXCASSERT
 *        Calls AFE_exception based on the assertion
 */
#define AFE_EXCASSERT(expr, code, inst) {                     \
  (expr) ? ((void)0) : AFE_exception((inst)->ID, code);       \
}

/* PCM Silence Codes */
/**
 *  @def  AFE_ALAW_SILENCE
 *        PCM A-law silence pattern for AFE corresponding to -8.0F
 */
#define AFE_ALAW_SILENCE    (0x55)
/**
 *  @def  AFE_ALAW_SILENCE
 *        PCM Mu-law silence pattern for AFE corresponding to 0.0F
 */
#define AFE_MULAW_SILENCE   (0x7F)
/**
 *  @def  AFE_ALAW_SILENCE
 *        PCM linear silence pattern for AFE
 */
#define AFE_LINEAR_SILENCE  (0x00)

/* ============ AFE_ChanInst () ========== */
/**
 *  @brief  AFE instance structure
 *          Each channel maintains its own AFE instance structure
 */
typedef struct AFE_ChanInst {
  tuint       chanID;               /**< channel number */
  tuint       rx_frame_length;      /**< Rx voice frame size supported */
  tuint       tx_frame_length;      /**< Tx voice frame size supported */
  tint        channel_rx_timeslot;  /**< PCM receive timeslot associated with this channel */
  tint        channel_tx_timeslot;  /**< PCM transmit timeslot associated with this channel */
  linSample  *toMspBuf;             /**< Double buffered PCM samples */
  linSample  *fromMspBuf;           /**< Double buffered PCM samples */
  Int16      *toMsp;
  Int16      *fromMsp;
  Uint16      numSamples;
} AFE_ChanInst;

/* ============ AFE_StreamInst () ========== */
/**
 *  @brief  AFE Stream instance structure
 *          Each AFE stream maintains its own AFE stream instance structure
 */
typedef struct AFE_StreamInst {
  tuint         streamId;                          /**< Stream number */
  ECO_Handle    ecoInst[AFE_MAX_ECO_SLOTS];        /**< Array of AEU instance */
  tuint         ecoAlgId[AFE_MAX_ECO_SLOTS];       /**< Array of AEU Algo ID */
  tuint         playoutTdmChanId[AFE_MAX_TDMCHAN_PER_STREAM];
  tuint         captureTdmChanId[AFE_MAX_TDMCHAN_PER_STREAM];
  tuint         numCaptureTdmChans;
  tuint         numPlayoutTdmChans;
  linSample    *recvBuf;
  tuint         rx_frame_length;
  tuint         tx_frame_length;
  tuint         state;
#define AFE_STREAM_INACTIVE   0x0000
#define AFE_STREAM_ACTIVE     0x0001
} AFE_StreamInst;

/* ============ AFE_NotifyObject ========== */
/**
 *  @brief  Structure containing AFE's notification object
 *
 */
typedef struct AFE_NotifyObject {
  SEM_Obj  afeSemObj;     /**< AFE's notification mechanism */
} AFE_NotifyObject;


typedef struct AFE_IsrTimerStats {
  Uint32 tIsr;
  Uint32 dtIsr;
  Uint32 dtIsrMax; /* 2.5 msec in ticks */
  Uint32 dtIsrMin; /* 2.5 msec in ticks */
  Uint32 dtIsrErrP;
  Uint32 dtIsrErrN;
  Uint32 firstIsr;
  Uint32 isrCnt;
}AFE_IsrTimerStats;

/* ============ AFE_Object ========== */
/**
 *  @brief  AFE object structure
 *
 *
 *
 */
typedef struct AFE_Object {
  tuint             maxNumberStreams;     /**< Number of streams */
  AFE_StreamInst   *afeStreamInst;        /**< Pointer to AFE's Stream instances */
  tuint             maxNumberChannels;    /**< Maximum numbers of TDM channels supported */
  tuint             maxNumberRxChannels;  /**< Maximum numbers of TDM channels supported */
  tuint             maxNumberTxChannels;  /**< Maximum numbers of TDM channels supported */
  AFE_ChanInst     *afeChanInst;          /**< Pointer to AFE's TDM channel instances */
  tuint             numActiveChannels;    /**< Number of active TDM channels */
  tuint             maxRxTdmTimeslots;    /**< Maximum number of Rx TDM timeslots */
  tuint             maxTxTdmTimeslots;    /**< Maximum number of Tx TDM timeslots */
  tuint             frameSize;            /**< Voice frame size in ms */
  tuint             rxSamplesPerTdmIsr;   /**< Number of rx samples per TDM ISR */
  tuint             txSamplesPerTdmIsr;   /**< Number of tx samples per TDM ISR */
  tulong            maxRxSampleRate;      /**< Max Rx sampling rate */
  tulong            maxTxSampleRate;      /**< Max Tx sampling rate */
  tulong            rxSampleRate;         /**< Actual Rx Sampling Rate, 0 means not set */
  tulong            txSampleRate;         /**< Actual Rx Sampling Rate, 0 means not set */
  tuint             pcmFrameCounter;      /**< counts the ISRs in a PCM frame */
  tint              halIdPcm;             /**< Hal port ID for PCM */
  halApi_t         *halApi;               /**< Handle to the HalAPI structure */
  AFE_NotifyObject  afeNotifyObj;         /**< AFE's notification object */
  Void             *pAfeDevObj;           /**< Pointer to DAF object */
  linSample        *gatherTDMBuf;         /**< Pointer to buffer to be used for gathering data from multiple timeslots */
  tuint             resync;               /**< When this is set to 1 the fromMsp/toMsp buffers are synced to the next isr */
  AFE_IsrTimerStats isrStats;             /**< ISR timer statistics */
} AFE_Object;

/* ============ AFE_processStreamConfig() ========== */
/**
 *  @brief  Process CHANNEL_CONFIG message - configures a AFE channel
 *
 *  @param[in]  obj         Pointer to AFE_Object
 *
 *  @param[in]  msg         Pointer to AFE_ChannelConfig structure
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
UInt16 AFE_processStreamConfig (AFE_Object *obj, AFE_StreamConfig *msg);

/* ============ AFE_init ========== */
/**
 *  @brief      Initializes AFE_inst and AFE_context global data structures
 *
 *  @param[in]  afeObj    Pointer to AFE_Object
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
void AFE_init (AFE_Object *obj);

Void AFE_resync (AFE_Object *obj, Uint32 timeout);

UInt16 AFE_processHwConfig (AFE_Object *obj, AFE_HwConfig *msg);

UInt16 AFE_shutDown (AFE_Object *obj);

AFE_Error AFE_open (AFE_Handle handle);

Int AFE_getNumBuffers (Void);

Int AFE_getSizes(AFE_CreateParams *createParams, IALG_MemRec memTab[]);

AFE_Error AFE_gatherTdmChans (AFE_Object *obj, tMicFrame * pTx);

AFE_Error AFE_scatterStream (AFE_Object *obj, tSpeakFrame *pRx);

/* ============ AFE_create() ========== */
/**
 *  @brief      Creates AFE object or instance based on the configuration parameter
 *              This is first API to be called by application. This API allocates
 *              necessary memory to support a given number of voice channels. It
 *              also sets up AFE's context and channel instance structures.
 *
 *  @param[in]  params      Pointer to AFE configuration parameters
 *
 *  @param[out] handle      Pointer to AFE handle
 *
 *  @return     AFE error code
 *
 *  @sa         AFE_delete
 *
 */
AFE_Error AFE_create (AFE_Params *params, AFE_Handle *handle);

/* ============ AFE_delete() ========== */
/**
 *  @brief      Deletes AFE object or instance created earlier and frees
 *              all the resources consumed by AFE. It also shuts down
 *              peripherals managed by AFE and un-installs and disable
 *              TDM ISR
 *
 *  @param[in]  handle      Handle representing AFE object
 *
 *  @return     AFE error code
 *
 *  @sa         AFE_create
 *
 */
AFE_Error AFE_delete (AFE_Handle handle);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* __AFELOC_H__ */
