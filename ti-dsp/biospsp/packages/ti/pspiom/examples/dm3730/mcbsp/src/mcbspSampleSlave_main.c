/*
 * mcbspSampleSlave_main.c
 *
 * This file contains the test / demo code to demonstrate the McBSP driver
 * slave functionality on DSP/BIOS 5.
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
 * \file    mcbspSample_main.c
 *
 * \brief   McBSP sample application
 *
 *          This file demonstrates the use of Mcbsp by using an
 *          EVM to EVM communication setup.
 *
 *          (C) Copyright 2009, Texas Instruments, Inc
 *
 * \author  PSP
 *
 * \version 0.1
 */

/* ========================================================================== */
/*                            INCLUDE FILES                                   */
/* ========================================================================== */

#include <log.h>
#include <std.h>
#include <ti/pspiom/mcbsp/Mcbsp.h>
#include <ti/pspiom/platforms/dm3730/Mcbsp_evmInit.h>
#include "ti/pspiom/cslr/soc_iva2.h"
#include "ti/pspiom/cslr/operations.h"
/* ========================================================================== */
/*                           MACRO DEFINTIONS                                 */
/* ========================================================================== */

extern LOG_Obj trace;

/*
 * Mcbsp device params. To be filled in userMcbspInit function which
 * is called before driver creation
 */
Mcbsp_Params  mcbspParams;

/*
 * Mcbsp sample rate configuration parameters.
 */
Mcbsp_srgConfig  mcbspSrgParams;

/* ========================================================================== */
/*                           FUNCTION DEFINITIONS                             */
/* ========================================================================== */


/**
 * \brief  Void main(Void)
 *
 *         Main function of the sample application. This function calls the
 *         function to configure the mcbsp instance

 * \param  None
 * \return None
 */

/*
 * Mcbsp init function called when creating the driver.
 */
void mcbspUserInit()
{

#ifdef BIOS_PWRM_ENABLE
	/* enable the EDMA in the PSC module                                      */
    PWRM_setDependency(PWRM_RSRC_EDMA3_CC_0);
    PWRM_setDependency(PWRM_RSRC_EDMA3_TC_0);
    PWRM_setDependency(PWRM_RSRC_EDMA3_TC_1);
    PWRM_setDependency(PWRM_RSRC_EDMA3_TC_2);
#endif

    Mcbsp_init();

    /* configure the SRG properties                                           */
    /* This is dummy code, not required for slave mode      */

    mcbspParams = Mcbsp_PARAMS;
    mcbspParams.mode = Mcbsp_DevMode_McBSP;
    mcbspParams.opMode = Mcbsp_OpMode_DMAINTERRUPT;
    mcbspParams.enablecache = TRUE;
    mcbspParams.emulationMode = Mcbsp_EmuMode_FREE;
    mcbspParams.dlbMode = Mcbsp_Loopback_DISABLE;
    mcbspParams.clkStpMode = Mcbsp_ClkStpMode_DISABLED;
    mcbspParams.srgSetup = &mcbspSrgParams;
    mcbspParams.prcmPwrmEnable = FALSE;
}

Void main(Void)
{
    LOG_printf(&trace,"Mcbsp Sample Application\n");

    /* enable the pinmux for the mcbsp device                                 */
	   /* McBSP CLK source is external 	*/
	   	/* For loopback testing */
	   	SETBIT_REGL(CSL_CONTROL_DEVCONF0, CONTROL_DEVCONF0_MCBSP1_CLKR_MASK);
	   	SETBIT_REGL(CSL_CONTROL_DEVCONF0, CONTROL_DEVCONF0_MCBSP1_FSR_MASK);
    return;
}
/* ========================================================================== */
/*                                END OF FILE                                 */
/* ========================================================================== */
