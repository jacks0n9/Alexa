#include <xdc/std.h>
#include <string.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/xdm.h>

#include "ti/mas/aeu/aeu.h"
#include "ti/mas/aeu/src/aeu_priv.h"

#include <ti/mas/vpe/svd.h>
#include <ti/mas/vpe/snl.h>
#include <ti/mas/vpe/slm.h>
#include <ti/mas/veu/veu.h>

#include <ti/mas/util/ecoxdaisxlat.h>

#define IALG_AEU_FXNS \
    &AEU_TI_IECO,       /* module ID */                         \
    NULL,            /* activate */                          \
    AEU_alloc,       /* alloc */                             \
    NULL,            /* control (NULL => no control ops) */  \
    NULL,            /* deactivate */                        \
    AEU_free,        /* free */                              \
    AEU_initObj,     /* init */                              \
    NULL,            /* moved */                             \
    AEU_numAlloc     /* numAlloc (NULL => IALG_MAXMEMRECS) */\

/*
 *  ======== AEU_IAEU ========
 *  This structure defines TI's implementation of the IAEU interface
 *  for the AEU_TI module.
 */
extern IECO_Fxns AEU_TI_IECO = {     /* module_vendor_interface */
    IALG_AEU_FXNS,
    AEU_Process, 
    AEU_Control,
};

veuContext_t veuContext;

AEU_Params AEU_TI_PARAMS = {
  {sizeof(AEU_Params), NULL, NULL},
  {2, 0, 0, 0, {AEU_SRATE_16K}, {AEU_SRATE_16K}, {AEU_SRATE_16K}, {AEU_SRATE_16K, 10, 0, 0, 0}} 
};

/*  ======== AEU_numAlloc ========
 *  Returns Number of Buffers Required by AEU for a particular configuration.
 */
/**
 *  @brief      Returns the maximum number of memory records that can be 
 *              processed by aeuAlloc().
 *
 *  @sa         aeuAlloc
 *
 */
Int  AEU_numAlloc(void)
{
  Int totalBufs = 0;
  tint nbufs;
  ecomemBuffer_t *ecoBufsPtr;
  veuSizeConfig_t cfg;

  /* One Buffer for AEU_Obj */
  totalBufs++;

  /* One Buffer for ALGO Handle array */
  totalBufs++;

  /* One Buffer for SVD object array */
  totalBufs++;

  /* One Buffer for SNL object array */
  totalBufs++;
  
  /* One Buffer for SLM object array */
  totalBufs++;

  /* Last buffers are for VEU */
  cfg.max_frame_size = veu_INFRAME_MAXLENGTH; /* TODO ? */
  veuGetSizes(&nbufs, &ecoBufsPtr, &cfg);
  totalBufs += nbufs;

  /* TODO ? Why is this still needed? */
  veuContext.exception = NULL;
  veuContext.max_frame_size = veu_INFRAME_MAXLENGTH;


  return(totalBufs);
}

/*
 *  ======== AEU_alloc ========
 */
/**
 *  @brief      get algorithm object's memory requirements
 *
 *              Returns a table of memory records that describe the
 *              size, alignment, type and memory space of all buffers
 *              required by an algorithm.
 *
 *  @param[in]  *algParams   parameters
 *
 *  @param[in]  **pf         pointer to parent IALG (not used by AEU)
 *
 *  @param[in,out] memTab[]  memory records
 *
 *  @return     number of initialized memory records
 *
 *  @sa         None
 *
 */
