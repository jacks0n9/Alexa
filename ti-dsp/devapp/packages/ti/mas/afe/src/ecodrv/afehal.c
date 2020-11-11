/* ========================================================================== */
/**
 *  @file   afehal.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afehal.c
 *
 *  @brief  Implements HAL initialization functionality
 *
 *          This file contains functions that are used to initialize
 *          the HAL. They are called only once at system startup
 *          and so can be overlayed with an uninitialized data section
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/* -- Standard C Headers -- */
#include <xdc/std.h>

#include <stdio.h>
#include <stdlib.h>

/* -- AFE Header -- */
#include <afe.h>

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- AFE Local Headers -- */
#include <src/hal.h>
#include <src/afeloc.h>
#include <src/afedbg.h>
#include <src/afeswcfg.h>
#include <src/afemem.h>

#define TIMER_FREQ 1000 //SachinD: THIS NEEDS TO BE FIXED
#define TICKS_TO_USEC(ticks)   (ticks)/(TIMER_FREQ)
#define USEC_TO_TICKS(usec)    (usec)*(TIMER_FREQ)

/* ============ AFE_initHal ========== */
/**
 *  @brief      HAL APIs are initilized and memory configuration performed
 *
 *  @param[in]  obj    Pointer to AFE_Object
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
AFE_Error AFE_initHal (AFE_Object *obj)
{
  obj->halApi = &hal;
  return (AFE_SUCCESS);
}

/* ============ AFE_subRateFcn ========== */
/**
 *  @brief      Periodic call to MSP for VPU operation
 *
 *              This function is called from the halTdmDmaIsr at every ISR interval
 *
 *  @param[in]  handle  Void pointer to AFE_Object structure
 *
 *  @param[out] ts      Timestamp
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
void AFE_subRateFcn (void *handle, tuint ts, Uint32 ticks)
{
  AFE_Object    *obj = (AFE_Object *)handle;
  ts = ts;    /* To avoid compiler warning */

  /* Wake up message processing AFE task if there are no active channels */
  if (obj->numActiveChannels == 0) {
    SEM_postBinary (&(obj->afeNotifyObj.afeSemObj));
  }

  /* calculate statistics */
  if (obj->isrStats.firstIsr == 1)  
  {  
    obj->isrStats.dtIsr = (Uint32)USEC_TO_TICKS(AFE_TDMISRINTERVALMS*1000);  
    obj->isrStats.dtIsrMin = obj->isrStats.dtIsr;
    obj->isrStats.firstIsr = 0;  
  }  
  else  
  {  
    obj->isrStats.dtIsr = ticks - obj->isrStats.tIsr;  
  }  
  if (obj->isrStats.dtIsr > obj->isrStats.dtIsrMax)  
    obj->isrStats.dtIsrMax = obj->isrStats.dtIsr;

  if (obj->isrStats.dtIsr > ((Uint32)USEC_TO_TICKS(AFE_TDMISRINTERVALMS*1100)))  //check if ISR occured after the expected time - > +10% variation (2.75ms)
    obj->isrStats.dtIsrErrP++;

  if (obj->isrStats.dtIsr < obj->isrStats.dtIsrMin)  
    obj->isrStats.dtIsrMin = obj->isrStats.dtIsr;

  if (obj->isrStats.dtIsr < ((Uint32)USEC_TO_TICKS(AFE_TDMISRINTERVALMS*900)))  //check if ISR occured before the expected time - < -10% variation (2.25ms)
    obj->isrStats.dtIsrErrN++;
  
  obj->isrStats.tIsr = ticks;  
  obj->isrStats.isrCnt++;
}

/* ============ AFE_newHal ========== */
/**
 *  @brief      Initializes HAL (Hardware Abstraction Layer)
 *
 *              HAL memory is allocated and HAL is configured
 *              NOTE: For Davinci IPP one one TDM port and one AIC port
 *              is supported
 *
 *  @param[in]    obj    Pointer to AFE_Object
 *
 *  @param[out]   revId     Chip revision ID
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
AFE_Error AFE_newHal (AFE_Object *obj, tuint *revId)
{
  AFE_Error       afeStatus = AFE_SUCCESS;
    FIX_COMPIL_WARNING(obj);
    FIX_COMPIL_WARNING(revId);

  return (afeStatus);
}

/* Nothing beyond this line */
