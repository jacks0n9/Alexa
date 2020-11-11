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
 *  ======== tal_gpp.h ========
 *
 *  TAL module definitions (GPP side).
 *
 */


#if !defined (TAL_GPP_H)
#define TAL_GPP_H

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


// typedef Void (*FnTalCbck)(Void * arg, Uint16 event, Uint16 payload);
typedef void (*FnTalCbck)(
        void * arg,
        unsigned short event,
        unsigned short payload);

typedef enum {
    TAL_SOK = 0,
    TAL_EFAIL
} TAL_Status;

typedef enum {
    TAL_AVAILABLE = 0,
    TAL_UNAVAILABLE
} TAL_TransportState;

typedef struct TalObject_tag {
//  Uint32              dspId;
    unsigned long       dspId;
    FnTalCbck           cbckFxn;
//  Pvoid               cbckArg;
    void                *cbckArg;
} TalObject, *TAL_Handle;


/*
 *  ======== TAL_changeState ========
 *  Change the availability state of the data transport.
 */
/* TODO add handle argument */
TAL_Status TAL_changeState (TAL_TransportState newstate,
    TAL_TransportState * oldState);


/*
 *  ======== TAL_close ========
 *  Breaks connection with data transport.
 */
TAL_Status TAL_close (TAL_Handle handle);


/*
 *  ======== TAL_getDSPAddress ========
 *  Fetch the DSP address corresponding to a symbol.
 */
TAL_Status TAL_getDSPAddress (
        TAL_Handle handle,
//      Pstr symbol,
        char *symbol,
//      Uint32 * address);
        unsigned long * address);


/*
 *  ======== TAL_open ========
 *  Establish a connection with the data transport.
 */
TAL_Status TAL_open (
//      Pstr name,
        char *name,
        TAL_Handle *handle);


/*
 *  ======== TAL_registerEvents ========
 *  Register a function to be called when a power event is recieved over the
 *  data transport.
 */
TAL_Status TAL_registerEvents (
        TAL_Handle handle,
        FnTalCbck callback,
//      Pvoid arg);
        void *arg);


/*
 *  ======== TAL_sendEvent ========
 *  Send a power-related event over the data transport.
 */
TAL_Status TAL_sendEvent (
        TAL_Handle handle,
//      Uint16 event,
        unsigned short event,
//      Uint16 payload);
        unsigned short payload);


/*
 *  ======== TAL_unregisterEvents ========
 *  Unregiter for callbacks on power events.
 */
TAL_Status TAL_unregisterEvents (TAL_Handle handle);


/* IPS channel values. Must agree with tal_dsp.h */
#define IPS_ID_POWER 1 
#define IPS_EVENT_POWER 4


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (TAL_GPP_H) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

