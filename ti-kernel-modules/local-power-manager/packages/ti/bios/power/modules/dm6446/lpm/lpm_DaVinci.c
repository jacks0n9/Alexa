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
 *  ======== lpm_DaVinci.c ========
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

#define SYSM_BASE               0x01C40000      /* size = 2K */
#define SYSM_DSPBOOTADDR            0x0008
#define SYSM_DEVICE_ID              0x0028
#define SYSM_CHP_SHRTSW             0x0038

#define PSC_BASE                0x01C41000      /* size = 4K */
#define PSC_GBLCTL                  0x0010
#define PSC_EPCPR                   0x0070
#define PSC_EPCCR                   0x0078
#define PSC_PTCMD                   0x0120
#define PSC_PTSTAT                  0x0128
#define PSC_PDSTAT0                 0x0200
#define PSC_PDSTAT1                 0x0204
#define PSC_PDCTL0                  0x0300
#define PSC_PDCTL1                  0x0304
#define PSC_MDCFG_DSP               0x069C      /* 0x0600 + (39 * 4) */
#define PSC_MDCFG_IMCOP             0x06A0      /* 0x0600 + (40 * 4) */
#define PSC_MDSTAT_DSP              0x089C      /* 0x0800 + (39 * 4) */
#define PSC_MDSTAT_IMCOP            0x08A0      /* 0x0800 + (40 * 4) */
#define PSC_MDCTL_DSP               0x0A9C      /* 0x0A00 + (39 * 4) */
#define PSC_MDCTL_IMCOP             0x0AA0      /* 0x0A00 + (40 * 4) */

#define PSC_MAX_WAIT_COUNT          0x0100
#define PSC_MAX_TRANSITION_WAIT     0x1000

#define DSP_PD_MASK         (~0x02)

#define REG(x)              *((volatile unsigned int *) (x))
#define MEM(x)              *((volatile unsigned int *) (x))


#define LPM_DEV_COUNT   1
#define PWRM_SAVE_DONE_VAL      0x01020304      /* from _pwrmDM420.h64P */

#define LPM_STATE_RESET         0
#define LPM_STATE_INITIALIZED   0x0001
#define LPM_STATE_CONNECTED     0x0002



/* private data structure */
typedef struct LPM_Private {
    unsigned int        state;
    unsigned long       bootAddr;
    LPM_Status        (*setBootAddr)(void *, unsigned int);
    char               *resumeSymbol;
    char               *saveDoneSymbol;
    LPM_ExtPwrOffFxn    lpm_extPwrOff;
    void               *lpm_extPwrOffArg;
    LPM_ExtPwrOnFxn     lpm_extPwrOn;
    void               *lpm_extPwrOnArg;
    unsigned long       sysmPA;
    unsigned long       sysmVA;
    unsigned long       pscPA;
    unsigned long       pscVA;
    unsigned long       saveDonePA;
    unsigned long       saveDoneVA;
    TAL_Handle          hTAL;
    LPM_RemapFxn        os_remap;
    LPM_SignalFxn       os_signal;
    LPM_UnmapFxn        os_unmap;
    LPM_WaitFxn         os_wait;
    LPM_TraceFxn        os_trace;
    void               *os_instance;
    unsigned int        imcopState;
    unsigned int        talData;

    /* os driver object pointer */
    LPM_Device         *driver;

} LPM_Private;

static struct LPM_Private LPM_OBJ[LPM_DEV_COUNT] = {

    /* LPM_OBJ[0] */
    {
        .state              = LPM_STATE_RESET,
        .resumeSymbol       = "__PWRM_restoreJouleIMem",
        .saveDoneSymbol     = "__PWRM_saveDone",
        .lpm_extPwrOff      = NULL,
        .lpm_extPwrOffArg   = NULL,
        .lpm_extPwrOn       = NULL,
        .lpm_extPwrOnArg    = NULL,
        .sysmPA             = SYSM_BASE,
        .sysmVA             = 0,
        .pscPA              = PSC_BASE,
        .pscVA              = 0,
        .saveDonePA         = 0,
        .saveDoneVA         = 0,
        .imcopState         = 0,
    },
};


static void _LPM_talCB(void *arg, unsigned short event, unsigned short payload);

// Private function to turn on power with proper context restore.
static LPM_Status _LPM_on_DaVinci(LPM_Private *hndl,
                        LPM_Context cntxt, int args);
static LPM_Status _LPM_setBootAddr_DaVinci(void *instance,
                        unsigned int bootAddr);

// Forward declarations of LPM Device functions.
static LPM_Status LPM_command_DaVinci
                    (void *instance, LPM_Action action, unsigned long arg);
