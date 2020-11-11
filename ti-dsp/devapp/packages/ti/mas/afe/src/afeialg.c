/* ========================================================================== */
/**
 *  @file   afeialg.c
 *
 *  path    /ti/mas/afe/src/afeialg.c
 *
 *  @brief  The API definitions for Audio Front End
 *
 *  ============================================================================
 */
/* --COPYRIGHT--,BSD
 * Copyright (c) 2009, Texas Instruments Incorporated
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
 * --/COPYRIGHT--*/

/*
 *  ======== afe_impl.c ========
 *  "Devnode" algorithm.
 *
 *  This file contains an implementation of the IALG interface
 *  required by XDAS.
 */
#include <xdc/std.h>
#include <string.h>
#define AFE_H_NEED_MSGQ
#include "ti/bios/include/msgq.h"

#include <ti/bios/include/std.h>
#include <ti/bios/include/dev.h>
#include <ti/bios/include/sio.h>
#include <ti/bios/include/tsk.h>

#include <ti/sdo/ce/trace/gt.h>
#include <ti/sdo/ce/osal/Memory.h>

#include <c6x.h>
#include <dsplink/gpp/inc/usr/dsplink.h>
#include <ti/xdais/ialg.h>

#include "ti/mas/afe/afe.h"
#include "ti/mas/afe/src/afedev.h"
#include "ti/mas/afe/src/afedevalg.h"


#include "control.h"
#include "dataLog.h"

UInt32    avgProcessCycles = 0;


/**
 *  @brief      This must be the first field of all AFE instance objects.
 */
typedef struct AFE_Obj {
    struct AFE_Fxns *fxns;
    SIO_Handle      playoutStream;
    SIO_Handle      captureStream;
    Void           *afedevObj;
} AFE_Obj;

Int   AFE_numAlloc(void);
Int   AFE_alloc(const IALG_Params *algParams, IALG_Fxns **pf, IALG_MemRec memTab[]);
Int   AFE_free(IALG_Handle handle, IALG_MemRec memTab[]);
Int   AFE_initObj(IALG_Handle handle, const IALG_MemRec memTab[], IALG_Handle p, const IALG_Params *algParams);
Int   AFE_process(IDEVNODE_Handle handle, IDEVNODE_InArgs *inArgs, IDEVNODE_OutArgs *outArgs);
Int   AFE_control(IDEVNODE_Handle handle, IDEVNODE_Cmd cmd, IDEVNODE_Ctrl_InArgs *inArgs, IDEVNODE_Ctrl_OutArgs *outArgs);

#define IALG_AFE_FXNS  \
    &AFE_TI_IDEVNODE,/* module ID */                       \
    NULL,          /* activate */                          \
    AFE_alloc,     /* alloc */                             \
    NULL,          /* control (NULL => no control ops) */  \
    NULL,          /* deactivate */                        \
    AFE_free,      /* free */                              \
    AFE_initObj,   /* init */                              \
    NULL,          /* moved */                             \
    AFE_numAlloc   /* numAlloc (NULL => IALG_MAXMEMRECS) */

/*
 *  ======== AFE_TI_IDEVNODE ========
 *  This structure defines TI's implementation of the IDEVNODE interface
 *  for the AFE_TI module.
 */
extern IDEVNODE_Fxns AFE_TI_IDEVNODE = {
    {IALG_AFE_FXNS},
    AFE_process,
    AFE_control,
};


/*
 *  ======== AFE_TI_PARAMS ========
 */
AFE_Params AFE_TI_PARAMS = {
    sizeof(AFE_Params),
	(16000),
	(16000),
	(10),
	(2),
	(10),
	(1)
};

MSGQ_Queue  replyMsgQueue;

/*  ======== AFE_numAlloc ========
 *  Returns Number of Buffers Required by AFE for a particular configuration.
 */
