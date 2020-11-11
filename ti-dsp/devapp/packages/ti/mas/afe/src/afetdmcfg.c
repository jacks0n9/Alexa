/* ========================================================================== */
/**
 *  @file   afetdmcfg.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afetdmcfg.c
 *
 *  @brief  This file implements processing of TDM_CONFIG message
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#include <xdc/std.h>

/* -- Standard C Headers -- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -- Telogy Types Header -- */
#include <ti/mas/types/types.h>

/* -- AFE Headers -- */
#include <afe.h>
#include <src/afedbg.h>
#include <src/afeloc.h>
#include <src/afeswcfg.h>
#include <src/afedevloc.h>

/* ============ AFE_halErrorCheck() ========== */
/**
 *  @brief      Check the HAL return value and take appropriate action
 *
 *              It generates an exception for unrecoverable HAL errors.
 *              It sends an ERROR_INDICATION message for other invalid
 *              configuration
 *
 *  @param[in]  obj         Pointer to AFE_Object structure
 *
 *  @param[in]  ret         HAL return code
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
tuint AFE_halErrorCheck (AFE_Object *obj, halReturn_t ret)
{
  tuint ecode;

  switch (ret) {
    case HAL_TDM_INVALID_HAL_TDM_ID:
      ecode = AFE_STATUS_INVALID_TDM_CONFIG;
      break;

    case HAL_TDM_INVALID_TDM_ON_IO:
      ecode = AFE_STATUS_INVALID_TDM_CONFIG;
      break;

    case HAL_TDM_INVALID_TS:
      ecode = AFE_STATUS_INVALID_TS;
      break;

    case HAL_TDM_INVALID_TDM_CONFIG:
      ecode = AFE_STATUS_INVALID_TDM_CONFIG;
      break;

    case HAL_TDM_TS_ALREADY_ENABLED:
      ecode = AFE_STATUS_TS_ALREADY_ENABLED;
      break;

    case HAL_TDM_NO_FREE_TS:
      ecode = AFE_STATUS_NO_FREE_TS;
      break;

    default:
      ecode = AFE_STATUS_INVALID_TDM_CONFIG;
      break;
  }

  return (ecode);
}

/* ============ AFE_processFromTdm() ========== */
/**
 *  @brief      Update and store PCM sample buffer pointer from TDM
 *
 *              Stores the PCM sample buffer pointer to AFE's MSP context for
 *              processing of the PCM data, and update this pointer such that
 *              in circular fashion
 *
 *  @param[in]  handle      Pointer to AFE_Object structure
 *
 *  @param[in]  afeChanInst Pointer to AFE channel instance
 *
 *  @param[in,out] fromTdm  Pointer to pointer to PCM frame received from TDM
 *
 *  @param[in]  timestamp   timestamp - not used
 *
 *  @param[in]  frame_length  length of the PCM frame received
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
/* ACH: This function is called from the TDM ISR once a buffer is ready */
void AFE_processFromTdm (
  void          *handle,
  void          *afeChanInst,
  linSample     **fromTdm,
  tuint         timestamp,
  tuint         frame_length,
  tuint         setSemaphore,
  tuint         unused
)
{
  AFE_ChanInst    *inst;
  linSample       *fromTdmStart;
  linSample       *fromTdmEnd;

      FIX_COMPIL_WARNING(unused);

  inst = (AFE_ChanInst *)afeChanInst;
  fromTdmStart = inst->toMspBuf;
  fromTdmEnd = inst->toMspBuf + inst->rx_frame_length*2;
  timestamp = timestamp;  /* Done to avoid compiler warning */

  /* Store the PCM buffer pointer received from TDM for processing */
  inst->toMsp = (Int16 *)*fromTdm;
  inst->numSamples = frame_length;

  /* Update the buffer pointer for HAL to operate on in circular fashion */
  *fromTdm += frame_length;
  if (*fromTdm >= fromTdmEnd) {
    *fromTdm = fromTdmStart;
  }
}

