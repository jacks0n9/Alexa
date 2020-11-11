/* 
 *  Copyright 2011 by Texas Instruments Incorporated.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 */

/*
 *  ======== lpm_omap.c ========
 *  LPM driver configured for omap3530 device.
 *
 */


#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#include "lpm_dev.h"
#include "tal_gpp.h"
#include "pwrm_sh.h"


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

#if defined(DEBUG)
#define TRACE hndl->os_trace
#else
#define TRACE(...)
#endif

// OMAP3530 module and mmr addresses (physical)
#define SYSTEM_CONTROL_ADDR     0x48002000
#define SYSTEM_CONTROL_SIZE     0xC00
#define CONTROL_REVISION        0x00000000
#define CONTROL_IVA2_BOOTADDR   0x00000400
#define CONTROL_IVA2_BOOTMOD    0x00000404

#define PRCM_IVA2_CM_ADDR       0x48004000
#define PRCM_IVA2_CM_SIZE       0x800
#define CM_FCLKEN_IVA2          0x00000000
#define CM_CLKEN_PLL_IVA2       0x00000004
#define CM_IDLEST_IVA2          0x00000020
#define CM_AUTOIDLE_PLL_IVA2    0x00000034
#define CM_CLKSEL1_PLL_IVA2     0x00000040
#define CM_CLKSEL2_PLL_IVA2     0x00000044
#define CM_CLKSTCTRL_IVA2       0x00000048

#define PRCM_IVA2_PRM_ADDR      0x48306000
#define PRCM_IVA2_PRM_SIZE      0x800
#define RM_RSTCTRL_IVA2         0x00000050
#define RM_RSTST_IVA2           0x00000058
#define PM_WKDEP_IVA2           0x000000C8
#define PM_PWSTCTRL_IVA2        0x000000E0
#define PM_PWSTST_IVA2          0x000000E4

#define MAX_WAIT_COUNT          0x50000

#define REG(x)              *((volatile unsigned int *) (x))
#define MEM(x)              *((volatile unsigned int *) (x))


#define LPM_DEV_COUNT   1

#define LPM_STATE_RESET         0
#define LPM_STATE_INITIALIZED   0x0001
#define LPM_STATE_CONNECTED     0x0002



/* private data structure */
typedef struct LPM_Private {
    unsigned int        state;
    unsigned long       bootAddr;
    char               *resumeSymbol;
    char               *saveDoneSymbol;
    LPM_ExtPwrOffFxn    lpm_extPwrOff;
    void               *lpm_extPwrOffArg;
    LPM_ExtPwrOnFxn     lpm_extPwrOn;
    void               *lpm_extPwrOnArg;
    unsigned long       controlVA;
    unsigned long       ivacmVA;
    unsigned long       ivaprmVA;
    TAL_Handle          hTAL;
    LPM_RemapFxn        os_remap;
    LPM_SignalFxn       os_signal;
    LPM_UnmapFxn        os_unmap;
    LPM_WaitFxn         os_wait;
    LPM_TraceFxn        os_trace;
    void               *os_instance;
    unsigned int        talData;

    /* os driver object pointer */
    LPM_Device         *driver;

} LPM_Private;

static struct LPM_Private LPM_OBJ[LPM_DEV_COUNT] = {

    /* LPM_OBJ[0] */
    {
        LPM_STATE_RESET,                /* .state               */
        0,                              /* .bootAddr            */
        "__PWRM_restoreJouleIMem",      /* .resumeSymbol        */
        "__PWRM_saveDone",              /* .saveDoneSymbol      */
        NULL,                           /* .lpm_extPwrOff       */
        NULL,                           /* .lpm_extPwrOffArg    */
        NULL,                           /* .lpm_extPwrOn        */
        NULL,                           /* .lpm_extPwrOnArg     */
        0,                              /* .controlVA           */
        0,                              /* .ivacmVA             */
        0,                              /* .ivaprmVA            */
        0,                              /* .hTAL                */
        0,                              /* .os_remap            */
        0,                              /* .os_signal           */
        0,                              /* .os_unmap            */
        0,                              /* .os_wait             */
        0,                              /* .os_trace            */
        0,                              /* .os_instance         */
        0,                              /* .talData             */
        0                               /* .driver              */
    }
};