/**
 *  @brief      Returns the maximum number of memory records that can be
 *              processed by AFE_alloc().
 *
 *  @sa         AFE_alloc
 *
 */
Int  AFE_numAlloc(void)
{
  Int totalBufs = 0;

  /* One Buffer for AFE_Obj */
  totalBufs++;

  // Add buffers for audio data
  totalBufs += AUDIO_BUFFER_NUM + AUDIO_BUFFER_NUM;
  // Add buffers for scratchpads
  totalBufs +=2;

  /* Buffers needed by the DEV driver */
  totalBufs += AFEDEV_getNumBuffers();

  return(totalBufs);
}

/*
 *  ======== AFE_alloc ========
 */
Int AFE_alloc(const IALG_Params *algParams, IALG_Fxns **pf, IALG_MemRec memTab[])
{
  Int i, num, buf = 0;
  AFE_Params *params = (AFE_Params *)algParams;
    FIX_COMPIL_WARNING(pf);

    num = AFE_numAlloc();
    for (i = 0; i < num; i++) {
      memTab[i].size = 0;
      memTab[i].alignment = 4;
      memTab[i].space = IALG_EXTERNAL;
      memTab[i].attrs = IALG_PERSIST;
    }

    /* Request memory for AFE_Obj */
    memTab[buf].size = sizeof(AFE_Obj);
    buf++;


    /*****************************************************************************
     * Request memory for:
     * 2 Speaker buffers
     * 2 Microphone buffers
     * 1 Scratch Buffer for control
     * 1 General scratch buffer to be used by all */
    for(i=0; i < AUDIO_BUFFER_NUM; i ++){
        memTab[buf].space = IALG_DARAM0;            // Allocate Memory in L1D
        memTab[buf++].size = AUDIO_SPEAK_BUF_SIZE;
    }
    for(i=0; i < AUDIO_BUFFER_NUM; i ++){
        memTab[buf].space = IALG_DARAM0;            // Allocate Memory in L1D
        memTab[buf++].size = AUDIO_MICRO_BUF_SIZE;
    }
    memTab[buf].space = IALG_DARAM0;                // Allocate Memory in L1D
    memTab[buf++].size = AUDIO_MICRO_BUF_SIZE;
    memTab[buf].space = IALG_DARAM0;                // Allocate Memory in L1D
    memTab[buf++].size = GENERAL_SCRATCH_BUF_SIZE;
    /****************************************************************************/


    /* Buffers needed by the DEV driver layer: Note that order is important, its the way allocation should happen in algInit() */
    buf += AFEDEV_getSizes(&(params->createParams), &(memTab[buf]));

    return (buf);
}

/*
 *  ======== AFE_initObj ========
 */
