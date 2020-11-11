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


/*
 *  ======== lpm_api.c ========
 *
 */


// #include <xdc/std.h> // TODO: use xdc/std.h instead of stdio.h
#include <stdio.h>

#if defined(OS_LINUX)

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#elif defined(OS_WINCE)

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <windows.h>
#include <winbase.h>

#define far  /* not used on WinCE */

#if defined(DEBUG)
#define __P(fmt, ...) RETAILMSG(1, (L##fmt, __VA_ARGS__))
#else
#define __P(fmt, ...)
#endif

#endif  /* OS_<type> */

/* common includes for all operating systems */
#include <ti/bios/power/lpm.h>
#include "lpm_prvt.h"
// #include "lpm_dev.h"
#include "../tal/pwrm_sh.h"


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

// ti.bios.power configuration parameters
extern far int PWRM_doImcopConfig;
extern far int PWRM_enableImcopAtPowerOn;

// Private callback function to register with TAL layer.
#if defined(OS_PROS_DAVINCI)
static void _LPM_talCB(void *arg, unsigned short event, unsigned short payload);

// Private function to turn on power with proper context restore.
static LPM_Status _LPM_on(LPM_Handle hndl, LPM_Context cntxt);
#endif


/*
 *  ======== LPM_clear ========
 */
LPM_Status LPM_clear(LPM_Handle hndl, unsigned int mask)
{
    LPM_Status  status = LPM_SOK;


#if defined(OS_PROS_DAVINCI)

    // TODO

#elif defined(OS_LINUX)

    int         istat;

    istat = ioctl(hndl->fd, LPM_CTRL_CLEAR, mask);

    if (istat < 0) {
        return LPM_EFAIL;
    }

#elif defined(OS_WINCE)
    BOOL ceStatus;
    DWORD dwBytesReturned;

    /* clear requested mask in driver state */
    ceStatus =
        DeviceIoControl(hndl->hLPM, LPM_CTRL_CLEAR,
        (LPVOID)&mask, 0, NULL, 0, &dwBytesReturned, NULL);

    if (FALSE == ceStatus) {
        return(LPM_EFAIL);
    }


#else

    #error "Operating System undefined";

#endif

    return status;
}


/*
 *  ======== LPM_close ========
 */
LPM_Status LPM_close(LPM_Handle lpmHandle)
{
    LPM_Status lpmStatus = LPM_SOK;

#if defined(OS_PROS_DAVINCI)

    /* destroy the OSAL event object */
    {
        ER stat = E_OK;

        stat = del_flg(0x32);

        if (stat != E_OK) {
            /* TODO: report error */
        }
    }
    return(lpmStatus);

#elif defined(OS_LINUX)

    close(lpmHandle->fd);
    free(lpmHandle);
    return(lpmStatus);

#elif defined(OS_WINCE)

    BOOL result = FALSE;

    __P("--> LPM_close\n");

    /* close handle to device driver */
    result = CloseHandle(lpmHandle->hLPM);

    if (!result) {
        lpmStatus = LPM_EFAIL;
        __P("Error: LPM_close failed, error %d\n", GetLastError());
        /* log error but continue with finalize */
    }

    /* free lpm object */
    free(lpmHandle);

    __P("<-- LPM_close\n");
    return(lpmStatus);

#else

    #error "Operating System undefined";

#endif
}


/*
 *  ======== LPM_connect ========
 */
LPM_Status LPM_connect(LPM_Handle hndl)
{
    LPM_Status  lpmStat = LPM_SOK;


#if defined(OS_PROS_DAVINCI)
    TAL_Status  talStat = TAL_SOK;
    int         talReg = 0;


    // Use TAL to connect to transport layer.
    talStat = TAL_open("foobar", &hndl->hTAL);

    if (talStat != TAL_SOK) {
        hndl->hTAL = NULL;
        // TODO: report error
        lpmStat = LPM_EFAIL;
        goto leave;
    }

    // Register event callback with TAL layer.
    talStat = TAL_registerEvents(hndl->hTAL, _LPM_talCB, (void*)hndl);

    if (talStat != TAL_SOK) {
        // TODO: report error
        lpmStat = LPM_EFAIL;
        goto leave;
    }
    talReg = 1;

    // Get warm boot address. Do this here to minimize run-time overhead.
    talStat = TAL_getDSPAddress(hndl->hTAL,
                                hndl->resumeSymbol, &hndl->bootAddr);

    if (talStat != TAL_SOK) {
        // TODO: report error
        lpmStat = LPM_EFAIL;
        goto leave;
    }


leave:
    /* error handlers */
    if (talStat != TAL_SOK) {
        hndl->bootAddr = 0;

        if (talReg) {
            TAL_unregisterEvents(hndl->hTAL);
            talReg = 0;
        }

        if (hndl->hTAL != NULL) {
            TAL_close(hndl->hTAL);
            hndl->hTAL = NULL;
        }
    }

#elif defined(OS_LINUX)

    size_t      foo;
    int         istat;

    istat = ioctl(hndl->fd, LPM_CTRL_CONNECT, &foo);

    if (istat < 0) {
        return LPM_EFAIL;
    }

#elif defined(OS_WINCE)

    // TODO

#else

    #error "Operating System undefined";

#endif

    return lpmStat;
}