static void _LPM_talCB (
                void *arg,
                unsigned short event,
                unsigned short payload);

// Forward declarations of LPM Device functions.
static LPM_Status LPM_connect(void *instance);
static LPM_Status LPM_disconnect(void *instance);
static LPM_Status LPM_off(void *instance);
static LPM_Status LPM_on(void *instance, int args);
static LPM_Status LPM_resume(void *instance);
static LPM_Status LPM_setPowerState(void *instance, LPM_PowerState state);

// Public functions.
LPM_Status LPM_init(int index, LPM_Device *device, LPM_DevAttrs *attrs);
LPM_Status LPM_exit(void *instance);


/*
 *  ======== LPM_connect ========
 */
static LPM_Status LPM_connect(void *instance)
{
    LPM_Status      lpmStat = LPM_SOK;
    LPM_Private    *hndl = (LPM_Private *)instance;
    TAL_Status      talStat = TAL_SOK;
//  unsigned long   pwrmSymTabPA, pwrmSymTabVA;


    TRACE("--> LPM_connect\n");

    // Use TAL to connect to transport layer.
    TRACE("    calling TAL_open\n");
    talStat = TAL_open("foobar", &hndl->hTAL);

    if (talStat != TAL_SOK) {
        hndl->hTAL = NULL;
        lpmStat = LPM_EFAIL;
        TRACE("Error: TAL_open failed (0x%X)\n", talStat);
        goto fail;
    }

    // Register event callback with TAL layer.
    TRACE("    calling TAL_registerEvents\n");
    talStat = TAL_registerEvents(hndl->hTAL, _LPM_talCB, (void*)hndl);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("Error: TAL_registerEvents failed (0x%X)\n", talStat);
        goto fail;
    }

#if 0
    // Query the DSP for the symbol table base address. This is a two part
    // request. This is a non-blocking request, so we must wait for reply.
    TRACE("= calling TAL_sendEvent(QUERY, SYMBOLS_H\n");
    hndl->talData = 0;
    talStat = TAL_sendEvent(hndl->hTAL, PWRM_SH_QUERY, PWRM_SH_SYMBOLS_H);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_connect: Error: "
                        "TAL_sendEvent(QUERY, SYMBOLS_H) failed (0x%X)\n",
                        talStat);
        goto fail;
    }

    // Wait for request to complete.
    hndl->os_wait(hndl->os_instance);

    // Now query for the second part.
    TRACE("= calling TAL_sendEvent(QUERY, SYMBOLS_L\n");
    talStat = TAL_sendEvent(hndl->hTAL, PWRM_SH_QUERY, PWRM_SH_SYMBOLS_L);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_connect: Error: "
                        "TAL_sendEvent(QUERY, SYMBOLS_L) failed (0x%X)\n",
                        talStat);
        goto fail;
    }

    // Wait for request to complete.
    hndl->os_wait(hndl->os_instance);

    // Map the symbol table into our virtual address space so we can
    // access the table contents.
    pwrmSymTabPA = hndl->talData;
    TRACE("= calling os_remap(0x%X) [pwrmSymTab]\n", pwrmSymTabPA);
    lpmStat = hndl->os_remap(pwrmSymTabPA, 0x1000, &pwrmSymTabVA);

    if (lpmStat != LPM_SOK) {
        TRACE("LPM_connect: Error: "
                        "os_remap(0x%X) failed [pwrmSymTab]\n", pwrmSymTabPA);
        goto fail;
    }

    // Verify there are at least two symbols in the table.
    if (((unsigned int *)pwrmSymTabVA)[0] < 2) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_connect: Error: pwrmSymTab < 2 symbols\n");
        goto fail;
    }

    // Save the DSP warm boot address; needed for resume sequence.
    hndl->bootAddr = (unsigned long)(((unsigned int *)pwrmSymTabVA)[1]);

    // Map the PWRM_saveDone physical address into our virtual address space.
    hndl->saveDonePA = (unsigned long)(((unsigned int *)pwrmSymTabVA)[2]);
    TRACE("= calling os_remap(0x%X) [saveDone]\n", hndl->saveDonePA);
    lpmStat = hndl->os_remap(hndl->saveDonePA, 0x1000, &hndl->saveDoneVA);

    if (lpmStat != LPM_SOK) {
        TRACE("LPM_connect: Error: "
                        "os_remap(0x%X) failed [saveDone]\n", hndl->saveDonePA);
        goto fail;
    }

    // Unmap the symbol table; no longer needed.
    TRACE("= calling os_unmap(0x%X) [pwrmSymTab]\n", pwrmSymTabVA);
    hndl->os_unmap((void *)pwrmSymTabVA);

    if (lpmStat != LPM_SOK) {
        TRACE("LPM_connect: Error: "
                        "os_unmap(0x%X) failed [pwrmSymTab]\n", pwrmSymTabVA);
        goto fail;
    }