Int AEU_alloc(const IALG_Params *algParams,
                 IALG_Fxns **pf, IALG_MemRec memTab[])
{
  const AEU_Params *allocParamsPtr = (AEU_Params *) algParams;
  Int totalBufs = 0;
  tint instSize = 0;
  Int i, j;
  tint nbufs;
  ecomemBuffer_t *ecoBufsPtr;
  veuSizeConfig_t cfg;

  if (allocParamsPtr == NULL) {
    allocParamsPtr = &AEU_TI_PARAMS;
  }

  for(i = 0; i < AEU_NUM_BUFF; i++)
  {
    memTab[i].size = 4;
    memTab[i].alignment = 4;
    memTab[i].space = IALG_EXTERNAL;
    memTab[i].attrs = IALG_PERSIST;
    memTab[i].base = 0;
  }

  /* Get for AEU */
  memTab[AEU_OBJ_BUFF].size = sizeof(AEU_Obj);
  totalBufs++;

  /* Get for ALGO Handle Array */
  memTab[AEU_ALGO_BUFF].size = (allocParamsPtr->basicParams.num_svd 
                               + allocParamsPtr->basicParams.num_snl
                               + allocParamsPtr->basicParams.num_slm 
                               + allocParamsPtr->basicParams.num_veu) * sizeof(ALGO_Handle);
  totalBufs++;

  /* Get for SVD */
  if(allocParamsPtr->basicParams.num_svd)
  {
    svdGetSizes (&instSize);
    instSize = (instSize >> 2) + 1;
    instSize = instSize << 2; /* rounded up to 32bit boundary */
    memTab[AEU_SVD_BUFF].size = allocParamsPtr->basicParams.num_svd * instSize;
    totalBufs++;
  }

  /* Get for SNL */
  if(allocParamsPtr->basicParams.num_snl)
  {
    snlGetSizes (&instSize);
    instSize = (instSize >> 2) + 1;
    instSize = instSize << 2; /* rounded up to 32bit boundary */
    memTab[AEU_SNL_BUFF].size = allocParamsPtr->basicParams.num_snl * instSize;
    totalBufs++;
  }

  /* Get for SLM */
  if(allocParamsPtr->basicParams.num_slm)
  {
    slmGetSizes (&instSize);
    instSize = (instSize >> 2) + 1;
    instSize = instSize << 2; /* rounded up to 32bit boundary */
    memTab[AEU_SLM_BUFF].size = allocParamsPtr->basicParams.num_slm * instSize;
    totalBufs++;
  }

  /* Get buffer requirement from VEU */
  if(allocParamsPtr->basicParams.num_veu)
  {
    cfg.max_frame_size = veu_INFRAME_MAXLENGTH; /* TODO ?*/
    veuGetSizes(&nbufs, &ecoBufsPtr, &cfg);
    ecoxdaisxlatEcoToXdais (nbufs, ecoBufsPtr, &memTab[AEU_VEU_BUFF]);
    for(j = 0; j < nbufs; j++)
    {
      instSize = memTab[AEU_VEU_BUFF+j].size;
      memTab[AEU_VEU_BUFF+j].size = allocParamsPtr->basicParams.num_veu * instSize;
      totalBufs++;
    }
  }

  return (AEU_NUM_BUFF);

}

/*  ======== AEU_initObj ========
 *  Initialize the memory allocated for our instance.
 */
/**
 *  @brief      Initialize an algorithm's instance object.
 *
 *              Performs all initialization necessary to complete the
 *              run-time creation of an algorithm's instance object.  After a
 *              successful return from aeuInit(), the algorithm's instance
 *              object is ready to be used to process data.
 *
 *  @param[in]  handle          Algorithm instance handle. This is a pointer
 *                              to an initialized aeuXdaisInst_t structure.  Its
 *                              value is identical to memTab[0].base.
 *
 *  @param[in]  memTab          Array of allocated buffers.
 *
 *  @param[in]  parent          Handle of algorithm's parent instance (not used by AEU).
 *
 *  @param[in]  params          Pointer to algorithm's instance parameters.
 *
 *  @return     IALG_EOK or IALG_EFAIL.
 *
 */