static LPM_Status LPM_connect_DaVinci(void *instance);
static LPM_Status LPM_disconnect_DaVinci(void *instance);
static LPM_Status LPM_off_DaVinci(void *instance);
static LPM_Status LPM_on_DaVinci(void *instance, int args);
static LPM_Status LPM_resume_DaVinci(void *instance);
static LPM_Status _LPM_setBootAddr_DaVinci(void *instance,
        unsigned int bootAddr);
static LPM_Status LPM_setPowerState_DaVinci(void *instance,
        LPM_PowerState state);

// Public functions.
LPM_Status LPM_init(int index, LPM_Device *device, LPM_DevAttrs *attrs);
LPM_Status LPM_exit(void *instance);



/*
 *  ======== LPM_command_DaVinci ========
 */
static LPM_Status LPM_command_DaVinci
                    (void *instance, LPM_Action action, unsigned long arg)
{
    LPM_Status      status = LPM_SOK;
    LPM_Private    *hndl = (LPM_Private *)instance;


    TRACE("+ LPM_command\n");

    // process the command
    switch (action) {

        case LPM_EXTERNAL_VOLTAGE_OFF_FXN:
            hndl->lpm_extPwrOff = (LPM_ExtPwrOffFxn)arg;
            break;

        case LPM_EXTERNAL_VOLTAGE_OFF_ARG:
            hndl->lpm_extPwrOffArg = (void *)arg;
            break;

        case LPM_EXTERNAL_VOLTAGE_ON_FXN:
            hndl->lpm_extPwrOn = (LPM_ExtPwrOnFxn)arg;
            break;

        case LPM_EXTERNAL_VOLTAGE_ON_ARG:
            hndl->lpm_extPwrOnArg = (void *)arg;
            break;
    }


    TRACE("- LPM_command: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_connect_DaVinci ========
 */
static LPM_Status LPM_connect_DaVinci(void *instance)
{
    LPM_Status      lpmStat = LPM_SOK;
    LPM_Private    *hndl = (LPM_Private *)instance;
    TAL_Status      talStat = TAL_SOK;
    unsigned long   pwrmSymTabPA, pwrmSymTabVA;


    TRACE("+ LPM_connect\n");

    // Use TAL to connect to transport layer.
    TRACE("= calling TAL_open\n");
    talStat = TAL_open("foobar", &hndl->hTAL);

    if (talStat != TAL_SOK) {
        hndl->hTAL = NULL;
        lpmStat = LPM_EFAIL;
        TRACE("LPM_connect: Error: "
                        "TAL_open failed (0x%X)\n", talStat);
        goto fail;
    }

    // Register event callback with TAL layer.
    TRACE("= calling TAL_registerEvents\n");
    talStat = TAL_registerEvents(hndl->hTAL, _LPM_talCB, (void*)hndl);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_connect: Error: "
                        "TAL_registerEvents failed (0x%X)\n", talStat);
        goto fail;
    }

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
        TRACE("LPM_connect_DaVinci: Error: "
                        "os_remap(0x%X) failed [pwrmSymTab]\n", pwrmSymTabPA);
        goto fail;
    }

    // Verify there are at least two symbols in the table.
    if (((unsigned int *)pwrmSymTabVA)[0] < 2) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_connect_DaVinci: Error: pwrmSymTab < 2 symbols\n");
        goto fail;
    }

    // Save the DSP warm boot address; needed for resume sequence.
    hndl->bootAddr = (unsigned long)(((unsigned int *)pwrmSymTabVA)[1]);

    // Map the PWRM_saveDone physical address into our virtual address space.
    hndl->saveDonePA = (unsigned long)(((unsigned int *)pwrmSymTabVA)[2]);
    TRACE("= calling os_remap(0x%X) [saveDone]\n", hndl->saveDonePA);
    lpmStat = hndl->os_remap(hndl->saveDonePA, 0x1000, &hndl->saveDoneVA);

    if (lpmStat != LPM_SOK) {
        TRACE("LPM_connect_DaVinci: Error: "
                        "os_remap(0x%X) failed [saveDone]\n", hndl->saveDonePA);
        goto fail;
    }

    // Unmap the symbol table; no longer needed.
    TRACE("= calling os_unmap(0x%X) [pwrmSymTab]\n", pwrmSymTabVA);
    hndl->os_unmap((void *)pwrmSymTabVA);

    if (lpmStat != LPM_SOK) {
        TRACE("LPM_connect_DaVinci: Error: "
                        "os_unmap(0x%X) failed [pwrmSymTab]\n", pwrmSymTabVA);
        goto fail;
    }

    // Update object state
    hndl->state |= LPM_STATE_CONNECTED;

fail:
    TRACE("- LPM_connect: stat = 0x%X\n", lpmStat);
    return lpmStat;
}


