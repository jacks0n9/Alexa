/* ========================================================================== */
/**
 *  @file   misc_ialg.c
 *
 *  path    ti/mas/misc/src/misc_ialg.c
 *
 *  @brief  It implements miscellaneous DSP features (e.g. DSP watchdog timer)
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
#include <ti/mas/misc/misc.h>
#include "misc_priv.h"
#include <ti/xdais/dm/xdm.h>

#define KICK_TSK
#define IALG_MISC_FXNS \
    &MISC_TI_IECO,       /* module ID */                         \
    NULL,            /* activate */                          \
    MISC_alloc,       /* alloc */                             \
    NULL,            /* control (NULL => no control ops) */  \
    NULL,            /* deactivate */                        \
    MISC_free,        /* free */                              \
    MISC_initObj,     /* init */                              \
    NULL,            /* moved */                             \
    MISC_numAlloc     /* numAlloc (NULL => IALG_MAXMEMRECS) */\

/*
 *  ======== MISC_IMISC ========
 */
extern IECO_Fxns MISC_TI_IECO = {     /* module_vendor_interface */
    IALG_MISC_FXNS,
    MISC_Process, 
    MISC_Control,
};

MISC_Params MISC_PARAMS = {
  sizeof(MISC_Params), NULL, NULL
};

static Uint32 num_kicks=0;

Int32 MISC_wdtctrl(MISC_Obj *miscInst , WDT_Ctrl *wdtctrl);
void dspWdtfxn(MISC_Obj *miscInst , Uint32 period);
void dspWdtctrl(MISC_Obj *miscInst , Uint32 period);
#ifdef TEST_DSP_TIMEOUT
void MISC_simulateDSPTimeout(MISC_Obj *miscInst);
#endif


/*  ======== PSIC_numAlloc ========
 *  Returns Number of Buffers Required by MISC for a particular configuration.
 */
/**
 *  @brief      Returns the maximum number of memory records that can be 
 *              processed by miscAlloc().
 *
 *  @sa         miscAlloc
 *
 */
Int  MISC_numAlloc(void)
{
  Int totalBufs = 0;

  /* One Buffer for MISC_Obj */
  totalBufs++;

  return(totalBufs);
}

/*
 *  ======== MISC_alloc ========
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
 *  @param[in]  **pf         pointer to parent IALG (not used by MISC)
 *
 *  @param[in,out] memTab[]  memory records
 *
 *  @return     number of initialized memory records
 *
 *  @sa         None
 *
 */
Int MISC_alloc(const IALG_Params *algParams,
                 IALG_Fxns **pf, IALG_MemRec memTab[])
{
  const MISC_Params *allocParamsPtr = (MISC_Params *) algParams;
  Int nbufs = 0;
  Int totalBufs = 0;

  if (allocParamsPtr == NULL) {
    allocParamsPtr = &MISC_PARAMS;
  }

  /* Get for MISC */
  memTab[0].size = sizeof(MISC_Obj);
  memTab[0].alignment = 4;
  memTab[0].space = IALG_EXTERNAL;
  memTab[0].attrs = IALG_PERSIST;
  memTab[0].base = 0;
  totalBufs++;

  totalBufs += nbufs;

  return (totalBufs);

}

/*  ======== MISC_initObj ========
 *  Initialize the memory allocated for our instance.
 */
/**
 *  @brief      Initialize an algorithm's instance object.
 *
 *              Performs all initialization necessary to complete the
 *              run-time creation of an algorithm's instance object.  After a
 *              successful return from miscInit(), the algorithm's instance
 *              object is ready to be used to process data.
 *
 *  @param[in]  handle          Algorithm instance handle. This is a pointer
 *                              to an initialized miscXdaisInst_t structure.  Its
 *                              value is identical to memTab[0].base.
 *
 *  @param[in]  memTab          Array of allocated buffers.
 *
 *  @param[in]  parent          Handle of algorithm's parent instance (not used by MISC).
 *
 *  @param[in]  params          Pointer to algorithm's instance parameters.
 *
 *  @return     IALG_EOK or IALG_EFAIL.
 *
 */
Int MISC_initObj(IALG_Handle handle, const IALG_MemRec memTab[],
                          IALG_Handle p, const IALG_Params *params)
{
  MISC_Obj *miscInstPtr = (MISC_Obj *) handle;
  const MISC_Params *newParamsPtr = (MISC_Params *) params;

  if (newParamsPtr == NULL) {
    newParamsPtr = &MISC_PARAMS; //default
  }

  /* Init MISC_Obj */
  //memTab[0] is nothing but MISC_Obj

  miscInstPtr->wdtStatus = WDT_STATUS_DISABLED;
  WDT_control(WDT_DISABLE);
  miscInstPtr->wdtTskHdl = NULL;

  return(IALG_EOK);
}

/*
 *  ======== MISC_free ========
 */
/**
 *  @brief      get algorithm object's memory requirements
 *
 *              Returns a table of memory records that describe the
 *              size, alignment, type and memory space of all buffers
 *              previously allocated for the algorithm's instance.
 *
 *  @param[in]  handle    Algorithm instance handle.  This is a pointer
 *                        to an initialized miscXdaisInst_t structure.  Its
 *                        value is identical to memTab[0].base.
 *
 *  @param[out] memTab    Contains pointers to all of the memory passed
 *                        to the algorithm via miscInit().
 *
 *  @return     number of initialized memory records
 *
 *  @sa         None
 *
 */