Int AEU_initObj(IALG_Handle handle, const IALG_MemRec memTab[],
                          IALG_Handle p, const IALG_Params *params)
{
  AEU_Obj *aeuInstPtr = (AEU_Obj *) handle;
  const AEU_Params *newParamsPtr = (AEU_Params *) params;
  tint instSize = 0;
  tint bufSize = 0;
  Int32 ret = IALG_EOK;
  int j, k;
  int i = 0;
  tint nbufs;
  ecomemBuffer_t *ecoBufsPtr;
  ecomemBuffer_t ecoBufs[2];
  Int totalBufs = 0;

  if (newParamsPtr == NULL) {
    newParamsPtr = &AEU_TI_PARAMS;
  }

  /* allocated AEU_Obj */
  totalBufs++;

  if(newParamsPtr->basicParams.num_svd <= AEU_MAX_NUM_SVD_INST)
   aeuInstPtr->num_svd = newParamsPtr->basicParams.num_svd;
  else
   return AEU_ERR_MAX_SVD_INST_EXCEEDED;

  if(newParamsPtr->basicParams.num_snl <= AEU_MAX_NUM_SNL_INST)
   aeuInstPtr->num_snl = newParamsPtr->basicParams.num_snl;
  else
   return AEU_ERR_MAX_SNL_INST_EXCEEDED;

  if(newParamsPtr->basicParams.num_slm <= AEU_MAX_NUM_SLM_INST)
   aeuInstPtr->num_slm = newParamsPtr->basicParams.num_slm;
  else
   return AEU_ERR_MAX_SLM_INST_EXCEEDED;

  if(newParamsPtr->basicParams.num_veu <= AEU_MAX_NUM_VEU_INST)
   aeuInstPtr->num_veu = newParamsPtr->basicParams.num_veu;
  else
   return AEU_ERR_MAX_SLM_INST_EXCEEDED;

  if((aeuInstPtr->num_svd + aeuInstPtr->num_snl + aeuInstPtr->num_slm + aeuInstPtr->num_veu) == 0)
   return AEU_ERR_NO_ALG_SPECIFIED;

  /* ALGO Handle Array */
  aeuInstPtr->algoHandleArray = memTab[AEU_ALGO_BUFF].base;

  /* SVD */
  if(aeuInstPtr->num_svd)
  {
    svdGetSizes (&instSize);
    j = 0;
    for( i = 0; i < aeuInstPtr->num_svd; i++)
    {
      aeuInstPtr->algoHandleArray[i] = (ALGO_Handle *)((char *)memTab[AEU_SVD_BUFF].base + (j * instSize)/sizeof(char));
      if((ret = svdInit (aeuInstPtr->algoHandleArray[i], (tint)(newParamsPtr->basicParams.svd_config.samp_rate))) != svd_NOERR)
      {
        return (ret + AEU_SVD_ERROR_BASE);
      }
      aeuInstPtr->svd_samp_rate = newParamsPtr->basicParams.svd_config.samp_rate; /* preserve this for reset */
      j++;
    }
  }

  /* SNL */
  if(aeuInstPtr->num_snl)
  {
    snlGetSizes (&instSize);
    j = 0;
    for(;i < (aeuInstPtr->num_svd + aeuInstPtr->num_snl); i++)
    {
      aeuInstPtr->algoHandleArray[i] = (ALGO_Handle *)((char *)memTab[AEU_SNL_BUFF].base + (j * instSize)/sizeof(char));
      if((ret = snlInit (aeuInstPtr->algoHandleArray[i], newParamsPtr->basicParams.snl_config.samp_rate)) != snl_NOERR)
      {
        return (ret + AEU_SNL_ERROR_BASE);
      }
      j++;
    }
  }

  /* SLM */
  if(aeuInstPtr->num_slm)
  {
    slmGetSizes (&instSize);
    j = 0;
    for(;i < (aeuInstPtr->num_svd + aeuInstPtr->num_snl + aeuInstPtr->num_slm); i++)
    {
      slmConfig_t config;

      aeuInstPtr->algoHandleArray[i] = (ALGO_Handle *)((char *)memTab[AEU_SLM_BUFF].base + (j * instSize)/sizeof(char));
      config.valid_bf = slm_CFG_BIT_SRATE;
      config.srate_factor = newParamsPtr->basicParams.slm_config.samp_rate;
      if((ret = slmInit (aeuInstPtr->algoHandleArray[i], &config)) != slm_NOERR)
      {
        return (ret + AEU_SLM_ERROR_BASE);
      }
      j++;
    }
  }

  /* VEU */
  if(aeuInstPtr->num_veu)
  {
    veuNewConfig_t new_config;
    veuConfig_t config;
    veuSizeConfig_t cfg;

    cfg.max_frame_size = veu_INFRAME_MAXLENGTH;
    veuGetSizes(&nbufs, &ecoBufsPtr, &cfg);
    ecoxdaisxlatXdaisToEco (nbufs, &memTab[AEU_VEU_BUFF], &ecoBufs[0]);
    j = 0;
    
    for(;i < (aeuInstPtr->num_svd + aeuInstPtr->num_snl + aeuInstPtr->num_slm + aeuInstPtr->num_veu); i++)
    {
      for (k = 0; k < nbufs; k++)
      {
        bufSize = ecoBufsPtr[k].size;
        ecoBufs[k].base = (void *)((char *)memTab[AEU_VEU_BUFF+k].base + (j * bufSize)/sizeof(char));
        ecoBufs[k].size =  ecoBufs[k].size - (j * bufSize);
      }
      if((ret = veuNew (&aeuInstPtr->algoHandleArray[i], nbufs, &ecoBufs[0], &new_config)) != veu_NOERR)
      {
        return (ret + AEU_VEU_ERROR_BASE);
      }
      config.frame_size = 8*(newParamsPtr->basicParams.veu_config.frame_size); //8kHz
      config.config_bitfield = newParamsPtr->basicParams.veu_config.config_bitfield;
      if(newParamsPtr->basicParams.veu_config.samp_rate == AEU_SRATE_16K)
      {
        config.config_bitfield = (config.config_bitfield | veu_CTLM_WB_SEND | veu_CTLM_WB_RECV);
      }
      config.anr_level = 12;
      config.alc_send_level = newParamsPtr->basicParams.veu_config.alc_send_level;
      config.alc_recv_level = newParamsPtr->basicParams.veu_config.alc_recv_level;
      config.bulk_delay = 162;
      config.waepl = 36;
      config.fgc_send_gain = 0;
      config.fgc_recv_gain = 0;
      veuOpen(aeuInstPtr->algoHandleArray[i], &config);
    }
    j++;
  }

  return(IALG_EOK);
}

