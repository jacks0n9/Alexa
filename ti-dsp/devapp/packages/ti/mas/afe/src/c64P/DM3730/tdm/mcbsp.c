/* ========================================================================== */
/**
 *  @file   tdmisr.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/c64P/DM3730/tdm/tdmisr.c
 *
 *  @brief  Contains TDM ISR function
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/* ========================================================================== */
/*                            INCLUDE FILES                                   */
/* ========================================================================== */

/* -- Standard C Headers -- */

/* -- XDC Types -- */
#include <xdc/std.h>

/* -- DSP/BIOS Headers -- */
#include <std.h>
#include <sio.h>
#include <iom.h>
#include <log.h>

/* -- DSP/BIOS PSP Headers -- */
#include <ti/sdo/edma3/drv/edma3_drv.h>
#include <ti/pspiom/mcbsp/Mcbsp.h>
#include <ti/pspiom/cslr/cslr.h>
#include <ti/pspiom/cslr/tistdtypes.h>
#include <ti/pspiom/cslr/soc_iva2.h>
#include <ti/pspiom/cslr/operations.h>
#include <ti/pspiom/prcm/Prcm.h>
#include <ti/pspiom/platforms/dm3730/Mcbsp_evmInit.h>

/* -- TDM Headers -- */
#include "tdmloc.h"

/* ========================================================================== */
/*                        IMPORTED VARIABLES                                  */
/* ========================================================================== */

extern LOG_Obj trace;
extern EDMA3_DRV_Handle hEdma[];

void halTxTdmIsr(void *ptr);
void halRxTdmIsr(void *ptr);

/* ========================================================================== */
/*                         MACRO DEFINITIONS                                  */
/* ========================================================================== */

#define BUFALIGN              128           /* align buffers to 128 bytes     */

#define RX_LOOP_JOB_LENGTH    (AFE_NUM_MIC_CH)
#define TX_LOOP_JOB_LENGTH    (AFE_NUM_TX_CHANNELS)

/* ========================================================================== */
/*                          LOCAL VARIABLES                                   */
/* ========================================================================== */

UInt16 rxLoopJobBuffer[RX_LOOP_JOB_LENGTH] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0};

UInt16 txLoopJobBuffer[TX_LOOP_JOB_LENGTH] = {0x0000, 0x0000};

/*
 * Mcbsp device params. To be filled in userMcbspInit function which
 * is called before driver creation
 */
Mcbsp_Params     mcbspTxParams;
Mcbsp_Params     mcbspRxParams;

/*
 * Mcbsp sample rate configuration parameters.
 */
Mcbsp_srgConfig  mcbspTxSrgParams;
Mcbsp_srgConfig  mcbspRxSrgParams;

/* Handles for the TX channels                            */
SIO_Handle mcbspOutHandle = NULL;

/* Handles for the RX channels                            */
SIO_Handle mcbspInHandle = NULL;

/**< settings to configure the RX and TX hardware sections */
Mcbsp_DataConfig mcbspChanConfig[2] =
{
  /**< Rx channel setup */
  {
    Mcbsp_Phase_SINGLE,
    Mcbsp_WordLength_16,
    Mcbsp_WordLength_16,   /* Dont care for single phase  */
    AFE_NUM_MIC_CH,
    AFE_NUM_MIC_CH,   /* Dont care for single phase  */
    Mcbsp_FrmSync_DETECT,
    Mcbsp_DataDelay_1_BIT,
    Mcbsp_BitReversal_DISABLE,
    Mcbsp_IntMode_ON_SYNCERR,
    Mcbsp_RxJust_RZF,      /* Dont care for RX            */
    Mcbsp_DxEna_OFF
  },
  /**< Tx channel setup */
  {
    Mcbsp_Phase_SINGLE,
    Mcbsp_WordLength_16,
    Mcbsp_WordLength_16,   /* Dont care for single phase  */
    AFE_NUM_TX_CHANNELS,
    AFE_NUM_TX_CHANNELS,   /* Dont care for single phase  */
    Mcbsp_FrmSync_DETECT,
    Mcbsp_DataDelay_1_BIT,
    Mcbsp_BitReversal_DISABLE,
    Mcbsp_IntMode_ON_SYNCERR,
    Mcbsp_RxJust_RZF,      /* Dont care for TX            */
    Mcbsp_DxEna_OFF
  }
};