/* ============ AFE_processToTdm() ========== */
/**
 *  @brief      Update and store the PCM sample buffer to be consumed by TDM
 *
 *              Stores the PCM sample buffer pointer to be filled by MSP
 *              to AFE's MSP context and update the pointer in circular fashion
 *
 *  @param[in]  handle        Pointer to AFE_Object structure
 *
 *  @param[in]  afeChanInst   Pointer to AFE channel instance
 *
 *  @param[in,out]  toTdm     Pointer to pointer to PCM frame send to TDM
 *
 *  @param[in]  timestamp     timestamp is not used
 *
 *  @param[in]  frame_length  length of the PCM frame received
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void AFE_processToTdm (
  void          *handle,
  void          *afeChanInst,
  linSample     **toTdm,
  tuint         timestamp,
  tuint         frame_length,
  tuint         setSemaphore,
  tuint         unused
)
{
  AFE_ChanInst    *inst;
  linSample       *toTdmStart;
  linSample       *toTdmEnd;
    FIX_COMPIL_WARNING(unused);

  /* Get local values */
  inst = (AFE_ChanInst *)afeChanInst;
  toTdmStart = inst->fromMspBuf;
  toTdmEnd = inst->fromMspBuf + inst->tx_frame_length*2;
  timestamp = timestamp;  /* Done to avoid compiler warning */

  /* Store the empty PCM buffer pointer for MSP to fill */
  inst->fromMsp = (Int16 *)*toTdm;

  /* Update the pointer in a circular fashion */
  *toTdm += frame_length;
  if (*toTdm >= toTdmEnd) {
    *toTdm = toTdmStart;
  }
}

/* ============ AFE_processStreamConfig() ========== */
/**
 *  @brief  Configures an AFE stream
 *
 *  @param[in]  obj         Pointer to AFE_Object
 *
 *  @param[in]  stream_config  Pointer to AFE_StreamConfig structure
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
UInt16 AFE_processStreamConfig (AFE_Object *obj, AFE_StreamConfig *stream_config)
{
  halReturn_t               retval;
  tuint                     i;
  tuint                     valid_params;
  AFE_StreamInst            *streamInst;
  AFE_ChanInst              *tdmChanInst;
  halTdmControl_t           halTdmControl;
  tint                     txTimeslot, rxTimeslot;

    /* validate stream ID */
    if (stream_config->streamId >= obj->maxNumberStreams) {
      return AFE_STATUS_INVALID_STREAM;
    }

    /* Get values in local variables */
    streamInst = &obj->afeStreamInst[stream_config->streamId];
    valid_params = stream_config->valid_params;

    if(valid_params & AFE_STREAM_CONFIG_VALID_ECO_CONFIG) {
      for(i = 0; i < AFE_MAX_ECO_SLOTS; i++) {
        streamInst->ecoInst[i] = stream_config->ecoConfig[i].ecoHandle;
        streamInst->ecoAlgId[i] = stream_config->ecoConfig[i].algId;
      }
    }

    if(valid_params & AFE_STREAM_CONFIG_VALID_TDMCHAN_CONFIG) {
      if(stream_config->numTdmChans > AFE_MAX_TDMCHAN_PER_STREAM) {
        return AFE_STATUS_NUM_TDMCHAN_ERROR;
      }

      streamInst->numPlayoutTdmChans = 0;
      streamInst->numCaptureTdmChans = 0;
      streamInst->rx_frame_length = (tuint)(((tulong)(obj->frameSize) * (obj->rxSampleRate))/1000);
      streamInst->tx_frame_length = (tuint)(((tulong)(obj->frameSize) * (obj->txSampleRate))/1000);

      for(i = 0; i < stream_config->numTdmChans; i++) {
        txTimeslot = stream_config->tdmChanConfig[i].playoutTs;
        rxTimeslot = stream_config->tdmChanConfig[i].captureTs;
        if(txTimeslot != -1) {
          streamInst->playoutTdmChanId[streamInst->numPlayoutTdmChans] = stream_config->tdmChanConfig[i].tdmChanId;
          streamInst->numPlayoutTdmChans++;
        }
        else {
          txTimeslot = rxTimeslot; //Since present hal doesn't accept only capture case
        }

        if(rxTimeslot != -1) {
          streamInst->captureTdmChanId[streamInst->numCaptureTdmChans] = stream_config->tdmChanConfig[i].tdmChanId;
          streamInst->numCaptureTdmChans++;
        }
        else {
          rxTimeslot = txTimeslot; //Since present hal doesn't accept only playout case
        }

        tdmChanInst = &obj->afeChanInst[stream_config->tdmChanConfig[i].tdmChanId];
        /* Configure frame_length here */
        tdmChanInst->rx_frame_length = (tuint)(((tulong)(obj->frameSize) * (obj->rxSampleRate))/1000);
        tdmChanInst->tx_frame_length = (tuint)(((tulong)(obj->frameSize) * (obj->txSampleRate))/1000);

        /* Fill out halTdmControl common information */
        halTdmControl.mode        = HAL_TDM_TIMESLOT_CONFIG;
        halTdmControl.phase       = 0;
        halTdmControl.txBuffer    = tdmChanInst->fromMspBuf;
        halTdmControl.rxBuffer    = tdmChanInst->toMspBuf;
        halTdmControl.txCallout   = AFE_processToTdm;
        halTdmControl.rxCallout   = AFE_processFromTdm;
        halTdmControl.rxFrameSize = tdmChanInst->rx_frame_length;
        halTdmControl.txFrameSize = tdmChanInst->tx_frame_length;
        halTdmControl.rxBufSize   = tdmChanInst->rx_frame_length;
        halTdmControl.txBufSize   = tdmChanInst->tx_frame_length;
        halTdmControl.compand     = HAL_BIT_EXACT;
        halTdmControl.initVal     = AFE_LINEAR_SILENCE;
        halTdmControl.txContext   = tdmChanInst;
        halTdmControl.rxContext   = tdmChanInst;

        if (stream_config->tsActive == AFE_STREAM_CONFIG_TS_ACTIVE) {
          /* Open the requested timeslot */
          halTdmControl.txTimeslot    = txTimeslot;
          halTdmControl.rxTimeslot    = rxTimeslot;
          halTdmControl.toTdmEnable   = TRUE;
          halTdmControl.fromTdmEnable = TRUE;

          /* Enable timeslot through halTdmControl() */
          retval = (*obj->halApi->halTdmControl) (obj->halIdPcm, &halTdmControl);
          if (retval == HAL_SUCCESS) {
            /* Modify to reflect the channel state */
            tdmChanInst->channel_rx_timeslot = rxTimeslot;
            tdmChanInst->channel_tx_timeslot = txTimeslot;
            tdmChanInst->fromMsp = tdmChanInst->fromMspBuf + tdmChanInst->tx_frame_length;
            tdmChanInst->toMsp = tdmChanInst->toMspBuf;
            obj->numActiveChannels++;
          }
          else {
            return AFE_halErrorCheck (obj, retval);
          }
        }
        else {
          /* Close the current timeslot for this channel */
          halTdmControl.txTimeslot    = txTimeslot;
          halTdmControl.rxTimeslot    = rxTimeslot;
          halTdmControl.toTdmEnable   = FALSE;
          halTdmControl.fromTdmEnable = FALSE;

          /* Disable timeslot through halTdmControl() */
          retval = (*obj->halApi->halTdmControl) (obj->halIdPcm, &halTdmControl);
          if (retval == HAL_SUCCESS) {
            /* Modify to reflect the channel state */
            tdmChanInst->channel_rx_timeslot = -1;
            tdmChanInst->channel_tx_timeslot = -1;
            obj->numActiveChannels--;
            if(obj->numActiveChannels == 0) {
              obj->resync = 1; /* so that next time first issue will do a resync */
            }
          }
          else {
            return AFE_halErrorCheck (obj, retval);
          }
        }
      }

      streamInst->state = AFE_STREAM_ACTIVE;
    }

    return (AFE_STATUS_OK);
}

