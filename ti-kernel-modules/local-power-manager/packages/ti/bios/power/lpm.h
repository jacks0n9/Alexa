/* 
 * Copyright (c) 2011, Texas Instruments Incorporated
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
 * 
 */

/**
 *  @file       ti/bios/power/lpm.h
 *
 *  @brief      This header defines the user level types, constants,
 *              and functions for controlling the Local Power Manager.
 *
 *  @addtogroup   ti_bios_power_LPM     Shared Definitions
 */


#if !defined (LPM_H)
#define LPM_H


#include <stdio.h>

#include <ti/bios/power/lpm_base.h>



#if defined(__cplusplus)
EXTERN "C" {
#endif

/** @ingroup    ti_bios_power_LPM */
/*@{*/

/**
 *  @brief      Opaque handle to LPM object.
 */
typedef struct LPM_Object *LPM_Handle;

/**
 *  @brief      Opaque handle to device object.
 */
typedef struct LPM_DevObj *LPM_DeviceHandle;

/**
 *  @brief      Configuration database structure.
 */
typedef struct {
    char               *name;       /**< Linux device name */
    LPM_DeviceHandle   lpm_dev;     /**< Linux device handle */
} LPM_Cfg;


/**
 *  @brief      External Power off function type.
 */
typedef void (*LPM_ExtOffFxn)(void *);

/**
 *  @brief      External Power on function type.
 */
typedef void (*LPM_ExtOnFxn)(void *);



/*
 *  ======== LPM_clear ========
 */
/**
 *  @brief      Clear control bits in the LPM driver
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         The LPM object was successfuly modified.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_clear(LPM_Handle handle, unsigned int mask);


/*
 *  ======== LPM_close ========
 */
/**
 *  @brief      Close an LPM object
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         The LPM object was successfuly closed
 *                              and all associated resources have been freed.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_close(LPM_Handle handle);


/*
 *  ======== LPM_connect ========
 */
/**
 *  @brief      Open a connection to the transport layer.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         A connection was made to the transport layer.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_connect(LPM_Handle handle);


/*
 *  ======== LPM_disconnect ========
 */
/**
 *  @brief      Close a connection to the transport layer.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         The connection was successfully closed.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 *
 *  @pre        There is an existing connection to the transport layer.
 */
LPM_Status LPM_disconnect(LPM_Handle handle);


/*
 *  ======== LPM_exit ========
 */
/**
 *  @brief      Finalize the LPM module.
 *
 *  Do all finalization required by the LPM module. LPM_exit
 *  must be called as part of the module shutdown procedure.
 */
void LPM_exit(void);


/*
 *  ======== LPM_init ========
 */
/**
 *  @brief      Initialize the LPM module.
 *
 *  Do all initialization required by the LPM module. LPM_init
 *  must be called before any of the LPM functions are used.
 */
void LPM_init(void);


/*
 *  ======== LPM_off ========
 */
/**
 *  @brief      Turn off power to the DSP Power Domain
 *
 *  This function will turn off the power to the DSP Power Domain which
 *  contains both the DSP and the VICP modules. This will eliminate all
 *  power leakage for these two modules.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         The power was successfully turned off.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_off(LPM_Handle handle);


/*
 *  ======== LPM_on ========
 */
/**
 *  @brief      Turn on power to the DSP Power Domain
 *
 *  This function will turn on the power to the DSP Power Domain which
 *  contains both the DSP and the VICP modules. This is considered a cold
 *  start, meaning the DSP will be held in local reset with the clock running.
 *
 *  The VICP default state is disabled (clock off), but this can be overriden
 *  through the enableVicp configuration parameter. In all cases, the VICP is
 *  also held in local reset.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         The power was successfully turned on.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_on(LPM_Handle handle);


/*
 *  ======== LPM_open ========
 */
/**
 *  @brief      Establish a connection with the LPM object.
 *
 *  @param[in]  name            The database name of the device.
 *  @param[out] handle          An opaque handle to the LPM object.

 *  @retval     LPM_SOK         The LPM object was successfuly opened.
 *  @retval     LPM_EFAIL       An error has occurred.
 */
LPM_Status LPM_open(char *name, LPM_Handle *handle);


/*
 *  ======== LPM_regExtPowerOff ========
 */
/**
 *  @brief      Register an external power switch off function.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *  @param[in]  func            A function address which will turn off the
 *                              external power supply to the DSP Power
 *                              Domain.
 *  @param[in]  arg             An argument passed to the callback function.
 *
 *  @retval     LPM_SOK         The function was successfully registered.
 *  @retval     LPM_EFAIL       An error has occurred.
 */
LPM_Status LPM_regExtPowerOff(LPM_Handle handle, LPM_ExtOffFxn func, void *arg);


/*
 *  ======== LPM_regExtPowerOn ========
 */
/**
 *  @brief      Register an external power switch on function.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *  @param[in]  func            A function address which will turn on the
 *                              external power supply to the DSP Power
 *                              Domain.
 *  @param[in]  arg             An argument passed to the callback function.
 *
 *  @retval     LPM_SOK         The function was successfully registered.
 *  @retval     LPM_EFAIL       An error has occurred.
 */
LPM_Status LPM_regExtPowerOn(LPM_Handle handle, LPM_ExtOnFxn func, void *arg);


/*
 *  ======== LPM_resume ========
 */
/**
 *  @brief      Return from hibernate state and resume the previous power state.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         The resume command has been successfully sent
 *                              to the LPM object.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_resume(LPM_Handle handle);


/*
 *  ======== LPM_set ========
 */
/**
 *  @brief      Set control bits in the LPM driver
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *
 *  @retval     LPM_SOK         The LPM object was successfuly modified.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_set(LPM_Handle handle, unsigned int mask);


/*
 *  ======== LPM_setPowerState ========
 */
/**
 *  @brief      Transition the DSP into a new power state.
 *
 *  @param[in]  handle          The handle to an LPM object, previously
 *                              acquired with a call to LPM_open().
 *  @param[in]  state           The new power state.
 *
 *  @retval     LPM_SOK         The power state command has been sent
 *                              to the LPM object.
 *  @retval     LPM_EFAIL       An error has occurred.
 *
 *  @pre        @c handle is a valid (non-NULL) LPM handle which is in
 *              the open state
 */
LPM_Status LPM_setPowerState(LPM_Handle handle, LPM_PowerState state);


/*@}*/  /* ingroup */

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (LPM_H) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

