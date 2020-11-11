/* ========================================================================== */
/**
 *  @file   genCtl_ialg.c
 *
 *  path    ti/mas/genCtl/src/genCtl_ialg.c
 *
 *  @brief  It implements DSP control logic 
 *
 *  ============================================================================
 */
/* --COPYRIGHT--,BSD
 , Texas Instruments Incorporated
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
 
#include <xdc/std.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/xdm.h>

#include <ti/mas/devnode/devnode.h>

#include <ti/mas/genCtl/genCtl.h>
#include "genCtl_priv.h"


#define IALG_GENCTL_FXNS \
    &GENCTL_TI_IECO,       /* module ID */                         \
    NULL,            /* activate */                          \
    GENCTL_alloc,       /* alloc */                             \
    NULL,            /* control (NULL => no control ops) */  \
    NULL,            /* deactivate */                        \
    GENCTL_free,        /* free */                              \
    GENCTL_initObj,     /* init */                              \
    NULL,            /* moved */                             \
    GENCTL_numAlloc     /* numAlloc (NULL => IALG_MAXMEMRECS) */\

/*
 *  ======== GENCTL_IGENCTL ========
 */
extern IECO_Fxns GENCTL_TI_IECO = {     /* module_vendor_interface */
    IALG_GENCTL_FXNS,
    GENCTL_Process, 
    GENCTL_Control,
};

GENCTL_Params GENCTL_PARAMS = {
  sizeof(GENCTL_Params), NULL, NULL
};



/*  ======== GENCTL_numAlloc ========
 *  Returns Number of Buffers Required by GENCTL for a particular configuration.
 */
/**
 *  @brief      Returns the maximum number of memory records that can be 
 *              processed by GENCTL_Alloc().
 *
 *  @sa         GENCTL_alloc
 *
 */
Int  GENCTL_numAlloc(void)
{
  Int totalBufs = 0;

  /* One Buffer for GENCTL_Obj */
  totalBufs++;

  return(totalBufs);
}

/*
 *  ======== GENCTL_alloc ========
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
 *  @param[in]  **pf         pointer to parent IALG (not used by GENCTL)
 *
 *  @param[in,out] memTab[]  memory records
 *
 *  @return     number of initialized memory records
 *
 *  @sa         None
 *
 */
Int GENCTL_alloc(const IALG_Params *algParams,
                 IALG_Fxns **pf, IALG_MemRec memTab[])
{
  const GENCTL_Params *allocParamsPtr = (GENCTL_Params *) algParams;
  Int nbufs = 0;
  Int totalBufs = 0;

  if (allocParamsPtr == NULL) {
    allocParamsPtr = &GENCTL_PARAMS;
  }

  /* Get for GENCTL */
  memTab[0].size = sizeof(GENCTL_Obj);
  memTab[0].alignment = 4;
  memTab[0].space = IALG_EXTERNAL;
  memTab[0].attrs = IALG_PERSIST;
  memTab[0].base = 0;
  totalBufs++;

  totalBufs += nbufs;

  return (totalBufs);

}

/*  ======== GENCTL_initObj ========
 *  Initialize the memory allocated for our instance.
 */
/**
 *  @brief      Initialize an algorithm's instance object.
 *
 *              Performs all initialization necessary to complete the
 *              run-time creation of an algorithm's instance object.  After a
 *              successful return from GENCTL_init(), the algorithm's instance
 *              object is ready to be used to process data.
 *
 *  @param[in]  handle          Algorithm instance handle. This is a pointer
 *                              to an initialized GENCTL_Obj structure.  Its
 *                              value is identical to memTab[0].base.
 *
 *  @param[in]  memTab          Array of allocated buffers.
 *
 *  @param[in]  parent          Handle of algorithm's parent instance (not used by GENCTL).
 *
 *  @param[in]  params          Pointer to algorithm's instance parameters.
 *
 *  @return     IALG_EOK or IALG_EFAIL.
 *
 */