Void AFE_resync (AFE_Object *obj, Uint32 timeout)
{
  halTdmControl_t  halTdmControl;
  AFE_ChanInst     *tdmChanInst;
  int              i;

    if(obj->resync) {

      HWI_disable();

      halTdmControl.mode        = HAL_TDM_PHASE_RESET;
      halTdmControl.phase       = 0;
      /* reset phase to 0 */
      (*obj->halApi->halTdmControl) (obj->halIdPcm, &halTdmControl);

      for (i = 0; i < obj->maxNumberChannels; i++) {
        tdmChanInst = &obj->afeChanInst[i];

        tdmChanInst->fromMsp = tdmChanInst->fromMspBuf + tdmChanInst->tx_frame_length;
        tdmChanInst->toMsp = tdmChanInst->toMspBuf;

        if(tdmChanInst->channel_tx_timeslot != -1) {
          halTdmControl.mode       = HAL_TDM_PHASE_CONFIG;
          halTdmControl.phase      = 0;
          halTdmControl.txTimeslot = tdmChanInst->channel_tx_timeslot;
          halTdmControl.rxTimeslot = tdmChanInst->channel_rx_timeslot;
          halTdmControl.txBuffer   = tdmChanInst->fromMspBuf;
          halTdmControl.rxBuffer   = tdmChanInst->toMspBuf;
          /* reset phase to 0 */
          (*obj->halApi->halTdmControl) (obj->halIdPcm, &halTdmControl);
        }
      }

      HWI_enable();

      obj->resync = 0;
    }
  /* Else do nothing. */
}
/* nothing past this point */