/*  ======== AEU_aeuControl ========
 *  Controls AEU state.
 */
XDAS_Int32 AEU_Control(IECO_Handle handle, IECO_Cmd command, IECO_DynamicParams *dynParams, IECO_Status *status)
{
  AEU_Obj *aeuInst = (AEU_Obj *)handle;
  Int32 ret = IALG_EOK;
  Uint16 index2, index = 0;
  AEU_ControlParams *ctrlParams = (AEU_ControlParams *)(dynParams->buf);
  AEU_ResponseParams *respParams = (AEU_ResponseParams *)(status->buf);

  if(aeuInst == NULL)
     return AEU_ERR_NOAEUINST;

  if(command == XDM_SETPARAMS)
  {
    switch(ctrlParams->controlId)
    {
    case AEU_SLM_CONTROL:
      index = ctrlParams->algId - (aeuInst->num_svd + aeuInst->num_snl);
      if(index < aeuInst->num_slm)
      {
        if((ret = slmControl (aeuInst->algoHandleArray[ctrlParams->algId], 
                                            ctrlParams->ctrl.slmControl.mode)) != slm_NOERR)
        {
          return (ret + AEU_SLM_ERROR_BASE);
        }
      }
      else
        return AEU_ERR_WRONG_SLM_ALG_ID;
      break;

    case AEU_VEU_CONTROL:
      index = ctrlParams->algId - (aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm);
      if(index <= aeuInst->num_veu) /* Note that there is only one AEU instance and (index) and (index +1) is for TX VEU and RX VEU */
      {
        veuControl_t ctl;
        ALGO_Handle aeuAlgoHandle;

        if(index == aeuInst->num_veu)
          aeuAlgoHandle = aeuInst->algoHandleArray[(ctrlParams->algId)-1];
        else
          aeuAlgoHandle = aeuInst->algoHandleArray[ctrlParams->algId];

        ctl.ctl_code = ctrlParams->ctrl.veuControl.ctl_code;
        if(ctl.ctl_code == veu_CTL_MODE)
        {
          ctl.u.mode_ctl.bit_masks = ctrlParams->ctrl.veuControl.u.mode_control.mask;
          ctl.u.mode_ctl.bit_values = ctrlParams->ctrl.veuControl.u.mode_control.value;
        }
        else if ((ctl.ctl_code == veu_CTL_ALC_SEND_LEVEL) || (ctl.ctl_code == veu_CTL_ALC_RECV_LEVEL)) 
        {
          ctl.u.alc_level = ctrlParams->ctrl.veuControl.u.alc_level;
        }
        if((ret = veuControl (aeuAlgoHandle, &ctl)) != veu_NOERR)
        {
          return (ret + AEU_VEU_ERROR_BASE);
        }
      }
      else
        return AEU_ERR_WRONG_VEU_ALG_ID;
      break;
    case AEU_ALGO_RESET:
      index = ctrlParams->algId;
      if(index < aeuInst->num_svd)
      {
        if((ret = svdInit (aeuInst->algoHandleArray[ctrlParams->algId], (tint)(aeuInst->svd_samp_rate))) != svd_NOERR)
          return ret + AEU_SVD_ERROR_BASE;
      }
      else
        return AEU_ERR_WRONG_SVD_ALG_ID;
      break;
    }
  }
  else if (command == XDM_GETSTATUS)
  {
    switch(ctrlParams->controlId)
    {
    case AEU_SVD_GET_DECISION:
      respParams->responseId = ctrlParams->controlId; 
      index = ctrlParams->algId;
      if(index < aeuInst->num_svd)
      {
        respParams->resp.svdDecision.decision = svdGetDecision (aeuInst->algoHandleArray[ctrlParams->algId]);
      }
      else
        return AEU_ERR_WRONG_SVD_ALG_ID;
      break;
    case AEU_SNL_GET_LEVELS:
      respParams->responseId = ctrlParams->controlId; 
      index = ctrlParams->algId - aeuInst->num_svd;
      if(index < aeuInst->num_snl)
      {
        if((ret = snlGetLevels (aeuInst->algoHandleArray[ctrlParams->algId],
                                      &(respParams->resp.snlLevels.speechdB),
                                      &(respParams->resp.snlLevels.noisedB),
                                      &(respParams->resp.snlLevels.threshold))) != slm_NOERR)
        {
          return (ret + AEU_SNL_ERROR_BASE);
        }
      }
      else
        return AEU_ERR_WRONG_SNL_ALG_ID;
      break;
    case AEU_VEU_GET_STATS:
      respParams->responseId = ctrlParams->controlId; 
      index = ctrlParams->algId - (aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm);
      if(index <= aeuInst->num_veu) /* Note that there is only one AEU instance and (index) and (index +1) is for TX VEU and RX VEU */
      {
        veuStats_t stats;
        ALGO_Handle aeuAlgoHandle;

        if(index == aeuInst->num_veu)
          aeuAlgoHandle = aeuInst->algoHandleArray[(ctrlParams->algId)-1];
        else
          aeuAlgoHandle = aeuInst->algoHandleArray[ctrlParams->algId];

        veuGetStats  (aeuAlgoHandle, &stats, ctrlParams->ctrl.veuGetStats.reset);
        respParams->resp.veuStats.send_gain      = (Int16)stats.send_gain;
        respParams->resp.veuStats.send_noise_enr = (Int16)stats.send_noise_enr;
        respParams->resp.veuStats.send_avg_enr   = (Int16)stats.send_avg_enr;
        respParams->resp.veuStats.recv_gain      = (Int16)stats.recv_gain;
        respParams->resp.veuStats.recv_noise_enr = (Int16)stats.recv_noise_enr;
        respParams->resp.veuStats.recv_avg_enr   = (Int32)stats.recv_avg_enr;
      }
      else
        return AEU_ERR_WRONG_VEU_ALG_ID;
      break;
    }
  }
  else if (command & XDM_GET_PERF_INFO)
  {
    index = command & (~(XDM_GET_PERF_INFO)); /* alg Id is embedded in LSBs */

    if((index+1) > (aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm + 2*aeuInst->num_veu))
      return AEU_ERR_INVALID_ALG_ID;

    if(index < aeuInst->num_svd)
    {
       AEU_SvdPerfInfo *perfPtr = (AEU_SvdPerfInfo *)status->buf;
       perfPtr->AeuVadDecision = svdGetDecision (aeuInst->algoHandleArray[index]);
    }
    else if (index >= aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm)
    {
      index2 = index - (aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm);
      if(index2 <= aeuInst->num_veu) /* Note that there is only one AEU instance and (index) and (index +1) is for TX VEU and RX VEU */
      {
        veuStats_t stats;
        ALGO_Handle aeuAlgoHandle;
        AEU_AlcPerfInfo *perfPtr = (AEU_AlcPerfInfo *)status->buf;

        if(index == aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm)
          aeuAlgoHandle = aeuInst->algoHandleArray[index];
        else
          aeuAlgoHandle = aeuInst->algoHandleArray[index-1];

        veuGetStats  (aeuAlgoHandle, &stats, 0);

        if(index == aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm)
          perfPtr->AeuAlcGain = (Int16)stats.send_gain;
        else
          perfPtr->AeuAlcGain = (Int16)stats.recv_gain;
      }
      else
        return AEU_ERR_WRONG_VEU_ALG_ID;
    }
  }

  return (ret);
}

