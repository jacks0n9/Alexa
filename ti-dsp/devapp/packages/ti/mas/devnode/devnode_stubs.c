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
 *  ======== devnode_stubs.c ========
 *  These functions are the "app-side" of a "remote" implementation.
 *
 */
#include <xdc/std.h>
#include <ti/sdo/ce/visa.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <string.h>

#include "idevnode.h"
#include "devnode.h"
#include "_devnode.h"

static XDAS_Int32 process(IDEVNODE_Handle h, IDEVNODE_InArgs *inArgs, IDEVNODE_OutArgs *outArgs);
static XDAS_Int32 control(IDEVNODE_Handle h, IDEVNODE_Cmd cmd, IDEVNODE_Ctrl_InArgs *inArgs, IDEVNODE_Ctrl_OutArgs *outArgs);
/*
 * Stubs must implement the interface they "stub" for.  This devnode stub
 * proxy's for IDEVNODE_Fxns, which is IALG + process() and control().
 *
 * Note that the IALG fxns for the stub currently aren't called, so
 * they're all set to NULL.
 */
IDEVNODE_Fxns DEVNODE_STUBS = {
    {&DEVNODE_STUBS, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    process, control,
};

//#define DEVNODE_INTERNAL_DBG 1

#ifdef DEVNODE_INTERNAL_DBG
#define MAX_DEV_DBG_ENTRY       500
#define MAX_DEV_PROBE_POINTS  20
extern Uint32 devnodeDbgTimeStamp[MAX_DEV_DBG_ENTRY][MAX_DEV_PROBE_POINTS];
extern Uint32 devnodeDbgTimeStampIdx;

extern int devnode_ReadTimer(Uint32 *pTimeRead);
#endif /* #ifdef DEVNODE_INTERNAL_DBG */

/*
 *  ======== process ========
 *  This is the stub-implementation for the process method
 */
static XDAS_Int32 process(IDEVNODE_Handle h, IDEVNODE_InArgs *inArgs, IDEVNODE_OutArgs *outArgs)
{
    XDAS_Int32 retVal;
    VISA_Handle visa = (VISA_Handle)h;
    _DEVNODE_Msg *msg;
    Int i;
    IDEVNODE_OutArgs *pMsgOutArgs;
 

    /*
     * Validate arguments.  Do we want to do this _every_ time, or just in
     * debug builds via GT_assert()?
     */
    if ((inArgs == NULL) || (inArgs->size < sizeof(IDEVNODE_InArgs)) ||
        (outArgs == NULL) || (outArgs->size < sizeof(IDEVNODE_OutArgs))) {

        /* invalid args, could even assert here, it's a spec violation. */
        return (IDEVNODE_EFAIL);
    }

    if (/* size of "stuff to marshall" > message size */
        (sizeof(VISA_MsgHeader) + inArgs->size + outArgs->size) > sizeof(_DEVNODE_Msg)) {

        return (IDEVNODE_EFAIL);
    }

    /* get a message appropriate for this algorithm */
    if ((msg = (_DEVNODE_Msg *)VISA_allocMsg(visa)) == NULL) 
    {
        return (IDEVNODE_EFAIL);
    }

    /* zero out msg->cmd (not msg->visa!) */
    memset(&(msg->cmd), 0, sizeof(msg->cmd));

    /*
     * Marshall the command: copy the client-passed arguments into flattened
     * message data structures, converting every pointer address to alg.
     * data buffer into physical address.
     */

    /* First specify the processing command that the skeleton should do */
    msg->visa.cmd = _DEVNODE_CPROCESS;

    /* Check if numStreams is within range */
    if (inArgs->numStreams > IDEVNODE_MAX_STREAMS)
    {
        retVal = IDEVNODE_EFAIL;
        goto exit;
    }
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][2]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
    msg->cmd.process.inArgs.numStreams = inArgs->numStreams;
    /*
     * 1) inArgs->issueStream is a pointer to a sparse array of pointers, so we take
     * individual pointers, convert them if non-NULL, and store them in the
     * message counterpart.
     */
    for (i = 0; i < inArgs->numStreams; i++) 
    {
        int j;
        IDEVNODE_StreamDesc **pStreamDst, **pStreamSrc;

        pStreamSrc = &(inArgs->issueStream[i]);
        pStreamDst = &(msg->cmd.process.inArgs.issueStream[i]); 

        if(!*pStreamSrc)
        {
            /* NULL stream descriptor */
            *pStreamDst = NULL;
            continue;
        }
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][10]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */    

    if(inArgs->v2pOffset)
    {
            *((Uint32 *)pStreamDst) = inArgs->v2pOffset + *((Uint32 *)pStreamSrc);
    }
    else
    {
       *pStreamDst = (IDEVNODE_StreamDesc *) Memory_getBufferPhysicalAddress( 
                                                           *pStreamSrc,
                                                           sizeof(IDEVNODE_StreamDesc),
                                                           NULL); 

    }
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][11]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */    
        if ((*pStreamSrc)->maxBufId > IDEVNODE_MAX_IO_BUFFERS)
        {
            retVal = IDEVNODE_EFAIL;
            goto exit;
        }

        for (j = 0; j < (*pStreamSrc)->maxBufId; j++)
        {
            if (!(*pStreamSrc)->buf[j])
            {
                /* NULL buffer - unused channel */
                continue;
            }
            if(inArgs->v2pOffset)
            {
              (*pStreamSrc)->buf[j] = (*pStreamSrc)->buf[j] + inArgs->v2pOffset;
            }
            else
            {
              (*pStreamSrc)->buf[j] = (UInt8*) Memory_getBufferPhysicalAddress(  
                                          		  (*pStreamSrc)->buf[j],
                                                          (*pStreamSrc)->bufSize[j],
                                                          NULL);
            }
         }
    }
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][3]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
    msg->cmd.process.inArgs.size = inArgs->size;
    msg->cmd.process.inArgs.timeStamp = inArgs->timeStamp;

    /* point at outArgs and set the "size" */
    pMsgOutArgs = &(msg->cmd.process.outArgs);

    /* set the size field - the rest is filled in by the codec */
    pMsgOutArgs->size = outArgs->size;
    pMsgOutArgs->numStreams = outArgs->numStreams;

    /*
     * 2) outArgs->reclaimedStream is a pointer to a sparse array of pointers, so we take
     * individual pointers, convert them if non-NULL, and store them in the
     * message counterpart.
     */
    for (i = 0; i < outArgs->numStreams; i++) 
    {
        IDEVNODE_StreamDesc **pStreamDst, **pStreamSrc;

        pStreamSrc = &(outArgs->reclaimedStream[i]);
        pStreamDst = &(pMsgOutArgs->reclaimedStream[i]); 

        if(!*pStreamSrc)
        {
            /* NULL stream descriptor */
            *pStreamDst = NULL;
            continue;
        }

        if(inArgs->v2pOffset)
        {
          *((Uint32 *)pStreamDst) = *((Uint32 *)pStreamSrc) + inArgs->v2pOffset;
        }
        else
        {
          *pStreamDst = (IDEVNODE_StreamDesc *) Memory_getBufferPhysicalAddress( 
                                                             *pStreamSrc,
                                                             sizeof(IDEVNODE_StreamDesc),
                                                             NULL); 
        }
    }

