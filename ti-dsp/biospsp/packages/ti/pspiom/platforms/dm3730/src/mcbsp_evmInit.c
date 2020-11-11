/*

 * mmcsd_evmInit.c\system32\cmd.
 *
 * This file contains MMCSD application related EVM (platform) specific routines
 * implementation. 
 * This is part of the EVM Initialization library (evmInit) which contains pinmux
 * enabling routines for peripherals used by the application, and may contai%systemroot%n
 * related device pre-driver initialization routines.
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
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
 *  \file   mcbsp_evmInit.c
 *
 *  \brief  This file contains the board specific code for enabling the use of
 *          mcbsp driver.
 *
 *  (C) Copyright 2008, Texas Instruments, Inc
 *
 */

#include <std.h>
#include "ti/pspiom/platforms/dm3730/Mcbsp_evmInit.h"
#include "ti/pspiom/mcbsp/Mcbsp.h"
#include "ti/pspiom/cslr/soc_iva2.h"
#include "ti/pspiom/cslr/operations.h"
#include "ti/pspiom/prcm/Prcm.h"


/*============================================================================*/
/*                          MACRO DEFINITIONS                                 */
/*============================================================================*/
extern Mcbsp_srgConfig		mcbspSrgParams;
/*
 * Function to enable the pinmux for the mcbsp device.
 *
 */
void configureMcbsp(void)
{
#ifdef MCBSP_MASTER
       /*Need to be implemented for the master mode operation */
           /* Enable 96M FCLK for MCBSP testing for now */
           SETBIT_REGL(CM_CLKSEL1_PLL, CM_CLKSEL1_PLL_SOURCE_96M);

       /* Configure MCBSP CLK SOURCE                                                  */
           CLRBIT_REGL(CSL_CONTROL_DEVCONF0, CONTROL_DEVCONF0_MCBSP1_CLKS_MASK);

           /* For loopbakc testing */
           SETBIT_REGL(CSL_CONTROL_DEVCONF0, CONTROL_DEVCONF0_MCBSP1_CLKR_MASK);
           SETBIT_REGL(CSL_CONTROL_DEVCONF0, CONTROL_DEVCONF0_MCBSP1_FSR_MASK);
#else
       /* McBSP CLK source is external     */
           /* For loopbakc testing */
           SETBIT_REGL(CSL_CONTROL_DEVCONF0, CONTROL_DEVCONF0_MCBSP1_CLKR_MASK);
           SETBIT_REGL(CSL_CONTROL_DEVCONF0, CONTROL_DEVCONF0_MCBSP1_FSR_MASK);
#endif
}
/* ========================================================================== */
/*                              END OF FILE                                   */
/* ========================================================================== */
