/** ============================================================================
 *  @file   omap3530.c
 *
 *  @path   $(DSPLINK)/gpp/src/arch/OMAP3530/
 *
 *  @desc   Implementation of DSP sub-component.
 *
 *  @ver    1.65.01.05_eng
 *  ============================================================================
 *  Copyright (C) 2002-2009, Texas Instruments Incorporated - http://www.ti.com/
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation version 2.
 *  
 *  This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 *  whether express or implied; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *  ============================================================================
 */



/*  ----------------------------------- DSP/BIOS Link               */
#include <gpptypes.h>
#include <dsplink.h>
#include <archdefs.h>
#include <errbase.h>
#include <_bitops.h>
#include <_intobject.h>
#include <_safe.h>

#include <loaderdefs.h>
#include <_dspdefs.h>
#include <_linkdefs.h>

/*  ----------------------------------- Trace & Debug               */
#include <_trace.h>
#include <trc.h>
#include <print.h>

/*  ----------------------------------- Profiling                   */
#if defined (DDSP_PROFILE)
#include <profile.h>
#endif /* if defined (DDSP_PROFILE) */

/*  ----------------------------------- OSAL Headers                */
#include <mem_os.h>
#include <mem.h>

/*  ----------------------------------- Link Driver                 */
#include <dsp.h>
#include <ldrv.h>
#include <ldrv_utils.h>

/*  ----------------------------------- Hardware Abstraction Layer  */
#include <omap3530_hal.h>
#include <omap3530_hal_mmu.h>
#include <omap3530_dspclk.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @macro  COMPONENT_ID
 *
 *  @desc   Component and Subcomponent Identifier.
 *  ============================================================================
 */
#define  COMPONENT_ID       ID_ARCH_DSP_INTF


/** ============================================================================
 *  @macro  SET_FAILURE_REASON
 *
 *  @desc   Sets failure reason.
 *  ============================================================================
 */