#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][4]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */

    /* send the message to the skeleton and wait for completion */
    retVal = VISA_call(visa, (VISA_Msg *)&msg);
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][5]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */    
    if (retVal != IDEVNODE_EOK) 
    {
        goto exit;
    }
    /*
     * unmarshall outArgs.
     * Do a wholesale replace of skeleton returned structure.
     * Pointer conversion of fields in outArgs is done below (only
     * in the case of a successful return value).
     */
    pMsgOutArgs = (IDEVNODE_OutArgs *)&(msg->cmd.process.outArgs);

    if (pMsgOutArgs->size !=  outArgs->size)
    {
        retVal = IDEVNODE_ERUNTIME;
        goto exit;
    }   

    
    /* if VISA_call was successful umarshall outArgs */
    outArgs->timeStamp  = pMsgOutArgs->timeStamp;
    outArgs->timeToAlgo = pMsgOutArgs->timeToAlgo;
    outArgs->numStreams = pMsgOutArgs->numStreams;

    if (!outArgs->numStreams)
    {
        goto exit;
    }

    for (i = 0; i < outArgs->numStreams; i++) 
    {
        int j;
        IDEVNODE_StreamDesc **pStreamDst, **pStreamSrc;

        pStreamSrc = &(pMsgOutArgs->reclaimedStream[i]);
        pStreamDst = &(outArgs->reclaimedStream[i]); 

        if(!*pStreamSrc)
        {
            /* NULL stream descriptor */
            continue;
        }
        
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][12]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
    if(inArgs->v2pOffset)
    {
      *((Uint32 *)pStreamSrc) = *((Uint32 *)pStreamSrc) - inArgs->v2pOffset;
    }
    else
    {
      *pStreamSrc = (IDEVNODE_StreamDesc *) Memory_getBufferVirtualAddress( 
                                                       (UInt32) *pStreamSrc,
                                                     sizeof(IDEVNODE_StreamDesc));
    }

