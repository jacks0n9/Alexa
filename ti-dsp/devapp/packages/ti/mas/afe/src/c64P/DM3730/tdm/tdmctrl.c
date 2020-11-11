/* ========================================================================== */
/**
 *  @file   tdmdma.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/c64P/Leo/tdmdma/tdmdma.c
 *
 *  @brief  This file contains TDM DMA control routines
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/* -- Standard C Headers -- */

/* -- RTSC Types Header -- */
#include <xdc/std.h>

/* -- DSP/BIOS Headers -- */
#include <std.h>
#include <tsk.h>
#include <hwi.h>
#include <c64.h>
#include <log.h>

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- AFE Headers -- */
#include "src/afedbg.h"

/* -- HAL Headers -- */
#include "src/hal.h"
#include "src/halloc.h"

#include "src/c64P/DM3730/tdm/tdm.h"
#include "src/c64P/DM3730/tdm/tdmloc.h"

/* ============ trace ============= */
/**
 *  @name   trace
 *
 *  @brief  trace LOG_Obj used to do LOG_printf
 *
 */
extern LOG_Obj trace;

/* ============ hal_find_ts_match() ========== */
/**
 *  @brief      Search timeslot map for timeslot match
 *
 *              Searches the timeslot array to see if there is a dummy slot
 *              available using the specified timeslot. Returns a pointer to
 *              that element if found, NULL if no match is found
 *
 *  @param[in]  mcbsp     Pointer to halTdmMcbsp_t structure
 *
 *  @param[in]  ts        timeslot
 *
 *  @return     Pointer to the matching timeslot element in timeslot map
 *
 *  @sa         None
 *
 */
halTsArray_t *hal_find_ts_match (halTdmMcBsp_t *mcbsp, tint ts)
{
  tint          i;
  halTsArray_t  *tsa;

  for (i = 0; i < mcbsp->maxTs; i++)  {
    tsa = &(mcbsp->tsArray[i]);
    if (tsa->ts == ts) {
      return (tsa);
    }
  }
  return (NULL);
}

/* ============ hal_find_dummy_channel() ========== */
/**
 *  @brief      Return pointer to first dummy channel found
 *
 *              Searches for and returns the 1st dummy channel found. Returns
 *              NULL if no dummy channel is available
 *
 *  @param[in]  mcbsp     Pointer to halTdmMcbsp_t structure
 *
 *  @return     Pointer to the dummy channel
 *
 *  @sa         None
 *
 */
halTsArray_t *hal_find_dummy_channel (halTdmMcBsp_t *mcbsp)
{
  tint i;
  halTsArray_t *tsa;

  for (i = 0; i < mcbsp->maxTs; i++)  {
    tsa = &(mcbsp->tsArray[i]);
    if (tsa->status == HAL_TS_DUMMY) {
      return (tsa);
    }
  }

  return (NULL);
}

/* ============ hal_enable_channel() ========== */
/**
 *  @brief      Enable TX/RX timeslots
 *
 *              Enables the TX/RX timeslot pair. Currently only identical
 *              timeslots are supported.
 *
 *  @param[in]  mcbsp   Pointer to halTdmMcbsp_t structure
 *
 *  @param[in]  ctl     Pointer to halTdmControl_t structure
 *
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t hal_enable_channel (halTdmMcBsp_t *mcbsp, halTdmControl_t *ctl)
{
  halTsArray_t *tsa;

  /* Search for timeslot match in timeslot array */
  tsa = hal_find_ts_match (mcbsp, ctl->rxTimeslot);

  /* If the match is already an active timeslot return an error */
  if (tsa) {
    if (tsa->status == HAL_TS_ACTIVE) {
      return (HAL_TDM_TS_ALREADY_ENABLED);
    }
  }
  /* tsa == NULL */
  else {
    /*
     * If there is no dummy channel with timeslot match, use the first
     * dummy channel available
     */
    tsa = hal_find_dummy_channel (mcbsp);

    /* If no dummy channel is available then there is no channel available */
    if (tsa == NULL) {
      return (HAL_TDM_NO_FREE_TS);
    }
  }

  /* Provide critical section by disabling the task schedular in DSP/BIOS */
  SWI_disable();

  /* Added this to keep a track of which timeslot is alloted where */
  tsa->ts = ctl->rxTimeslot;

  /* Setup the timeslots */

  /*
   * Seting up of tsa->dmaIndex is not dynamic since we don't have more
   * channels than the number of phases in 1 frame. The dmaIndex is already
   * initialized during HAL intialization should remain the same. Hence it
   * is not configured here again.
   */

  tsa->rxContext = ctl->rxContext;
  tsa->txContext = ctl->txContext;

  tsa->tx.base = (tuint *)ctl->txBuffer;
  tsa->tx.size = ctl->txBufSize;
  tsa->tx.frameSize = ctl->txFrameSize;
  tsa->tx.frameIndex = 0;

  tsa->rx.base = (tuint *)ctl->rxBuffer;
  tsa->rx.size = ctl->rxBufSize;
  tsa->rx.frameSize = ctl->rxFrameSize;
  tsa->rx.frameIndex = 0;

  /* Callout Function initialization */
  tsa->txCallout = ctl->txCallout;
  tsa->rxCallout = ctl->rxCallout;
  tsa->status = HAL_TS_ACTIVE;

  if (tsa->ts < mcbsp->maxRxTs) {
    mcbsp->numActiveRxTs++;
  }

  if (tsa->ts < mcbsp->maxTxTs) {
    mcbsp->numActiveTxTs++;
  }

  /* End critical section by enabling the DSP/BIOS task schedular */
  SWI_enable();

  return (HAL_SUCCESS);
}

