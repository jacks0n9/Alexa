/* ========================================================================== */
/**
 *  @file   misc/miscwdt.h
 *
 *  @brief  The Control Interface definitions for Audio Processing Unit
 *
 *  ============================================================================
 */
/* --COPYRIGHT--,BSD
 * Copyright (c) 2009, Texas Instruments Incorporated
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

/**
 *  @defgroup   misc_MISCWDT    MISC Control/Message defines
 *
 *  This deines all the messages and controls for the MISC.
 */

#ifndef _MISC_MISCWDT_H
#define _MISC_MISCWDT_H

#include <xdc/std.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    misc_MISCWDT */
/*@{*/


/**
 * @brief  MISC Parameters needed during instance creation
 */
/*  Note: WDT1_CLK        100MHz */
#define COUNT_half_MS   50000       /**< prescale to 0.5ms=500us*/
#define MAX_WDT_PERIOD  30000           /**< MAX period is 30s */
#define MIN_WDT_PERIOD  1000            /**< MIN period is 1s */
#define DEFAULT_WDT_PERIOD 10000   /**< default period is 10s */



/**
 * @brief  DSP WDT control message to pass control parameters to DSP
 */
typedef struct WDT_Ctrl {
    Uint16 enable;
#define WDT_ENABLE 1
#define WDT_DISABLE 0
/** @brief  timeout period in ms */
    Uint16 period;
}WDT_Ctrl;


/**
 * @brief  DSP WDT status
 */
typedef enum 
{
   /** @brief  DSP WDT status is disabled */
    WDT_STATUS_DISABLED, 
   /** @brief  DSP WDT status is enabled */
    WDT_STATUS_ENABLED,
   /** @brief  DSP WDT status is failed to start */
    WDT_STATUS_FAILED_STARTED
}WDT_Status;


/**
 * @brief  DSP WDT response messages returned by DSP upon query
 */
typedef struct WDT_Resp{
   /** @brief  DSP WDT status */
    WDT_Status wdtStatus;
   /** @brief  internal counters for number of kicks happened. */
    Uint32 kicks;
}WDT_Resp;

/* @} */ /* ingroup */

#ifdef __cplusplus
}
#endif

#endif /* (_MISC_MISCWDT_H) */