#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][13]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */    
        if (*pStreamSrc != *pStreamDst)
        {
            retVal = IDEVNODE_ERUNTIME;
            goto exit;
        }     

        if ((*pStreamDst)->maxBufId > IDEVNODE_MAX_IO_BUFFERS)
        {
            retVal = IDEVNODE_ERUNTIME;
            goto exit;
        }

        for (j = 0; j < (*pStreamDst)->maxBufId; j++)
        {
            if (!(*pStreamDst)->buf[j])
            {
                /* NULL buffer - unused channel */
                continue;
            }
            if(inArgs->v2pOffset)
            {
               (*pStreamDst)->buf[j] = (*pStreamDst)->buf[j] - inArgs->v2pOffset;
            }
            else
            {
              (*pStreamDst)->buf[j] = (UInt8*) Memory_getBufferVirtualAddress(  
                                       		(UInt32) (*pStreamDst)->buf[j],
                                                        (*pStreamDst)->bufSize[j]);
            }
         }
    }


exit:
    VISA_freeMsg(visa, (VISA_Msg)msg);
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer(&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][6]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
    return (retVal);
}

/*
 *  ======== control ========
 *  this is the stub-implementation for the co
 */
static XDAS_Int32 control(IDEVNODE_Handle h, IDEVNODE_Cmd cmd, IDEVNODE_Ctrl_InArgs *inArgs, IDEVNODE_Ctrl_OutArgs *outArgs)
{
    XDAS_Int32 retVal;
    VISA_Handle visa = (VISA_Handle)h;
    _DEVNODE_Msg *msg;
    IDEVNODE_Ctrl_OutArgs *pMsgOutArgs;

    /* get a message appropriate for this algorithm */
    if ((msg = (_DEVNODE_Msg *)VISA_allocMsg(visa)) == NULL) {
        return (DEVNODE_ERUNTIME);
    }

    /* zero out msg->cmd (not msg->visa!) */
    memset(&(msg->cmd), 0, sizeof(msg->cmd));

    /* marshall the command */
    msg->visa.cmd = _DEVNODE_CCONTROL;

    /*
     * 1) inArgs->message is a pointer.
     */
    msg->cmd.control.inArgs.size = inArgs->size;
    msg->cmd.control.inArgs.messageSize = inArgs->messageSize;
    msg->cmd.control.cmd = cmd;
    msg->cmd.control.inArgs.message = (XDAS_UInt8 *) Memory_getBufferPhysicalAddress(inArgs->message,
                    inArgs->messageSize, NULL);

    /* point at outArgs and set the values */
    pMsgOutArgs =  (IDEVNODE_Ctrl_OutArgs *)&(msg->cmd.control.outArgs);

    /* set the size field - the rest is filled in by the algorithm */
    pMsgOutArgs->size = outArgs->size;
    pMsgOutArgs->messageSize = outArgs->messageSize;

    pMsgOutArgs->message = (XDAS_UInt8 *)Memory_getBufferPhysicalAddress(outArgs->message,
                    outArgs->messageSize, NULL);
    
    /* send the message to the skeleton and wait for completion */
    retVal = VISA_call(visa, (VISA_Msg *)&msg);
    /* if VISA_call was successful umarshall outArgs */
    if (retVal == IDEVNODE_EOK) 
    {
        pMsgOutArgs =  (IDEVNODE_Ctrl_OutArgs *)&(msg->cmd.control.outArgs);
        if(pMsgOutArgs->size !=  outArgs->size)
            goto exit;    
        outArgs->message = Memory_getBufferVirtualAddress(
                    (UInt32)msg->cmd.control.outArgs.message,
                    msg->cmd.control.outArgs.messageSize);
    }
exit:  

    VISA_freeMsg(visa, (VISA_Msg)msg);

    return (retVal);
}