#if defined (DDSP_DEBUG)
#define SET_FAILURE_REASON   TRC_SetReason (status, FID_C_ARCH_DSP_INTF, __LINE__)
#else
#define SET_FAILURE_REASON
#endif /* if defined (DDSP_DEBUG) */


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_init
 *
 *  @desc   Resets the DSP and initializes the components required by DSP.
 *          Puts the DSP in RESET state.
 *
 *  @arg    dspId
 *              Processor Id.
 *  @arg    dspState
 *              DSP state Object.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument.
 *          DSP_EVALUE
 *              Invalid DSP MMU endianism configuration.
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *
 *  @leave  None.
 *
 *  @see    OMAP3530_setup, OMAP3530_exit
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_init (IN ProcessorId  dspId, IN DSP_Object * dspState) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_exit
 *
 *  @desc   Resets the DSP and puts it into IDLE Mode.
 *
 *  @arg    dspId
 *              Processor Id.
 *  @arg    dspState
 *              DSP state Object.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument..
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *
 *  @leave  None.
 *
 *  @see    OMAP3530_init
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_exit (IN ProcessorId dspId, IN DSP_Object * dspState) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_start
 *
 *  @desc   Causes DSP to start execution from the given DSP address.
 *          DSP is put to STARTED state after successful completion.
 *
 *  @arg    dspId
 *              Processor Id.
 *  @arg    dspState
 *              DSP state Object.
 *  @arg    dspAddr
 *              Address to start execution from.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument..
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *
 *  @leave  None.
 *
 *  @see    OMAP3530_stop
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_start (IN ProcessorId  dspId,
                  IN DSP_Object * dspState,
                  IN Uint32       dspAddr) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_stop
 *
 *  @desc   Stops execution on DSP.
 *          DSP is put to STOPPED state after successful completion.
 *
 *  @arg    dspId
 *              Processor Id.
 *  @arg    dspState
 *              DSP state Object.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument..
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *
 *  @leave  None.
 *
 *  @see    OMAP3530_start
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_stop (IN ProcessorId dspId, IN DSP_Object * dspState) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_idle
 *
 *  @desc   Idles the DSP.
 *          DSP is put to IDLE state after successful completion.
 *
 *  @arg    dspId
 *              Processor Id.
 *  @arg    dspState
 *              DSP state Object.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument..
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *
 *  @leave  None.
 *
 *  @see    OMAP3530_start
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_idle (IN ProcessorId dspId, IN DSP_Object * dspState) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_intCtrl
 *
 *  @desc   Perform the specified DSP interrupt control activity.
 *
 *  @arg    dspId
 *              Processor ID.
 *  @arg    dspState
 *              DSP state Object.
 *  @arg    intId
 *              Interrupt ID.
 *  @arg    cmd
 *              Interrupt control command to be performed.
 *  @arg    arg
 *              Optional input/output argument specific to each control command.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument.
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *          isrObj must be a valid pointer.
 *
 *  @leave  None.
 *
 *  @see    DSP_start
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_intCtrl (IN         ProcessorId       dspId,
                   IN         DSP_Object     *  dspState,
                   IN         Uint32            intId,
                   IN         DSP_IntCtrlCmd    cmd,
                   IN OUT     Pvoid             arg) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_read
 *
 *  @desc   Reads data from DSP.
 *
 *  @arg    dspId
 *              Processor ID.
 *  @arg    dspState
 *              DSP state Object.
 *  @arg    dspAddr
 *              DSP address to read from.
 *  @arg    endianInfo
 *              endianness of data - indicates whether swap is required or not.
 *  @arg    numBytes
 *              Number of bytes to read.
 *  @arg    buffer
 *              Buffer to hold the read data.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument..
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *          numBytes must be a valid pointer.
 *          buffer must be a valid pointer.
 *
 *  @leave  On successful completion, holds the number of bytes read.
 *
 *  @see    None.
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_read (IN  ProcessorId  dspId,
                IN  DSP_Object * dspState,
                IN  Uint32       dspAddr,
                IN  Endianism    endianInfo,
                IN  Uint32       numBytes,
                OUT Uint8 *      buffer) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_write
 *
 *  @desc   Writes data to DSP.
 *
 *  @arg    dspId
 *              Processor ID.
 *  @arg    dspState
 *              DSP state Object.
 *  @arg    dspAddr
 *              DSP address to write to.
 *  @arg    endianInfo
 *              endianness of data - indicates whether swap is required or not.
 *  @arg    numBytes
 *              Number of bytes to write.
 *  @arg    buffer
 *              Buffer containing data to be written.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument..
 *          DSP_EFAIL
 *              DSP_setup function wasn't called before calling this function.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *          buffer must be a valid pointer.
 *
 *  @leave  None.
 *
 *  @see    None.
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_write (IN ProcessorId  dspId,
                 IN DSP_Object * dspState,
                 IN Uint32       dspAddr,
                 IN Endianism    endianInfo,
                 IN Uint32       numBytes,
                 IN Uint8 *      buffer) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_addrConvert
 *
 *  @desc   Convert address between GPP and DSP address space.
 *
 *  @arg    dspId
 *              Processor Id.
 *  @arg    dspState
 *              DSP state Object.
 *  @arg    addr
 *              Address to be converted. If DSP address, the addr parameter
 *              reflects the DSP MADU address.
 *  @arg    type
 *              Type of address conversion.
 *
 *  @ret    Converted address.
 *              Operation successfully completed.
 *          ADDRMAP_INVALID.
 *              Specified address is not in mapped range.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *
 *  @leave  None.
 *
 *  @see    DSP_AddrConvType
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
Uint32
OMAP3530_addrConvert (IN  ProcessorId        dspId,
                       IN  DSP_Object *       dspState,
                       IN  Uint32             addr,
                       IN  DSP_AddrConvType   type) ;


/** ----------------------------------------------------------------------------
 *  @func   OMAP3530_control
 *
 *  @desc   Hook for performing device dependent control operation.
 *
 *  @arg    dspId
 *              Processor ID.
 *  @arg    dspState
 *              DSP state Object.
 *  @arg    cmd
 *              Command id.
 *  @arg    arg
 *              Optional argument for the specified command.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid arguments specified.
 *
 *  @enter  DSP_setup must be called before calling this function.
 *          dspId must be a valid DSP identifier.
 *
 *  @leave  None.
 *
 *  @see    None.
 *  ----------------------------------------------------------------------------
 */
NORMAL_API
DSP_STATUS
OMAP3530_control (IN  ProcessorId  dspId,
                   IN  DSP_Object * dspState,
                   IN  Int32        cmd,
                       Pvoid        arg) ;


/** ============================================================================
 *  @name   OMAP3530_Interface
 *
 *  @desc   The interface table exposed for the DSP component.
 *  ============================================================================
 */
DSP_Interface OMAP3530_Interface = {
    &OMAP3530_init,
    &OMAP3530_exit,
    &OMAP3530_start,
    &OMAP3530_stop,
    &OMAP3530_idle,
    &OMAP3530_intCtrl,
    &OMAP3530_read,
    &OMAP3530_write,
    &OMAP3530_addrConvert,
    &OMAP3530_control,
} ;


