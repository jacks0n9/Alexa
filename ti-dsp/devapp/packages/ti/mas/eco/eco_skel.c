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
 *  ======== eco_skel.c ========
 *  This file contains the implemenation of the SKEL interface for
 *  ECO algorithms.
 *
 *  These functions are the "server-side" of the the stubs defined in
 *  eco_stubs.c
 */
#include <xdc/std.h>
#include <ti/sdo/ce/skel.h>
#include <ti/sdo/ce/osal/Memory.h>

#include "eco.h"
#include "_eco.h"

/*
 *  ======== call ========
 */
static VISA_Status call(VISA_Handle visaHandle, VISA_Msg visaMsg)
{
    Int i,j;
    _ECO_Msg *msg  = (_ECO_Msg *)visaMsg;
    ECO_Handle handle = (ECO_Handle)visaHandle;

    /* perform the requested ECO operation by parsing message. */
    switch (msg->visa.cmd) {

        case _ECO_CPROCESS: 
        {
            IECO_InArgs *pInArgs = &(msg->cmd.process.inArgs);
            IECO_OutArgs *pOutArgs = &(msg->cmd.process.outArgs);


            for (i = 0; i < IECO_MAX_STREAMS; i++) 
            {
                IECO_StreamDesc *pStream = pInArgs->inStream[i];

                if(!pStream)
                {
                    continue;
                }

                Memory_cacheInv(pStream, sizeof(IECO_StreamDesc));

                for(j = 0; j < IECO_MAX_IO_BUFFERS; j++)
                {
                    if (!pStream->buf[j])
			                continue;
                              
                    Memory_cacheInv(pStream->buf[j], pStream->bufSize[j]); 
                }
                  
            }
            for (i = 0; i < IECO_MAX_STREAMS; i++) 
            {
                IECO_StreamDesc *pStream = pOutArgs->outStream[i];

                if(!pStream)
                {
                    continue;
                }

                Memory_cacheInv(pStream, sizeof(IECO_StreamDesc));
            }


            /* make the process call */
            msg->visa.status = ECO_process(handle, pInArgs, pOutArgs);

            Memory_cacheWb(pOutArgs->status, sizeof(Int32)); 

            for (i = 0; i < IECO_MAX_STREAMS; i++) 
            {
                Int j;
                IECO_StreamDesc *pStream = pOutArgs->outStream[i];

                if (!pStream)
                {
                    continue;
                }


                Memory_cacheWbInv(pStream, sizeof(IECO_StreamDesc));

                for(j = 0; j < IECO_MAX_IO_BUFFERS; j++)
                {
                    if (!pStream->buf[j])
			                continue;
                              
                    Memory_cacheWbInv(pStream->buf[j], pStream->bufSize[j]); 
                }
                  
            }

        }
        break;

        case _ECO_CCONTROL: 
        {
            /* invalidate cache for all input buffers */
            Memory_cacheInv(msg->cmd.control.params.buf, msg->cmd.control.params.bufSize);

            msg->visa.status = ECO_control(handle, msg->cmd.control.cmd,
                &(msg->cmd.control.params), &(msg->cmd.control.status));

            /* Invalidate the cache for input buffers again - in case the algorithm used
               it for its own calculations */            
            Memory_cacheInv(msg->cmd.control.params.buf, msg->cmd.control.params.bufSize);

            Memory_cacheWb(msg->cmd.control.status.buf, msg->cmd.control.status.bufSize);

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
 *  ======== ECO_SKEL ========
 */
SKEL_Fxns ECO_SKEL = {
    call,
    (SKEL_CREATEFXN)ECO_create,
    (SKEL_DESTROYFXN)ECO_delete,
};
