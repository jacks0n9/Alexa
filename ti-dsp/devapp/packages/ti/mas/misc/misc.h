/* ========================================================================== */
/**
 *  @file   misc/misc.h
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
 *  @defgroup   misc_MISC    MISC Control/Message defines
 *
 *  This deines all the messages and controls for the MISC.
 */

#ifndef _MISC_MISC_H
#define _MISC_MISC_H

#include <xdc/std.h>

#include "ti/mas/eco/eco.h"
#include "ti/mas/misc/miscwdt.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    misc_MISC */
/*@{*/


/**
 * @brief  MISC control ID
 */
#define MISC_MSG_WDT                              0x0001
#define MISC_MSG_SIMULATE_DSPTIMEOUT              0x0002



/**
 * @brief  MISC Parameters needed during instance creation
 */
typedef struct MISC_Params {
    ECO_Params      params; /**< First Element of the derived class */
} MISC_Params;



/**
 * @brief  MISC module's Control Parameters
 */
typedef struct MISC_ControlParams {
    Uint16 controlId;
    union
    {
        WDT_Ctrl               wdtCtrl;
    }ctrl;
} MISC_ControlParams;



/**
 * @brief  MISC module's Response.
 */
typedef struct MISC_ResponseParams {
    Uint16                responseId;
    union 
    {
      WDT_Resp           wdtResp;
    } resp;
} MISC_ResponseParams;


extern IECO_Fxns MISC_TI_IECO;

/* @} */ /* ingroup */

#ifdef __cplusplus
}
#endif

#endif /* (_MISC_MISC_H) */

