/*
 * i2cSample_main.c
 *
 * This file contains the test / demo code to demonstrate the I2C driver functi- 
 * nality in Interrupt mode of operation on DSP/BIOS 5.
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

/** \file   i2csample_main.c
 *
 *  \brief  This file contains the sample code to demonstrate the usage of the
 *          I2C driver implemented for DSP/BIOS
 *
 *  (C) Copyright 2008, Texas Instruments, Inc
 *
 *  \author     PSG
 *
 *  \version    0.1  Created newly for the OMAPL138 platform
 */

/* ========================================================================== */
/*                          INCLUDE FILES                                     */
/* ========================================================================== */

#include <std.h>
#include <stdio.h>
#include <string.h>
#include <sem.h>
#include <sys.h>
#include <bios.h>
#include <tsk.h>
#include <ecm.h>

#include <log.h>
#include "../../../i2c/I2c.h"
#include "../../../prcm/Prcm.h"


LOG_Obj trace;

void echo(void);

void user_init();

static void tskHeartBeat();

void i2c_test();

void main()
{
	echo();
    user_init();
	i2c_test();
    tskHeartBeat();
    return;
}

void echo()
{
	printf("\r\n I2C Sample Application\n\r");
}

/*
 * init function called when creating the driver.
 */
void user_init()
{
    /* enable the I2C and McBSP1 in the PRCM module  */
	Prcm_ModuleClkCtrl(CM_FCLKEN1_CORE, 15, Prcm_moduleClkCtrl_ENABLE);
	Prcm_ModuleClkCtrl(CM_ICLKEN1_CORE, 15, Prcm_moduleClkCtrl_ENABLE);
	Prcm_ModuleClkCtrl(CM_FCLKEN1_CORE, 9, Prcm_moduleClkCtrl_ENABLE);
	Prcm_ModuleClkCtrl(CM_ICLKEN1_CORE, 9, Prcm_moduleClkCtrl_ENABLE);

	i2c_set_bus_num(1);

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

//	i2c_probe(0x48);
}


void i2c_test()
{
	unsigned char value[0x10] = {0x3, 0x1};

	unsigned char new_value[0x10] = 46;

    i2c_read (0x4b, 0x16, 0x0, value, 0x1);

    printf("\r\nReturned Value %d\n\r", value[0]);


    i2c_write (0x4b, 0x16, 0x0, new_value, 0x1);

    i2c_read (0x4b, 0x16, 0x0, value, 0x1);

    printf("\r\nReturned Value %d\n\r", value[0]);

    return;
}

void tskHeartBeat()
{
    static Uint32 counter = 0;

    while (counter < 0xFFFFFFFF)
    {
        TSK_sleep(1000u);    /* Add large delay */
        printf("\r\n\r\n!!! PSP HrtBt %l", counter);
        counter++;
    }
}