/*
 *  ======== LPM_disconnect_DaVinci ========
 */
static LPM_Status LPM_disconnect_DaVinci(void *instance)
{
    LPM_Private    *hndl = (LPM_Private *)instance;
    LPM_Status      lpmStat = LPM_SOK;
    TAL_Status      talStat = TAL_SOK;


    TRACE("+ LPM_disconnect\n");

    // Verify object is in the connected state.
    if (! (hndl->state & LPM_STATE_CONNECTED)) {
        TRACE("LPM_disconnect_DaVinci: Error: LPM not in connected state\n");
        lpmStat = LPM_EFAIL;
        goto fail;
    }

    // Unregister event callback with TAL layer.
    talStat = TAL_unregisterEvents(hndl->hTAL);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_disconnect_DaVinci: Error: "
                        "TAL_unregisterEvents failed (0x%X)\n", talStat);
        goto fail;
    }

    // Disconnect from transport layer.
    talStat = TAL_close(hndl->hTAL);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_disconnect_DaVinci: Error: "
                        "TAL_close failed (0x%X)\n", talStat);
        goto fail;
    }
    hndl->hTAL = NULL;

    // Clear the boot address.
    hndl->bootAddr = 0;

    // Unmap the "save done" flag memory.
    hndl->os_unmap((void *)(hndl->saveDoneVA));
    hndl->saveDoneVA = 0;
    hndl->saveDonePA = 0;

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
    LPM_Status      status = LPM_SOK;
    LPM_Private    *hndl = (LPM_Private *)instance;


    TRACE("+ LPM_exit\n");

    hndl->os_unmap((void *)(hndl->sysmVA));
    hndl->sysmVA = 0;

    hndl->os_unmap((void *)(hndl->pscVA));
    hndl->pscVA = 0;

    hndl->state &= ~LPM_STATE_INITIALIZED;

    TRACE("- LPM_exit: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_off_DaVinci ========
 *
 *  Turn the DSP power domain off.
 *
 *  The first step is to transition the MSMs into the SyncRst state.
 *  The second step is to transition the MSMs into the SwRstDisable
 *  state and to transition the PSM into the OFF state.
 *
 *  Notes: If the IMCOP is already in SwRstDisable state, then leave it
 *  there, otherwise, transition it into SyncRst state in step 1.
 *
 *  Precondition: The DSP power domain is currently ON.
 */
static LPM_Status LPM_off_DaVinci(void *instance)
{
    int             i;
    int             imcopActive;
    LPM_Status      status = LPM_SOK;
    LPM_Private    *hndl = (LPM_Private *)instance;
    unsigned int    psc = hndl->pscVA;
    unsigned int    sysm = hndl->sysmVA;


    TRACE("+ LPM_off\n");

    // Wait for current transition to complete.
    //     Reg: PTSTAT (PSC), read-only register
    //     Bit: 1, GOSTAT[1] = DSP Power Domain
    //     Val: 0 = no transition in progress
    for (i = 0; i < (PSC_MAX_TRANSITION_WAIT); i++) {
        if ((REG(psc + PSC_PTSTAT) & 0x02) == 0) {
            break;
        }
    }

    // Check if transition completed.
    if (i == (PSC_MAX_TRANSITION_WAIT)) {
        TRACE("LPM_off: Error: domain stuck in transition\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // Step 1: transition DSP and IMCOP into SyncRst state.
    TRACE("LPM_off: PDSTAT1 status = 0x%X\n", REG(psc + PSC_PDSTAT1));
    TRACE("LPM_off: DSP status = 0x%X\n", REG(psc + PSC_MDSTAT_DSP));
    TRACE("LPM_off: IMCOP status = 0x%X\n", REG(psc + PSC_MDSTAT_IMCOP));
    TRACE("LPM_off: step 1, transition DSP and IMCOP into SyncRst state\n");

    // Set DSP module next state to SyncReset
    //     Reg: MDCTL39 (PSC)
    //     Bit: 4:0, NEXT
    //     Val: 00001 = SyncReset
    REG(psc + PSC_MDCTL_DSP) = (REG(psc + PSC_MDCTL_DSP) & ~(0x1F)) | 0x01;

    // Save the IMCOP state; used in WARM BOOT.
    //     Reg: MDSTAT40, (PSC)
    //     Bit: 5:0, STATE
    //     Bit: 8, LRSTz
    hndl->imcopState = (REG(psc + PSC_MDSTAT_IMCOP) & 0x13F);

    // Take note if IMCOP is already in SwRstDisable state.
    //     Bit: 5:0
    //     Val: 000000 = SwRstDisable state
    imcopActive = (hndl->imcopState & 0x3F ? 1 : 0);

    // Ensure IMCOP is in a valid state, reset if not.
    //     Bit: 5:0
    //     Val: 1111xx = transitional states (i.e. invalid)
    if (hndl->imcopState & 0x3C) {
        hndl->imcopState = 0;      // assert local reset, SwRstDisable
    }

    // If IMCOP is active, then set next state to SyncReset.
    //     Reg: MDCTL40 (PSC)
    //     Bit: 4:0, NEXT
    //     Val: 00001 = SyncReset
    if (imcopActive) {
        REG(psc + PSC_MDCTL_IMCOP) =
                (REG(psc + PSC_MDCTL_IMCOP) & ~(0x1F)) | 0x01;
    }

    // Current work around for hardware bug: turn on force mode.
    //     Reg: GBLCTL (PSC)
    //     Bit: 0, FORCE
    //     Val: 1 = enable force mode
    REG(psc + PSC_GBLCTL) = REG(psc + PSC_GBLCTL) | 0x01;

    // Start power state transition.
    //     Reg: PTCMD (PSC), write-only register
    //     Bit: 1, GO[1] = DSP Power Domain
    //     Val: 1 = start state transitions
    REG(psc + PSC_PTCMD) = 0x02;

    // Wait for the modules to complete state transition.
    //     Reg: PTSTAT (PSC), read-only register
    //     Bit: 1, GOSTAT[1] = DSP Power Domain
    //     Val: 0 = no transition in progress
    for (i = 0; i < PSC_MAX_WAIT_COUNT; i++) {
        if ((REG(psc + PSC_PTSTAT) & 0x02) == 0) {
            break;
        }
    }

    // Current work around for hardware bug: turn off force mode.
    //     Reg: GBLCTL (PSC)
    //     Bit: 0, FORCE
    //     Val: 0 = disable force mode
    REG(psc + PSC_GBLCTL) = REG(psc + PSC_GBLCTL) & ~(0x1);

    // Check if transition completed.
    if (i == PSC_MAX_WAIT_COUNT) {
        TRACE("LPM_off: Error: transition failure\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // Check if DSP in SyncRst state.
    //     Reg: MDSTAT39 (PSC)
    //     Bit: 5:0, STATE
    //     Val: 000001 = SyncRst
    if ((REG(psc + PSC_MDSTAT_DSP) & 0x3F) != 0x01) {
        TRACE("LPM_off: Error: DSP not in SyncRst state\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // If IMCOP is active, verify its in SyncRst state.
    //     Reg: MDSTAT40 (PSC)
    //     Bit: 5:0, STATE
    //     Val: 000001 = SyncRst
    if (imcopActive) {
        if ((REG(psc + PSC_MDSTAT_IMCOP) & 0x3F) != 0x01) {
            TRACE("LPM_off: Error: IMCOP not in SyncRst state\n");
            status = LPM_EFAIL;
            goto fail;
        }
    }

    // Step 2: transition DSP power domain into OFF state.
    TRACE("LPM_off: step 2, transition DSP power domain into OFF state\n");

    // Set DSP power domain next state to off.
    //     Reg: PDCTL1 (PSC)
    //     Bit: 0, NEXT
    //     Val: 0 = off
    REG(psc + PSC_PDCTL1) = REG(psc + PSC_PDCTL1) & ~(0x1);

    // Ensure the external power control pending indicator is cleared.
    //     Reg: EPCCR (PSC), write-only register
    //     Bit: 1, EPC[1] = DSP Power Domain
    //     Val: 1 = causes EPCRP bit n to be cleared
    REG(psc + PSC_EPCCR) = 0x2;

    // Set DSP module next state to software reset disable.
    //     Reg: MDCTL39 (PSC)
    //     Bit: 4:0, NEXT
    //     Val: 00000 = SwRstDisable
    REG(psc + PSC_MDCTL_DSP) = REG(psc + PSC_MDCTL_DSP) & ~(0x1F);

    // Set IMCOP module next state to software reset disable.
    //     Reg: MDCTL40 (PSC)
    //     Bit: 4:0, NEXT
    //     Val: 00000 = SwRstDisable
    REG(psc + PSC_MDCTL_IMCOP) = REG(psc + PSC_MDCTL_IMCOP) & ~(0x1F);

    // Start power state transition.
    //     Reg: PTCMD (PSC), write-only register
    //     Bit: 1, GO[1] = DSP Power Domain
    //     Val: 1 = start state transitions
    REG(psc + PSC_PTCMD) = 0x02;

    // Wait for external power control pending register bit to go high.
    //     Reg: EPCPR (PSC), read-only register
    //     Bit: 1, EPC[1] = DSP Power Domain
    //     Val: 1 = external power control intervention is needed
    for (i = 0; i < PSC_MAX_WAIT_COUNT; i++) {
        if (REG(psc + PSC_EPCPR) & 0x02) {
            break;
        }
    }

    // Check if intervention request given.
    if (i == PSC_MAX_WAIT_COUNT) {
        TRACE("LPM_off: no EPCPR bit\n");
        // TODO: status = LPM_ENOEPCPR;
        // For now, just keep going.
        // goto fail;
    }

    // Open the chip shorting switch.
    //     Reg: CHP_SHRTSW (SYSM)
    //     Bit: 0, DSPPWRON
    //     Val: 0 = open the switch (OFF)
    REG(sysm + SYSM_CHP_SHRTSW) = 0;

    // Call external power control to turn off external power supply.
    if (hndl->lpm_extPwrOff != NULL) {
        (hndl->lpm_extPwrOff)(hndl->lpm_extPwrOffArg);
    }
    else {
        hndl->os_trace("Warning: [LPM] cannot turn off external power switch, "
            "(function not registered)");
    }

    // Clear external power control pending register bit.
    //     Reg: EPCCR (PSC), write-only register
    //     Bit: 1, EPC[1] = DSP Power Domain
    //     Val: 1 = causes EPCRP bit n to be cleared
    REG(psc + PSC_EPCCR) = 0x2;

    // Notify PSM that external power is OFF.
    //     Reg: PDCTL1 (PSC)
    //     Bit: 8, EPCGOOD
    //     Val: 0 = external power for domain is off
    REG(psc + PSC_PDCTL1) = REG(psc + PSC_PDCTL1) & ~(0x100);

    // Wait for power domain and all modules to complete state transitions.
    //     Reg: PTSTAT (PSC), read-only register
    //     Bit: 1, GOSTAT[1] = DSP Power Domain
    //     Val: 0 = no transition in progress
    for (i = 0; i < PSC_MAX_WAIT_COUNT; i++) {
        if ((REG(psc + PSC_PTSTAT) & 0x02) == 0) {
            break;
        }
    }

    // Check if transition completed.
    if (i == PSC_MAX_WAIT_COUNT) {
        TRACE("LPM_off: transition failure\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // Check Power Domain Status
    //     Reg: PDSTAT1 (PSC)
    //     Bit: 4:0, STATE
    //     Val: 00000 = off
    if ((REG(psc + PSC_PDSTAT1) & 0x1F) == 0) {
        status = LPM_SOK;
    }
    else {
        TRACE("LPM_off: failure, domain not off\n");
        status = LPM_EFAIL;
        goto fail;
    }

fail:
    TRACE("LPM_off: PDSTAT1 status = 0x%X\n", REG(psc + PSC_PDSTAT1));
    TRACE("LPM_off: DSP status = 0x%X\n", REG(psc + PSC_MDSTAT_DSP));
    TRACE("LPM_off: IMCOP status = 0x%X\n", REG(psc + PSC_MDSTAT_IMCOP));
    TRACE("- LPM_off: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_on_DaVinci ========
 */
static LPM_Status LPM_on_DaVinci(void *instance, int args)
{
    LPM_Private    *hndl = (LPM_Private *)instance;

    return _LPM_on_DaVinci(hndl, LPM_COLDBOOT, args);
}


/*
 *  ======== _LPM_on_DaVinci ========
 */
static LPM_Status _LPM_on_DaVinci(LPM_Private *hndl, LPM_Context cntxt,
        int args)
{
    int             i;
    LPM_Status      status = LPM_SOK;
    unsigned int    psc = hndl->pscVA;
    unsigned int    sysm = hndl->sysmVA;


    TRACE("+ LPM_on\n");

    // Wait for current transition to complete.
    //     Reg: PTSTAT (PSC), read-only register
    //     Bit: 1, GOSTAT[1] = DSP Power Domain
    //     Val: 0 = no transition in progress
    for (i = 0; i < (PSC_MAX_TRANSITION_WAIT); i++) {
        if ((REG(psc + PSC_PTSTAT) & 0x02) == 0) {
            break;
        }
    }

    // Check if transition completed.
    if (i == (PSC_MAX_TRANSITION_WAIT)) {
        TRACE("LPM_on: Error: domain stuck in transition\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // Trace some useful initial status values.
    TRACE("LPM_on: PDSTAT1 status = 0x%X\n", REG(psc + PSC_PDSTAT1));
    TRACE("LPM_on: DSP status = 0x%X\n", REG(psc + PSC_MDSTAT_DSP));
    TRACE("LPM_on: IMCOP status = 0x%X\n", REG(psc + PSC_MDSTAT_IMCOP));

    // Set DSP power domain next state to on.
    //     Reg: PDCTL1 (PSC)
    //     Bit: 0, NEXT
    //     Val: 1 = on
    REG(psc + PSC_PDCTL1) = REG(psc + PSC_PDCTL1) | 0x01;

    // Enable DSP module.
    //     Reg: MDCTL39 (PSC)
    //     Bit: 4:0, NEXT
    //     Val: 00011 = enable
    REG(psc + PSC_MDCTL_DSP) = (REG(psc + PSC_MDCTL_DSP) & ~(0x1F)) | 0x3;

    // Set DSP boot mode.
    switch (cntxt) {

        case LPM_COLDBOOT:
            // Hold DSP in local reset.
            //     Reg: MDCTL39 (PSC)
            //     Bit: 8, LRSTz
            //     Val: 0 = assert local reset
            REG(psc + PSC_MDCTL_DSP) = REG(psc + PSC_MDCTL_DSP) & ~(0x100);
            break;

        case LPM_WARMBOOT:
            // Let DSP run free. It will boot to SYSM BOOT_ADDR.
            //     Reg: MDCTL39 (PSC)
            //     Bit: 8, LRSTz
            //     Val: 1 = de-assert local reset
            REG(psc + PSC_MDCTL_DSP) = REG(psc + PSC_MDCTL_DSP) | 0x0100;
            break;
    }

    // Set IMCOP boot mode.
    switch (cntxt) {
        unsigned int state;

        case LPM_COLDBOOT:
            // IMCOP module state is passed in through args
            state = (args & 0x3) == 3 ? 0x3 : 0;

            // Set the IMCOP module to enable state.
            //     Reg: MDCTL40 (PSC)
            //     Bit: 4:0, NEXT
            //     Val: 00000 = SwRstDisable, 00011 = enable
            REG(psc + PSC_MDCTL_IMCOP) =
                    (REG(psc + PSC_MDCTL_IMCOP) & ~(0x1F)) | state;

            // In all cases, hold IMCOP in local reset.
            //     Reg: MDCTL40 (PSC)
            //     Bit: 8, LRSTz
            //     Val: 0 = assert local reset
            REG(psc + PSC_MDCTL_IMCOP) = REG(psc + PSC_MDCTL_IMCOP) & ~(0x100);
            break;

        case LPM_WARMBOOT:
            // Set the IMCOP module next state.
            //     Reg: MDCTL40 (PSC)
            //     Bit: 4:0, NEXT
            //     Val: saved in LPM_off()
            REG(psc + PSC_MDCTL_IMCOP) =
                    (REG(psc + PSC_MDCTL_IMCOP) & ~(0x11F)) | hndl->imcopState;
            break;
    }

    // Ensure the external power control pending indicator is cleared.
    //     Reg: EPCCR (PSC), write-only register
    //     Bit: 1, EPC[1] = DSP Power Domain
    //     Val: 1 = causes EPCRP bit n to be cleared
    REG(psc + PSC_EPCCR) = 0x2;

    // Start power state transition.
    //     Reg: PTCMD (PSC), write-only register
    //     Bit: 1, GO[1] = DSP Power Domain
    //     Val: 1 = start state transitions
    REG(psc + PSC_PTCMD) = 0x02;

    // Wait for external power control pending register bit to go high.
    //     Reg: EPCPR (PSC), read-only register
    //     Bit: 1, EPC[1] = DSP Power Domain
    //     Val: 1 = external power control intervention is needed
    for (i = 0; i < PSC_MAX_WAIT_COUNT; i++) {
        if (REG(psc + PSC_EPCPR) & 0x02) {
            break;
        }
    }

    // Check if intervention request given.
    if (i == PSC_MAX_WAIT_COUNT) {
        TRACE("LPM_on: no EPCPR bit\n");
        // TODO: status = LPM_ENOEPCPR;
        // For now, just keep going.
        // goto fail;
    }

    // Call external power control to turn on external power supply.
    if (hndl->lpm_extPwrOn != NULL) {
        (hndl->lpm_extPwrOn)(hndl->lpm_extPwrOnArg);
    }
    else {
        hndl->os_trace("Warning: [LPM] cannot turn on external power switch, "
            "(function not registered)");
    }

    // Close the chip shorting switch.
    //     Reg: CHP_SHRTSW (SYSM)
    //     Bit: 0, DSPPWRON
    //     Val: 1 = close the switch (ON)
    REG(sysm + SYSM_CHP_SHRTSW) = 1;

    // Clear external power control pending register bit.
    //     Reg: PSC EPCCR (write-only register)
    //     Bit: 1 (one bit per domain, bit 1 = DSP power domain)
    //     Val: 1 = EPCPR bit 1 to be cleared
    REG(psc + PSC_EPCCR) = 0x2;

    // Notify PSM that external power is ON.
    //     Reg: PDCTL1 (PSC)
    //     Bit: 8, EPCGOOD
    //     Val: 1 = external power for domain is on
    REG(psc + PSC_PDCTL1) = REG(psc + PSC_PDCTL1) | 0x0100;

    // Wait for power domain and all modules to complete state transitions.
    //     Reg: PTSTAT (PSC), read-only register
    //     Bit: 1, GOSTAT[1] = DSP Power Domain
    //     Val: 0 = no transition in progress
    for (i = 0; i < PSC_MAX_WAIT_COUNT; i++) {
        if ((REG(psc + PSC_PTSTAT) & 0x02) == 0) {
            break;
        }
    }

    // Check if transition completed.
    if (i == PSC_MAX_WAIT_COUNT) {
        TRACE("LPM_on: transition failure\n");
        status = LPM_EFAIL;
        goto fail;
    }

    // Check Power Domain Status
    //     Reg: PDSTAT1 (PSC)
    //     Bit: 4:0, STATE
    //     Val: 00001 = on
    if ((REG(psc + PSC_PDSTAT1) & 0x1F) == 0x01) {
        status = LPM_SOK;
    }
    else {
        TRACE("LPM_on: failure, domain not on\n");
        status = LPM_EFAIL;
        goto fail;
    }


fail:
    TRACE("LPM_on: PDSTAT1 status = 0x%X\n", REG(psc + PSC_PDSTAT1));
    TRACE("LPM_on: DSP status = 0x%X\n", REG(psc + PSC_MDSTAT_DSP));
    TRACE("LPM_on: IMCOP status = 0x%X\n", REG(psc + PSC_MDSTAT_IMCOP));
    TRACE("- LPM_on: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_resume_DaVinci ========
 */
static LPM_Status LPM_resume_DaVinci(void *instance)
{
    LPM_Status          lpmStat = LPM_SOK;
    LPM_Private        *hndl = (LPM_Private *)instance;
    TAL_Status          talStat = TAL_SOK;
    TAL_TransportState  transState;


    TRACE("+ LPM_resume\n");

    // Set warm boot address in SYSTEM MODULE.
    TRACE("= calling setBootAddr(0x%X)\n", hndl->bootAddr);
    lpmStat = hndl->setBootAddr(instance, hndl->bootAddr);

    if (lpmStat != LPM_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_resume_DaVinci: Error: "
                        "setBootAddr failed (0x%X)\n", lpmStat);
        goto fail;
    }

    // Turn on power to GEM; warm boot.
    lpmStat = _LPM_on_DaVinci(hndl, LPM_WARMBOOT, 0);

    if (lpmStat != LPM_SOK) {
        TRACE("LPM_resume_DaVinci: Error: "
                        "_LPM_on_DaVinci failed (0x%X)\n", lpmStat);
        goto fail;
    }

    // Block here until TAL callback signals this object.
    hndl->os_wait(hndl->os_instance);

    // Set the transport state to available.
    TRACE("= calling TAL_changeState\n");
    talStat = TAL_changeState(TAL_AVAILABLE, &transState);

    if (talStat != TAL_SOK) {
        lpmStat = LPM_EFAIL;
        TRACE("LPM_resume_DaVinci: Error: "
                        "TAL_changeState failed (0x%X)\n", talStat);
        goto fail;
    }


fail:
    TRACE("- LPM_resume: stat = 0x%X\n", lpmStat);
    return lpmStat;
}


/*
 *  ======== _LPM_setBootAddr_DaVinci ========
 */
static LPM_Status _LPM_setBootAddr_DaVinci(void *instance,
        unsigned int bootAddr)
{
    LPM_Status      status = LPM_SOK;
    LPM_Private    *hndl = (LPM_Private *)instance;


    TRACE("+ LPM_setBootAddr\n");

    // Set DSP boot address in SYSTEM MODULE.
    REG(hndl->sysmVA + SYSM_DSPBOOTADDR) = bootAddr;

    TRACE("- LPM_setBootAddr: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== LPM_setPowerState_DaVinci ========
 */
static LPM_Status LPM_setPowerState_DaVinci(void *instance,
        LPM_PowerState state)
{
    LPM_Status          lpmStat = LPM_SOK;
    LPM_Private        *hndl = (LPM_Private *)instance;
    TAL_Status          talStat;
    TAL_TransportState  transState;
    unsigned int        i;


    TRACE("+ LPM_setPowerState: state = 0x%X\n", state);

    // Initiate the requested power state change.
    switch (state) {

        case LPM_HIBERNATE:
            // Set the transport state to unavailable.
            TRACE("= calling TAL_changeState\n");
            talStat = TAL_changeState(TAL_UNAVAILABLE, &transState);

            if (talStat != TAL_SOK) {
                TRACE(
                    "LPM_setPowerState_DaVinci: TAL_changeState failed (0x%X)\n",
                    talStat);
                lpmStat = LPM_EFAIL;
                goto fail;
            }

            // Send hibernate command to DSP.
            TRACE("= calling TAL_sendEvent\n");
            talStat = TAL_sendEvent(hndl->hTAL, PWRM_SH_SLEEP,
                                    PWRM_SH_HIBERNATE);

            if (talStat != TAL_SOK) {
                TRACE(
                    "LPM_setPowerState_DaVinci: TAL_sendEvent failed (0x%X)\n",
                    talStat);
                lpmStat = LPM_EFAIL;
                goto fail;
            }

            // Block here until callback signals this object.
            hndl->os_wait(hndl->os_instance);


            // Spin on the "save done" flag.
            for (i = 1; (MEM(hndl->saveDoneVA) != PWRM_SAVE_DONE_VAL); i++) {
                // empty
            }
            TRACE("= saveDone detected, i = %d\n", i);

            // Turn off power to DSP.
            lpmStat = LPM_off_DaVinci(instance);
            break;

#if 0
        case LPM_STANDBY:
            break;
#endif
    }


fail:
    TRACE("- LPM_setPowerState: stat = 0x%X\n", lpmStat);
    return lpmStat;
}


/*
 *  ======== LPM_init ========
 */
LPM_Status LPM_init (int index, LPM_Device *device, LPM_DevAttrs *attrs)
{
    LPM_Status      status = LPM_SOK;
    LPM_Private    *hndl;
    unsigned int    devID;


    // validate device index
    if ((index < 0) || (index > (LPM_DEV_COUNT -1))) {
        return LPM_EFAIL;
    }

    // initialize function pointers
    device->command         = LPM_command_DaVinci;
    device->connect         = LPM_connect_DaVinci;
    device->disconnect      = LPM_disconnect_DaVinci;
    device->off             = LPM_off_DaVinci;
    device->on              = LPM_on_DaVinci;
    device->resume          = LPM_resume_DaVinci;
    device->setPowerState   = LPM_setPowerState_DaVinci;


    // initialize private instance data
    device->instance = (void *)&LPM_OBJ[index];

    // now create the private handle
    hndl = (LPM_Private *)device->instance;
    hndl->setBootAddr = _LPM_setBootAddr_DaVinci;
    hndl->os_remap = attrs->os_remap;
    hndl->os_signal = attrs->os_signal;
    hndl->os_unmap = attrs->os_unmap;
    hndl->os_wait = attrs->os_wait;
    hndl->os_trace = attrs->os_trace;
    hndl->os_instance = attrs->os_instance;

    // set back pointer to device structure
    hndl->driver = device;

    // remap System Module
    status = hndl->os_remap(hndl->sysmPA, 0x0800, &hndl->sysmVA);

    if (status != LPM_SOK) {
        TRACE(
            "LPM_init: os_remap failed: sysmPA = 0x%X\n", hndl->sysmPA);
        goto fail;
    }

    // remap Power Sleep Controller
    status = hndl->os_remap(hndl->pscPA, 0x1000, &hndl->pscVA);

    if (status != LPM_SOK) {
        TRACE(
            "LPM_init: os_remap failed: pscPA = 0x%X\n", hndl->pscPA);
        goto fail;
    }

    // Update object state
    hndl->state |= LPM_STATE_INITIALIZED;

    // Print the device ID.
    devID = REG(hndl->sysmVA + SYSM_DEVICE_ID);
    TRACE("Manufacture ID:  0x%X\n", (devID & 0x00000FFE) >> 1);
    TRACE("Part number:     0x%X\n", (devID & 0x0FFFF000) >> 12);
    TRACE("Device revision: 0x%X\n", (devID & 0xF0000000) >> 28);


fail:
    // check for failure
    if (status != LPM_SOK) {
        if (hndl->sysmVA != 0) {
            hndl->os_unmap((void *)(hndl->sysmVA));
            hndl->sysmVA = 0;
        }
        if (hndl->pscVA != 0) {
            hndl->os_unmap((void *)(hndl->pscVA));
            hndl->pscVA = 0;
        }
    }
    TRACE("- LPM_init: stat = 0x%X\n", status);
    return status;
}


/*
 *  ======== _LPM_talCB ========
 */
void _LPM_talCB(void *arg, unsigned short event, unsigned short payload)
{
    LPM_Private    *hndl = (LPM_Private *)arg;


    TRACE("+ _LPM_talCB: event = 0x%X, payload = 0x%X\n", event, payload);

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
    TRACE("- _LPM_talCB\n");
    return;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

