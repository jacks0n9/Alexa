/*****************************************************************************
 * FILE PURPOSE: Hardware Abstraction Layer (HAL) initialization
 *****************************************************************************
 * FILE NAME: halinit.c
 * 
 * DESCRIPTION: Provides HAL API functions halGetSizes and halNew
 *
 * TABS: None
 *
 * Copywrite (C) 2002, Telogy Networks, Inc.
 *****************************************************************************/
/* -- Standard C Headers -- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "src/hal.h"
#include "ti/mas/types/types.h"
#include "src/halloc.h"

#include "src/c64P/DM3730/halport.h"
#include "src/c64P/DM3730/tdm/haltdm.h"

static halMemBuffer_t memBuf[HAL_NBUFS] = {
   /* class                                 log2align       size        volat       base  */
  HAL_MEM_CLASS_EXTERNAL,          0,           0,          FALSE,     NULL,
  HAL_MEM_CLASS_INTERNAL,          0,           0,          FALSE,     NULL,
  HAL_MEM_CLASS_INTERNAL,          0,           0,          FALSE,     NULL
};  

/******************************************************************************
 * FUNCTION PURPOSE: Returns buffer requirements for HAL
 ******************************************************************************
 * DESCRIPTION: Returns the memory requirements for HAL based on the 
 *              information provided in the sizeConfig structure.
 *
 *              Hal returns its buffer requests in the following order:
 *              heap (always 1 heap buffer), tdm (always 2 buffers per port, tx
 *              followed by rx).
 *****************************************************************************/
halReturn_t halGetSizes (tint *nbufs, halMemBuffer_t **bufs,
                         halNewConfig_t *cfg)
{
  tint  i;
  tuint memSize;
  tint  tdmTxDmaBufs[] = { HAL_TDM0_DMA_TX_BUFN };
  tint  tdmRxDmaBufs[] = { HAL_TDM0_DMA_RX_BUFN };

  /* Determine the size of the heap buffer. */
  memSize = sizeof(halContext_t);

  /* Memory required by halContext Tdm pointers */
  memSize += cfg->nTdmPorts * sizeof (void *);

  HAL_ALIGN_STRUCT(memSize);
   
  /* Add the heap size required to manage tdm ports */
  if (halInitCtable.halTdmGetHeapSize)
    memSize += (*halInitCtable.halTdmGetHeapSize) (cfg);

  HAL_ALIGN_STRUCT(memSize);

  /* Complete the 1st buffer. The first buffer is always heap, but it will
   * return a size request of 0 if none is needed */
  memBuf[HAL_HEAP_BUFN].mclass     = HAL_MEM_CLASS_EXTERNAL; 
  memBuf[HAL_HEAP_BUFN].log2align = 0;    /* no alignment requirements */    
  memBuf[HAL_HEAP_BUFN].size      = memSize;
  memBuf[HAL_HEAP_BUFN].volat     = FALSE;
  memBuf[HAL_HEAP_BUFN].base      = NULL;

  /* Determine buffer requirements for the TDM ports, if any */
  for (i = 0; i < cfg->nTdmPorts; i++)  {
    /* tx Buffer */
   memBuf[tdmTxDmaBufs[i]].mclass = HAL_MEM_CLASS_INTERNAL;
   memBuf[tdmTxDmaBufs[i]].volat = FALSE;
   memBuf[tdmTxDmaBufs[i]].base  = NULL;
   (*halInitCtable.halTdmGetTxBufInfo) (i, cfg, &(memBuf[tdmTxDmaBufs[i]].log2align), 
                                        &(memBuf[tdmTxDmaBufs[i]].size), 1);

   /* rx Buffer */
   memBuf[tdmRxDmaBufs[i]].mclass = HAL_MEM_CLASS_INTERNAL;  
   memBuf[tdmRxDmaBufs[i]].volat = FALSE;
   memBuf[tdmRxDmaBufs[i]].base  = NULL;
   (*halInitCtable.halTdmGetTxBufInfo) (i, cfg, &(memBuf[tdmRxDmaBufs[i]].log2align), 
                                        &(memBuf[tdmRxDmaBufs[i]].size), 0);
  }

  /* Return the buffer information */
  *nbufs = HAL_NBUFS;
  *bufs  = memBuf;

  return (HAL_SUCCESS);

} /* halGetSizes */

/*****************************************************************************
 * FUNCTION PURPOSE: Create HAL
 *****************************************************************************
 * DESCRIPTION: Creates and initializes new hal structures.
 *****************************************************************************/
halReturn_t halNew (tint nbufs, halMemBuffer_t *bufs, halNewConfig_t *cfg)
{
  tword *bufBase;
  tuint  used;
  tint   i;
  halReturn_t retVal;
  tint  tdmTxDmaBufs[] = { HAL_TDM0_DMA_TX_BUFN };
  tint  tdmRxDmaBufs[] = { HAL_TDM0_DMA_RX_BUFN };

  nbufs = nbufs;  /* To avoid compiler warning */

  /* Allocate and initialize heap amoung the parts of hal */
  bufBase = (tword *)bufs[HAL_HEAP_BUFN].base;
  used = 0;

  /***************************************************************************
   *       Heap memory required by halContext                                *
   ***************************************************************************/
  /* The halContext strucure itself */
  if (bufs[HAL_HEAP_BUFN].size < sizeof (halContext_t))
    return (HAL_FAIL);
  halContext = (halContext_t *)bufBase;
  used = sizeof (halContext_t);

  /* TDM port pointers */
  halContext->nTdmPorts = cfg->nTdmPorts;
  if (halContext->nTdmPorts)  {
    halContext->tdmPorts = (void **) &(bufBase[used]);
    used += (sizeof (void *)) * halContext->nTdmPorts;
  }

  HAL_ALIGN_STRUCT(used);
  
  /***************************************************************************
   *       TDM Initialization                                                *
   ***************************************************************************/
  if (halInitCtable.halTdmResourceInit)
    (*halInitCtable.halTdmResourceInit)();
  for (i = 0; i < halContext->nTdmPorts; i++)  {
    halContext->tdmPorts[i] = 
       (*halInitCtable.halTdmAllocAndInit) (i, bufBase, &used,  bufs[HAL_HEAP_BUFN].size, 
                                            cfg, memBuf[tdmTxDmaBufs[i]].base, 
                                            memBuf[tdmTxDmaBufs[i]].size, 
                                            memBuf[tdmRxDmaBufs[i]].base,
                                            memBuf[tdmRxDmaBufs[i]].size, &retVal);
    if (halContext->tdmPorts[i] == NULL)
      return (retVal);
    cfg->tdm[i].halId = i;  /* Return halId for port */
  }

  HAL_ALIGN_STRUCT(used);
  
  return (HAL_SUCCESS);

} /* halNew */

/*****************************************************************************
 * FUNCTION PURPOSE: Start the Peripherals Controlled by HAL
 *****************************************************************************
 * DESCRIPTION: Starts Up the Peripherals.
 *****************************************************************************/
halReturn_t halControl (halControl_t *hal_ctrl)
{

  if (hal_ctrl->periph == HAL_TDM)  {
    if (hal_ctrl->op == HAL_PORT_ENABLE) {
      return ((*halInitCtable.halTdmInitStart) (halContext->tdmPorts[hal_ctrl->portHandle]));
    }
    if (hal_ctrl->op == HAL_PORT_DISABLE) {
      return ((*halInitCtable.halTdmStop) ());
    }
  }

  return (HAL_SUCCESS);
} /* halControl */
/* Nothing Past This Point */
