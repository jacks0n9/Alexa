/* ========================================================================== */
/**
 *  @file   haltdm.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/c64P/DM3730/tdm/haltdm.c
 *
 *  @brief  This file contains functions to initialize TDM
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#include <xdc/std.h>

/* -- Standard C Headers -- */
#include <stdio.h>
#include <string.h>

/* -- DSP/BIOS Headers -- */
#include <std.h>
#include <hwi.h>
#include <c64.h>
#include <sio.h>
#include <iom.h>
#include <log.h>
#include <tsk.h>

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- AFE Headers -- */
#include "src/afedbg.h"

/* -- HAL Headers -- */
#include "src/hal.h"
#include "haltdm.h"
#include "tdmloc.h"

#include <ti/sdo/edma3/drv/edma3_drv.h>
#include <ti/pspiom/mcbsp/Mcbsp.h>
#include <ti/pspiom/cslr/cslr.h>
#include <ti/pspiom/cslr/tistdtypes.h>

/* ========================================================================== */
/*                        IMPORTED VARIABLES                                  */
/* ========================================================================== */

extern LOG_Obj trace;
extern EDMA3_DRV_Handle hEdma[];
extern Mcbsp_ChanParams mcbspChanparamRx;
extern Mcbsp_ChanParams mcbspChanparamTx;

/* ========================================================================== */
/*                        EXTERNAL FUNCTIONS                                  */
/* ========================================================================== */

extern Int edma3init();
extern Int edma3deinit();
 
/* ============ halTdmGetHeapSize() ========== */
/**
 *  @brief      Returns the amount of heap memory required for TDM
 *
 *              Computes the amount of memory needed for every serial port 
 *              requested in bytes.
 *
 *  @param[in]  cfg    Pointer to halNewConfig_t structure
 *
 *  @return     HAL heap size
 *
 *  @sa         None
 *
 */
tuint halTdmGetHeapSize (halNewConfig_t *cfg)
{
  int   i;
  tuint size = 0;

  /*
   * Every tdm port consists of a tdm instance, an array of timeslots and
   * an array of timeslot identifiers
   */
  for (i = 0; i < cfg->nTdmPorts; i++) {
    if (cfg->tdm->fcn == HAL_TDM_AS_TDM) {
      size += sizeof(halTdmMcBsp_t);  
      size += cfg->tdm[i].u.tdm.maxChannels * sizeof(halTsArray_t);
    }
    else {
      /* NO support for TDM as GPIO on Leo IPP */
      DBGLOG3((&trace, "[TDM->ERROR]: Requested not supported GPIO functionality\n"));
    }
  }
  return (size);
}

/* ============ halTdmGetDmaBufInfo() ========== */
/**
 *  @brief      Returns the memory for DMA buffer requirements
 *
 *              The DMA requires buffers that are aligned on a power of two
 *              boundary that is strictly greater than the required buffer
 *              size. The same function is used for the Tx and Rx buffers, and 
 *              is defined to halTdmGetTxBufInfo and halTdmGetRxBufInfo in 
 *              tdmdma.h
 *
 *  @param[in]  port   TDM port number
 *  @param[in]  cfg    Pointer to halNewConfig_t structure
 *  @param[out] align  Alignment
 *  @param[out] size   Total size in bytes required for DMA double buffer
 *
 *  @return     HAL heap size
 *
 *  @sa         None
 *
 */
tint halTdmGetDmaBufInfo (
  tint              port,
  halNewConfig_t    *cfg,
  tuint             *align,
  tuint             *size,
  tint               tdmType
)
{
  tuint tmpSize = 0;
  if (cfg->tdm[port].fcn == HAL_TDM_AS_TDM) {
    /* Determine the total size in bytes required for DMA double buffer */
    /*
     * It is assumed that DMA will operate on 16-bit samples. If DMA 
     * operates on 32-bit sample value then size has to be doubled
     * Leo operates on 32-bit sample hence the extra multiplier of 2
     */
    if (tdmType == 1) {
      tmpSize = cfg->tdm[port].u.tdm.txSubFrameRate * cfg->tdm[port].u.tdm.maxTxChannels; 
    }
    else {
      tmpSize = cfg->tdm[port].u.tdm.rxSubFrameRate * cfg->tdm[port].u.tdm.maxRxChannels; 
    }
    tmpSize = tmpSize * sizeof(linSample); // Size in bytes
    tmpSize = tmpSize * 2; // Size in bytes for ping+pong buffers
    tmpSize = tmpSize * 2; // Size in bytes for ping+pong buffers + DMA operating on 32 bits
    *size = tmpSize;
    /* Determine the required alignment */
    *align = 0;
    while ((1 << *align) < *size) (*align)++;
   }
   else  {
     *size = 0;
     *align = 0;
  }
  return (HAL_SUCCESS);
}