#endif

    // Update object state
    hndl->state |= LPM_STATE_CONNECTED;

fail:
    TRACE("<-- LPM_connect: stat = 0x%X\n", lpmStat);
    return lpmStat;
}


/*
 *  ======== LPM_disconnect ========
 */
static LPM_Status LPM_disconnect(void *instance)
{
    LPM_Private    *hndl = (LPM_Private *)instance;
    LPM_Status      lpmStat = LPM_SOK;
    TAL_Status      talStat = TAL_SOK;


    TRACE("+ LPM_disconnect\n");

    // Verify object is in the connected state.
    if (! (hndl->state & LPM_STATE_CONNECTED)) {
        TRACE("LPM_disconnect: Error: LPM not in connected state\n");
        lpmStat = LPM_EFAIL;
        goto fail;
    }

    // Unregister event callback with TAL layer.
    talStat = TAL_unregisterEvents(hndl->hTAL);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_disconnect: Error: "
                        "TAL_unregisterEvents failed (0x%X)\n", talStat);
        goto fail;
    }

    // Disconnect from transport layer.
    talStat = TAL_close(hndl->hTAL);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_disconnect: Error: "
                        "TAL_close failed (0x%X)\n", talStat);
        goto fail;
    }
    hndl->hTAL = NULL;

    // Clear the boot address.
    hndl->bootAddr = 0;

    // Update object state
    hndl->state &= ~LPM_STATE_CONNECTED;

fail:
    TRACE("- LPM_disconnect: stat = 0x%X\n", lpmStat);
    return lpmStat;
}


/*
 *  ======== LPM_exit ========
 */
