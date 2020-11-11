/*
 * Prcm.h
 *
 * This file contains Application programming interface for the Prcm driver and
 * command/macro definitions used by the Prcm driver.
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/**
 * \file        Prcm.h
 *
 * \brief       Prcm driver interface definition file
 *
 *              This file contains the interfaces, data types and symbolic
 *              definitions that are needed by the application to utilize the
 *              services of the Prcm device driver.
 *
 *              (C) Copyright 2012, Texas Instruments, Inc
 *
 *  \authors    Platform Support Group
 *
 *  \note       Set tabstop to 4 (:se ts=4) while viewing this file in an
 *              editor
 *
 *  \version    0.1   created newly
 *
 */

#ifndef _PRCM_H_
#define _PRCM_H_

#include <std.h>
#include <ti/pspiom/cslr/cslr.h>
#include <ti/pspiom/cslr/tistdtypes.h>
/*============================================================================*/
/*                            INCLUDE FILES                                   */
/*============================================================================*/




#ifdef __cplusplus
extern "C" {
#endif


/*============================================================================*/
/*                         ENUMERATED DATA TYPES                              */
/*============================================================================*/
typedef enum {
	Prcm_moduleClkCtrl_ENABLE			= 0,
	Prcm_moduleClkCtrl_DISABLE			= 1
} Prcm_moduleClkCtrl;

/* This is to enable the DSP peripherals */
Void Prcm_McBSP_Enable();

Void Prcm_McBSP_Disable();

/**
 *   The function enables/disables the clock power domain of a particular module
 *   depending on the moduleID passed.
 */
Void Prcm_ModuleClkCtrl(Uint32                   domain,
		                Uint32                   moduleId,
                        Prcm_moduleClkCtrl        clkCtrl);


/*============================================================================*/
/*                         CONSTANTS                                          */
/*============================================================================*/

/* PRCM */
#define CM_FCLKEN_IVA2      0x48004000
#define CM_CLKEN_PLL_IVA2   0x48004004
#define CM_IDLEST_PLL_IVA2  0x48004024
#define CM_CLKSEL1_PLL_IVA2 0x48004040
#define CM_CLKSEL2_PLL_IVA2 0x48004044
#define CM_CLKEN_PLL_MPU    0x48004904
#define CM_IDLEST_PLL_MPU   0x48004924
#define CM_CLKSEL1_PLL_MPU  0x48004940
#define CM_CLKSEL2_PLL_MPU  0x48004944
#define CM_FCLKEN1_CORE     0x48004a00

#define EN_MCBSP1_FCLK_BIT  9

#define CM_ICLKEN1_CORE     0x48004a10

#define EN_MCBSP1_ICLK_BIT  9

#define CM_ICLKEN2_CORE     0x48004a14
#define CM_CLKSEL_CORE      0x48004a40
#define CM_FCLKEN_GFX       0x48004b00
#define CM_ICLKEN_GFX       0x48004b10
#define CM_CLKSEL_GFX       0x48004b40
#define CM_FCLKEN_WKUP      0x48004c00
#define CM_ICLKEN_WKUP      0x48004c10
#define CM_CLKSEL_WKUP      0x48004c40
#define CM_IDLEST_WKUP      0x48004c20
#define CM_CLKEN_PLL        0x48004d00
#define CM_IDLEST_CKGEN     0x48004d20
#define CM_CLKSEL1_PLL      0x48004d40

/* To driver internal clock to McBSP for testing */
#define CM_CLKSEL1_PLL_SOURCE_96M		10

#define CM_CLKSEL2_PLL      0x48004d44
#define CM_CLKSEL3_PLL      0x48004d48
#define CM_FCLKEN_DSS       0x48004e00
#define CM_ICLKEN_DSS       0x48004e10
#define CM_CLKSEL_DSS       0x48004e40
#define CM_FCLKEN_CAM       0x48004f00
#define CM_ICLKEN_CAM       0x48004f10
#define CM_CLKSEL_CAM       0x48004F40
#define CM_FCLKEN_PER       0x48005000

#define EN_MCBSP2_FCLK_BIT  0
#define EN_MCBSP4_FCLK_BIT  2

#define CM_ICLKEN_PER       0x48005010

#define EN_MCBSP2_ICLK_BIT  0
#define EN_MCBSP4_ICLK_BIT  2

#define CM_CLKSEL_PER       0x48005040
#define CM_CLKSEL1_EMU      0x48005140


// NEON_CM Register Mapping
#define CM_IDLEST_NEON             0x48005320
#define CM_CLKSTCTRL_NEON          0x48005348

// USBHOST_CM Register Mapping
#define CM_FCLKEN_USBHOST          0x48005400
#define CM_ICLKEN_USBHOST          0x48005410
#define CM_IDLEST_USBHOST          0x48005420
#define CM_AUTOIDLE_USBHOST        0x48005430
#define CM_SLEEPDEP_USBHOST        0x48005444
#define CM_CLKSTCTRL_USBHOST       0x48005448
#define CM_CLKSTST_USBHOST         0x4800544C


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* _MCBSP_H_ */

/*============================================================================*/
/*                         END OF FILE                                        */
/*============================================================================*/
