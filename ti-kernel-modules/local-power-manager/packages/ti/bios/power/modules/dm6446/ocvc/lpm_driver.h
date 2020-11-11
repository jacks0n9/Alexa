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
 *  ======== lpm_driver.h ========
 *
 */


/**
 *  @file       lpm_driver.h
 *
 *  @brief      This header defines the functions for turning the
 *              external power supply on and off.
 *
 *  @addtogroup   ti_bios_power_LPM     Shared Definitions
 */


/**
 *  @brief      Function typedef of the functions which control the
 *              external power supply.
 */
typedef void (*LPM_ExtPwrFxn)(void *arg);


/**
 *  @brief      Register an external power off function.
 *
 *  @param[in]  device          Linux device name.
 *  @param[in]  fxn             A function address which will turn off the
 *                              external power supply to the DSP Power
 *                              Domain.
 *  @param[in]  arg             An argument passed to the callback function.
 *
 *  @retval     0               The function was successfully registered.
 *  @retval     -1              The given device name was not found.
 */
extern int lpm_regExtOffFxn(char *device, LPM_ExtPwrFxn fxn, void *arg);


/**
 *  @brief      Register an external power on function.
 *
 *  @param[in]  device          Linux device name.
 *  @param[in]  fxn             A function address which will turn off the
 *                              external power supply to the DSP Power
 *                              Domain.
 *  @param[in]  arg             An argument passed to the callback function.
 *
 *  @retval     0               The function was successfully registered.
 *  @retval     -1              The given device name was not found.
 */
extern int lpm_regExtOnFxn(char *device, LPM_ExtPwrFxn fxn, void *arg);
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