LPM_Status LPM_exit(void *instance)
{
    LPM_Private *hndl = (LPM_Private *)instance;
    LPM_Status  status = LPM_SOK;


    TRACE("+ LPM_exit\n");

    TRACE("LPM_exit: os_unmap va = 0x%X\n", hndl->controlVA);
    hndl->os_unmap((void *)(hndl->controlVA));
    hndl->controlVA = 0;
    TRACE("LPM_exit: os_unmap va = 0x%X\n", hndl->ivacmVA);
    hndl->os_unmap((void *)(hndl->ivacmVA));
    hndl->ivacmVA = 0;
    TRACE("LPM_exit: os_unmap va = 0x%X\n", hndl->ivaprmVA);
    hndl->os_unmap((void *)(hndl->ivaprmVA));
    hndl->ivaprmVA = 0;

    hndl->state &= ~LPM_STATE_INITIALIZED;

    TRACE("- LPM_exit: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_off ========
 *
 *  Turn the IVA power domain off. To ensure a clean power-off
 *  transition, the IVA subsystem must first be turned on so that
 *  the DSP can initiate an autonomous power-off transition.
 */
static LPM_Status LPM_off(void *instance)
{
    unsigned int    i;
    LPM_Status      status = LPM_SOK;
    LPM_Private     *hndl = (LPM_Private *)instance;
    unsigned int    cm = hndl->ivacmVA;
    unsigned int    prm = hndl->ivaprmVA;
    unsigned int    control = hndl->controlVA;


    TRACE("lpm.ko: --> LPM_off\n");

    /* make sure the IVA2 DPLL is enabled */
    if ((REG(cm + CM_CLKEN_PLL_IVA2) & 0x7) == 0x1) {
        hndl->os_trace("Error: IVA2 PLL is not enabled (needed by LPM)");
        status = LPM_EFAIL;
        goto fail;
    }

    /* MPU controlled ON + ACTIVE */

    // clear power state change request
    REG(cm + CM_CLKSTCTRL_IVA2) = 0;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // turn off IVA2 domain functional clock (required to assert reset)
    REG(cm + CM_FCLKEN_IVA2) = 0;
    REG(cm + CM_FCLKEN_IVA2);

    // assert reset on DSP, SEQ
    REG(prm + RM_RSTCTRL_IVA2) = 0x5;
    REG(prm + RM_RSTCTRL_IVA2);

    // assert reset on MMU, interconnect, etc.
    REG(prm + RM_RSTCTRL_IVA2) |= (1 << 1);
    REG(prm + RM_RSTCTRL_IVA2);

    // set next IVA2 power state to ON
    REG(prm + PM_PWSTCTRL_IVA2) |= (3 << 0);
    REG(prm + PM_PWSTCTRL_IVA2);

    // start a software supervised wake-up transition
    REG(cm + CM_CLKSTCTRL_IVA2) = 0x2;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // wait for IVA2 power domain to switch ON
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + PM_PWSTST_IVA2) & 0x3) == 0x3) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2 power domain not ON in LPM_on\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear power-on transition request
    REG(cm + CM_CLKSTCTRL_IVA2) = 0;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // turn on IVA2 domain functional clock
    REG(cm + CM_FCLKEN_IVA2) = 1;
    REG(cm + CM_FCLKEN_IVA2);

    // ensure the IVA2_SW_RST2 status bit is cleared
    REG(prm + RM_RSTST_IVA2) |= (1 << 9);
    REG(prm + RM_RSTST_IVA2);

    // release IVA2_RST2 reset (MMU, SYSC, local interconnect, etc)
    REG(prm + RM_RSTCTRL_IVA2) &= ~(1 << 1);
    REG(prm + RM_RSTCTRL_IVA2);

    // wait for IVA2_RST2 signal to be released
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + RM_RSTST_IVA2) & (1 << 9))) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2_RST2 signal was not released\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear the IVA2_SW_RST2 status bit
    REG(prm + RM_RSTST_IVA2) |= (1 << 9);
    REG(prm + RM_RSTST_IVA2);

    // wait for IVA2 state to become active (~2 loops @ 312MHz)
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(cm + CM_IDLEST_IVA2) & 0x1) == 0) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2 did not become active\n");
        status = LPM_EFAIL;
        goto fail;
    }


    /* MPU controlled OFF */

    // turn off IVA2 domain functional clock (required to assert reset)
    REG(cm + CM_FCLKEN_IVA2) = 0;
    REG(cm + CM_FCLKEN_IVA2);

    // assert reset on MMU, interconnect, etc.
    REG(prm + RM_RSTCTRL_IVA2) |= (1 << 1);
    REG(prm + RM_RSTCTRL_IVA2);

    // wait for IVA2 state to become idle
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(cm + CM_IDLEST_IVA2) & 0x1) == 0x1) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2 did not become idle\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // set next IVA2 power state to OFF
    REG(prm + PM_PWSTCTRL_IVA2) &= ~(0x3);
    REG(prm + PM_PWSTCTRL_IVA2);

    // start a software supervised sleep transition
    REG(cm + CM_CLKSTCTRL_IVA2) = 0x1;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // wait for IVA2 power domain to switch OFF
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + PM_PWSTST_IVA2) & 0x3) == 0) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2 power domain not OFF (1)\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear power state change request
    REG(cm + CM_CLKSTCTRL_IVA2) = 0;
    REG(cm + CM_CLKSTCTRL_IVA2);


    /* MPU controlled ON + ACTIVE */

    // set next IVA2 power state to ON
    REG(prm + PM_PWSTCTRL_IVA2) |= (3 << 0);
    REG(prm + PM_PWSTCTRL_IVA2);

    // start a software supervised wake-up transition
    REG(cm + CM_CLKSTCTRL_IVA2) = 0x2;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // wait for IVA2 power domain to switch ON
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + PM_PWSTST_IVA2) & 0x3) == 0x3) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2 power domain not ON in LPM_on\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear power-on transition request
    REG(cm + CM_CLKSTCTRL_IVA2) = 0;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // turn on IVA2 domain functional clock
    REG(cm + CM_FCLKEN_IVA2) = 1;
    REG(cm + CM_FCLKEN_IVA2);

    // ensure the IVA2_SW_RST2 status bit is cleared
    REG(prm + RM_RSTST_IVA2) |= (1 << 9);
    REG(prm + RM_RSTST_IVA2);

    // release IVA2_RST2 reset (MMU, SYSC, local interconnect, etc)
    REG(prm + RM_RSTCTRL_IVA2) &= ~(1 << 1);
    REG(prm + RM_RSTCTRL_IVA2);

    // wait for IVA2_RST2 signal to be released
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + RM_RSTST_IVA2) & (1 << 9))) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2_RST2 signal was not released\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear the IVA2_SW_RST2 status bit
    REG(prm + RM_RSTST_IVA2) |= (1 << 9);
    REG(prm + RM_RSTST_IVA2);

    // first wait for IVA2 state to become active
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(cm + CM_IDLEST_IVA2) & 0x1) == 0) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2 did not become active\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // set DSP boot address and boot mode
    REG(control + CONTROL_IVA2_BOOTADDR) = 0;
    REG(control + CONTROL_IVA2_BOOTADDR);
    REG(control + CONTROL_IVA2_BOOTMOD) = 1;
    REG(control + CONTROL_IVA2_BOOTMOD);

    // enable automatic clock gating
    REG(cm + CM_CLKSTCTRL_IVA2) = 3;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // set next IVA2 power state to OFF
    REG(prm + PM_PWSTCTRL_IVA2) &= ~(0x3);
    REG(prm + PM_PWSTCTRL_IVA2);

    // ensure the IVA2_SW_RST1 status bit is cleared
    REG(prm + RM_RSTST_IVA2) |= (1 << 8);
    REG(prm + RM_RSTST_IVA2);

    // release DSP from reset
    REG(prm + RM_RSTCTRL_IVA2) &= ~(1 << 0);
    REG(prm + RM_RSTCTRL_IVA2);

    // wait for IVA2_RST1 signal to be released
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + RM_RSTST_IVA2) & (1 << 8))) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2_RST1 signal was not released\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear the IVA2_SW_RST1 status bit
    REG(prm + RM_RSTST_IVA2) |= (1 << 8);
    REG(prm + RM_RSTST_IVA2);

    // wait for IVA2 power domain to switch OFF (~7 loops @ 312MHz)
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + PM_PWSTST_IVA2) & 0x3) == 0) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("lpm.ko: Error: IVA2 power domain not OFF (2)\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // assert reset on DSP, SEQ
    REG(prm + RM_RSTCTRL_IVA2) = 0x5;
    REG(prm + RM_RSTCTRL_IVA2);

    // assert reset on MMU, interconnect, etc.
    REG(prm + RM_RSTCTRL_IVA2) |= (1 << 1);
    REG(prm + RM_RSTCTRL_IVA2);

    // disable automatic clock gating
    REG(cm + CM_CLKSTCTRL_IVA2) = 0;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // turn off IVA2 domain functional clock
    REG(cm + CM_FCLKEN_IVA2) = 0;
    REG(cm + CM_FCLKEN_IVA2);

fail:
    TRACE("lpm.ko: <-- LPM_off: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_on ========
 *
 *  Power on the IVA subsystem, hold the DSP and SEQ in reset, and
 *  release IVA2_RST. This is a hostile reset of the IVA. If the IVA
 *  is already powered on, then it must be powered off in order to
 *  terminate all current activity and initiate a power-on-reset
 *  transition to bring the IVA to a know good state.
 */
static LPM_Status LPM_on(void *instance, int args)
{
    unsigned int    i;
    LPM_Status      status = LPM_SOK;
    LPM_Private     *hndl = (LPM_Private *)instance;
    unsigned int    cm = hndl->ivacmVA;
    unsigned int    prm = hndl->ivaprmVA;


    TRACE("--> LPM_on\n");

    // ensure a clean power-off transition
    status = LPM_off(instance);
    if (status != LPM_SOK) {
        goto fail;
    }

    // disable automatic clock gating
    REG(cm + CM_CLKSTCTRL_IVA2) = 0;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // turn off IVA2 domain functional clock
    REG(cm + CM_FCLKEN_IVA2) = 0;
    REG(cm + CM_FCLKEN_IVA2);

    // assert reset on DSP, SEQ
    REG(prm + RM_RSTCTRL_IVA2) = 0x5;
    REG(prm + RM_RSTCTRL_IVA2);

    // assert reset on MMU, interconnect, etc.
    REG(prm + RM_RSTCTRL_IVA2) |= (1 << 1);
    REG(prm + RM_RSTCTRL_IVA2);

    // set next IVA2 power state to ON
    REG(prm + PM_PWSTCTRL_IVA2) |=
        (3 << 20) |     // SharedL2CacheFlatOnState = ON
        (3 << 0);       // PowerState = ON
    REG(prm + PM_PWSTCTRL_IVA2);

    // start a software supervised wake-up transition
    REG(cm + CM_CLKSTCTRL_IVA2) = 0x2;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // wait for IVA2 power domain to switch ON
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + PM_PWSTST_IVA2) & 0x3) == 0x3) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("Error: IVA2 power domain not ON in LPM_on\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear power-on transition request
    REG(cm + CM_CLKSTCTRL_IVA2) = 0;
    REG(cm + CM_CLKSTCTRL_IVA2);

    // turn on IVA2 domain functional clock
    REG(cm + CM_FCLKEN_IVA2) = 1;
    REG(cm + CM_FCLKEN_IVA2);

    // ensure the IVA2_SW_RST2 status bit is cleared
    REG(prm + RM_RSTST_IVA2) |= (1 << 9);
    REG(prm + RM_RSTST_IVA2);

    // release IVA2_RST2 reset (MMU, SYSC, local interconnect, etc)
    REG(prm + RM_RSTCTRL_IVA2) &= ~(1 << 1);
    REG(prm + RM_RSTCTRL_IVA2);

    // wait for IVA2_RST2 signal to be released
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(prm + RM_RSTST_IVA2) & (1 << 9))) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("Error: IVA2_RST2 signal was not released\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // clear the IVA2_SW_RST2 status bit
    REG(prm + RM_RSTST_IVA2) |= (1 << 9);
    REG(prm + RM_RSTST_IVA2);

    // wait for IVA2 state to become active
    for (i = 0; i < MAX_WAIT_COUNT; i++) {
        if ((REG(cm + CM_IDLEST_IVA2) & 0x1) == 0) {
            break;
        }
    }
    if (i == MAX_WAIT_COUNT) {
        TRACE("Error: IVA2 did not become active\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // enable automatic hardware supervised transitions
    REG(cm + CM_CLKSTCTRL_IVA2) = 0x3;
    REG(cm + CM_CLKSTCTRL_IVA2);

fail:
    TRACE("<-- LPM_on: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_resume ========
 */
static LPM_Status LPM_resume(void *instance)
{
    LPM_Status lpmStat = LPM_SOK;
    LPM_Private *hndl = (LPM_Private *)instance;
    TAL_Status talStat = TAL_SOK;
    TAL_TransportState transState;


    TRACE("--> LPM_resume\n");

#if 0
    // Set warm boot address in SYSTEM MODULE.
    TRACE("= calling setBootAddr(0x%X)\n", hndl->bootAddr);
    lpmStat = hndl->setBootAddr(instance, hndl->bootAddr);

    if (lpmStat != LPM_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_resume: Error: "
                        "setBootAddr failed (0x%X)\n", lpmStat);
        goto fail;
    }

    // Turn on power to GEM; warm boot.
    lpmStat = _LPM_on(hndl, LPM_WARMBOOT, 0);

    if (lpmStat != LPM_SOK) {
        TRACE("LPM_resume: Error: "
                        "_LPM_on failed (0x%X)\n", lpmStat);
        goto fail;
    }

    // Block here until TAL callback signals this object.
    hndl->os_wait(hndl->os_instance);
#endif

    TRACE("calling TAL_sendEvent(PWRM_SH_RESUME)\n");
    talStat = TAL_sendEvent(hndl->hTAL, PWRM_SH_RESUME, 0);

    if (talStat != TAL_SOK) {
        TRACE("TAL_sendEvent failed (0x%X)\n", talStat);
        lpmStat = LPM_EFAIL;
        goto fail;
    }

    // block here until callback signals this object
    hndl->os_wait(hndl->os_instance);

    // Set the transport state to available.
    TRACE("calling TAL_changeState\n");
    talStat = TAL_changeState(TAL_AVAILABLE, &transState);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_resume: Error: "
                        "TAL_changeState failed (0x%X)\n", talStat);
        goto fail;
    }


fail:
    TRACE("<-- LPM_resume: stat = 0x%X\n", lpmStat);
    return lpmStat;
}


/*
 *  ======== LPM_setPowerState ========
 */
static LPM_Status LPM_setPowerState(void *instance, LPM_PowerState state)
{
    LPM_Status          lpmStat = LPM_SOK;
    LPM_Private        *hndl = (LPM_Private *)instance;
    TAL_Status          talStat;
    TAL_TransportState  transState;
//  unsigned int        i;


    TRACE("--> LPM_setPowerState: state = 0x%X\n", state);

    // Initiate the requested power state change.
    switch (state) {

        case LPM_HIBERNATE:
            // Set the transport state to unavailable.
            TRACE("calling TAL_changeState\n");
            talStat = TAL_changeState(TAL_UNAVAILABLE, &transState);

            if (talStat != TAL_SOK) {
                TRACE("TAL_changeState failed (0x%X)\n", talStat);
                lpmStat = LPM_EFAIL;
                goto fail;
            }

            // Send hibernate command to DSP.
            TRACE("calling TAL_sendEvent\n");
            talStat = 
                TAL_sendEvent(hndl->hTAL, PWRM_SH_SLEEP, PWRM_SH_HIBERNATE);

            if (talStat != TAL_SOK) {
                TRACE("TAL_sendEvent failed (0x%X)\n", talStat);
                lpmStat = LPM_EFAIL;
                goto fail;
            }

            // Block here until callback signals this object.
            hndl->os_wait(hndl->os_instance);

#if 0
            // Spin on the "save done" flag.
            for (i = 1; (MEM(hndl->saveDoneVA) != PWRM_SAVE_DONE_VAL); i++) {
                // empty
            }
            TRACE("= saveDone detected, i = %d\n", i);

            // Update max loop counter.
            hndl->driver->max_wait_loop =
                (i > hndl->driver->max_wait_loop ? i : hndl->driver->max_wait_loop);

            // Turn off power to DSP.
            lpmStat = LPM_off(instance);
#endif
            break;

#if 0
        case LPM_STANDBY:
            break;
#endif
    }


fail:
    TRACE("<-- LPM_setPowerState: stat = 0x%X\n", lpmStat);
    return lpmStat;
}


/*
 *  ======== LPM_init ========
 */
LPM_Status LPM_init(int index, LPM_Device *device, LPM_DevAttrs *attrs)
{
    LPM_Private     *hndl;
    unsigned int    devID;
    LPM_Status      status = LPM_SOK;

    // validate device index
    if ((index < 0) || (index > (LPM_DEV_COUNT -1))) {
        return LPM_EFAIL;
    }

    // initialize function pointers
    device->connect         = LPM_connect;
    device->disconnect      = LPM_disconnect;
    device->off             = LPM_off;
    device->on              = LPM_on;
    device->resume          = LPM_resume;
    device->setPowerState   = LPM_setPowerState;

    // initialize private instance data
    device->instance = (void *)&LPM_OBJ[index];

    // now create the private handle
    hndl = (LPM_Private *)device->instance;
    hndl->os_remap = attrs->os_remap;
    hndl->os_signal = attrs->os_signal;
    hndl->os_unmap = attrs->os_unmap;
    hndl->os_wait = attrs->os_wait;
    hndl->os_trace = attrs->os_trace;
    hndl->os_instance = attrs->os_instance;

    // set back pointer to device structure
    hndl->driver = device;

    TRACE("--> LPM_init\n");

    // virtual address for system control module
    status = hndl->os_remap(SYSTEM_CONTROL_ADDR, SYSTEM_CONTROL_SIZE,
        &hndl->controlVA);

    if (status != LPM_SOK) {
        TRACE("LPM_init: os_remap failed: addr = 0x%X\n", SYSTEM_CONTROL_ADDR);
        goto fail;
    }
    TRACE("LPM_init: os_remap pa = 0x%X, va = 0x%X\n", SYSTEM_CONTROL_ADDR,
        hndl->controlVA);

    // virtual address for iva clock manager
    status = hndl->os_remap(PRCM_IVA2_CM_ADDR, PRCM_IVA2_CM_SIZE,
        &hndl->ivacmVA);

    if (status != LPM_SOK) {
        TRACE("LPM_init: os_remap failed: addr = 0x%X\n", PRCM_IVA2_CM_ADDR);
        goto fail;
    }
    TRACE("LPM_init: os_remap pa = 0x%X, va = 0x%X\n", PRCM_IVA2_CM_ADDR,
        hndl->ivacmVA);

    // virtual address for iva power and reset manager
    status = hndl->os_remap(PRCM_IVA2_PRM_ADDR, PRCM_IVA2_PRM_SIZE,
        &hndl->ivaprmVA);

    if (status != LPM_SOK) {
        TRACE("LPM_init: os_remap failed: addr = 0x%X\n", PRCM_IVA2_PRM_ADDR);
        goto fail;
    }
    TRACE("LPM_init: os_remap pa = 0x%X, va = 0x%X\n", PRCM_IVA2_PRM_ADDR,
        hndl->ivaprmVA);

    // update object state
    hndl->state |= LPM_STATE_INITIALIZED;

    // Print the device ID.
    devID = REG(hndl->controlVA + CONTROL_REVISION);
    TRACE("Control Revision: 0x%X\n", (devID & 0x000000FF));


fail:
    // check for failure
    if (status != LPM_SOK) {
        if (hndl->controlVA != 0) {
            hndl->os_unmap((void *)(hndl->controlVA));
            hndl->controlVA = 0;
        }
        if (hndl->ivacmVA != 0) {
            hndl->os_unmap((void *)(hndl->ivacmVA));
            hndl->ivacmVA = 0;
        }
        if (hndl->ivaprmVA != 0) {
            hndl->os_unmap((void *)(hndl->ivaprmVA));
            hndl->ivaprmVA = 0;
        }
    }

    TRACE("<-- LPM_init: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== _LPM_talCB ========
 */
void _LPM_talCB(void *arg, unsigned short event,
                 unsigned short payload)
{
    LPM_Private    *hndl = (LPM_Private *)arg;


    TRACE("--> _LPM_talCB: event = 0x%X, payload = 0x%X\n", event, payload);

    // Decode the event and payload.
    if ((event == PWRM_SH_ACKNOWLEDGE) && (payload == PWRM_SH_HIBERNATE)) {
        // TODO: need separate event objects for hibernate and resume
    }
    else if ((event == PWRM_SH_AWAKE) && (payload == PWRM_SH_HIBERNATE)) {
        // TODO: need separate event objects for hibernate and resume
    }
    else if (event == PWRM_SH_DATA_H) {
        hndl->talData |= ((unsigned int)payload << 16);
    }
    else if (event == PWRM_SH_DATA_L) {
        hndl->talData |= ((unsigned int)payload);
    }
    else {
        // error
        goto fail;
    }

    // signal object to release application thread
    hndl->os_signal(hndl->os_instance);


fail:
    TRACE("<-- _LPM_talCB\n");
    return;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:32; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

