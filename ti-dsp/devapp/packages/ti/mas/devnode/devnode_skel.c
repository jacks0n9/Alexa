/* 
 *  Copyright 2007
 *  Texas Instruments Incorporated
 *
 *  All rights reserved.  Property of Texas Instruments Incorporated
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 * 
 */
/*
 *  ======== devnode_skel.c ========
 *  This file contains the implemenation of the SKEL interface for
 *  DEVNODE algorithms.
 *
 *  These functions are the "server-side" of the the stubs defined in
 *  devnode_stubs.c
 */
#include <xdc/std.h>
#include <ti/sdo/ce/skel.h>
#include <ti/sdo/ce/osal/Memory.h>

#include "devnode.h"
#include "_devnode.h"

/*
 *  ======== call ========
 */
static VISA_Status call(VISA_Handle visaHandle, VISA_Msg visaMsg)
{
    Int i;
    _DEVNODE_Msg *msg  = (_DEVNODE_Msg *)visaMsg;
    DEVNODE_Handle handle = (DEVNODE_Handle)visaHandle;

    /* perform the requested DEVNODE operation by parsing message. */
    switch (msg->visa.cmd) {

        case _DEVNODE_CPROCESS: 
        {
            IDEVNODE_InArgs  *pInArgs  = &(msg->cmd.process.inArgs);
            IDEVNODE_OutArgs *pOutArgs = &(msg->cmd.process.outArgs);
            Uint32            t1       = 0;

#ifdef DM6446
            t1 = Timer_phys0->tim34; 
#endif
#ifdef LEO
            t1 = Timer_phys1->tim12;                    // timer1 count on DSP
#endif
            if(t1 > pInArgs->timeStamp)
                pOutArgs->timeToAlgo = t1 - pInArgs->timeStamp;
            else
                pOutArgs->timeToAlgo = 0xFFFFFFFF - pInArgs->timeStamp + t1;


            for (i = 0; i < pInArgs->numStreams; i++) 
            {
                Int j;
                IDEVNODE_StreamDesc *pStream = pInArgs->issueStream[i];

                if(!pStream)
                {
                    continue;
                }

                Memory_cacheInv(pStream, sizeof(IDEVNODE_StreamDesc));

                for(j = 0; j < pStream->maxBufId; j++)
                {
                    if (!pStream->buf[j])
			continue;
                              
                    Memory_cacheInv(pStream->buf[j], pStream->bufSize[j]); 
                }
                  
            }

#if 0
            /* unmarshall outArgs based on the "size" of inArgs */
            pOutArgs = (IDEVNODE_OutArgs *)((UInt)(&(msg->cmd.process.inArgs)) +
                msg->cmd.process.inArgs.size);
#endif

            /* make the process call */
            msg->visa.status = DEVNODE_process(handle, pInArgs, pOutArgs);

            if (pOutArgs->numStreams > IDEVNODE_MAX_STREAMS)
            {
                msg->visa.status = VISA_EFAIL;
                break;
            }

            for (i = 0; i < pOutArgs->numStreams; i++) 
            {
                Int j;
                IDEVNODE_StreamDesc *pStream = pOutArgs->reclaimedStream[i];

                if (!pStream)
                {
                    continue;
                }


                Memory_cacheWb(pStream, sizeof(IDEVNODE_StreamDesc));

                for(j = 0; j < pStream->maxBufId; j++)
                {
                    if (!pStream->buf[j])
			            continue;
                              
                    Memory_cacheWb(pStream->buf[j], pStream->bufSize[j]); 
                }
                  
            }
#ifdef DM6446
            t1 = Timer_phys0->tim34; 
#endif
#ifdef LEO
            t1 = Timer_phys1->tim12;                    // timer1 count on DSP
#endif    
            pOutArgs->timeStamp = t1;

        }
        break;

        case _DEVNODE_CCONTROL: 
        {
            /* invalidate cache for all input buffers */
            Memory_cacheInv(msg->cmd.control.inArgs.message, msg->cmd.control.inArgs.messageSize);

            msg->visa.status = DEVNODE_control(handle, msg->cmd.control.cmd,
                &(msg->cmd.control.inArgs), &(msg->cmd.control.outArgs));

            
            /* Invalidate the cache for input buffers again - in case the algorithm used
               it for its own calculations */            
            Memory_cacheInv(msg->cmd.control.inArgs.message, msg->cmd.control.inArgs.messageSize);

            Memory_cacheWb(msg->cmd.control.outArgs.message, msg->cmd.control.outArgs.messageSize);

        }
        break;

        default: 
        {
            msg->visa.status = VISA_EFAIL;
        }
        break;
    }

    return (msg->visa.status);
}

/*
 *  ======== DEVNODE_SKEL ========
 */
SKEL_Fxns DEVNODE_SKEL = {
    call,
    (SKEL_CREATEFXN)DEVNODE_create,
    (SKEL_DESTROYFXN)DEVNODE_delete,
};