/** ============================================================================
 *  @func   OMAP3530_init
 *
 *  @desc   Initializes the DSP.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_init (IN ProcessorId  dspId, IN DSP_Object * dspState)
{
    DSP_STATUS          status = DSP_SOK ;
    OMAP3530_HalObj  *  halObj    = NULL ;
    LINKCFG_DspConfig * dspCfg           ;
    MemMapInfo          mapInfo          ;
    LINKCFG_Dsp *       dspObj           ;
    LINKCFG_MemEntry *  memTable         ;
    Uint32              i                ;
    Uint32              cpuFreq          ;

    TRC_2ENTER ("OMAP3530_init", dspId, dspState) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;

    if (IS_VALID_PROCID (dspId) == FALSE) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }
    else {
        dspCfg    = LDRV_LinkCfgPtr->dspConfigs [dspId] ;
        dspObj    = dspCfg->dspObject ;
        memTable  = dspCfg->memTables [dspObj->memTableId] ;
        if (     (dspObj->doDspCtrl != DSP_BootMode_Boot_Pwr)
             &&  (dspObj->doDspCtrl != DSP_BootMode_NoLoad_NoPwr)
             &&  (dspObj->doDspCtrl != DSP_BootMode_NoLoad_Pwr)
             &&  (dspObj->doDspCtrl != DSP_BootMode_NoBoot)
             &&  (dspObj->doDspCtrl != DSP_BootMode_Boot_NoPwr)) {
            /* Check if the doDspCtrl is valid. */
            PRINT_Printf ("Configuration error:"
                          " Incorrect DSP doDspCtrl specified [0x%x]\n",
                          dspObj->doDspCtrl) ;
            status = DSP_ECONFIG ;
            SET_FAILURE_REASON ;
        }
        else {
            for (i = 0 ;
                 (i < dspObj->memEntries) && DSP_SUCCEEDED (status) ;
                 i++) {
                /* If the configured GPP virtual address is invalid, get the actual
                 * address by mapping the physical address into GPP kernel memory
                 * space.
                 */
                if ((memTable [i].gppVirtAddr == (Uint32) -1) &&
                    (memTable [i].shared == TRUE)) {
                    mapInfo.src  = memTable [i].physAddr ;
                    mapInfo.size = memTable [i].size ;
                    mapInfo.memAttrs = MEM_UNCACHED ;

                    status = MEM_Map (&mapInfo) ;
                    if (DSP_SUCCEEDED (status)) {
                        memTable [i].gppVirtAddr = mapInfo.dst ;
                    }
                    else {
                        SET_FAILURE_REASON ;
                    }
                }
            }

            if (DSP_SUCCEEDED (status)) {
                status = OMAP3530_halInit (&dspState->halObject, NULL) ;
                if (DSP_FAILED (status)) {
                    SET_FAILURE_REASON ;
                }
                else  {
                    halObj = (OMAP3530_HalObj *) dspState->halObject ;
                    halObj->procId = dspId ;
                }
            }

            if (DSP_SUCCEEDED (status)) {
#if defined (OS_WINCE)
                status = OMAP3530_getDspClkRate ((Pvoid)dspState->halObject, &cpuFreq) ;
#else
                status = OMAP3530_getDspClkRate ("iva2_ck", &cpuFreq) ;
#endif
                if (DSP_SUCCEEDED (status)) {
                    /* Use default by getting clock rate from Linux
                     * Override the configured value
                     */
                    dspObj->cpuFreq = cpuFreq ;
                }
            }
            TRC_1PRINT (TRC_LEVEL3, "Setting DSP Clk Rate %d\n", dspObj->cpuFreq) ;

            if (DSP_SUCCEEDED (status)) {
                if (   (dspObj->doDspCtrl == DSP_BootMode_Boot_Pwr)
                    || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_Pwr)) {
                    status = halObj->interface->pwrCtrl ((Pvoid) halObj,
                                                         DSP_PwrCtrlCmd_PowerUp,
                                                         NULL) ;
                    if (DSP_FAILED (status)) {
                        SET_FAILURE_REASON ;
                    }
                }

                if (   (dspObj->doDspCtrl == DSP_BootMode_Boot_Pwr)
                    || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_NoPwr)
                    || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_Pwr)
                    || (dspObj->doDspCtrl == DSP_BootMode_Boot_NoPwr)) {
                    /* Put the DPS in reset mode */
                    status = halObj->interface->pwrCtrl ((Pvoid) halObj,
                                                         DSP_PwrCtrlCmd_Reset,
                                                         NULL) ;
                    if (DSP_FAILED (status)) {
                        SET_FAILURE_REASON ;
                    }

                    if (DSP_SUCCEEDED (status)) {
                        /* Handle the enabling of the DSP's interrupts */
                        status = halObj->interface->intCtrl ((Pvoid) halObj,
                                                             DSP_IntCtrlCmd_Enable,
                                                             0,
                                                             NULL) ;
                        if (DSP_FAILED (status)) {
                            SET_FAILURE_REASON ;
                        }
                    }
                    if (DSP_SUCCEEDED (status)) {
                        /* Handle the enabling of the DSP's MMU */
                        status = halObj->interface->mmuCtrl ((Pvoid) halObj,
                                         DSP_MmuCtrlCmd_Enable,
                                         dspCfg->memTables [dspObj->memTableId],
                                         (Uint32)dspObj->memEntries,
                                         0);
                        if (DSP_FAILED (status)) {
                            SET_FAILURE_REASON ;
                        }
                    }
                }
            }
        }
    }

    TRC_1LEAVE ("OMAP3530_init", status) ;

    return status ;
}