Int GENCTL_initObj(IALG_Handle handle, const IALG_MemRec memTab[],
                          IALG_Handle p, const IALG_Params *params)
{
  //GENCTL_Obj *genCtlInstPtr = (GENCTL_Obj *) handle;
  const GENCTL_Params *newParamsPtr = (GENCTL_Params *) params;

  if (newParamsPtr == NULL) {
    newParamsPtr = &GENCTL_PARAMS; //default
  }

  /* Init GENCTL_Obj */
  //memTab[0] is nothing but GENCTL_Obj  

  return(IALG_EOK);
}

/*
 *  ======== GENCTL_free ========
 */
/**
 *  @brief      get algorithm object's memory requirements
 *
 *              Returns a table of memory records that describe the
 *              size, alignment, type and memory space of all buffers
 *              previously allocated for the algorithm's instance.
 *
 *  @param[in]  handle    Algorithm instance handle.  This is a pointer
 *                        to an initialized GENCTL_Obj structure.  Its
 *                        value is identical to memTab[0].base.
 *
 *  @param[out] memTab    Contains pointers to all of the memory passed
 *                        to the algorithm via GENCTL_init().
 *
 *  @return     number of initialized memory records
 *
 *  @sa         None
 *
 */
Int GENCTL_free(IALG_Handle handle, IALG_MemRec memTab[])
{
  Int nbufs = 0;
  memTab[0].base = handle;
  nbufs++;

  return(nbufs); 
}

/*  ======== GENCTL_Control ========
 *  Controls GENCTL state.
 */
XDAS_Int32 GENCTL_Control(IECO_Handle handle, IECO_Cmd command, IECO_DynamicParams *dynParams, IECO_Status *status)
{
    GENCTL_Obj *genCtlInst = (GENCTL_Obj *)handle;
    Int32 ret = IALG_EOK;
    GENCTL_ControlParams *ctrlParams = (GENCTL_ControlParams *)(dynParams->buf);

    if(genCtlInst == NULL)
        return IALG_EFAIL;

    switch(ctrlParams->controlId)
    {
        case GENCTL_DEVNODE_CTL_ID:
        {            
            DEVNODE_Ctrl_InArgs  devnodeCtrlInArgs;
            DEVNODE_Ctrl_OutArgs devnodeCtrlOutArgs;
            DEVNODE_Handle       devHandle;
            
            devnodeCtrlInArgs.size = sizeof(DEVNODE_Ctrl_InArgs);
            devnodeCtrlInArgs.messageSize = dynParams->bufSize-sizeof(GENCTL_ControlParams);
            devnodeCtrlInArgs.message = (XDAS_UInt8 *)dynParams->buf+sizeof(GENCTL_ControlParams);    
            devHandle = (DEVNODE_Handle)ctrlParams->handle;

            devnodeCtrlOutArgs.size = sizeof(DEVNODE_Ctrl_OutArgs);
            devnodeCtrlOutArgs.messageSize = status->bufSize;
            devnodeCtrlOutArgs.message = (XDAS_UInt8 *)status->buf;

            ret = DEVNODE_control(devHandle, command, &devnodeCtrlInArgs, &devnodeCtrlOutArgs);
        }
        break;

        case GENCTL_ECO_CTL_ID:
        {
            ECO_DynamicParams ecoDynParams;
            ECO_Status        ecoStatus;
            ECO_Handle        ecoHandle;

            ecoDynParams.size    = sizeof(ECO_DynamicParams);
            ecoDynParams.bufSize = dynParams->bufSize-sizeof(GENCTL_ControlParams);
            ecoDynParams.buf     = (XDAS_Int8 *)dynParams->buf+sizeof(GENCTL_ControlParams);
            ecoHandle            = (ECO_Handle)ctrlParams->handle;
            
            ecoStatus.size    = (XDAS_Int32)(sizeof(ECO_Status));
            ecoStatus.buf     = (XDAS_Int8 *)status->buf;
            ecoStatus.bufSize = (XDAS_Int32)status->bufSize;

            ret = ECO_control(ecoHandle, command, &ecoDynParams, &ecoStatus);
        }
        break;
    }

    return (ret);
}


XDAS_Int32 GENCTL_Process(IECO_Handle handle, IECO_InArgs *inArgs, IECO_OutArgs *outArgs)
{
  return (IALG_EOK);
}

