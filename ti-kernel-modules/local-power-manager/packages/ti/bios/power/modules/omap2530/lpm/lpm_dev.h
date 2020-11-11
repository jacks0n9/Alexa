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
 *  ======== lpm_dev.h ========
 *
 */


#if !defined (LPM_DEV_H)
#define LPM_DEV_H


#if defined(OS_PROS_DAVINCI)

#include <stdio.h>

#elif defined(OS_LINUX)


#elif defined(WIN32)


#else

    #error "Operating System undefined";

#endif


#include "lpm_base.h"


#if defined(__cplusplus)
extern "C" {
#endif


typedef enum {
    LPM_EXTERNAL_VOLTAGE_OFF_FXN,       /* LPM_ExtPwrOffFxn */
    LPM_EXTERNAL_VOLTAGE_OFF_ARG,
    LPM_EXTERNAL_VOLTAGE_ON_FXN,        /* LPM_ExtPwrOnFxn */
    LPM_EXTERNAL_VOLTAGE_ON_ARG,
} LPM_Action;


/* External Power ON/OFF functions provided by the driver. */
typedef void (*LPM_ExtPwrOffFxn)(void *);
typedef void (*LPM_ExtPwrOnFxn)(void *);

/* Operating System service functions */
typedef LPM_Status (*LPM_RemapFxn)(unsigned long pa,
                                 unsigned long size, unsigned long *va);
typedef LPM_Status  (*LPM_SignalFxn)(void *);
typedef LPM_Status  (*LPM_UnmapFxn)(void *va);
typedef LPM_Status  (*LPM_WaitFxn)(void *);
typedef void        (*LPM_TraceFxn)(char *fmt, ...);

// Device function types.
typedef LPM_Status (*LPM_Command)(void *, LPM_Action, unsigned long value);
typedef LPM_Status (*LPM_ConnectFxn)(void *);
typedef LPM_Status (*LPM_DisonnectFxn)(void *);
typedef LPM_Status (*LPM_OffFxn)(void *);
typedef LPM_Status (*LPM_OnFxn)(void *, int);
typedef LPM_Status (*LPM_ResumeFxn)(void *);
typedef LPM_Status (*LPM_SetPowerStateFxn)(void *, LPM_PowerState);


// LPM Device Structure
typedef struct LPM_Device {

    /* device operational functions */
    LPM_Command                 command;
    LPM_ConnectFxn              connect;
    LPM_DisonnectFxn            disconnect;
    LPM_OffFxn                  off;
    LPM_OnFxn                   on;
    LPM_ResumeFxn               resume;
    LPM_SetPowerStateFxn        setPowerState;

    /* LPM_Device private instance pointer */
    void                       *instance;
} LPM_Device;


typedef struct LPM_DevAttrs {
    LPM_RemapFxn        os_remap;
    LPM_SignalFxn       os_signal;
    LPM_UnmapFxn        os_unmap;
    LPM_WaitFxn         os_wait;
    LPM_TraceFxn        os_trace;
    void               *os_instance;
} LPM_DevAttrs;

LPM_Status LPM_init (int index, LPM_Device *device, LPM_DevAttrs *attrs);
LPM_Status LPM_exit (void *instance);


#if defined(__cplusplus)
}
#endif


#endif /* !defined (LPM_DEV_H) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:32; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