/**< clock setup for the RX and TX section               */
Mcbsp_ClkSetup mcbspClkConfig[2] =
{
    /* RX clk setup */
  {
    Mcbsp_FsClkMode_EXTERNAL,
    16000,                   /* 16KHz                   */
    Mcbsp_TxRxClkMode_EXTERNAL,
    Mcbsp_FsPol_ACTIVE_HIGH,
    Mcbsp_ClkPol_RISING_EDGE
  },
    /* TX clk setup */
  {
    Mcbsp_FsClkMode_EXTERNAL,
    16000,                   /* 16KHz                   */
    Mcbsp_TxRxClkMode_EXTERNAL,
    Mcbsp_FsPol_ACTIVE_HIGH,
    Mcbsp_ClkPol_FALLING_EDGE
  }
};

/**< Multi channel setup                                */
Mcbsp_McrSetup mcbspMultiChanCtrl =
{
    Mcbsp_McmMode_ALL_CHAN_ENABLED_UNMASKED,
    Mcbsp_PartitionMode_CHAN_0_15,
    Mcbsp_PartitionMode_CHAN_16_31,
    Mcbsp_PartitionMode_2
};

/* McBSP RX channel param	*/
Mcbsp_ChanParams mcbspChanparamRx =
{
    Mcbsp_WordLength_16,  /* wordlength configured    */
    rxLoopJobBuffer,      /* loop job buffer internal */
    (2*RX_LOOP_JOB_LENGTH),   /* user loopjob length      */
    NULL,                 /* global error callback    */
    NULL,                 /* edma Handle              */
    1,                    /* EDMA event queue         */
    9,                    /* hwi number               */
    Mcbsp_BufferFormat_1SLOT,
    FALSE,                /* FIFO mode enabled        */
    &mcbspChanConfig[0],  /* channel configuration    */
    &mcbspClkConfig[0],   /* clock configuration      */
    &mcbspMultiChanCtrl,  /* multi channel control    */
    0x00,
    0x00,
    0x00,
    0x00
};

/* McBSP TX channel param	*/
Mcbsp_ChanParams mcbspChanparamTx =
{
    Mcbsp_WordLength_16,  /* wordlength configured    */
    txLoopJobBuffer,      /* loop job buffer internal */
    (2*TX_LOOP_JOB_LENGTH),   /* user loopjob length      */
    NULL,                 /* global error callback    */
    NULL,                 /* edma Handle              */
    0,                    /* EDMA event queue         */
    8,                    /* hwi number               */
    Mcbsp_BufferFormat_1SLOT,
    FALSE,                /* FIFO mode enabled        */
    &mcbspChanConfig[1],  /* channel configuration    */
    &mcbspClkConfig[1],   /* clock configuration      */
    &mcbspMultiChanCtrl,  /* multi channel control    */
    0x00,
    0x00,
    0x00,
    0x00
};

Bool postTxSWI = FALSE;
Bool postRxSWI = FALSE;

/* ========================================================================== */
/*                           FUNCTION DEFINTIONS                              */
/* ========================================================================== */

/*
 * \brief   Function to submit request the driver.
 *
 * \param   None
 *
 * \return  None
 */
static UInt16 mcbspDriverPrime(halTdmMcBsp_t *tdm)
{
    UInt32 bufSize;
    Ptr    rcv;
    Ptr    xmt;
    Int    status;

    postTxSWI = TRUE;
    postRxSWI = TRUE;

    bufSize = tdm->rxSubFrame * AFE_NUM_MIC_CH;      /* = size of Rx subFrame in number of samples */
    bufSize = bufSize * 2;                           /* = size of Rx subFrame in number of bytes   */

    /* Issue Rx PING buffer */
    rcv = tdm->dma.rxAddr;
    status = SIO_issue(mcbspInHandle, rcv, bufSize, NULL);
    if (status != IOM_COMPLETED) {
        LOG_printf(&trace, "Issue to input stream failed. Error = %d\n", status);
        return(1);
    }

    /* Issue Rx PONG buffer */
    rcv = tdm->dma.rxReldAddr;
    status = SIO_issue(mcbspInHandle, rcv, bufSize, NULL);
    if (status != IOM_COMPLETED) {
        LOG_printf(&trace, "Issue to input stream failed. Error = %d\n", status);
        return(1);
    }

    bufSize = tdm->txSubFrame * AFE_NUM_TX_CHANNELS; /* = size of Tx subFrame in number of samples */
    bufSize = bufSize * 2;                           /* = size of Tx subFrame in number of bytes   */

    /* Issue Tx PING buffer */
    xmt = tdm->dma.txAddr;
    status = SIO_issue(mcbspOutHandle, xmt, bufSize, NULL);
    if (status != IOM_COMPLETED) {
        LOG_printf(&trace, "Issue to output stream failed. Error = %d\n", status);
        return(1);
    }

    /* Issue Tx PONG buffer */
    xmt = tdm->dma.txReldAddr;
    status = SIO_issue(mcbspOutHandle, xmt, bufSize, NULL);
    if (status != IOM_COMPLETED) {
        LOG_printf(&trace, "Issue to output stream failed. Error = %d\n", status);
        return(1);
    }

    LOG_printf(&trace, "Requests submitted to the Mcbsp driver.");
    return(0);
}