Int AFE_initObj(IALG_Handle handle, const IALG_MemRec memTab[], IALG_Handle p, const IALG_Params *algParams)
{
  Int32  retVal = IDEVNODE_EOK;
  Int totalBufs = 0;
  AFE_Obj *inst = (AFE_Obj *)handle;
  AFE_Params *params = (AFE_Params *)algParams;
  SIO_Attrs sio_attrs = SIO_ATTRS;

  DEV_Attrs dafAttrs;
  DAF_Params dafParams;

  Void *afedevHandle = NULL;

  Int               msgqStatus;
  MSGQ_LocateAttrs  syncLocateAttrs ;
  int i;

    FIX_COMPIL_WARNING(p);
    /*
     * In lieu of xDM defined default params, use algo-specific ones.
     * Note that these default values _should_ be documented in your algorithm
     * documentation so users know what to expect.
     */
    if (params == NULL) {
        params = &AFE_TI_PARAMS;
    }

    /* allocated AFE_Obj */
    totalBufs++;


    /*****************************************************************************
     * Request memory for:
     * 2 Speaker buffers
     * 2 Microphone buffers
     * 1 Scratch Buffer for control
     * 1 General scratch buffer to be used by all */
    for(i=0; i < AUDIO_BUFFER_NUM; i ++){
        pgSpeakBuf[i] = (u32 *) memTab[totalBufs++].base;
    }
    for(i=0; i < AUDIO_BUFFER_NUM; i ++){
        pgMicroBuf[i] = (u32 *) memTab[totalBufs++].base;
    }
    pgCtrlScratchBuf  = (u32 *) memTab[totalBufs++].base;
    pgAllScratchBuf   = (u32 *) memTab[totalBufs++].base;
    /****************************************************************************/

    /* Create AFEDEV driver instance */
    afedevHandle = AFEDEV_create(&(params->createParams), &memTab[totalBufs]);
    if (!afedevHandle) {
      return IDEVNODE_EFAIL;
    }
    inst->afedevObj = afedevHandle;

    /* Handover this AFEDEV driver instance to a DEV entity */
    dafParams.afedevObj = afedevHandle;
    dafParams.maxFrameSize = params->createParams.frame_size_ms;
    dafParams.maxRxSamplingFreq = params->createParams.rx_sampling_rate;
    dafParams.maxNumRxChannels = params->createParams.num_rx_tdmchans;
    dafParams.maxTxSamplingFreq = params->createParams.tx_sampling_rate;
    dafParams.maxNumTxChannels = params->createParams.num_tx_tdmchans;

    dafAttrs.devid = NULL;          /* Int devid */
    dafAttrs.params = &dafParams;   /* Ptr params */
    dafAttrs.type = DEV_SIOTYPE;    /* Uns type   */
    dafAttrs.devp = NULL;           /* Ptr devp   */
    retVal = DEV_createDevice("/afedev", &DAF_FXNS, (Fxn)DAF_init, &dafAttrs);
    if (retVal != SYS_OK) {
      return retVal;
    }

    /* Create an SIO stream for Capture */
    sio_attrs.align = 4;
    sio_attrs.model = SIO_ISSUERECLAIM;
    sio_attrs.nbufs = 2;
    inst->captureStream = SIO_create("/afedev", SIO_INPUT, IDEVNODE_MAX_STREAMS*DAF_STREAM_USAGE_MONO_NUM_CHANNELS_CAPTURE*sizeof(DAF_Frame), &sio_attrs);
    if (!inst->captureStream) {
      return (IDEVNODE_EFAIL);
    }

    /* Create an SIO stream for Playout */
    inst->playoutStream = SIO_create("/afedev", SIO_OUTPUT, IDEVNODE_MAX_STREAMS*DAF_STREAM_USAGE_MONO_NUM_CHANNELS_PLAYOUT*sizeof(DAF_Frame), &sio_attrs);
    if (!inst->playoutStream) {
      return (IDEVNODE_EFAIL);
    }

    /*
     *  Locate GPP message queue handle.
     *  Spin until GPP message is located
     *  while allowing other TSKs to execute.
     */
    msgqStatus = SYS_ENOTFOUND;
    while ((msgqStatus == SYS_ENOTFOUND) || (msgqStatus == SYS_ENODEV)) {
      syncLocateAttrs.timeout = SYS_FOREVER;
      msgqStatus = MSGQ_locate("outputGPP", &replyMsgQueue, &syncLocateAttrs);
      if ((msgqStatus == SYS_ENOTFOUND) || (msgqStatus == SYS_ENODEV)) {
        TSK_sleep(1000);
      }
      else if (msgqStatus != SYS_OK) {
          EVT_LOG("AFE_initObj msgQ ERROR %x\n", msgqStatus);
      }
    }

    return (IALG_EOK);
}

/*
 *  ======== AFE_process ========
 */
Int32 AFE_process(IDEVNODE_Handle handle, IDEVNODE_InArgs *inArgs, IDEVNODE_OutArgs *outArgs)
{
    ctrlProcess();

    return (IDEVNODE_EOK);
}