/* ============ halTdmMcBspInit() ========== */
/**
 *  @brief      Allocates memory and initializes TDM port
 *
 *              Memory is allocated from the heap, and the initial port
 *              settings are setup. Returns a pointer to the tdm port
 *              structure, returns NULL on heap allocation error.
 *
 *  @param[in]  port      HAL TDM port number, (indexed from 0)
 *  @param[in]  vbase     heap base address
 *  @param[out] used      heap used - must be updated when heap is allocated
 *  @param[in]  size      heap size
 *  @param[in]  newCfg    TDM configuration structure
 *  @param[in]  dmaTx     DMA tx memory buffer address
 *  @param[in]  dmaTxSize Size of the DMA tx buffer
 *  @param[in]  dmaRx     DMA tx memory buffer address
 *  @param[in]  dmaRxSize Size of the DMA rx address
 *  @param[out] ecode     Returns success or hal error code
 *
 *  @return     Pointer to halTdmMcBsp_t structure
 *
 *  @sa         None
 *
 */
halTdmMcBsp_t *halTdmMcBspInit (
  tint               port,
  void              *vbase,
  tuint             *used,
  tuint              size, 
  halNewConfig_t    *newCfg,
  tuint             *dmaTx,
  tuint              dmaTxSize,
  tuint             *dmaRx,
  tuint              dmaRxSize,
  halReturn_t       *ecode
)
{
  int           i;
  halTdmMcBsp_t *tdm;
  tword         *base = (tword *)vbase; 
  tuint         dmaWordSize;

  /* -- Memory Allocation -- */

  /* TDM Instance */
  tdm = (halTdmMcBsp_t *)&(base[(*used)]);
  *used += sizeof(halTdmMcBsp_t);  
  if (*used > size) {
    *ecode = HAL_HEAP_MEMORY_ERR;
    return (NULL);
  }

  /* Timeslot Array */
  tdm->tsArray = (halTsArray_t *)&(base[(*used)]);
  *used += newCfg->tdm[port].u.tdm.maxChannels * sizeof (halTsArray_t);
  if (*used > size) {
    *ecode = HAL_HEAP_MEMORY_ERR;
    return (NULL);
  }

  /* -- Initialize TDM Instance -- */
  tdm->function       = HAL_TDM_AS_TDM;
  tdm->port           = port;
  tdm->start          = newCfg->tdm[port].u.tdm.start;
  tdm->restart        = HAL_TDM_MCBSP_RESTART;
  tdm->maxTs          = newCfg->tdm[port].u.tdm.maxChannels;
  tdm->maxRxTs        = newCfg->tdm[port].u.tdm.maxRxChannels;
  tdm->maxTxTs        = newCfg->tdm[port].u.tdm.maxTxChannels;
  tdm->rxSubFrame     = newCfg->tdm[port].u.tdm.rxSubFrameRate;
  tdm->txSubFrame     = newCfg->tdm[port].u.tdm.txSubFrameRate;
  tdm->numActiveRxTs  = 0;
  tdm->numActiveTxTs  = 0;
  tdm->silence        = newCfg->tdm[port].u.tdm.silence;

  /*
   * NOTE: dmaTxSize or dmaRxSize passed to this function is in
   * minimum addressable unit (MAU) of this platform i.e., in 
   * bytes. Convert it into 16-bit word size for proper operation
   * of halTdmDmaIsr. It is also assumed that dmaTxSize is equal
   * to dmaRxSize.
   * For Leo the transfer is in 32-bit word
   */
  dmaWordSize = sizeof(linSample) * 2;
  tdm->dma.rxSize     = ((dmaRxSize/dmaWordSize) >> 1);
  tdm->dma.txSize     = ((dmaTxSize/dmaWordSize) >> 1);

  /* -- Initialize TDM-DMA -- */
  tdm->dma.txAddr       = dmaTx;
  tdm->dma.rxAddr       = dmaRx;
  tdm->dma.txReldAddr   = dmaTx + ((dmaTxSize / sizeof(linSample)) / 2);
  tdm->dma.rxReldAddr   = dmaRx + ((dmaRxSize / sizeof(linSample)) / 2);

  /* -- Run time serial port configuration -- */
  tdm->sport.tsSize      = newCfg->tdm[port].u.tdm.wordSize;
  tdm->handle            = newCfg->tdm[port].u.tdm.handle;
  tdm->subFrameCallout   = newCfg->tdm[port].u.tdm.subFrameCallout;

  /* Initialize 1st maxTs timeslots as dummy timeslots */
  for (i = 0; i < tdm->maxTs; i++)  { 
    tdm->tsArray[i].ts          = i;
    tdm->tsArray[i].status      = HAL_TS_DUMMY;
  }

  for (i=0;i<AFE_MAX_TDMCHAN_PER_STREAM;i++){
      tdm->tsArray[i].dmaIndex = i;
  }


  /* Return the pointer to new TDM Instance structure */
  *ecode = HAL_SUCCESS;
  return (tdm);
}