/** ============================================================================
 *  @func   OMAP3530_exit
 *
 *  @desc   Finalizes the DSP sub-component. Either of following can be
 *          implemeted here based on application requirement:
 *          1. Reset the DSP as it it no longer required by application.
 *          2. Call OMAP3530_idle () to let the DSP run in 'self loop'.
 *          3. Let the DSP run free without modifying its execution state.
 *
 *          Option 1 is implemented here.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_exit (IN ProcessorId dspId, IN DSP_Object * dspState)
{
    DSP_STATUS          status = DSP_SOK ;
    MemUnmapInfo        unmapInfo        ;
    LINKCFG_Dsp *       dspObj           ;
    LINKCFG_MemEntry *  memTable         ;
    OMAP3530_HalObj *  halObj           ;
    LINKCFG_DspConfig * dspCfg           ;
    Uint32              i                ;

    TRC_2ENTER ("OMAP3530_exit", dspId, dspState) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (dspState->halObject != NULL) ;

    if (IS_VALID_PROCID (dspId) == FALSE) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }
    else {
        dspCfg    = LDRV_LinkCfgPtr->dspConfigs [dspId] ;
        dspObj    = dspCfg->dspObject ;
        memTable  = dspCfg->memTables [dspObj->memTableId] ;

        /* Check if DSP needs to be finalized. */
        if (dspState->halObject != NULL) {
            halObj = (OMAP3530_HalObj *) dspState->halObject ;
            if (   (dspObj->doDspCtrl == DSP_BootMode_Boot_Pwr)
                    || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_NoPwr)
                    || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_Pwr)
                    || (dspObj->doDspCtrl == DSP_BootMode_Boot_NoPwr)) {
                /* Handle the disabling of the DSP's MMU */
                status = halObj->interface->mmuCtrl ((Pvoid) halObj,
                                                     DSP_MmuCtrlCmd_Disable,
                                                     0,
                                                     0,
                                                     0);
                DBC_Assert (DSP_SUCCEEDED (status)) ;
                if (DSP_FAILED (status)) {
                   SET_FAILURE_REASON ;
                }

                if (DSP_SUCCEEDED (status)) {
                    /* Handle the enabling of the DSP's interrupts */
                    status = halObj->interface->intCtrl ((Pvoid) halObj,
                                                     DSP_IntCtrlCmd_Disable,
                                                     0,
                                                     NULL);
                    if (DSP_FAILED (status)) {
                       SET_FAILURE_REASON ;
                    }
                }
            }
            if (   (dspObj->doDspCtrl == DSP_BootMode_Boot_Pwr)
                || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_Pwr)) {
                status = halObj->interface->pwrCtrl ((Pvoid) halObj,
                                         DSP_PwrCtrlCmd_PowerDown,
                                         NULL) ;
                if (DSP_FAILED (status)) {
                    SET_FAILURE_REASON ;
                }
            }
            for (i = 0 ; i < dspObj->memEntries ; i++) {
                if (memTable [i].shared == TRUE) {
                    unmapInfo.addr = memTable [i].gppVirtAddr ;
                    unmapInfo.size = memTable [i].size ;
                    if (unmapInfo.addr != 0) {
                        status = MEM_Unmap (&unmapInfo) ;
                        if (DSP_FAILED (status)) {
                            SET_FAILURE_REASON ;
                        }
                    }
                    memTable [i].gppVirtAddr = (Uint32) -1 ;
                }
            }
            OMAP3530_halExit (&dspState->halObject) ;
        }
    }

    TRC_1LEAVE ("OMAP3530_exit", status) ;

    return status ;
}