/*
 *  ======== LPM_disconnect ========
 */
LPM_Status LPM_disconnect(LPM_Handle hndl)
{
    LPM_Status  lpmStat = LPM_SOK;


#if defined(OS_PROS_DAVINCI)
    TAL_Status  talStat = TAL_SOK;


    // Unregister event callback with TAL layer.
    talStat = TAL_unregisterEvents(hndl->hTAL);

    if (talStat != TAL_SOK) {
        /* TODO: report error */
        lpmStat = LPM_EFAIL;
        goto leave;
    }

    // Disconnect from transport layer.
    talStat = TAL_close(hndl->hTAL);

    if (talStat != TAL_SOK) {
        /* TODO: report error */
        lpmStat = LPM_EFAIL;
        goto leave;
    }
    hndl->hTAL = NULL;

    // Clear the boot address.
    hndl->bootAddr = 0;


leave:

#elif defined(OS_LINUX)

    size_t      foo;
    int         istat;

    istat = ioctl(hndl->fd, LPM_CTRL_DISCONNECT, &foo);

    if (istat < 0) {
        return LPM_EFAIL;
    }


#elif defined(OS_WINCE)

    // TODO

#else

    #error "Operating System undefined";

#endif

    return lpmStat;
}


/*
 *  ======== LPM_exit ========
 */
void LPM_exit (void)
{

}


/*
 *  ======== LPM_init ========
 */
void LPM_init (void)
{

}


/*
 *  ======== LPM_off ========
 */
LPM_Status LPM_off(LPM_Handle hndl)
{
    LPM_Status  status = LPM_SOK;


#if defined(OS_PROS_DAVINCI)

    // Turn off power to GEM.
    if (hndl->lpm_off != NULL) {
        status = (hndl->lpm_off)(hndl);
    }

    if (status != LPM_SOK) {
        /* report error */
    }

#elif defined(OS_LINUX)

    size_t      foo;
    int         istat;

    istat = ioctl(hndl->fd, LPM_CTRL_OFF, &foo);

    if (istat < 0) {
        return LPM_EFAIL;
    }

#elif defined(OS_WINCE)
    BOOL ceStatus;
    DWORD dwBytesReturned;

    /* send the power-off request to the driver */
    ceStatus =
        DeviceIoControl(hndl->hLPM, LPM_CTRL_OFF,
        NULL, 0, NULL, 0, &dwBytesReturned, NULL);

    if (FALSE == ceStatus) {
        return(LPM_EFAIL);
    }

#else

    #error "Operating System undefined";

#endif

    return status;
}


/*
 *  ======== LPM_on ========
 */
LPM_Status LPM_on(LPM_Handle hndl)
{
    LPM_Status  status = LPM_SOK;


#if defined(OS_PROS_DAVINCI)

    // Turn on power to GEM; cold boot.
    status = _LPM_on(hndl, LPM_COLDBOOT);

    if (status != LPM_SOK) {
        /* report error */
    }

#elif defined(OS_LINUX)

    int         istat;
    int         args = 0;

    if (PWRM_doImcopConfig) {
        if (PWRM_enableImcopAtPowerOn) {
            args = args | 0x3;  // TODO - add macro in private header
        }
        else {
            args = args | 0x2;  // TODO - add macro in private header
        }
    }

    istat = ioctl(hndl->fd, LPM_CTRL_ON, args);

    if (istat < 0) {
        return LPM_EFAIL;
    }

#elif defined(OS_WINCE)
    BOOL ceStatus;
    DWORD dwBytesReturned;

    /* send the power-on request to the driver */
    ceStatus =
        DeviceIoControl(hndl->hLPM, LPM_CTRL_ON,
        NULL, 0, NULL, 0, &dwBytesReturned, NULL);

    if (FALSE == ceStatus) {
        return(LPM_EFAIL);
    }

#else

    #error "Operating System undefined";

#endif

    return status;
}