/* ============ halTdmAllocAndInit() ========== */
/**
 *  @brief      Allocate heap memory to TDM port and initializes data structures
 *
 *              Allocate memory for the TDM control structure. Returns a pointer
 *              to the tdm structure. The halPortId (handle) for the port is
 *              returned in the configuration structure.
 *
 *  @param[in]  port      HAL TDM port number, (indexed from 0)
 *  @param[in]  base      heap base address
 *  @param[in]  used      heap used - must be updated when heap is allocated
 *  @param[in]  size      heap size
 *  @param[in]  newCfg    TDM configuration structure
 *  @param[in]  dmaTx     DMA tx memory buffer address
 *  @param[in]  dmaTxSize Size of the DMA tx buffer
 *  @param[in]  dmaRx     DMA tx memory buffer address
 *  @param[in]  dmaRxSize Size of the DMA rx address
 *  @param[out] ret       Returns success or hal error code
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void *halTdmAllocAndInit (
  tint             port,
  void            *base,
  tuint           *used,
  tuint            size, 
  halNewConfig_t  *newCfg,
  tuint           *dmaTx, 
  tuint            dmaTxSize,
  tuint           *dmaRx,
  tuint            dmaRxSize,
  halReturn_t     *ret
)
{
  halTdmMcBsp_t *tdm = NULL;

  /* Allocate memory and initialize based on port function */
  if (newCfg->tdm->fcn == HAL_TDM_AS_TDM) {
    tdm = halTdmMcBspInit (port, base, used, size, newCfg, dmaTx, dmaTxSize, 
                           dmaRx, dmaRxSize, ret);
    return ((void*)tdm);
  }
  return (NULL);
}

/* ============ halStartupTdm() ========== */
/**
 *  @brief      Start the serial port 
 *
 *              This routine is called from halinit.c file during HAL initialization. 
 *              It is assumed that TDM and EDMA are in reset. This routine enables
 *              TDM and EDMA and TDM is taken out of reset
 *
 *  @param[in]  tdm    Pointer to halTdmTdm_t structure
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
halReturn_t halStartupTdm (halTdmMcBsp_t *tdm)
{
  Int32    status = IOM_COMPLETED;

  /* initialise the edma library                                            */
  status = edma3init();

  if (EDMA3_DRV_SOK != status)
  {
      LOG_printf(&trace, "EDMA initialization failed");
  }
  else
  {
      LOG_printf(&trace, "EDMA initialized");

      /* update the edma handle to the channel parameters.Need to use       *
       * EDMA instance 0 only for Mcbsp                                     */
      mcbspChanparamRx.edmaHandle = hEdma[0];
      mcbspChanparamTx.edmaHandle = hEdma[0];

      /* call the function to create the channel & demo the transceive      *
       * operation                                                          */
      mcbspinit(tdm);

      LOG_printf(&trace, "Sample Application completed successfully...");
  }
  return (HAL_SUCCESS);
}

/* ============ halTdmInitStart() ========== */
/**
 *  @brief      Init time startup of serial port
 *
 *              Enables the TDM, DMA and DMA interrupts for a TDM port
 *
 *  @param[in]  pTdm    Pointer to halTdmTdm_t structure
 *
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t halTdmInitStart (void *pTdm)
{
  halReturn_t   retval = HAL_SUCCESS;
  halTdmMcBsp_t *tdm = (halTdmMcBsp_t *)pTdm;

  /* Make sure this port is used for TDM */
  if (tdm->function == HAL_TDM_AS_TDM) {
    if (tdm->start == HAL_TDM_ALWAYS_ON) {
      retval = halStartupTdm (tdm);
    }
  }
  return retval;
}

/* ============ halTdmStop() ========== */
/**
 *  @brief      Stop serial port
 *
 *  @param[in]  None
 *
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t halTdmStop (void)
{
  EDMA3_DRV_Result edma3Result;
  halReturn_t      retval = HAL_SUCCESS;

    mcbspstop();

    mcbspdelete();

    edma3Result = edma3deinit();
    if (EDMA3_DRV_SOK != edma3Result) {
      LOG_printf(&trace, "EDMA3 deinit failed. Error = %d\n", edma3Result);
    }

    return (retval);
}

/* Nothing past this line */
