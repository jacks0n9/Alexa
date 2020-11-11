/* ========================================================================== */
/**
 *  @file   afe.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afe.c
 *
 *  @brief  Implements AFEDEV external APIs in afedevalg.h file
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2009
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/* -- Standard C Headers -- */
#include <xdc/std.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -- AFE Header -- */
#include <afe.h>

/* -- DSP/BIOS Headers -- */
#include <std.h>
#include <sem.h>
#include <log.h>

/* -- Types Header -- */

/* -- Other Pkges Headers -- */

/* -- AFE Local Headers -- */
#include <src/afeloc.h>
#include <src/afedbg.h>
#include <src/afeswcfg.h>
#include <src/afemem.h>
#include <src/ecodrv/afehal.h>
#include <src/afedevalg.h>
#include <ti/mas/util/ecoxdaisxlat.h>

#ifdef __cplusplus
extern "C" {
#endif

Int AFEDEV_getNumBuffers (Void)
{
  Int totalBufs = 0;

  totalBufs = AFEDEV_MAX_NUM_BUFFERS;

  return (totalBufs);
}

Int AFEDEV_getSizes(AFE_CreateParams *createParams, IALG_MemRec memTab[])
{
    UInt size, buf = 0;
    halNewConfig_t  cfg;
    halTdmInit_t    tdmInit[AFE_MAX_TDM_PORTS];
    tint            nbufs; /* Number of memory buffers hal requests    */
    halMemBuffer_t  *ecoBufsPtr; /* Information about HALs requested buffers */

    size  = sizeof(AFE_Object);
    memTab[buf].size = size;
    memTab[buf].alignment = 4;
    memTab[buf].space = IALG_EXTERNAL;
    memTab[buf].attrs = IALG_PERSIST;  
    buf++;

    /* Stream Instant */
    size = sizeof(AFE_StreamInst) * createParams->num_streams;
    memTab[buf].size = size;
    memTab[buf].alignment = 4;
    memTab[buf].space = IALG_EXTERNAL;
    memTab[buf].attrs = IALG_PERSIST;  
    buf++;

    /* Channel Instant */
    size = sizeof(AFE_ChanInst) * createParams->num_chans;
    memTab[buf].size = size;
    memTab[buf].alignment = 4;
    memTab[buf].space = IALG_EXTERNAL;
    memTab[buf].attrs = IALG_PERSIST;  
    buf++;

    /* Playout Buffer */
    size = createParams->num_tx_tdmchans * sizeof(linSample) * 2 * ((createParams->tx_sampling_rate * createParams->frame_size_ms)/1000);
    memTab[buf].size = size;
    memTab[buf].alignment = 4;
    memTab[buf].space = IALG_EXTERNAL;
    memTab[buf].attrs = IALG_PERSIST;  
    buf++;

    /* Capture Buffer */
    size = createParams->num_rx_tdmchans * sizeof(linSample) * 2 * ((createParams->rx_sampling_rate * createParams->frame_size_ms)/1000);
    memTab[buf].size = size;
    memTab[buf].alignment = 4;
    memTab[buf].space = IALG_EXTERNAL;
    memTab[buf].attrs = IALG_PERSIST;  
    buf++;

    /* Gather Buffer */
    size = createParams->num_rx_tdmchans * sizeof(linSample) * ((createParams->rx_sampling_rate * createParams->frame_size_ms)/1000);
    memTab[buf].size = size;
    memTab[buf].alignment = 4;
    memTab[buf].space = IALG_EXTERNAL;
    memTab[buf].attrs = IALG_PERSIST;  
    buf++;

    /* Get HAL memory requirements from configured parameters */
    cfg.nTdmPorts = AFE_MAX_TDM_PORTS;

    /* Initialize the configuration for 1st and only TDM port */
    tdmInit[0].fcn                  = HAL_TDM_AS_TDM;
    tdmInit[0].u.tdm.maxChannels    = createParams->num_chans;;
    tdmInit[0].u.tdm.maxRxChannels  = createParams->num_rx_tdmchans;
    tdmInit[0].u.tdm.maxTxChannels  = createParams->num_tx_tdmchans;
    tdmInit[0].u.tdm.wordSize       = (AFE_TDMWORDLENGTH);
    tdmInit[0].u.tdm.rxSubFrameRate = (tuint)((AFE_TDMISRINTERVALMS) * (createParams->rx_sampling_rate)/1000);
    tdmInit[0].u.tdm.txSubFrameRate = (tuint)((AFE_TDMISRINTERVALMS) * (createParams->tx_sampling_rate)/1000);
    tdmInit[0].u.tdm.silence        = 0;
    tdmInit[0].u.tdm.start          = HAL_TDM_ALWAYS_ON;
    tdmInit[0].u.tdm.mcbsp_block    = 0;
    tdmInit[0].u.tdm.type           = HAL_TDM_MCBSP;
    tdmInit[0].u.tdm.maxPhase       = 0;
    tdmInit[0].u.tdm.handle         = NULL;
    tdmInit[0].u.tdm.subFrameCallout= NULL;
    tdmInit[0].u.tdm.initTs         = NULL;
    cfg.tdm = &tdmInit[0];

    hal.halGetSizes(&nbufs, &ecoBufsPtr, &cfg);
    ecoxdaisxlatEcoToXdais (nbufs, ecoBufsPtr, &memTab[buf]);
    buf += nbufs;

    return buf;
}

/* ============ AFE_create() ========== */
/**
 *  @brief      Creates AFE object based on the configuration parameter
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
Void* AFEDEV_create (AFE_CreateParams *createParams, const IALG_MemRec memTab[])
{
  AFE_Object  *obj = NULL;
  AFE_ChanInst *inst = NULL;
  Int size, i, j;
  UInt buf = 0;
  halNewConfig_t  cfg;
  halTdmInit_t    tdmInit[AFE_MAX_TDM_PORTS];
  tint            nbufs; /* Number of memory buffers hal requests    */
  halMemBuffer_t  *ecoBufsPtr; /* Information about HALs requested buffers */
  halReturn_t     halReturn;

  /* Allocate memory for the AFE_Object structure */
  if(memTab[buf].size >= sizeof(AFE_Object))
  {
    obj = memTab[buf].base;
  }
  else
    return NULL;

  buf++;

  /* Init AFE Context */
  obj->maxNumberStreams    = createParams->num_streams;
  obj->maxNumberChannels   = createParams->num_chans;
  obj->maxNumberRxChannels = createParams->num_rx_tdmchans;
  obj->maxNumberTxChannels = createParams->num_tx_tdmchans;
  obj->frameSize           = createParams->frame_size_ms;
  obj->maxRxSampleRate     = createParams->rx_sampling_rate;
  obj->maxTxSampleRate     = createParams->tx_sampling_rate;
  obj->rxSampleRate        = 0; /* Not set */
  obj->txSampleRate        = 0; /* Not set */
  obj->rxSamplesPerTdmIsr  = 0; /* Not set */
  obj->txSamplesPerTdmIsr  = 0; /* Not set */
  obj->numActiveChannels   = 0; /* Not set */
  obj->maxRxTdmTimeslots   = 0; /* Not set */
  obj->maxTxTdmTimeslots   = 0; /* Not set */
  obj->resync              = 1; /* Will resync to the tdm during first issue/reclaim */
  /* initialize ISR timer stats */
  obj->isrStats.tIsr       = 0;  
  obj->isrStats.dtIsr      = 0;
  obj->isrStats.dtIsrMax   = 0; 
  obj->isrStats.dtIsrMin   = 0; 
  obj->isrStats.dtIsrErrP  = 0;
  obj->isrStats.dtIsrErrN  = 0;
  obj->isrStats.firstIsr   = 1; 
  obj->isrStats.isrCnt     = 0;

  /* Allocate memory for the AFE_StreamInst for maximum streams */
  if(memTab[buf].size >= (sizeof(AFE_StreamInst) * obj->maxNumberStreams)) {
    obj->afeStreamInst = memTab[buf].base;
  } 
  else {
    return NULL;
  }

  buf++;

  /* Allocate memory for the AFE_ChanInst for maximum voice channels */
  if(memTab[buf].size >= sizeof(AFE_ChanInst)*obj->maxNumberChannels)
  {
    obj->afeChanInst = memTab[buf].base;
  }
  else
    return NULL;

  buf++;

  /* ====== Initialize AFE channel instance ====== */
  for (i = 0; i < obj->maxNumberChannels; i++) {
    inst = &obj->afeChanInst[i];
    inst->chanID              = AFE_MAKEID(0, AFE_FIRSTCHAN + i);
    inst->rx_frame_length     = 0; /*Not set*/
    inst->tx_frame_length     = 0; /*Not set*/
    inst->channel_rx_timeslot = -1;
    inst->channel_tx_timeslot = -1;
  }

  /* Allocate memory for the double framed playout buffers per TDM channel */
  size = sizeof(linSample) * 2 * ((obj->maxTxSampleRate * obj->frameSize)/1000);
  if(memTab[buf].size >= size*obj->maxNumberTxChannels) {
    for (i = 0; i < obj->maxNumberChannels; i++) {
      inst = &obj->afeChanInst[i];
      if (i < obj->maxNumberTxChannels) {
        inst->fromMspBuf = (linSample *)(memTab[buf].base) + (i * size)/2;
      }
      else {
        inst->fromMspBuf = NULL;
      }
    }
  }
  else {
    return NULL;
  }

  buf++;

  /* Allocate memory for the double framed capture buffers per TDM channel */
  size = sizeof(linSample) * 2 * ((obj->maxRxSampleRate * obj->frameSize)/1000);
  if(memTab[buf].size >= size*obj->maxNumberRxChannels) {
    for (i = 0; i < obj->maxNumberChannels; i++) {
      inst = &obj->afeChanInst[i];
      if (i < obj->maxNumberRxChannels) {
        inst->toMspBuf = (linSample *)(memTab[buf].base) + (i * size)/2;
      }
      else {
        inst->toMspBuf = NULL;
      }
    }
  }
  else {
    return NULL;
  }

  buf++;

  /* Allocate memory for the PCM buffers for gathering data from all active TS for a stream         */
  /* The assumption is that the gathering will be done only for 1 stream and max_num_tdmchans chans */ 
  size = sizeof(linSample) * ((obj->maxRxSampleRate * obj->frameSize)/1000);
  if(memTab[buf].size >= size*obj->maxNumberRxChannels) {
    obj->gatherTDMBuf = memTab[buf].base;
  }
  else {
    return NULL;
  }

  buf++;

  for (i = 0; i <  obj->maxNumberStreams; i++) {
    obj->afeStreamInst[i].streamId = 0;
    for(j = 0; j < AFE_MAX_ECO_SLOTS; j++) {
      obj->afeStreamInst[i].ecoInst[j] = NULL;
      obj->afeStreamInst[i].ecoAlgId[j] = 0;
    }
    for(j = 0; j < AFE_MAX_TDMCHAN_PER_STREAM; j++) {
      obj->afeStreamInst[i].playoutTdmChanId[j] = 0xffff;
      obj->afeStreamInst[i].captureTdmChanId[j] = 0xffff;
    }
    obj->afeStreamInst[i].numCaptureTdmChans = 0;
    obj->afeStreamInst[i].numPlayoutTdmChans = 0;
    obj->afeStreamInst[i].rx_frame_length = 0;
    obj->afeStreamInst[i].tx_frame_length = 0;
    obj->afeStreamInst[i].state = AFE_STREAM_INACTIVE;
  }

  /* Initialize HAL */
  AFE_initHal (obj);

  /* Init HAL parameters */
  cfg.nTdmPorts = AFE_MAX_TDM_PORTS;

  /* Initialize the configuration for 1st and only TDM port */
  tdmInit[0].fcn                  = HAL_TDM_AS_TDM;
  tdmInit[0].u.tdm.maxChannels    = createParams->num_chans;;
  tdmInit[0].u.tdm.maxRxChannels  = createParams->num_rx_tdmchans;;
  tdmInit[0].u.tdm.maxTxChannels  = createParams->num_tx_tdmchans;;
  tdmInit[0].u.tdm.wordSize       = (AFE_TDMWORDLENGTH);
  tdmInit[0].u.tdm.rxSubFrameRate = (tuint)((AFE_TDMISRINTERVALMS) * (createParams->rx_sampling_rate)/1000);
  tdmInit[0].u.tdm.txSubFrameRate = (tuint)((AFE_TDMISRINTERVALMS) * (createParams->tx_sampling_rate)/1000);
  tdmInit[0].u.tdm.bdxDelay       = 0;
  tdmInit[0].u.tdm.silence        = 0;
  tdmInit[0].u.tdm.start          = HAL_TDM_ALWAYS_ON;
  tdmInit[0].u.tdm.mcbsp_block    = 0;
  tdmInit[0].u.tdm.type           = HAL_TDM_MCBSP;
  tdmInit[0].u.tdm.maxPhase       = (tuint)((createParams->frame_size_ms)/(AFE_TDMISRINTERVALMS))/*AFE_BSPMAXPHASE*/;
  tdmInit[0].u.tdm.handle         = (void *)obj;
  tdmInit[0].u.tdm.subFrameCallout= AFE_subRateFcn;
  tdmInit[0].u.tdm.initTs         = NULL;
  cfg.tdm = &tdmInit[0];

  /* Get memory requirements from HAL  */
  (*obj->halApi->halGetSizes) (&nbufs, &ecoBufsPtr, &cfg);

  ecoxdaisxlatXdaisToEco (nbufs, &memTab[buf], ecoBufsPtr);

  /* Initialize the HAL unit */
  halReturn = (*obj->halApi->halNew)(nbufs, ecoBufsPtr, &cfg);
  if (halReturn != HAL_SUCCESS) {
    return NULL;
  }

  /* Record the HAL pcm port handle and secondary pcm port handle */
  obj->halIdPcm = cfg.tdm[0].halId;

  /* Initialize AFE notification semaphore to a known state */
  SEM_new (&(obj->afeNotifyObj.afeSemObj), 0);

  return ((Void *)obj);
}

/* ============ AFE_open ========== */
/**
 *  @brief      Open phase of the AFE.
 *
 *  @param[in]  handle  Pointer to AFE_Object structure
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
AFE_Error AFE_open (AFE_Handle handle)
{
    FIX_COMPIL_WARNING(handle);
  return (AFE_SUCCESS);
}

AFE_Error AFE_close (AFE_Handle handle)
{
    FIX_COMPIL_WARNING(handle);
  return (AFE_SUCCESS);
}

/* ============ AFEDEV_delete ========== */
/**
 *  @brief      Delete phase of AFE task
 *
 *
 *  @param[in]  handle      AFE Handle - Pointer to AFE_Object
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
Int AFE_delete(Void *afedevObj)
{
    FIX_COMPIL_WARNING(afedevObj);
  return (AFE_SUCCESS);
}

Int AFEDEV_delete(Void *afedevObj)
{
FIX_COMPIL_WARNING(afedevObj);
  return (AFE_SUCCESS);
}
#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