void txSWI_andnHook (Arg swi, Arg key)
{
    if (postTxSWI == TRUE)
      SWI_andn((SWI_Handle)swi, key);

    return;
}

void rxSWI_andnHook (Arg swi, Arg key)
{
    if (postRxSWI == TRUE)
      SWI_andn((SWI_Handle)swi, key);

    return;
}

/*
 * \brief    Function to create the required streams for the transmission of
 *           Mcbsp data.
 *
 * \params   NONE
 *
 * \return   STATUS: 0=SUCCESS 1=FAILURE
 */
static UInt16 mcbspCreateStreams(halTdmMcBsp_t *tdm)
{
    SWI_Attrs      swiAttrs;
    SWI_Handle     swiHandle = NULL;

    DEV_Callback   callBack;

    SIO_Attrs      sioAttrs;
    Uint32         bufSize;

    /* TX ISR setup	*/
    swiAttrs = SWI_ATTRS;
    swiAttrs.fxn = (SWI_Fxn)halTxTdmIsr;
    swiAttrs.arg0 = (Arg)tdm;
    swiAttrs.mailbox = 1;
    swiHandle = SWI_create(&swiAttrs);
    if (swiHandle == NULL) {
        LOG_printf(&trace, "TX SWI Creation Failed");
        return(1);
    }

    callBack.fxn = (Fxn)txSWI_andnHook;
    callBack.arg0 = (Arg)swiHandle;

    sioAttrs = SIO_ATTRS;
    sioAttrs.nbufs = 2;
    sioAttrs.align = BUFALIGN;
    sioAttrs.align = TRUE;
    sioAttrs.model = SIO_ISSUERECLAIM;
    sioAttrs.callback = (DEV_Callback *)&callBack;

    /* Create a TX channel for the transmission                               */

    bufSize = tdm->txSubFrame * AFE_NUM_TX_CHANNELS; /* = size of Tx subFrame in number of samples */
    bufSize = bufSize * 2;                           /* = size of Tx subFrame in number of bytes   */

    callBack.arg1 = (Arg)0x1;
    mcbspChanparamTx.edmaHandle = hEdma[0];

    mcbspOutHandle = SIO_create("/dioMcbspOUT", SIO_OUTPUT, bufSize, &sioAttrs);
    if (NULL == mcbspOutHandle) {
        LOG_printf(&trace, "Output Stream Creation Failed");
        return(1);
    }

    /* RX ISR setup	*/
    swiAttrs = SWI_ATTRS;
    swiAttrs.fxn = (SWI_Fxn)halRxTdmIsr;
    swiAttrs.arg0 = (Arg)tdm;
    swiAttrs.mailbox = 2;
    swiHandle = SWI_create(&swiAttrs);
    if (swiHandle == NULL) {
        LOG_printf(&trace, "RX SWI Creation Failed");
        return(1);
    }

    callBack.fxn = (Fxn)rxSWI_andnHook;
    callBack.arg0 = (Arg)swiHandle;

    sioAttrs = SIO_ATTRS;
    sioAttrs.nbufs = 2;
    sioAttrs.align = BUFALIGN;
    sioAttrs.align = TRUE;
    sioAttrs.model = SIO_ISSUERECLAIM;
    sioAttrs.callback = (DEV_Callback *)&callBack;

    /* Create a RX channel for the transmission                               */
    bufSize = tdm->rxSubFrame * AFE_NUM_MIC_CH;      /* = size of Rx subFrame in number of samples */
    bufSize = bufSize * 2;                           /* = size of Rx subFrame in number of bytes   */

    callBack.arg1 = (Arg)0x2;
    mcbspChanparamRx.edmaHandle = hEdma[0];

    mcbspInHandle = SIO_create("/dioMcbspIN", SIO_INPUT, bufSize, &sioAttrs);
    if (NULL == mcbspInHandle) {
        LOG_printf(&trace, "Input Stream Creation Failed");
        return(1);
    }

    return(0);
}