/*
 *  ======== LPM_open ========
 */
LPM_Status LPM_open(char *deviceName, LPM_Handle *lpmHandle)
{
    LPM_Status lpmStatus = LPM_SOK;
    LPM_Object  *obj;

#if defined(OS_PROS_DAVINCI)

    // Hardwire driver object until configuration database available.
    extern LPM_DeviceHandle LPM_Dev_DaVinci;

    // Public API functions
    LPM_OBJ.lpm_off                 = LPM_Dev_DaVinci->lpm_off;
    LPM_OBJ.lpm_on                  = LPM_Dev_DaVinci->lpm_on;

    // Private functions
    LPM_OBJ.lpm_setBootAddr         = LPM_Dev_DaVinci->lpm_setBootAddr;
    LPM_OBJ.lpm_waitGemPowerDown    = LPM_Dev_DaVinci->lpm_waitGemPowerDown;

    // Callbacks registered by client
    LPM_OBJ.lpm_extPwrOff           = NULL;
    LPM_OBJ.lpm_extPwrOffArg        = NULL;
    LPM_OBJ.lpm_extPwrOn            = NULL;
    LPM_OBJ.lpm_extPwrOnArg         = NULL;

    // Private object handles
    LPM_OBJ.resumeSymbol            = LPM_Dev_DaVinci->resumeSymbol;
    LPM_OBJ.bootAddr                = 0;
    LPM_OBJ.hTAL                    = NULL;

    // Create an OSAL event object for synchronizing with TAL callback.
    {
        ER  stat = E_OK;

        LPM_OBJ.pk_cflg.flgatr = TA_TFIFO | TA_WMUL;
        LPM_OBJ.pk_cflg.iflgptn = 0;

        stat = cre_flg(0x32, &(LPM_OBJ.pk_cflg));

        if (stat != E_OK) {
            lpmStatus = LPM_EFAIL;
            goto leave;
        }
    }

    // Return opaque handle to LPM object.
    *lpmHandle = &LPM_OBJ;


leave:
    /* error handlers */
    if (lpmStatus != LPM_SOK) {
        *lpmHandle = NULL;
    }

fail:
    return(lpmStatus);

#elif defined(OS_LINUX)

    int flags;

    // Allocate a new LPM_Object for this client.
    obj = (LPM_Object *)malloc(sizeof(LPM_Object));

    if (obj == NULL) {
        lpmStatus = LPM_EFAIL;
        goto fail;
    }

    // Open a handle to the device driver.
    obj->fd = open(deviceName, O_RDWR);

    if (obj->fd < 0) {
        lpmStatus = LPM_EFAIL;
        goto fail;
    }

    // Set close-on-exec flag on file handle.
    flags = fcntl(obj->fd, F_GETFD);

    if (flags == -1) {
        lpmStatus = LPM_EFAIL;
        goto fail;
    }

    flags |= FD_CLOEXEC;
    flags = fcntl(obj->fd, F_SETFD, flags);

    if (flags == -1) {
        lpmStatus = LPM_EFAIL;
        goto fail;
    }

    // Return opaque handle to LPM object.
    *lpmHandle = (LPM_Handle)obj;

    return lpmStatus;

fail:
    if (obj->fd >= 0) {
        close(obj->fd);
    }

    if (obj != NULL) {
        free(obj);
    }
    return lpmStatus;

#elif defined(OS_WINCE)

    __P("--> LPM_open\n");

    /* allocate a new object for this client */
    obj = (LPM_Object *)malloc(sizeof(LPM_Object));

    if (NULL == obj) {
        lpmStatus = LPM_EFAIL;
        goto fail;
    }

    /* open a handle to the device driver */
    obj->hLPM =
        CreateFile(L"LPM1:" /* deviceName */,
        GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == obj->hLPM) {
        __P("Error: LPM_open failed on LPM1:, error %d\n", GetLastError());
        lpmStatus = LPM_EFAIL;
        goto fail;
    }

    /* return opaque handle to LPM object */
    *lpmHandle = (LPM_Handle)obj;

    __P("<-- LPM_open\n");
    return(lpmStatus);

fail:
    /* free up allocated resources */
    if (NULL != obj) {
        free(obj);
    }

    __P("<-- LPM_open\n");
    return(lpmStatus);

#else

    #error "Operating System undefined";

#endif
}