Int MISC_free(IALG_Handle handle, IALG_MemRec memTab[])
{
  Int nbufs = 0;
  memTab[0].base = handle;
  nbufs++;

  return(nbufs); 
}

/*  ======== MISC_Control ========
 *  Controls MISC state.
 */
XDAS_Int32 MISC_Control(IECO_Handle handle, IECO_Cmd command, IECO_DynamicParams *dynParams, IECO_Status *status)
{
  MISC_Obj *miscInst = (MISC_Obj *)handle;
  Int32 ret = IALG_EOK;
  MISC_ControlParams *ctrlParams = (MISC_ControlParams *)(dynParams->buf);
  MISC_ResponseParams *respParams = (MISC_ResponseParams *)(status->buf);

  if(miscInst == NULL)
     return IALG_EFAIL;

  if(command == XDM_SETPARAMS)
  {
    switch(ctrlParams->controlId )
    {
      case MISC_MSG_WDT:
        ret = MISC_wdtctrl(miscInst, &(ctrlParams->ctrl.wdtCtrl));
        respParams->resp.wdtResp.wdtStatus =  miscInst->wdtStatus;
        respParams->resp.wdtResp.kicks =  miscInst->wdt_kicks;
        respParams->responseId = ctrlParams->controlId;     
      break;

      
#if TEST_DSP_TIMEOUT
      case MISC_MSG_SIMULATE_DSPTIMEOUT:
        MISC_simulateDSPTimeout(miscInst);
      break;
#endif    

    }
  }
  else if (command == XDM_GETSTATUS)
  {
    switch(ctrlParams->controlId )
    {
      case MISC_MSG_WDT:
      respParams->resp.wdtResp.wdtStatus =  miscInst->wdtStatus;
#ifdef KICK_TSK
      respParams->resp.wdtResp.kicks =  miscInst->wdt_kicks;
#else  /* KICK_TSK */
      respParams->resp.wdtResp.kicks =  num_kicks;
#endif  /* KICK_TSK */
      respParams->responseId = ctrlParams->controlId;     
      break;
    }
  }
  return (ret);
}


XDAS_Int32 MISC_Process(IECO_Handle handle, IECO_InArgs *inArgs, IECO_OutArgs *outArgs)
{
  return (IALG_EOK);
}

#ifdef TEST_DSP_TIMEOUT
void MISC_simulateDSPTimeout(MISC_Obj *miscInst)
{   
    while (1);
}
#endif


Int32 MISC_wdtctrl(MISC_Obj *miscInst , WDT_Ctrl *wdtctrl)
{
    TSK_Attrs  attrs = TSK_ATTRS;
    UInt32 period = wdtctrl->period;

    attrs.name = "DSPWDT";
    attrs.stacksize = 256;
    attrs.priority = TSK_MAXPRI;
    if(wdtctrl->enable)     /* request to enable watchdog timer */
    {
#ifdef KICK_TSK
        if(!miscInst->wdtTskHdl)
        {
            miscInst->wdtTskHdl = TSK_create((Fxn)dspWdtfxn, &attrs, miscInst, period);
            if ( miscInst->wdtTskHdl == NULL) 
            {
                miscInst->wdtTskHdl = NULL;
                miscInst->wdtStatus = WDT_STATUS_FAILED_STARTED;
                return IALG_EFAIL;
            }
            else
            {
                miscInst->wdtStatus = WDT_STATUS_ENABLED;
                return IALG_EOK;
            }
            
        }
#else  /* KICK_TSK */
        if(miscInst->wdtStatus != WDT_STATUS_ENABLED)
        {
            dspWdtctrl(miscInst, period);
            miscInst->wdtStatus = WDT_STATUS_ENABLED;
            num_kicks=0;
            return IALG_EOK;
        }
#endif  /* KICK_TSK */
        else
        {
            /* timer already enabled, just return */
            return IALG_EOK;
        }
    }
    else    /* request to disable watchdog timer */
    {
        WDT_control(WDT_DISABLE);
        miscInst->wdtStatus = WDT_STATUS_DISABLED;
#ifdef KICK_TSK
        if(miscInst->wdtTskHdl)
        {
            TSK_delete(miscInst->wdtTskHdl);
            miscInst->wdtTskHdl = NULL;
        }
#endif /* KICK_TSK */
        return IALG_EOK;
    }
}


void dspWdtctrl(MISC_Obj *miscInst , Uint32 period)
{
    if(period > MAX_WDT_PERIOD || period < MIN_WDT_PERIOD)
    {
        period = DEFAULT_WDT_PERIOD;
    }
    WDT_prescale(COUNT_half_MS);
    WDT_change(period*2);
    WDT_control(WDT_ENABLE);
}

void dspWdtkick()
{
    WDT_kick();
    num_kicks++;
}

void dspWdtfxn(MISC_Obj *miscInst , Uint32 period)
{
    if(period > MAX_WDT_PERIOD || period < MIN_WDT_PERIOD)
    {
        period = DEFAULT_WDT_PERIOD;
    }
    WDT_prescale(COUNT_half_MS);
    WDT_change(period*2);
    WDT_control(WDT_ENABLE);
    miscInst->wdt_kicks=0;
    
    for(;;)
    {
        WDT_kick();
        miscInst->wdt_kicks++;
        /* BIOS CLK uses default CLK.MICROSECONDS configure: 1000us/tick=1ms/tick  */
        /* We sleep 1/4 of timeout period here */
        TSK_sleep(period/4);
    }
}
