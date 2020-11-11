/* ========================================================================== */
/**
 *  @file   tdmloc.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/c64P/Leo/tdmdma/tdmloc.h
 *
 *  @brief  Header file containing local TDM data structures and APIs
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __TDMLOC_H__
#define __TDMLOC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- TDM/DMA Headers -- */
#include "src/c64P/DM3730/tdm/tdm.h"

/* ========================================================================== */
/*                         MACRO DEFINITIONS                                  */
/* ========================================================================== */

/**
 *  @def  CSL_TDM_MAX_TIMESLOTS
 *        The maximum number of timeslots a McBSP can have
 */
//#define CSL_TDM_MAX_TIMESLOTS  32

/*****************************************************************************
 * DEFINITION: McBSP restart status. Because a frame sync can occur one or
 * two cycles after the McBSP is taken out of resync the DMA can get out of
 * sync. This is because the frame sync tells the McBSP to send out data, but
 * the DSP has not yet gotten any data from the DMA. So the McBSP sends out
 * whatever was in its tx buffer, and what was supposed to go on the 1st
 * timeslot goes on the second, and the timeslots will be shifted from then on.
 * By restarting the McBSP after the 1st frame sync this is avoided.
 *****************************************************************************/
typedef enum {
   HAL_TDM_MCBSP_RUN,
   HAL_TDM_MCBSP_RESTART
} halTdmRestart_t;

/* ============ halDmaInfo_t ========== */
/**
 *  @brief  DMA information structure
 *
 */
typedef struct halDmaInfo_s {
  tuint *rxAddr;      /**< single address to buffer - buffer base address */
  tuint *rxReldAddr;  /**< single address to buffer - buffer base address */
  tuint *txAddr;      /**< buffer to single address - buffer base address */
  tuint *txReldAddr;  /**< buffer to single address - buffer base address */
  tuint rxSize;       /**< Number of words in rx buffer                   */
  tuint txSize;       /**< Number of words in tx buffer                   */
} halDmaInfo_t;

/* ============ halSportInfo_t ========== */
/**
 *  @brief  Structure used to maintain serial port information
 *
 */
typedef struct halSportInfo_s {
  tint    tsSize;         /**< Number of bits per timeslot        */
  tint    rxTsPerFrame;   /**< Number of timeslots per frame sync */
  tint    txTsPerFrame;   /**< Number of timeslots per frame sync */
} halSportInfo_t;

/*****************************************************************************
 * Definition: Because the driver uses the McBSP in symmetric mode, there
 *             will be channels receiving data that are not tx active. This
 *             allows channel enable without DMA re-sizing.
 *****************************************************************************/
typedef enum {
  HAL_TS_ACTIVE,        /**< Timeslot is sending and receiving              */
  HAL_TS_DUMMY          /**< Timeslot is an inactive "placeholder" timeslot */
} halTsStatus_t;

/* ============ halTdmaBuffer_t ========== */
/**
 *  @brief  HAL TDM buffers used to feed DMA buffers
 */
typedef struct halTdmaBuffer_s {
   tuint  *base;      /**< Base address of buffer */
   tuint  size;       /**< Buffer size, words     */
   tuint  index;      /**< Current buffer index   */
   tuint  frameSize;  /**< Size of frame          */
   tuint  frameIndex; /**< Frame Index            */
} halTdmaBuffer_t;

/* ============ halTsArray_t ========== */
/**
 *  @brief  The timeslot array element definition. Only symmetrical rx/tx
 *          timeslots are supported.
 *
 */
typedef struct halTsArray_s {
  tint            ts;             /**< Timeslot number indexed from 0 */
  halTsStatus_t   status;         /**< Timeslot status */
  tuint           memBase;        /**< DMA channel base index from DMA base address */
  tint            dmaIndex;       /**< DMA buffer index for this timeslot */
  void           *rxContext;      /**< Application rx channel identifier */
  void           *txContext;      /**< Application tx channel identifier */
  tint            phase;          /**< Initial subframe "phase" for TDM rx data */
  halTdmaBuffer_t tx;             /**< Tx to-TDM data buffers */
  halTdmaBuffer_t rx;             /**< Rx from-TDM data buffers */
  void          (*txCallout)(void *, void *, linSample **, tuint, tuint, tuint, tuint); /**< Functions to call when data is available  */
  void          (*rxCallout)(void *, void *, linSample **, tuint, tuint, tuint, tuint); /**< Functions to call when data is available  */
  tuint           bufferSync;     /**< Count that indicates how many times the buffer half syncronization required in halTdmDmaIsr */
} halTsArray_t;

/* ============ halTdmMcBsp_t ========== */
/**
 *  @brief  McBSP serial port structure. The 1st element of structure must
 *          identify the function of this port.
 */
typedef struct halTdmMcBsp_s {
  halSportFcn_t     function;       /**< Identifies structure as TDM control */
  tint              port;           /**< McBSP port number, indexed from 0 */
  halTdmStart_t     start;          /**< Identifies startup conditions */
  halTdmRestart_t   restart;        /**< Restart flag */
  tint              maxTs;          /**< Max number of timeslots operational at one time */
  tint              maxRxTs;        /**< Max number of rx timeslots operational at one time */
  tint              maxTxTs;        /**< Max number of tx timeslots operational at one time */
  tint              rxSubFrame;     /**< Size, in samples, of a rx subframe */
  tint              txSubFrame;     /**< Size, in samples, of a tx subframe */
  tuint             rxSamplingFreq; /**< Rx sampling frequency */
  tuint             txSamplingFreq; /**< Tx sampling frequency */
  tint              numActiveRxTs;  /**< Number of timeslots currently active */
  tint              numActiveTxTs;  /**< Number of timeslots currently active */
  tuint             silence;        /**< Silence Pattern */
  tuint             timeStamp;      /**< Current TDM rx timestamp */
  halDmaInfo_t      dma;            /**< DMA Configuration structure */
  halSportInfo_t    sport;          /**< McBSP Configuration structure */
  void            (*subFrameCallout) (void *handle, tuint ts, Uint32 ticks);/**< Function to call at every subframe */
  void             *handle;         /**< Handle to call with */
  halTsArray_t     *tsArray;        /**< Timeslot array with maxTs elements  */
} halTdmMcBsp_t;

/* ============ halTdmIo_t ========== */
/**
 *  @brief  McBSP structure when used as a GPIO
 */
typedef struct {
  halSportFcn_t function;  /**< Identifies structure as GPIO control */
  tint          port;      /**< Port number, indexed from 0 */
} halTdmIo_t;

/* ============ halTdmIsr() ========== */
/**
 *  @brief      DMA interrupt service routine
 *
 *              Sorts interleved data from DMA Ping/Pong buffer.
 *
 *  @param[in]  vmcbsp  Void pointer to halTdmMcBsp_t structure
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void halTdmIsr (void *vmcbsp);

/* ============ halStartupTdm() ========== */
/**
 *  @brief      Start the serial port
 *
 *              This routine is called from halinit.c file during HAL initialization.
 *              It is assumed that ASP and EDMA are in reset. This routine enables
 *              ASP and EDMA and ASP is taken out of reset
 *
 *  @param[in]  tdm    Pointer to halTdmMcbsp_t structure
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
halReturn_t halStartupTdm (halTdmMcBsp_t *tdm);

Void mcbspstop(Void);
UInt16 mcbspinit(halTdmMcBsp_t *tdm);
Void mcbspdelete(Void);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* __TDMLOC_H__ */