/** ============================================================================
 *  @func   OMAP3530_start
 *
 *  @desc   Causes DSP to start execution from the given DSP address.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_start (IN ProcessorId  dspId,
                  IN DSP_Object * dspState,
                  IN Uint32       dspAddr)
{
    DSP_STATUS          status         = DSP_SOK ;
    Uint32              gppAddr        = ADDRMAP_INVALID ;
    Uint32              entryPtAddrHi  = 0x00000000 ;
    Uint32              entryPtAddrLo  = 0x00000000 ;
    Uint32              startOpCodeLo  = 0x0000002A ;
    Uint32              startOpCodeHi  = 0x0000006A ;
    Uint32              noOp           = 0x00400000 ;
    Uint32              branch         = 0x00000362 ;
    OMAP3530_HalObj *  halObj          = NULL       ;
    LINKCFG_Ips *       ipsTable       = NULL       ;
    Uint32              resetVector                 ;
    LINKCFG_Dsp *       dspObj                      ;
    LINKCFG_DspConfig * dspCfg                      ;
    LINKCFG_LinkDrv *   linkDrv                     ;

    TRC_3ENTER ("OMAP3530_start", dspId, dspState, dspAddr) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (dspState->halObject != NULL) ;

    if (IS_VALID_PROCID (dspId) == FALSE) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }
    else {
        dspCfg    = LDRV_LinkCfgPtr->dspConfigs [dspId] ;
        dspObj    = dspCfg->dspObject ;
        halObj = (OMAP3530_HalObj *) dspState->halObject ;
        linkDrv  = &(dspCfg->linkDrvObjects [dspObj->linkDrvId]) ;
        ipsTable = &(dspCfg->ipsTables [linkDrv->ipsTableId][0]) ;

        if (   (dspObj->doDspCtrl == DSP_BootMode_Boot_Pwr)
            || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_NoPwr)
            || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_Pwr)
            || (dspObj->doDspCtrl == DSP_BootMode_Boot_NoPwr)) {
            resetVector = dspObj->resetVector ;
            gppAddr = OMAP3530_addrConvert (
                                   dspId,
                                   dspState,
                                   BYTE_TO_MADU (resetVector, dspObj->maduSize),
                                   DspToGpp) ;
            if (gppAddr != ADDRMAP_INVALID) {
                /* Specify the DSP boot address in the boot config register */
                status = halObj->interface->bootCtrl (
                                                  (Pvoid) halObj,
                                                  DSP_BootCtrlCmd_SetEntryPoint,
                                                  (Pvoid) resetVector);

                if (DSP_SUCCEEDED (status)) {
                    /* Write the branch instruction to at the boot address to
                     * branch to _c_int00
                     */
                    entryPtAddrHi  = dspAddr >> 16 ;
                    entryPtAddrLo  = dspAddr & 0xFFFF ;
                    startOpCodeHi |= (entryPtAddrHi << 7) ;
                    startOpCodeLo |= (entryPtAddrLo << 7) ;
                    REG (gppAddr)  = startOpCodeLo ;
                    gppAddr += 4 ;
                    REG (gppAddr) = startOpCodeHi ;
                    gppAddr += 4 ;
                    REG (gppAddr) = branch        ;
                    gppAddr += 4 ;

                    /* Write 5 no-ops for pipeline flush */
                    REG (gppAddr) = noOp          ;
                    gppAddr += 4 ;
                    REG (gppAddr) = noOp          ;
                    gppAddr += 4 ;
                    REG (gppAddr) = noOp          ;
                    gppAddr += 4 ;
                    REG (gppAddr) = noOp          ;
                    gppAddr += 4 ;
                    REG (gppAddr) = noOp          ;

                    /* Release the DSP from reset */
                    status = halObj->interface->pwrCtrl ((Pvoid) halObj,
                                                         DSP_PwrCtrlCmd_Release,
                                                         NULL);
                    if (DSP_FAILED (status)) {
                        SET_FAILURE_REASON ;
                    }
                }
                else {
                    SET_FAILURE_REASON ;
                }
            }
            else {
                status = DSP_ERANGE ;
                PRINT_Printf ("Error: DSP reset vector is out of range.\n"
                              " Check /dsplink/config/all/CFG_<PLATFORM>.c for "
                              " RESETVECTOR.\n") ;
            }
        }
        if (   (dspObj->doDspCtrl == DSP_BootMode_NoBoot)) {
            if (DSP_SUCCEEDED (status)) {
                status = halObj->interface->intCtrl ((Pvoid) halObj,
                                                     DSP_IntCtrlCmd_Send,
                                                     ipsTable->dspIntId,
                                                     NULL) ;
                if (DSP_FAILED (status)) {
                    SET_FAILURE_REASON ;
                }
            }
        }
    }

    if (DSP_SUCCEEDED (status)) {
            TRC_0PRINT (TRC_LEVEL1, "DSP started !\n") ;
        }
        else {
            SET_FAILURE_REASON ;
            TRC_0PRINT (TRC_LEVEL7, "DSP couldn't be started !\n") ;
    }

    TRC_1LEAVE ("OMAP3530_start", status) ;

    return status ;
}