/*
 * \brief   This function demostrates the use of Mcbsp using an EVM to EVM
 *          communication setup.
 *
 * \param   None
 *
 * \return  Status: 0=SUCCESS 1=FAIL
 */
UInt16 mcbspinit(halTdmMcBsp_t *tdm)
{
  UInt16 status = 1;

    /* create the streams required for the transactions                       */
    status = mcbspCreateStreams(tdm);

    if (status == 0) {
      /* prime the driver                                                       */
      status = mcbspDriverPrime(tdm);
    }

    return (status);
}

/*
 * \brief   This function demostrates the use of Mcbsp using an EVM to EVM
 *          communication setup.
 *
 * \param   None
 *
 * \return  None
 */
Void mcbspstop(Void)
{
  Uns oldCSR;
  Uns cmd;
  Arg arg = NULL;
  Int status;

    oldCSR = HWI_disable();
    postTxSWI = FALSE;
    postRxSWI = FALSE;
    HWI_restore(oldCSR);

    cmd = Mcbsp_IOCTL_STOP;
    status = SIO_ctrl(mcbspInHandle, cmd, arg);
    if (status != SYS_OK) {
        LOG_printf(&trace, "Stop to input stream failed. Error = %d\n", status);
    }

    cmd = Mcbsp_IOCTL_STOP;
    status = SIO_ctrl(mcbspOutHandle, cmd, arg);
    if (status != SYS_OK) {
        LOG_printf(&trace, "Stop to output stream failed. Error = %d\n", status);
    }
}

/*
 * \brief   This function demostrates the use of Mcbsp using an EVM to EVM
 *          communication setup.
 *
 * \param   None
 *
 * \return  None
 */
Void mcbspdelete(Void)
{
  Int status;

    if (NULL != mcbspInHandle) {
      status = SIO_delete(mcbspInHandle);
      if (status != SYS_OK) {
        LOG_printf(&trace, "Delete to input stream failed. Error = %d\n", status);
      }
      mcbspInHandle = NULL;
    }

    if (NULL != mcbspOutHandle) {
      status = SIO_delete(mcbspOutHandle);
      if (status != SYS_OK) {
        LOG_printf(&trace, "Delete to output stream failed. Error = %d\n", status);
      }
      mcbspOutHandle = NULL;
    }
}

/*
 * Mcbsp init function called when creating the driver.
 */
void mcbspTxUserInit()
{

    Mcbsp_init();

    mcbspTxParams = Mcbsp_PARAMS;
    mcbspTxParams.mode = Mcbsp_DevMode_McBSP;
    mcbspTxParams.opMode = Mcbsp_OpMode_DMAINTERRUPT;
    mcbspTxParams.enablecache = TRUE;
    mcbspTxParams.emulationMode = Mcbsp_EmuMode_FREE;
    mcbspTxParams.dlbMode = Mcbsp_Loopback_DISABLE;
    mcbspTxParams.clkStpMode = Mcbsp_ClkStpMode_DISABLED;
    mcbspTxParams.srgSetup = &mcbspTxSrgParams;
    mcbspTxParams.prcmPwrmEnable = FALSE;
}

/*
 * Mcbsp init function called when creating the driver.
 */
void mcbspRxUserInit()
{
    mcbspRxParams = Mcbsp_PARAMS;
    mcbspRxParams.mode = Mcbsp_DevMode_McBSP;
    mcbspRxParams.opMode = Mcbsp_OpMode_DMAINTERRUPT;
    mcbspRxParams.enablecache = TRUE;
    mcbspRxParams.emulationMode = Mcbsp_EmuMode_FREE;
    mcbspRxParams.dlbMode = Mcbsp_Loopback_DISABLE;
    mcbspRxParams.clkStpMode = Mcbsp_ClkStpMode_DISABLED;
    mcbspRxParams.srgSetup = &mcbspRxSrgParams;
    mcbspRxParams.prcmPwrmEnable = FALSE;
}
