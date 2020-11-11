/* ========================================================================== */
/**
 *  @file   dspwdt.c
 *
 *  path    ti/mas/misc/src/dspwdt.c
 *
 *  @brief  It implements APIs for setting DSP watchdog registers
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

typedef struct __WDT_REGS__ {
    volatile Uint32    kick_lock;
    volatile Uint32    kick;
    volatile Uint32    change_lock;   
    volatile Uint32    change;   
    volatile Uint32    disable_lock;
    volatile Uint32    disable;   
    volatile Uint32    prescale_lock;
    volatile Uint32    prescale;    
} WDT_REGS, *PWDT_REGS;

#define WDT1_BASE     0x08086800

static volatile PWDT_REGS wdt_regs=(PWDT_REGS)WDT1_BASE;

void WDT_kick(void){
	wdt_regs->kick_lock = 0x5555;
	wdt_regs->kick_lock = 0xaaaa;
	wdt_regs->kick      = 0x0; // it can be any value;
}

void WDT_change(Uint32 value){
	wdt_regs->change_lock = 0x6666;
	wdt_regs->change_lock = 0xbbbb;
	wdt_regs->change      = value;
}

void WDT_control(Uint32 enable){
	wdt_regs->disable_lock = 0x7777;
	wdt_regs->disable_lock = 0xcccc;
	wdt_regs->disable_lock = 0xdddd;
	wdt_regs->disable      = (enable)? 1: 0;   /* disable bit =1 means enable */
}

void WDT_prescale(Uint32 value){
	wdt_regs->prescale_lock = 0x5a5a;
	wdt_regs->prescale_lock = 0xa5a5;
	wdt_regs->prescale      = value;
}