/*
 *  ======== LPM_regExtPowerOff ========
 */
LPM_Status LPM_regExtPowerOff(LPM_Handle hndl, LPM_ExtOffFxn func, void *arg)
{

#if defined(OS_PROS_DAVINCI)

    // Store in LPM_Object.
    LPM_OBJ.lpm_extPwrOff = func;
    LPM_OBJ.lpm_extPwrOffArg = arg;

#elif defined(OS_LINUX)

    // TODO

#elif defined(OS_WINCE)

    // TODO

#else

    #error "Operating System undefined";

#endif

    return LPM_SOK;
}


/*
 *  ======== LPM_regExtPowerOn ========
 */
LPM_Status LPM_regExtPowerOn(LPM_Handle hndl, LPM_ExtOnFxn func, void *arg)
{

#if defined(OS_PROS_DAVINCI)

    // Store in LPM_Object.
    LPM_OBJ.lpm_extPwrOn = func;
    LPM_OBJ.lpm_extPwrOnArg = arg;

#elif defined(OS_LINUX)

    // TODO

#elif defined(OS_WINCE)

    // TODO

#else

    #error "Operating System undefined";

#endif

    return LPM_SOK;
}


/*
 *  ======== LPM_resume ========
 */
LPM_Status LPM_resume(LPM_Handle hndl)
{
    LPM_Status          lpmStat = LPM_SOK;


#if defined(OS_PROS_DAVINCI)
    TAL_Status          talStat = TAL_SOK;
    TAL_TransportState  transState;


    // Set warm boot address in SYSTEM MODULE.
    lpmStat = hndl->lpm_setBootAddr(hndl, hndl->bootAddr);

    if (lpmStat != LPM_SOK) {
        /* TODO: report error */
        lpmStat = LPM_EFAIL;
        goto leave;
    }

    // Turn on power to GEM; warm boot.
    lpmStat = _LPM_on(hndl, LPM_WARMBOOT);

    if (lpmStat != LPM_SOK) {
        /* TODO: report error */
        goto leave;
    }

    // Block here until TAL callback signals this object.
    // TODO: move this to OSAL layer.
    {
        ER      stat = E_OK;
        FLGPTN  flgptn;

        stat = wai_flg(0x32, 0x02, TWF_ANDW, &flgptn);

        if (stat != E_OK) {
            lpmStat = LPM_EFAIL;
            goto leave;
        }
    }

    // Check status and return to caller.
    // TODO: move this to OSAL layer.
    {
        ER      stat = E_OK;
        FLGPTN  clrptn = ~0x02;

        stat = clr_flg(0x32, clrptn);

        if (stat != E_OK) {
            lpmStat = LPM_EFAIL;
            goto leave;
        }
    }

    // Set the transport state to available.
    talStat = TAL_changeState(TAL_AVAILABLE, &transState);

    if (talStat != TAL_SOK) {
        /* report error */
        lpmStat = LPM_EFAIL;
        goto leave;
    }

leave:
    if (lpmStat != LPM_SOK) {
        // TODO: add error handlers.
    }

#elif defined(OS_LINUX)

    size_t      foo;
    int         istat;

    istat = ioctl(hndl->fd, LPM_CTRL_RESUME, &foo);

    if (istat < 0) {
        return LPM_EFAIL;
    }

#elif defined(OS_WINCE)

    // TODO

#else

    #error "Operating System undefined";

#endif

    return lpmStat;
}


/*
 *  ======== LPM_set ========
 */
LPM_Status LPM_set(LPM_Handle hndl, unsigned int mask)
{
    LPM_Status  status = LPM_SOK;


#if defined(OS_PROS_DAVINCI)

    // TODO

#elif defined(OS_LINUX)

    int         istat;

    istat = ioctl(hndl->fd, LPM_CTRL_SET, mask);

    if (istat < 0) {
        return LPM_EFAIL;
    }

#elif defined(OS_WINCE)
    BOOL ceStatus;
    DWORD dwBytesReturned;

    /* set requested mask in driver state */
    ceStatus =
        DeviceIoControl(hndl->hLPM, LPM_CTRL_SET,
        (LPVOID)&mask, 0, NULL, 0, &dwBytesReturned, NULL);

    if (FALSE == ceStatus) {
        return(LPM_EFAIL);
    }

#else

    #error "Operating System undefined";

#endif

    return status;
}