XDAS_Int32 AEU_Process(IECO_Handle handle, IECO_InArgs *inArgs, IECO_OutArgs *outArgs)
{
  AEU_Obj *aeuInst = (AEU_Obj *) handle;
  Uint16 index = 0;
  Int32 ret = IALG_EOK;
  IECO_StreamDesc *inStream = inArgs->inStream[0]; 
  IECO_StreamDesc *outStream = outArgs->outStream[0];
  int check_copy = 0;

  index = inArgs->message;
  if((index+1) > (aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm + 2*aeuInst->num_veu))
    return AEU_ERR_INVALID_ALG_ID;

  /* SVD */
  if(index < aeuInst->num_svd)
  {
    ret = svdProcess (aeuInst->algoHandleArray[index], (linSample *)(inStream->buf[AEU_BUFF_IN]), (inStream->bufSize[AEU_BUFF_IN])/sizeof(linSample));
    if (ret != svd_NOERR)
    {
      return (ret + AEU_SVD_ERROR_BASE);
    }
    check_copy = 1;

    /* application needs VAD decision on per frame basis */
    if(outArgs->status)
    {
      *(outArgs->status) = (Int32) svdGetDecision (aeuInst->algoHandleArray[index]);
    }
  }

  /* SNL */
  else if((index >= aeuInst->num_svd) && (index < (aeuInst->num_svd + aeuInst->num_snl)))
  {
    ret = snlProcess (aeuInst->algoHandleArray[index], (linSample *)(inStream->buf[AEU_BUFF_IN]), (inStream->bufSize[AEU_BUFF_IN])/sizeof(linSample));
    if (ret != snl_NOERR)
    {
      return (ret + AEU_SNL_ERROR_BASE);
    }
    check_copy = 1;
  }

  /* SLM */
  else if((index >= aeuInst->num_svd + aeuInst->num_snl) && (index < (aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm)))
  {
    ret = slmProcess (aeuInst->algoHandleArray[index], inStream->buf[AEU_BUFF_IN], (inStream->bufSize[AEU_BUFF_IN])/sizeof(linSample));
    if (ret != slm_NOERR)
    {
      return (ret + AEU_SLM_ERROR_BASE);
    }
    check_copy = 1;
  }
  /* VEU TX and VEU RX */
  else
  {
    ALGO_Handle aeuAlgoHandle;

    index = inArgs->message - (aeuInst->num_svd + aeuInst->num_snl + aeuInst->num_slm);

    if(index == aeuInst->num_veu) /* VEU RX */
    {
      aeuAlgoHandle = aeuInst->algoHandleArray[(inArgs->message)-1];
      veuReceiveIn(aeuAlgoHandle, (linSample *)(inStream->buf[AEU_BUFF_IN]));
      check_copy = 1;
    }
    else /* VEU TX */
    {
      aeuAlgoHandle = aeuInst->algoHandleArray[inArgs->message];
      veuSendIn(aeuAlgoHandle, (linSample *)(inStream->buf[AEU_BUFF_IN]), 
                                                  (linSample *)(inStream->buf[AEU_BUFF_REF_IN]),
                                                  (linSample *)(outStream->buf[AEU_BUFF_OUT]),
                                                  NULL);
    }
  }

  if(check_copy) /* check if we need to copy from input to output */
  {
    if(inStream->buf[AEU_BUFF_IN] != outStream->buf[AEU_BUFF_OUT])
    {
      memcpy(outStream->buf[AEU_BUFF_OUT], inStream->buf[AEU_BUFF_IN], outStream->bufSize[AEU_BUFF_OUT]);
    }
  }

  return (ret);
}

/*
 *  ======== AEU_free ========
 */
/**
 *  @brief      get algorithm object's memory requirements
 *
 *              Returns a table of memory records that describe the
 *              size, alignment, type and memory space of all buffers
 *              previously allocated for the algorithm's instance.
 *
 *  @param[in]  handle    Algorithm instance handle.  This is a pointer
 *                        to an initialized aeuXdaisInst_t structure.  Its
 *                        value is identical to memTab[0].base.
 *
 *  @param[out] memTab    Contains pointers to all of the memory passed
 *                        to the algorithm via aeuInit().
 *
 *  @return     number of initialized memory records
 *
 *  @sa         None
 *
 */
Int AEU_free(IALG_Handle handle, IALG_MemRec memTab[])
{
  AEU_Obj *aeuInst = (AEU_Obj *) handle;
  Int nbufs = AEU_NUM_BUFF;

  memTab[AEU_OBJ_BUFF].base = handle;
  memTab[AEU_ALGO_BUFF].base = aeuInst->algoHandleArray;

  return(nbufs); 
}
