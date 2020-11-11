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
 *  ======== eco_stubs.c ========
 *  These functions are the "app-side" of a "remote" implementation.
 *
 */
#include <xdc/std.h>
#include <ti/sdo/ce/visa.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <string.h>

#include "ieco.h"
#include "eco.h"
#include "_eco.h"

static XDAS_Int32 process(IECO_Handle h, IECO_InArgs *inArgs, IECO_OutArgs *outArgs);
static XDAS_Int32 control(IECO_Handle h, IECO_Cmd cmd, IECO_DynamicParams *params, IECO_Status *status);
/*
 * Stubs must implement the interface they "stub" for.  This eco stub
 * proxy's for IECO_Fxns, which is IALG + process() and control().
 *
 * Note that the IALG fxns for the stub currently aren't called, so
 * they're all set to NULL.
 */
IECO_Fxns ECO_STUBS = {
    {&ECO_STUBS, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    process, control
};

/*
 *  ======== process ========
 *  This is the stub-implementation for the process method
 */
static XDAS_Int32 process(IECO_Handle h, IECO_InArgs *inArgs, IECO_OutArgs *outArgs)
{
    XDAS_Int32 retVal;
    VISA_Handle visa = (VISA_Handle)h;
    _ECO_Msg *msg;
    Int i;
    IECO_OutArgs *pMsgOutArgs;
 

    /*
     * Validate arguments.  Do we want to do this _every_ time, or just in
     * debug builds via GT_assert()?
     */
    if ((inArgs == NULL) || (inArgs->size < sizeof(IECO_InArgs)) ||
        (outArgs == NULL) || (outArgs->size < sizeof(IECO_OutArgs))) {

        /* invalid args, could even assert here, it's a spec violation. */
        return (IECO_EFAIL);
    }

    if (/* size of "stuff to marshall" > message size */
        (sizeof(VISA_MsgHeader) + inArgs->size + outArgs->size) > sizeof(_ECO_Msg)) {

        return (IECO_EFAIL);
    }

    /* get a message appropriate for this algorithm */
    if ((msg = (_ECO_Msg *)VISA_allocMsg(visa)) == NULL) 
    {
        return (IECO_EFAIL);
    }

    /* zero out msg->cmd (not msg->visa!) */
    memset(&(msg->cmd), 0, sizeof(msg->cmd));

    /*
     * Marshall the command: copy the client-passed arguments into flattened
     * message data structures, converting every pointer address to alg.
     * data buffer into physical address.
     */

    /* First specify the processing command that the skeleton should do */
    msg->visa.cmd = _ECO_CPROCESS;

    msg->cmd.process.inArgs.message = inArgs->message;
    /*
     * inArgs->inStream is a pointer to a sparse array of pointers, so we take
     * individual pointers, convert them if non-NULL, and store them in the
     * message counterpart.
     */
    for (i = 0; i < IECO_MAX_STREAMS; i++) 
    {
        int j;
        IECO_StreamDesc **pStreamDst, **pStreamSrc;

        pStreamSrc = &(inArgs->inStream[i]);
        pStreamDst = &(msg->cmd.process.inArgs.inStream[i]); 

        if(!*pStreamSrc)
        {
            /* NULL stream descriptor */
            *pStreamDst = NULL;
            continue;
        }
        
        *pStreamDst = (IECO_StreamDesc *) Memory_getBufferPhysicalAddress( 
                                                           *pStreamSrc,
                                                           sizeof(IECO_StreamDesc),
                                                           NULL); 

        for (j = 0; j < IECO_MAX_IO_BUFFERS; j++)
        {
            if (!(*pStreamSrc)->buf[j])
            {
                /* NULL buffer - unused channel */
                continue;
            }

            (*pStreamSrc)->buf[j] = (UInt8*) Memory_getBufferPhysicalAddress(  
                                          		            (*pStreamSrc)->buf[j],
                                                          (*pStreamSrc)->bufSize[j],
                                                          NULL);
        }

        Memory_cacheWbInv(*pStreamSrc, sizeof(IECO_StreamDesc));
    }

    msg->cmd.process.inArgs.size = inArgs->size;

    /* point at outArgs and set the "size" */
    pMsgOutArgs = &(msg->cmd.process.outArgs);

    /* set the size field - the rest is filled in by the algo */
    pMsgOutArgs->size = outArgs->size;
    
    if(outArgs->status)
    {
       pMsgOutArgs->status = (Int32 *) Memory_getBufferPhysicalAddress(
                                                           outArgs->status,
                                                           sizeof(Int32),
                                                           NULL);
    }

    /*
     * outArgs->outStream is a pointer to a sparse array of pointers, so we take
     * individual pointers, convert them if non-NULL, and store them in the
     * message counterpart.
     */
    for (i = 0; i < IECO_MAX_STREAMS; i++) 
    {
        int j;
        IECO_StreamDesc **pStreamDst, **pStreamSrc;

        pStreamSrc = &(outArgs->outStream[i]);
        pStreamDst = &(pMsgOutArgs->outStream[i]); 

        if(!*pStreamSrc)
        {
            /* NULL stream descriptor */
            *pStreamDst = NULL;
            continue;
        }
        
        *pStreamDst = (IECO_StreamDesc *) Memory_getBufferPhysicalAddress( 
                                                           *pStreamSrc,
                                                           sizeof(IECO_StreamDesc),
                                                           NULL);

        for (j = 0; j < IECO_MAX_IO_BUFFERS; j++)
        {
            if (!(*pStreamSrc)->buf[j])
            {
                /* NULL buffer - unused channel */
                continue;
            }

            (*pStreamSrc)->buf[j] = (UInt8*) Memory_getBufferPhysicalAddress(  
                                          		            (*pStreamSrc)->buf[j],
                                                          (*pStreamSrc)->bufSize[j],
                                                          NULL);
        }
        
        Memory_cacheWbInv(*pStreamSrc, sizeof(IECO_StreamDesc));
    }


    /* send the message to the skeleton and wait for completion */
    retVal = VISA_call(visa, (VISA_Msg *)&msg);
    if (retVal != IECO_EOK) 
    {
        goto exit;
    }

    /*
     * unmarshall outArgs.
     * Do a wholesale replace of skeleton returned structure.
     * Pointer conversion of fields in outArgs is done below (only
     * in the case of a successful return value).
     */
    pMsgOutArgs = (IECO_OutArgs *)&(msg->cmd.process.outArgs);

    if (pMsgOutArgs->size !=  outArgs->size)
    {
        retVal = IECO_ERUNTIME;
        goto exit;
    }

    

    
    /* if VISA_call was successful umarshall outArgs */
    if(pMsgOutArgs->status)
    {
        outArgs->status = (Int32 *) Memory_getBufferVirtualAddress( 
                                                           (UInt32) pMsgOutArgs->status,
                                                           sizeof(IECO_StreamDesc));
    }

    for (i = 0; i < IECO_MAX_STREAMS; i++) 
    {
        int j;
        IECO_StreamDesc **pStreamDst, **pStreamSrc;

        pStreamSrc = &(pMsgOutArgs->outStream[i]);
        pStreamDst = &(outArgs->outStream[i]); 

        if(!*pStreamSrc)
        {
            /* NULL stream descriptor */
            continue;
        }
        
        *pStreamSrc = (IECO_StreamDesc *) Memory_getBufferVirtualAddress( 
                                                           (UInt32) *pStreamSrc,
                                                           sizeof(IECO_StreamDesc));
        if (*pStreamSrc != *pStreamDst)
        {
            retVal = IECO_ERUNTIME;
            goto exit;
        }
        
        Memory_cacheInv(*pStreamSrc, sizeof(IECO_StreamDesc));

        for (j = 0; j < IECO_MAX_IO_BUFFERS; j++)
        {
            if (!(*pStreamDst)->buf[j])
            {
                /* NULL buffer - unused channel */
                continue;
            }

            (*pStreamDst)->buf[j] = (UInt8*) Memory_getBufferVirtualAddress(  
                                          		(UInt32) (*pStreamDst)->buf[j],
                                                        (*pStreamDst)->bufSize[j]);
         }
    }


exit:
    VISA_freeMsg(visa, (VISA_Msg)msg);

    return (retVal);
}

/*
 *  ======== control ========
 *  this is the stub-implementation for the co
 */
static XDAS_Int32 control(IECO_Handle h, IECO_Cmd cmd, IECO_DynamicParams *params, IECO_Status *status)
{
    XDAS_Int32 retVal;
    VISA_Handle visa = (VISA_Handle)h;
    _ECO_Msg *msg;
    IECO_Status *pMsgOutArgs;

    /* get a message appropriate for this algorithm */
    if ((msg = (_ECO_Msg *)VISA_allocMsg(visa)) == NULL) {
        return (ECO_ERUNTIME);
    }

    /* zero out msg->cmd (not msg->visa!) */
    memset(&(msg->cmd), 0, sizeof(msg->cmd));

    /* marshall the command */
    msg->visa.cmd = _ECO_CCONTROL;

    /*
     * 1) params->buf is a pointer.
     */
    msg->cmd.control.params.size = params->size;
    msg->cmd.control.cmd = cmd;
    if(params->bufSize)
    {
      msg->cmd.control.params.buf = (Int8 *)Memory_getBufferPhysicalAddress(params->buf,params->bufSize , NULL);
      msg->cmd.control.params.bufSize = params->bufSize;
    }

    /* point at status and set the values */
    pMsgOutArgs =  (IECO_Status *)&(msg->cmd.control.status);

    /* set the size field - the rest is filled in by the algorithm */
    pMsgOutArgs->size = status->size;

    if(status->bufSize)
    {
      pMsgOutArgs->buf = (Int8 *)Memory_getBufferPhysicalAddress(status->buf, status->bufSize, NULL);
      pMsgOutArgs->bufSize = status->bufSize;
    }
    
    /* send the message to the skeleton and wait for completion */
    retVal = VISA_call(visa, (VISA_Msg *)&msg);
    if (retVal == IECO_EOK)
    {

        pMsgOutArgs =  (IECO_Status *)&(msg->cmd.control.status);

        if(pMsgOutArgs->size !=  status->size)
            goto exit;

        /* if VISA_call was successful umarshall outArgs */
        if (msg->cmd.control.status.bufSize > 0) 
        {
            status->buf = Memory_getBufferVirtualAddress(
                (UInt32)msg->cmd.control.status.buf,
                msg->cmd.control.status.bufSize);
        }
    }
exit:  

    VISA_freeMsg(visa, (VISA_Msg)msg);

    return (retVal);
}

