/** ============================================================================
 *  @file   dra44xgem_hal.c
 *
 *  @path   $(DSPLINK)/gpp/src/arch/DRA44XGEM/
 *
 *  @desc   Power management module.
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
#include <dsplink.h>
#include <_dsplink.h>

/*  ----------------------------------- Trace & Debug               */
#include <_trace.h>
#include <trc.h>
#include <print.h>
#include <mem.h>

/*  ----------------------------------- Hardware Abstraction Layer  */
#include <dra44xgem_hal.h>
#if (DRA44XGEM_PHYINTERFACE == SHMEM_INTERFACE)
#include <dra44xgem_phy_shmem.h>
#endif /* if (DRA44XGEM_PHYINTERFACE == SHMEM_INTERFACE) */


#if defined (__cplusplus)
extern "C" {
#endif


/** ============================================================================
 *  @macro  COMPONENT_ID
 *
 *  @desc   Component and Subcomponent Identifier.
 *  ============================================================================
 */
#define  COMPONENT_ID       ID_ARCH_HAL

/** ============================================================================
 *  @macro  SET_FAILURE_REASON
 *
 *  @desc   Sets failure reason.
 *  ============================================================================
 */
#if defined (DDSP_DEBUG)
#define SET_FAILURE_REASON      TRC_SetReason (status, FID_C_ARCH_HAL, __LINE__)
#else
#define SET_FAILURE_REASON   { }
#endif /* if defined (DDSP_DEBUG) */


/** ============================================================================
 *  @func   DRA44XGEM_halInitialize
 *
 *  @desc   Initializes the HAL object.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
DRA44XGEM_halInit (IN     Pvoid * halObj,
                   IN     Pvoid   initParams)
{
    DSP_STATUS         status   = DSP_SOK ;
    DRA44XGEM_HalObj * halObject ;

    TRC_2ENTER ("DRA44XGEM_halInit", halObj, initParams) ;

    DBC_Require (halObj != NULL) ;

    (void) initParams ;

    status = MEM_Alloc ((Pvoid) &halObject,
                        sizeof (DRA44XGEM_HalObj),
                        MEM_DEFAULT) ;
    if (DSP_FAILED (status)) {
        SET_FAILURE_REASON ;
    }
    else {
        *halObj = (Pvoid) halObject ;
#if (DRA44XGEM_PHYINTERFACE == SHMEM_INTERFACE)
        halObject->interface = &DRA44XGEM_shmemInterface ;
#endif /* if (DRA44XGEM_PHYINTERFACE == SHMEM_INTERFACE) */
        status = halObject->interface->phyInit ((Pvoid) halObject) ;
        if (DSP_FAILED (status)) {
            FREE_PTR (halObject) ;
            SET_FAILURE_REASON ;
        }
    }

    TRC_1LEAVE ("DRA44XGEM_halInit", status) ;

    return status ;
}


/** ============================================================================
 *  @func   DRA44XGEM_halExit
 *
 *  @desc   Finalizes the HAL object.
 *
 *  @modif  None.
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
DRA44XGEM_halExit (IN     Pvoid * halObj)
{
    DSP_STATUS status   = DSP_SOK ;
    DRA44XGEM_HalObj * halObject ;

    TRC_1ENTER ("DRA44XGEM_halExit", halObj) ;

    DBC_Require (halObj != NULL) ;

    halObject = (DRA44XGEM_HalObj *) (*halObj);
    status = halObject->interface->phyExit ((Pvoid) halObject) ;
    if (DSP_FAILED (status)) {
        SET_FAILURE_REASON ;
    }

    FREE_PTR ((*halObj)) ;

    TRC_1LEAVE ("DRA44XGEM_halInit", status) ;

    return status ;
}


#if defined (__cplusplus)
}
#endif