/** ============================================================================
 *  @func   OMAP3530_stop
 *
 *  @desc   Stops execution on DSP.
 *          DSP transitions to STOPPED state after successful completion.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_stop (IN ProcessorId dspId, IN DSP_Object * dspState)
{
    DSP_STATUS          status  = DSP_SOK ;
    OMAP3530_HalObj *   halObj            ;
    LINKCFG_Dsp *       dspObj            ;
    LINKCFG_DspConfig * dspCfg            ;

    TRC_2ENTER ("OMAP3530_stop", dspId, dspState) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (dspState->halObject != NULL) ;

    if (IS_VALID_PROCID (dspId) == FALSE) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }
    else {
        dspCfg = LDRV_LinkCfgPtr->dspConfigs [dspId] ;
        dspObj = dspCfg->dspObject ;
        halObj = (OMAP3530_HalObj *) dspState->halObject ;
        /* Release the DSP from reset */
        if (   (dspObj->doDspCtrl == DSP_BootMode_Boot_Pwr)
            || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_NoPwr)
            || (dspObj->doDspCtrl == DSP_BootMode_NoLoad_Pwr)
            || (dspObj->doDspCtrl == DSP_BootMode_Boot_NoPwr)) {
            status = halObj->interface->pwrCtrl ((Pvoid) halObj,
                                             DSP_PwrCtrlCmd_Reset,
                                             NULL);
            DBC_Assert (DSP_SUCCEEDED (status)) ;
            if (DSP_FAILED (status)) {
                SET_FAILURE_REASON ;
            }
        }
    }

    TRC_1LEAVE ("OMAP3530_stop", status) ;

    return status ;
}


/** ============================================================================
 *  @func   OMAP3530_idle
 *
 *  @desc   Places the DSP into idle.
 *          This function provides hook for power handling. Instead of an empty
 *          function, power handling can be written as part of 'idle code'.

 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_idle (IN ProcessorId dspId, IN DSP_Object * dspState)
{
    DSP_STATUS status = DSP_SOK ;

    TRC_2ENTER ("OMAP3530_idle", dspId, dspState) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (dspState->halObject != NULL) ;

    if (IS_VALID_PROCID (dspId) == FALSE) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }

    TRC_1LEAVE ("OMAP3530_idle", status) ;

    return status ;
}


/** ============================================================================
 *  @func   OMAP3530_intCtrl
 *
 *  @desc   Performs the specified DSP interrupt control activity.
 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_intCtrl (IN         ProcessorId       dspId,
                   IN         DSP_Object *      dspState,
                   IN         Uint32            intId,
                   IN         DSP_IntCtrlCmd    cmd,
                   IN OUT     Pvoid             arg)
{
    DSP_STATUS          status = DSP_SOK ;
    OMAP3530_HalObj *  halObj           ;

    TRC_5ENTER ("OMAP3530_intCtrl", dspId, dspState, intId, cmd, arg) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (dspState->halObject != NULL) ;

    if (IS_VALID_PROCID (dspId) == FALSE) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }
    else {
        halObj = (OMAP3530_HalObj *) dspState->halObject ;
        status = halObj->interface->intCtrl ((Pvoid) halObj,
                                             cmd,
                                             intId,
                                             (Pvoid) arg) ;
        if (DSP_FAILED (status)) {
            SET_FAILURE_REASON ;
        }
    }

    TRC_1LEAVE ("OMAP3530_intCtrl", status) ;

    return status ;
}


/** ============================================================================
 *  @func   OMAP3530_read
 *
 *  @desc   Read data from DSP.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_read (IN  ProcessorId  dspId,
                 IN DSP_Object *  dspState,
                 IN  Uint32       dspAddr,
                 IN  Endianism    endianInfo,
                 IN  Uint32       numBytes,
                 OUT Uint8 *      buffer)
{
    DSP_STATUS          status  = DSP_SOK ;
    Uint32              gppAddr = ADDRMAP_INVALID ;
    Uint8  *            dspPtr8 = NULL ;
    LINKCFG_Dsp *       dspObj ;
    LINKCFG_DspConfig * dspCfg ;

    TRC_6ENTER ("OMAP3530_read",
                dspId,
                dspState,
                dspAddr,
                endianInfo,
                numBytes,
                buffer) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (numBytes != 0) ;
    DBC_Require (buffer   != NULL) ;
    DBC_Require (dspState->halObject != NULL) ;

    if (   (IS_VALID_PROCID (dspId) == FALSE)
        || (numBytes == 0)
        || (buffer   == NULL)
        || (   (endianInfo != Endianism_Big)
            && (endianInfo != Endianism_Little)
            && (endianInfo != Endianism_Default))) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }
    else {
        dspCfg    = LDRV_LinkCfgPtr->dspConfigs [dspId] ;
        dspObj    = dspCfg->dspObject ;

        gppAddr = OMAP3530_addrConvert (
                                   dspId,
                                   dspState,
                                   BYTE_TO_MADU (dspAddr, dspObj->maduSize),
                                   DspToGpp) ;
        if (gppAddr != ADDRMAP_INVALID) {
            dspPtr8 = (Uint8 *)  gppAddr ;
            status = MEM_Copy (buffer,
                               dspPtr8,
                               numBytes,
                               Endianism_Default) ;
            DBC_Assert (DSP_SUCCEEDED (status)) ;

        }
        else {
            status = DSP_ERANGE ;
            SET_FAILURE_REASON ;
        }
#if defined (DDSP_PROFILE)
        DSP_State [dspId].dspStats.dataDspToGpp += numBytes ;
#endif /* defined (DDSP_PROFILE) */
    }

    TRC_1LEAVE ("OMAP3530_read", status) ;

    return status ;
}


