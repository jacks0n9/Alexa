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
 *  ======== tal_gpp_dsplink_gpp.c ========
 *
 */

#include <gpptypes.h>
#include <dsplink.h>
#include <errbase.h>
#include <mpcs.h>
#include <list.h>
#include <ips.h>
#include <ldrv_proc.h>
#include <prcs.h>
#include <pmgr_proc.h>

// Private header files.
#include "tal_gpp.h"

#define LINK_PROCID_DSP 0

#if defined (__cplusplus)
EXTERN "C" {
#endif /* defined (__cplusplus) */

STATIC TalObject TAL_Obj ;

Void _TAL_translateCallback(Uint32, Pvoid, Pvoid);



/*
 *  ======== TAL_changeState ========
 */
EXPORT_API
TAL_Status TAL_changeState(TAL_TransportState newState,
    TAL_TransportState * oldState)
{
    DSP_STATUS status;
    PROC_State linkState;
    TAL_Status talStatus = TAL_EFAIL;
    PROC_State newTransportState = ProcState_Unknown;

    status = LDRV_PROC_getState((ProcessorId) 0, &linkState);

    if (DSP_SUCCEEDED (status)) {

        if (linkState == ProcState_Started) {
            *oldState = TAL_AVAILABLE;
        }
        else {
            *oldState = TAL_UNAVAILABLE;
        }

        if ((newState == TAL_AVAILABLE) && (*oldState == TAL_UNAVAILABLE)) {
            newTransportState = ProcState_Started;
        }
        else if ((newState == TAL_UNAVAILABLE) &&
            (*oldState == TAL_AVAILABLE)) {
            newTransportState = ProcState_Unavailable;
        }

        if (newTransportState != ProcState_Unknown) {
            status = LDRV_PROC_setState((ProcessorId) 0, newTransportState);
            if (DSP_SUCCEEDED (status)) {
                talStatus = TAL_SOK;
            }
        }
    }

    return talStatus;
}


/*
 *  ======== TAL_close ========
 */
EXPORT_API
TAL_Status TAL_close(TAL_Handle handle)
{
    return TAL_SOK;
}


/*
 *  ======== TAL_getDSPAddress ========
 */
EXPORT_API
TAL_Status TAL_getDSPAddress(TAL_Handle handle, Pstr symbol, Uint32 *address)
{
    DSP_STATUS  status;
    TAL_Status  talStatus = TAL_EFAIL;

    status = PMGR_PROC_getSymbolAddress(0, symbol, address);

    if (DSP_SUCCEEDED (status)) {
        talStatus = TAL_SOK;
    }

    return talStatus;

}


/*
 *  ======== TAL_open ========
 */
EXPORT_API
TAL_Status TAL_open(Pstr name, TAL_Handle *handle)
{
    /* initialize TAL object */
    /* for time being, hardwire since no system database yet */
    TAL_Obj.dspId = LINK_PROCID_DSP;

    /* set handle to TAL object */
    *handle = &TAL_Obj;

    return TAL_SOK;
}


/*
 *  ======== TAL_registerEvents ========
 */
EXPORT_API
TAL_Status TAL_registerEvents(TAL_Handle handle, FnTalCbck callback, Pvoid arg)
{
    TAL_Status  talStatus = TAL_SOK;
    DSP_STATUS  linkStatus = DSP_SOK;

    /* store callback function and argument in TAL object */
    handle->cbckFxn = callback;
    handle->cbckArg = arg;

    /* register with IPS module */
    linkStatus = IPS_register(handle->dspId, IPS_ID_POWER, IPS_EVENT_POWER,
            _TAL_translateCallback, (Pvoid)handle);

    if (DSP_FAILED(linkStatus)) {
        talStatus = TAL_EFAIL;
    }

    return talStatus;
}


/*
 *  ======== TAL_sendEvent ========
 */
EXPORT_API
TAL_Status TAL_sendEvent(TAL_Handle handle, Uint16 event, Uint16 payload)
{
    Uint32      shmipsEvent;
    TAL_Status  talStatus = TAL_SOK;
    DSP_STATUS  linkStatus = DSP_SOK;

    /* pack event and payload */
    shmipsEvent =((Uint32)event<<16) + (Uint32)payload;

    linkStatus = IPS_notify(handle->dspId, IPS_ID_POWER, IPS_EVENT_POWER,
            shmipsEvent, FALSE);

    if (DSP_FAILED(linkStatus)) {
        talStatus = TAL_EFAIL;
    }

    return talStatus;
}


/*
 *  ======== TAL_unregisterEvents ========
 */
EXPORT_API
TAL_Status TAL_unregisterEvents(TAL_Handle handle)
{
    TAL_Status  talStatus = TAL_SOK;
    DSP_STATUS  linkStatus = DSP_SOK;

    linkStatus = IPS_unregister(0, /* TODO - dspId, get from LPM_open? */
            IPS_ID_POWER, IPS_EVENT_POWER, _TAL_translateCallback,
            (Pvoid)handle);


    /* clear callback function and argument in TAL object */
    handle->cbckFxn = NULL;
    handle->cbckArg = NULL;

    if (DSP_FAILED(linkStatus)) {
        talStatus = TAL_EFAIL;
    }

    return talStatus;
}


/*
 *  ======== _TAL_translateCallback ========
 */
Void _TAL_translateCallback(Uint32 eventNum, Pvoid arg, Pvoid info)
{
    Uint16      event;       // pwrm event
    Uint16      payload;     // pwrm payload for given event
    TAL_Handle  handle = (TAL_Handle)arg;

    /* unpack pwerm event and payload returned in IPS info payload */
    event = (Uint16) (((Uint32)info >> 16) & 0xffff);
    payload = (Uint16) ((Uint32)info & 0xffff);

    /* call regisered callback */
    handle->cbckFxn(handle->cbckArg, event, payload);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