/* ============ hal_disable_channel() ========== */
/**
 *  @brief      Disable a timeslot
 *
 *              Disables specified timeslot
 *
 *  @param[in]  mcbsp   Pointer to halTdmMcbsp_t structure
 *
 *  @param[in]  ts      Timeslot to disable
 *
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t hal_disable_channel (halTdmMcBsp_t *mcbsp, tint ts)
{
  halTsArray_t *tsa;

  tsa = hal_find_ts_match (mcbsp, ts);

  if (tsa == NULL) {
    return  (HAL_TDM_INVALID_TS);
  }

  if (tsa->status == HAL_TS_DUMMY) {
    /* timeslot already disabled; return success */
    return (HAL_SUCCESS);
  }

  /* Critical section begin */
  /* TODO: Check if critical section is really required */
  SWI_disable();

  /* Change the channel status to dummy channel */
  tsa->status = HAL_TS_DUMMY;

  if (ts < mcbsp->maxRxTs) {
    mcbsp->numActiveRxTs--;
  }

  if (ts < mcbsp->maxTxTs) {
    mcbsp->numActiveTxTs--;
  }

  /* Critical section end */
  SWI_enable();

  return (HAL_SUCCESS);
}

/* ============ halTdmControl() ========== */
/**
 *  @brief      TDM control API
 *
 *              Configures TDM/DMA/Int system
 *
 *  @param[in]  halId   Hal identifier
 *
 *  @param[in]  ctl     Pointer to halTdmControl_t structure
 *
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t halTdmControl (tint halId, halTdmControl_t *ctl)
{
  halTdmMcBsp_t   *mcbsp;
  halReturn_t     ret;

  /* Check that the input port is valid */
  if (halId >= halContext->nTdmPorts) {
    return (HAL_TDM_INVALID_HAL_TDM_ID);
  }

  mcbsp = (halTdmMcBsp_t *)(halContext->tdmPorts[halId]);

  /* Check that the port is used in TDM mode */
  if (mcbsp->function != HAL_TDM_AS_TDM) {
    return (HAL_TDM_INVALID_TDM_ON_IO);
  }

  if (ctl->mode == HAL_TDM_TDM_CONFIG) {
    mcbsp->rxSubFrame         = ctl->tdmCtrl.rxSubFrameRate;
    mcbsp->txSubFrame         = ctl->tdmCtrl.txSubFrameRate;
    mcbsp->rxSamplingFreq     = ctl->tdmCtrl.rxSamplingFreq;
    mcbsp->txSamplingFreq     = ctl->tdmCtrl.txSamplingFreq;
    mcbsp->dma.rxSize         = ctl->tdmCtrl.rxSubFrameRate * mcbsp->maxTs;
    mcbsp->dma.txSize         = ctl->tdmCtrl.txSubFrameRate * mcbsp->maxTs;
    mcbsp->sport.tsSize       = ctl->tdmCtrl.wordSize;
    mcbsp->sport.rxTsPerFrame = ctl->tdmCtrl.rxClocksPerFrame;
    mcbsp->sport.txTsPerFrame = ctl->tdmCtrl.txClocksPerFrame;
    return (HAL_SUCCESS);
  }

  if (ctl->mode == HAL_TDM_TIMESLOT_CONFIG) {
    if ( (ctl->txTimeslot > AFE_NUM_MAX_CHANNELS) ||
         (ctl->rxTimeslot > AFE_NUM_MAX_CHANNELS) )
    {
      return (HAL_TDM_INVALID_TS);
    }

    /* Tempararily only allow same tx and rx timeslots */
    if (ctl->txTimeslot != ctl->rxTimeslot) {
      return (HAL_TDM_INVALID_TX_RX_TS_PAIR);
    }

    /* Check for straight enable */
    if (ctl->toTdmEnable == TRUE && ctl->fromTdmEnable == TRUE) {
      ret = hal_enable_channel (mcbsp, ctl);
    }
    else if (ctl->toTdmEnable == FALSE && ctl->fromTdmEnable == FALSE) {
      ret = hal_disable_channel (mcbsp, ctl->rxTimeslot);
    }
    else {
      ret = HAL_TDM_INVALID_TDM_CONFIG;
    }
    return (ret);
  }

  return (HAL_FAIL);
}
