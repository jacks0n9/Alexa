/* ========================================================================== */
/**
 *  @file   genCtl/genCtl.h
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
 *  @defgroup   genCtl_GENCTL    GENCTL Control/Message defines
 *
 *  This deines all the messages and controls for the GENCTL.
 */

#ifndef _GENCTL_GENCTL_H
#define _GENCTL_GENCTL_H

#include <xdc/std.h>
#include <ti/sdo/ce/visa.h>
#include <ti/mas/eco/eco.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    genCtl_GENCTL */
/*@{*/

/** @brief GENCTL Control IDs for request control messages received from user application */
#define GENCTL_DEVNODE_CTL_ID   0x1
#define GENCTL_ECO_CTL_ID       0x2

/**
 * @brief  GENCTL Parameters needed during instance creation
 */
typedef struct GENCTL_Params {
    ECO_Params      params; /**< First Element of the derived class */
} GENCTL_Params;

/**
 * @brief  GENCTL module's Control Parameters
 */
typedef struct GENCTL_ControlParams {    
    UInt32              controlId;
    VISA_Handle         handle;
}GENCTL_ControlParams;

extern IECO_Fxns GENCTL_TI_IECO;

/* @} */ /* ingroup */

#ifdef __cplusplus
}
#endif

#endif /* (_GENCTL_GENCTL_H) */