/** ============================================================================
 *  @func   OMAP3530_write
 *
 *  @desc   Write data to DSP.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_write (IN ProcessorId   dspId,
                  IN DSP_Object *  dspState,
                  IN Uint32        dspAddr,
                  IN Endianism     endianInfo,
                  IN Uint32        numBytes,
                  IN Uint8 *       buffer)
{
    DSP_STATUS          status  = DSP_SOK ;
    Uint32              gppAddr = ADDRMAP_INVALID ;
    Uint8  *            dspPtr8 = NULL ;
    LINKCFG_Dsp *       dspObj ;
    LINKCFG_DspConfig * dspCfg ;
    Uint8               temp8_1 ;
    Uint8               temp8_2 ;
    Uint8               temp8_3 ;
    Uint8               temp8_4 ;
    Uint32              temp ;

    TRC_6ENTER ("OMAP3530_write",
                dspId,
                dspState,
                dspAddr,
                endianInfo,
                numBytes,
                buffer) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (buffer != NULL) ;
    DBC_Require (dspState->halObject != NULL) ;

    if (   (IS_VALID_PROCID (dspId) == FALSE)
        || (buffer == NULL)
        || (   (endianInfo != Endianism_Big)
            && (endianInfo != Endianism_Little)
            && (endianInfo != Endianism_Default))) {
        status = DSP_EINVALIDARG ;
        SET_FAILURE_REASON ;
    }
    else {
        dspCfg    = LDRV_LinkCfgPtr->dspConfigs [dspId] ;
        dspObj    = dspCfg->dspObject ;

        /*  --------------------------------------------------------------------
         *  Check if 'dspAddr' lies in 'Self Loop Area'
         *  --------------------------------------------------------------------
         */
        if (    ((dspAddr + numBytes) > dspObj->resetVector)
            &&  (dspAddr              < (   dspObj->resetVector
                                         +  dspObj->resetCodeSize))) {
            status = DSP_EFAIL ;
            SET_FAILURE_REASON ;
        }
        else {
            gppAddr = OMAP3530_addrConvert (
                                       dspId,
                                       dspState,
                                       BYTE_TO_MADU (dspAddr, dspObj->maduSize),
                                       DspToGpp) ;
            if (gppAddr != ADDRMAP_INVALID) {
                if (numBytes != sizeof(Uint32)) {
                    dspPtr8 = (Uint8 *)  gppAddr ;
                    status = MEM_Copy (dspPtr8,
                                        buffer,
                                        numBytes,
                                        Endianism_Default) ;
                    DBC_Assert (DSP_SUCCEEDED (status)) ;
                }
                else  {
                     /* For 4 bytes, directly write as a Uint32 */
                    temp8_1 = ((Uint8 *)buffer)[0] ;
                    temp8_2 = ((Uint8 *)buffer)[1] ;
                    temp8_3 = ((Uint8 *)buffer)[2] ;
                    temp8_4 = ((Uint8 *)buffer)[3] ;
                    temp = (Uint32) (    ((Uint32) temp8_4 << 24)
                                     |   ((Uint32) temp8_3 << 16)
                                     |   ((Uint32) temp8_2 << 8)
                                     |   ((Uint32) temp8_1)) ;
                    *((Uint32*)gppAddr)      = temp ;
                }
            }
            else {
                status = DSP_ERANGE ;
                SET_FAILURE_REASON ;
            }
        }

#if defined (DDSP_PROFILE)
        if (DSP_SUCCEEDED (status)) {
            DSP_State [dspId].dspStats.dataGppToDsp += numBytes ;
        }
#endif /* defined (DDSP_PROFILE) */
    }

    TRC_1LEAVE ("OMAP3530_write", status) ;

    return status ;
}


