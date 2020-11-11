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
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:32; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

