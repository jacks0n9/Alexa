/*
 * Prcm.c
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
 * \file        Prcm.c
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

#include <std.h>
#include <stdio.h>
#include <string.h>
#include <gio.h>
#include <log.h>
#include <tsk.h>

#include <ti/pspiom/prcm/Prcm.h>

#define __raw_readl(a)    (*(volatile unsigned int *)(a))
#define __raw_writel(v,a) (*(volatile unsigned int *)(a) = (v))
#define __raw_readw(a)    (*(volatile unsigned short *)(a))
#define __raw_writew(v,a) (*(volatile unsigned short *)(a) = (v))

/*****************************************************************
 * sr32 - clear & set a value in a bit range for a 32 bit address
 *****************************************************************/
void sr32(Uint32 addr, Uint32 start_bit, Uint32 num_bits, Uint32 value)
{
		Uint32 tmp, msk = 0;
        msk = 1 << num_bits;
        --msk;
        tmp = __raw_readl(addr) & ~(msk << start_bit);
        tmp |=  value << start_bit;
        __raw_writel(tmp, addr);
}

/**
 *  \fn     Void Prcm_ModuleClkCtrl()
 *
 *  \brief  Function which enables or disables the clock power domain.
 *
 *   The function enables/disables the clock power domain of a particular module
 *   depending on the moduleID passed.
 *
 *  \param  domain      [IN] domain of the module to be controlled
 *  \param  moduleId    [IN] module to be controlled
 *  \param  clkCtrl     [IN] command (clock enable or disable)
 *
 *  \enter  instHandle       should be valid and not null
 *          moduleId         should be a valid module ID for this PRCM instance
 *          clkCtrl          should be control command of type Prcm_moduleClkCtrl
 *          eb               if null raises error and aborts program execution
 *                           if not null and valid, raises error & shall allow
 *                           continuation of exectuion
 *  \leave  not implemented
 *  \return DriverTypes_COMPLETED or ERROR
 */
Void Prcm_ModuleClkCtrl(Uint32                   domain,
		                Uint32                   moduleId,
                        Prcm_moduleClkCtrl        clkCtrl)
{
    volatile Uint32   count    = 0;
    Uint32            hwiKey   = 0x00;


    /* protect the dependency count                                       */
    hwiKey = (Uint32)_disable_interrupts();

    /* check if the command issued is for clock enable or disable         */
    if (Prcm_moduleClkCtrl_ENABLE == clkCtrl)
    {
        /* Enable module clocks */
        sr32(domain, moduleId, 1, 0x1);
        sr32(domain, moduleId, 1, 0x1);
    }
    else
    {
        /* Disable module clocks */
        sr32(domain, moduleId, 1, 0x0);
        sr32(domain, moduleId, 1, 0x0);
    }

    _restore_interrupts(hwiKey);

}


/* ========================================================================== */
/*                                END OF FILE                                 */
/* ========================================================================== */













