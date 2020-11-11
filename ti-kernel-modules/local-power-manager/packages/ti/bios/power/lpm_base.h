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
 *  @file       ti/bios/power/lpm_base.h
 *
 *  @brief      This header defines base level type for the Local
 *              Power Manager.
 *
 *  @addtogroup   ti_bios_power_LPM     Shared Definitions
 *
 *  Base level types for the LPM interface.
 */


#if !defined (LPM_BASE_H)
#define LPM_BASE_H


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/** @ingroup    ti_bios_power_LPM */
/*@{*/

/**
 *  @brief      LPM return status codes
 */
typedef enum {
    LPM_SOK = 0,        /**< success */
    LPM_EFAIL,          /**< general failure */
    LPM_EBUSY,          /**< device is busy */
} LPM_Status;


/**
 *  @brief      LPM power states, in ranking order
 */
typedef enum {
    LPM_HIBERNATE = 0x00402001,         /**< hibernation mode */
} LPM_PowerState;


/**
 *  @brief      LPM control operations
 */
typedef enum {
    LPM_CTRL_CONNECT = 0x10001000,      /**< connect to transport layer */
    LPM_CTRL_DISCONNECT,                /**< disconnect from transport layer */
    LPM_CTRL_OFF,                       /**< turn dsp power domain off */
    LPM_CTRL_ON,                        /**< turn dsp power domain on */
    LPM_CTRL_RESUME,                    /**< wake dsp from hibernation mode */
    LPM_CTRL_SETPOWERSTATE,             /**< place dsp into new power state */
    LPM_CTRL_SET,                       /**< set given mask in driver state */
    LPM_CTRL_CLEAR,                     /**< clear given mask in driver state */
} LPM_Control;


/* *** THIS IS PRIVATE, should be moved to a private header file. */
// Device context states.
typedef enum {
    LPM_COLDBOOT = 0,
    LPM_WARMBOOT
} LPM_Context;



#define LPM_CTRL_REFCOUNTOVR    0x0001  // reference counter override


/*@}*/  /* ingroup */

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (LPM_BASE_H) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