/***************************************************************************
 *   int AFE_SendMsg(void * pMsg, int Size)
 *   Send message to ARM
 **************************************************************************/
int AFE_SendMsg(void * pMsg, int Size)
{
    int status;
    tMsgFromDsp *inputMsg;

    /* Allocate a message */
    status = MSGQ_alloc(0, (MSGQ_Msg *)&inputMsg, APPMSGSIZE);
    if (status != SYS_OK) {
        EVT_LOG( "AFE_SendMsg ERROR: MSGQ_alloc FAILED\n");
    }

    // Add DSP msg ID
    inputMsg->header.msgId = APPMSGSIGNATURE;

    memcpy( &inputMsg->labmsg, pMsg, Size);

    /* Put a message on the GPP message queue to get system going */
    status = MSGQ_put(replyMsgQueue, (MSGQ_Msg)inputMsg);
    if (status != SYS_OK) {
        EVT_LOG("AFE_SendMsg ERROR: MSGQ_put FAILED\n");
    }
    else {
        //          EVT_LOG("AFE_SendMsg SUCCESS\n");
    }

    return status;
}



/*
 *  ======== AFE_control ========
 */
XDAS_Int32 AFE_control(IDEVNODE_Handle handle, IDEVNODE_Cmd cmd, IDEVNODE_Ctrl_InArgs *inArgs, IDEVNODE_Ctrl_OutArgs *outArgs)
{
    Int32                   retVal = IDEVNODE_EOK;
    AFE_ControlParams      *afeCtrl;
    AFE_ResponseParams     *afeResp;
    DAF_SioCtrl             sioCtrl;

    AFE_Obj *inst = (AFE_Obj *)handle;
    FIX_COMPIL_WARNING(cmd);

    afeCtrl = (AFE_ControlParams *)(inArgs->message);
    afeResp = (AFE_ResponseParams *)(outArgs->message);

    /* check for creation/deletion of the instance: Note: This is done here in order to avoid the VISA_free()
       implementation problem in Codec Engine. CE may not call the AFE_free() */
    if(afeCtrl->controlId == AFE_MSG_SHUTDOWN) {
        // Call the crtl function to terminate
        ctrlTerminate();

        // Wait for any AFE_process() to get finished.
        afeCtrl->controlId = AFE_MSG_SHUTDOWN;
        sioCtrl.ctrl = &(afeCtrl->ctrl);
        sioCtrl.resp = afeResp;
        retVal = SIO_ctrl (inst->playoutStream, (Uns) (afeCtrl->controlId), (Arg) (&sioCtrl));

        // Then shutdown the AFEDEV driver
        if(inst->afedevObj)
            AFEDEV_delete(inst->afedevObj);

    }
    else {
        sioCtrl.ctrl = &(afeCtrl->ctrl);
        sioCtrl.resp = afeResp;
        retVal = SIO_ctrl (inst->playoutStream, (Uns) afeCtrl->controlId, (Arg) &sioCtrl);
    }

    return (retVal);
}

/*
 *  ======== AFE_free ========
 */
Int AFE_free(IALG_Handle handle, IALG_MemRec memTab[])
{
    AFE_Obj *inst = (AFE_Obj *)handle;
    AFE_ControlParams       afeCtrl;
    AFE_ResponseParams      afeResp;
    DAF_SioCtrl             sioCtrl;
    Int32                   retVal = IDEVNODE_EOK;
    FIX_COMPIL_WARNING(memTab);

    afeCtrl.controlId = AFE_MSG_SHUTDOWN;
    sioCtrl.ctrl = &(afeCtrl.ctrl);
    sioCtrl.resp = &afeResp;
    retVal = SIO_ctrl (inst->playoutStream, (Uns) (afeCtrl.controlId), (Arg) (&sioCtrl));

    /* Free AFEDEV driver instance */
    if(inst->afedevObj) {
      AFEDEV_delete(inst->afedevObj);
    }


    return (retVal);
}