/*
 *  ======== LPM_setPowerState ========
 */
LPM_Status LPM_setPowerState(LPM_Handle hndl, LPM_PowerState state)
{
    LPM_Status          lpmStat = LPM_SOK;


#if defined(OS_PROS_DAVINCI)
    TAL_Status          talStat = TAL_SOK;
    TAL_TransportState  transState;
    unsigned int        undoMask = 0;


    // Initiate the requested power state change.
    switch (state) {

        case LPM_HIBERNATE:
            // Set the transport state to unavailable.
            talStat = TAL_changeState(TAL_UNAVAILABLE, &transState);

            if (talStat != TAL_SOK) {
                /* report error */
                lpmStat = LPM_EFAIL;
                goto leave;
            }
            undoMask |= 0x01;

            // Send hibernate command to DSP.
            talStat = TAL_sendEvent(hndl->hTAL, PWRM_SH_SLEEP,
                                    PWRM_SH_HIBERNATE);

            if (talStat != TAL_SOK) {
                lpmStat = LPM_EFAIL;
                goto leave;
            }
            undoMask |= 0x02;
            break;

        case LPM_STANDBY:
            break;
    }

    /* block here until callback signals this object */
    // TODO: add this to OSAL layer.
    {
        ER      stat = E_OK;
        FLGPTN  flgptn;

        stat = wai_flg(0x32, 0x01, TWF_ANDW, &flgptn);

        if (stat != E_OK) {
            // What to do here? Attempt to resume the DSP? Delay for 10 msec
            // and return error code? Something else? For now, take no action
            // (clear undoMask) and simply return error code.
            undoMask = 0;
            lpmStat = LPM_EFAIL;
            goto leave;
        }
    }

    /* check status */
    // TODO: add this to OSAL layer.
    {
        ER      stat = E_OK;
        FLGPTN  clrptn = ~0x01;

        stat = clr_flg(0x32, clrptn);

        if (stat != E_OK) {
            // What to do here? Attempt to resume the DSP? Delay for 10 msec
            // and return error code? Something else? For now, take no action
            // (clear undoMask) and simply return error code.
            undoMask = 0;
            lpmStat = LPM_EFAIL;
            goto leave;
        }
    }

    // Wait for the GEM to complete its power down transitions.
    lpmStat = (hndl->lpm_waitGemPowerDown)(hndl);

    /* Turn off power to DSP. */
    lpmStat = LPM_off(hndl);


leave:
    /* error handlers */
    if (lpmStat != LPM_SOK) {

        // DSP has been hibernated. What should we do here?
        if (undoMask & 0x02) {
            ;
        }

        // Restore transport to previous state.
        if (undoMask & 0x01) {
            TAL_TransportState  oldState;
            TAL_changeState(transState, &oldState);
        }
    }

#elif defined(OS_LINUX)

    int         istat;

    istat = ioctl(hndl->fd, LPM_CTRL_SETPOWERSTATE, state);

    if (istat < 0) {
        return LPM_EFAIL;
    }

#elif defined(OS_WINCE)

    // TODO

#else

    #error "Operating System undefined";

#endif

    return lpmStat;
}


#if defined(OS_PROS_DAVINCI)
/*
 *  ======== _LPM_on ========
 */
LPM_Status _LPM_on(LPM_Handle hndl, LPM_Context cntxt)
{
    LPM_Status  status = LPM_SOK;


    // Turn on power to GEM.
    if (hndl->lpm_on != NULL) {
        status = (hndl->lpm_on)(hndl, cntxt);
    }

    if (status != LPM_SOK) {
        /* report error */
    }

    return status;
}
#endif


#if defined(OS_PROS_DAVINCI)
/*
 *  ======== _LPM_talCB ========
 */
void _LPM_talCB(void *arg, unsigned short event, unsigned short payload)
{

    ER      stat = E_OK;
    FLGPTN  setptn;

    if ((event == PWRM_SH_ACKNOWLEDGE) && (payload == PWRM_SH_HIBERNATE)) {
        setptn = 0x01;
    }
    else if ((event == PWRM_SH_AWAKE) && (payload == PWRM_SH_HIBERNATE)) {
        setptn = 0x02;
    }
    else {
        /* error */
        goto leave;
    }

    /* signal object to release application thread */
    stat = iset_flg(0x32, setptn);

    if (stat != E_OK) {
        /* TODO: report error */
        goto leave;
    }

leave:
    // TODO: add error handlers.


    return;
}
#endif


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:32; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