/*  ============================================================================
 *  @func   OMAP3530_addrConvert
 *
 *  @desc   Convert address between GPP and DSP address space.
 *
 *  @modif  None
 *  ============================================================================
 */
NORMAL_API
Uint32
OMAP3530_addrConvert (IN  ProcessorId         dspId,
                        IN  DSP_Object *       dspState,
                        IN  Uint32             addr,
                        IN  DSP_AddrConvType   type)
{
    Uint32              convAddr = (Uint32) ADDRMAP_INVALID ;
    Bool                found    = FALSE ;
    LINKCFG_DspConfig * dspCfg ;
    LINKCFG_Dsp *       dspObj ;
    LINKCFG_MemEntry *  memEntry ;
    Uint32              byteAddr ;
    Uint32              i ;

    TRC_4ENTER ("OMAP3530_addrConvert", dspId, dspState, addr, type) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;
    DBC_Require (dspState->halObject != NULL) ;

    dspCfg    = LDRV_LinkCfgPtr->dspConfigs [dspId] ;
    dspObj    = dspCfg->dspObject ;

    for (i = 0 ; (found == FALSE) && (i < dspObj->memEntries) ; i++) {
        memEntry = &(dspCfg->memTables [dspObj->memTableId][i]) ;

        /* Determine whether which way to convert */
        if (type == GppToDsp) {
            if (IS_RANGE_VALID (addr,
                                memEntry->gppVirtAddr,
                                (  memEntry->gppVirtAddr
                                 + memEntry->size))) {
                found = TRUE ;
                convAddr = BYTE_TO_MADU ((  (addr - memEntry->gppVirtAddr)
                                          + memEntry->dspVirtAddr),
                                         dspObj->maduSize) ;
            }
        }
        else if (type == DspToGpp) {
            byteAddr = MADU_TO_BYTE (addr, dspObj->maduSize) ;
            if (IS_RANGE_VALID (byteAddr,
                                memEntry->dspVirtAddr,
                                (  memEntry->dspVirtAddr
                                 + memEntry->size))) {
                found = TRUE ;
                convAddr =    byteAddr
                           -  memEntry->dspVirtAddr
                           +  memEntry->gppVirtAddr ;
            }
        }
        else {
            /* Added for MISRAC compliance */
        }
    }

    TRC_2PRINT (TRC_LEVEL1,
                "OMAP3530_addrConvert addr [0x%x] convAddr [0x%x]\n",
                addr,
                convAddr) ;

    TRC_0LEAVE ("OMAP3530_addrConvert") ;

    return convAddr ;
}


/** ============================================================================
 *  @func   OMAP3530_control
 *
 *  @desc   Hook for performing device dependent control operation.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
OMAP3530_control (IN  ProcessorId dspId,
                    IN DSP_Object * dspState,
                    IN  Int32       cmd,
                        Pvoid       arg)
{
    DSP_STATUS status = DSP_SOK ;
    ProcMemMapInfo  * mmuInfo = arg    ;
    OMAP3530_HalObj * halObj           ;

    halObj = (OMAP3530_HalObj *) dspState->halObject ;

    TRC_4ENTER ("OMAP3530_control", dspId, dspState, cmd, arg) ;

    DBC_Require (IS_VALID_PROCID (dspId)) ;

    switch (cmd) {

        case PROC_CTRL_CMD_MMU_ADD_ENTRY:
            {

                status = halObj->interface->mmuCtrl ((Pvoid) halObj,
                                                     DSP_MmuCtrlCmd_AddEntry,
                                                     0,
                                                     0,
                                                     mmuInfo);
                if (DSP_FAILED (status)) {
                    SET_FAILURE_REASON ;
                }
            }
            break ;

        case PROC_CTRL_CMD_MMU_DEL_ENTRY:
            {

                status = halObj->interface->mmuCtrl ((Pvoid) halObj,
                                                     DSP_MmuCtrlCmd_DeleteEntry,
                                                     0,
                                                     0,
                                                     mmuInfo);
                if (DSP_FAILED (status)) {
                    SET_FAILURE_REASON ;
                }
            }
            break ;

        case PROC_CTRL_CMD_MAP_DSPMEM:
            {
                /* Map requested region to user space address */
                mmuInfo->mappedAddr = mmuInfo->dspAddr ;
                mmuInfo->mappedSize = mmuInfo->size ;
            }
            break ;

        case PROC_CTRL_CMD_UNMAP_DSPMEM:
            {
                /* Unmap the requested mapped region */
                mmuInfo->mappedAddr = mmuInfo->dspAddr ;
                mmuInfo->mappedSize = mmuInfo->size ;
            }
            break ;

        default:
            status = DSP_ENOTIMPL ;
            /* do nothing */
            break ;
    }

    TRC_1LEAVE ("OMAP3530_control", status) ;

    return status ;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
