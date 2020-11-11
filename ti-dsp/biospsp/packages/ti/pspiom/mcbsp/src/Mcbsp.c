/*
 * Mcbsp.c
 *
 * This file contains Driver Layer Interface implementation for the McBSP Driver.
 * McBSP Driver provides Driver Layer Interface to do operations on the McBSP
 * peripheral like device initialization, channel creation, control commands for
 * peripheral specific operations etc
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/**
 * \file      Mcbsp.c
 *
 * \brief     Generic McBSP Driver for DSP/BIOS Platform.It uses the EDMA for
 *            data transfer
 *            both master and slave mode(should be supported also by the SOC).
 *
 *            This file implements the McBSP IOM driver to be used with
 *            DSP-BIOS operating system.
 *
 *           (C) Copyright 2009,Texas Instruments, Inc
 *
 * \author    Platform Support Group
 *
 * \version
 *            0.2  [25-06-2008]  Added the support for non loop job mode
 *            0.1  Created
 *
 */

/*============================================================================*/
/*                              INCLUDE FILES                                 */
/*============================================================================*/

#include "McbspLocal.h"
#include <assert.h>
#include <tsk.h>
#include <ecm.h>
#include <swi.h>
#include <hwi.h>
#include <string.h>
#include <bcache.h>
#include <c64.h>
#include <ti/pspiom/prcm/Prcm.h>
#include <clk.h>

/*============================================================================*/
/*                            GLOBAL VARIABLES                                */
/*============================================================================*/

/**
 * \brief    Mcbsp Sample rate generator default parameters.
 *
 */
static Mcbsp_srgConfig mcbspSrgCfg =
{
    FALSE,                     /* No gsync to be used as input is not CLKS    */
    Mcbsp_ClkSPol_RISING_EDGE, /* Dont care as input clock is not clks        */
    Mcbsp_SrgClk_CLKCPU,       /* McBSP internal clock to be used             */
    CSL_MCBSP_1_MODULE_FREQ,   /* Mcbsp internal clock frequency              */
    0                          /* frame sync pulse width (val+1) is used      */
};

/**
 * \brief    Mcbsp device creation default parameters.
 *
 */
const Mcbsp_Params Mcbsp_PARAMS =
{
    Mcbsp_DevMode_McBSP,       /* Use the device as MCBSP                     */
    Mcbsp_OpMode_DMAINTERRUPT, /* Use DMA mode of operation                   */
    TRUE,                      /* cache coherency taken care of by driver     */
    Mcbsp_EmuMode_SOFT_STOP,   /* Soft mode is to be enabled                  */
    Mcbsp_Loopback_DISABLE,    /* Loop back mode disabled                     */
    Mcbsp_ClkStpMode_DISABLED, /* clock Stop mode is disabled                 */
    &mcbspSrgCfg,              /* sample rate generator configuration         */
};

/* ========================================================================== */
/*                       GLOBAL MODULE STATE                                  */
/* ========================================================================== */
/**
 * \brief  Array to store the state of the module (whether it is in use or not).
 *
 */
static Bool inUse[CSL_MCBSP_PER_CNT];

/**
 * \brief  McBsp Module State Object
 */
static Mcbsp_Module_State Mcbsp_module = {&inUse[0]};

/**
 * \brief  Array of Mcbsp instance State objects array
 */
static Mcbsp_Object Mcbsp_Instances[CSL_MCBSP_PER_CNT];

/**
 * \brief  Mcbsp structure containing the information specific to an instance.
 */
static Mcbsp_HwInfo Mcbsp_deviceInstInfo[CSL_MCBSP_PER_CNT];

/**
 *  \brief  Mute buffer per instance
 *
 *  \note   Buffer used when the mcbsp is placed in mute.
 */
Mcbsp_TempBuffer Mcbsp_muteBuf[CSL_MCBSP_PER_CNT];

#ifdef Mcbsp_LOOPJOB_ENABLE
/**
 * \brief  Destination loop buffer per instance
 *
 * \note   Buffer used during the loop job mode for the Transmit section
 */
Mcbsp_TempBuffer Mcbsp_loopDstBuf[CSL_MCBSP_PER_CNT];

/**
 * \brief  Receive loop buffer per instance
 *
 * \note   Buffer used during the loop job mode for the Receive section
 */
Mcbsp_TempBuffer Mcbsp_loopSrcBuf[CSL_MCBSP_PER_CNT];
#endif /* Mcbsp_LOOPJOB_ENABLE */

/*============================================================================*/
/*                         LOCAL FUNCTION PROTOTYPES                          */
/*============================================================================*/

static Int mcbspMdBindDev(Ptr *devp, Int devid, Ptr devParams);
static Int mcbspMdUnBindDev(Ptr devp);
static Int mcbspMdCreateChan(Ptr                *chanp,
                             Ptr                 devp,
                             String              name,
                             Int                 mode,
                             Ptr                 chanParams,
                             IOM_TiomCallback    cbFxn,
                             Ptr                 cbArg);
static Int mcbspMdDeleteChan(Ptr chanp);
static Int mcbspMdSubmitChan(Ptr chanp, IOM_Packet *const ioPacket);
static Int mcbspMdControlChan(Ptr chanp, Uns cmd, Ptr arg);
static Int32 mcbspSubmitReq(Mcbsp_Object      *instHandle,
                            Mcbsp_ChannelObj  *chanHandle,
                            IOM_Packet        *ioPacket);
static Void mcbspConfigureFifo(Mcbsp_HwInfo        *hMcbsp,
                               Mcbsp_ChannelObj    *chanHandle,
                               Bool                 enableHwFifo);
static Void mcbspGblXmtIsr(Mcbsp_ChannelObj *chanHandle);
static Void mcbspGblRcvIsr(Mcbsp_ChannelObj *chanHandle);
static Int32 mcbspRegisterInt(Ptr chanp,Uint32 intNum,Uint16 evt,ECM_Fxn intIsr);
static Void msbspUnRegisterInt(Uint16 evt);

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
static Int32 McbspRegisterNotification(Mcbsp_Object *instHandle);
static Int32 McbspUnregisterNotification(Mcbsp_Object *instHandle);
static Int32 McbspUnregisterConstraints(Mcbsp_Object *instHandle);
static PWRM_NotifyResponse McbspNotifyCallback(PWRM_Event    eventType,
                                               Arg           eventArg1,
                                               Arg           eventArg2,
                                               Arg           clientArg);
static PWRM_NotifyResponse McbspSuspendCurrentIops(Mcbsp_Object  *instHandle,
                                                   PWRM_Event     eventType);
static Int32 McbspCalculateConstraints(Mcbsp_Object     *instHandle,
                                       Mcbsp_ChannelObj *chanHandle);
static PWRM_NotifyResponse McbspConfigureNewVfSettings(Mcbsp_Object *instHandle,
                                                       Uns           eventArg1,
                                                       Uns           eventArg2);
static Int32 McbspCalculateClockSettings(Mcbsp_Object      *instHandle,
                                         Mcbsp_ChannelObj  *chanHandle,
                                         Uint32             setpoint,
                                         Uint32            *clkgDiv,
                                         Uint32            *framePeriod);
static Int32 McbspLoadPendedIops(Mcbsp_ChannelObj *chanHandle);
#endif

#ifndef Mcbsp_LOOPJOB_ENABLE
static Void Mcbsp_swiTxFifo(Arg arg0,Arg arg1);
#endif /* Mcbsp_LOOPJOB_ENABLE */

/*============================================================================*/
/*                             GLOBAL VARIABLES                               */
/*============================================================================*/

/**
 * \brief IOM driver interface table
 *
 * \note  This table holds the interface functions exposed by the IOM driver to
 *        the GIO layer.
 */
IOM_Fxns Mcbsp_IOMFXNS =
{
    &mcbspMdBindDev,
    &mcbspMdUnBindDev,
    &mcbspMdControlChan,
    &mcbspMdCreateChan,
    &mcbspMdDeleteChan,
    &mcbspMdSubmitChan
};

/*============================================================================*/
/*                           IMPORTED VARIABLES                               */
/*============================================================================*/

/* ========================================================================== */
/*                          FUNCTION DEFINTIONS                               */
/* ========================================================================== */

/* ========================================================================== */
/*                           MODULE FUNCTIONS                                 */
/* ========================================================================== */

/**
 * \brief   Initializes McBSP IOM driver's data structures
 *
 *          This function initializes the McBSP IOM driver's data structures
 *          including instance objects and channel objects.This function needs
 *          to be called at part of BIOS initialization by setting initFxn for
 *          that particular UDEV instance or by calling this function as part of
 *          user specific initFxn.
 *
 * \return  None
 */
Void Mcbsp_init(Void)
{
    Int   devId = 0;

    /* intialise the loop job buffers and the mute buffers for all instances  */
#ifdef Mcbsp_LOOPJOB_ENABLE
    memset((Void *)Mcbsp_loopDstBuf,0x0,
        sizeof(Mcbsp_TempBuffer) * CSL_MCBSP_PER_CNT);
    memset((Void *)Mcbsp_loopSrcBuf,0x0,
        sizeof(Mcbsp_TempBuffer) * CSL_MCBSP_PER_CNT);
#endif /* Mcbsp_LOOPJOB_ENABLE */
    memset((Void *)Mcbsp_muteBuf,0x0,
        sizeof(Mcbsp_TempBuffer) * CSL_MCBSP_PER_CNT);

    /* initialise the information for all the device instances                */
    /* I can't pass device ID other than 0 from BIOS                          */
    for (devId = 0; devId < CSL_MCBSP_PER_CNT; devId++)
    {
        /* we have to initialize statically this information                  */
        Mcbsp_module.inUse[devId] = FALSE;

        /* initialise the instance object                                     */
        memset((Void *)&Mcbsp_Instances[devId], 0x00, sizeof(Mcbsp_Object));

        if (0 == devId)
        {
            /* instance 0 initialization                                    */
            Mcbsp_deviceInstInfo[devId].instNum = (Uint32)devId;
            Mcbsp_deviceInstInfo[devId].regs =
                (CSL_McbspRegsOvly)CSL_MCBSP_1_REGS;	/* McBSP 1 is used for now */
            Mcbsp_deviceInstInfo[devId].edmaTxEventNum =
                (Uint32)CSL_EDMA3_CHA_MCBSP1_TX;
            Mcbsp_deviceInstInfo[devId].edmaRxEventNum =
                (Uint32)CSL_EDMA3_CHA_MCBSP1_RX;
            Mcbsp_deviceInstInfo[devId].cpuTxEventNum =
                (Uint32)CSL_MCBSP1_IRQ_TX_IVA2_IRQ;
            Mcbsp_deviceInstInfo[devId].cpuRxEventNum =
                (Uint32)CSL_MCBSP1_IRQ_RX_IVA2_IRQ;
            Mcbsp_deviceInstInfo[devId].moduleClkFreq =
                (Uint32)CSL_MCBSP_1_MODULE_FREQ;
        }
        else if (1 == devId)
        {
            /* instance 2 initialization                                    */
            Mcbsp_deviceInstInfo[devId].instNum = (Uint32)devId;
            Mcbsp_deviceInstInfo[devId].regs =
                (CSL_McbspRegsOvly)CSL_MCBSP_2_REGS;	/* McBSP 2 is used for now */
            Mcbsp_deviceInstInfo[devId].edmaTxEventNum =
                (Uint32)CSL_EDMA3_CHA_MCBSP2_TX;
            Mcbsp_deviceInstInfo[devId].edmaRxEventNum =
                (Uint32)CSL_EDMA3_CHA_MCBSP2_RX;
            Mcbsp_deviceInstInfo[devId].cpuTxEventNum =
                (Uint32)CSL_MCBSP2_IRQ_TX_IVA2_IRQ;
            Mcbsp_deviceInstInfo[devId].cpuRxEventNum =
                (Uint32)CSL_MCBSP2_IRQ_RX_IVA2_IRQ;
            Mcbsp_deviceInstInfo[devId].moduleClkFreq =
                (Uint32)CSL_MCBSP_2_MODULE_FREQ;
        }
        else if (2 == devId)
        {
            /* instance 2 initialization                                    */
            Mcbsp_deviceInstInfo[devId].instNum = (Uint32)devId;
            Mcbsp_deviceInstInfo[devId].regs =
                (CSL_McbspRegsOvly)CSL_MCBSP_4_REGS;	/* McBSP 4 is used for now */
            Mcbsp_deviceInstInfo[devId].edmaTxEventNum =
                (Uint32)CSL_EDMA3_CHA_MCBSP4_TX;
            Mcbsp_deviceInstInfo[devId].edmaRxEventNum =
                (Uint32)CSL_EDMA3_CHA_MCBSP4_RX;
            Mcbsp_deviceInstInfo[devId].cpuTxEventNum =
                (Uint32)CSL_MCBSP4_IRQ_TX_IVA2_IRQ;
            Mcbsp_deviceInstInfo[devId].cpuRxEventNum =
                (Uint32)CSL_MCBSP4_IRQ_RX_IVA2_IRQ;
            Mcbsp_deviceInstInfo[devId].moduleClkFreq =
                (Uint32)CSL_MCBSP_4_MODULE_FREQ;
        }
        else
        {
            /* do nothing                                                     */
        }
#ifdef Mcbsp_LOOPJOB_ENABLE
        /* align the buffers to the cache line size                           */
        Mcbsp_loopSrcBuf[devId].scratchBuffer = (Uint32 *)
            (((Uint32)Mcbsp_loopSrcBuf[devId].scratchBuf + 0x7F) & ~0x7F);

        Mcbsp_loopDstBuf[devId].scratchBuffer = (Uint32 *)
            (((Uint32)Mcbsp_loopDstBuf[devId].scratchBuf + 0x7F) & ~0x7F);
#endif /* Mcbsp_LOOPJOB_ENABLE */
        Mcbsp_muteBuf[devId].scratchBuffer = (Uint32 *)
            (((Uint32)Mcbsp_muteBuf[devId].scratchBuf + 0x7F) & ~0x7F);
    }
}

/**
 * \brief   Allocates and configures the McBSP instance specified by devid.
 *
 *  Acquires the Handle of the McBSP and configure the McBSP by
 *  default for the following things.
 *      o   Data output for audio playback
 *      o   Data input for audio recording
 *                      (or)
 *      o   Configure for SPI transceive operation(only in the case of supported
 *          SOCs like DM6437).
 *      o   Configure the McBSP in DSP/TDM data format mode of the audio
 *          codec.
 *      o   Configure the McBSP to receive the Frame Sync and bit clock
 *          externally for both receiver and transmitter.
 *      o   McBSP can also be configured to generate Frame Sync and
 *          bit clock internally by enabling sample rate generator and
 *          frame sync generator blocks respectively depending on the
 *          fields set in the device parameter structure which shall
 *          be passed to mdBindDev() function as one of the parameter.
 *
 * \param   devp         [OUT]  pointer to hold allocated instance object ptr
 * \param   devid        [IN]   instance number of the mcbsp
 * \param   devParams    [IN]   user supplied data params.
 *
 * \return  IOM_COMPLETED     if successful
 *
 *          IOM_EBADARGS      if not successful
 *          IOM_EALLOC
 *
 * \enter   devp        should be non NULL and valid pointer,
 *          devId       should be < CSL_MCBSP_PER_CNT
 *          devParams   should be non NULL and valid pointer,
 *
 * \leave   Not implemented
 */
static Int mcbspMdBindDev(Ptr *devp, Int devid, Ptr devParams)
{
    Mcbsp_Object        *instHandle = NULL;
    Mcbsp_Params        *params     = NULL;
    Uint32               ctrlMask   = 0x00;
    Uint32               count      = 0x00;
#ifndef Mcbsp_LOOPJOB_ENABLE
    SWI_Attrs            swiParams  = {0};
#endif
    Int32                status     = IOM_COMPLETED;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
    if ((NULL == devp) || (NULL == devParams) || (devid >= CSL_MCBSP_PER_CNT))
    {
        status = IOM_EBADARGS;
    }

    if (IOM_COMPLETED == status)
    {
        if (TRUE == Mcbsp_module.inUse[devid])
        {
            status = IOM_EINUSE;
        }
        else
        {
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */
            /* copy the pointer to the instance object                        */
            instHandle = &Mcbsp_Instances[devid];

            /* set the module state as in use                                 */
            Mcbsp_module.inUse[devid] = TRUE;

            params = (Mcbsp_Params *)devParams;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
            /* Only DMA mode of the operation is supported for Mcbsp mode     */
            if ((Mcbsp_DevMode_McBSP == params->mode) &&
                (Mcbsp_OpMode_DMAINTERRUPT != params->opMode))
            {
                status = IOM_EBADMODE;
            }
            else
            {
            	if (NULL == params->srgSetup)
            	{
            		status = IOM_EBADMODE;
            	}
            }
        }
    }
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */

    if (IOM_COMPLETED == status)
    {
        /* update the instance of the device being created                    */
        instHandle->instNum = devid;

        /* update the user supplied params to the instance object             */
        instHandle->mode = params->mode;
        instHandle->opMode = params->opMode;
        instHandle->enablecache = params->enablecache;

        /* copy the SOC related information in to the instance object         */
        instHandle->hwInfo = Mcbsp_deviceInstInfo[devid];

        /* stop the state machine of RX and TX                                */
        instHandle->stopSmFsXmt = TRUE;
        instHandle->stopSmFsRcv = TRUE;

        instHandle->retryCount = Mcbsp_POLLED_RETRYCOUNT;

        /* configure the default values for the transmit channel              */
        instHandle->xmtObj.mode = (Uint16)IOM_OUTPUT;
        instHandle->xmtObj.devHandle = NULL;
        instHandle->xmtObj.cbFxn   = NULL;
        instHandle->xmtObj.cbArg   = NULL;
        instHandle->xmtObj.edmaHandle = NULL;
        instHandle->xmtObj.edmaEventQue = Mcbsp_TXEVENTQUE;
        instHandle->xmtObj.edmaCallback = NULL;
        instHandle->xmtObj.xferChan = 0x00;
        instHandle->xmtObj.tcc = 0x00;
        instHandle->xmtObj.tempPacket = NULL;
        instHandle->xmtObj.submitCount = 0x00;
        instHandle->xmtObj.dataFormat = Mcbsp_BufferFormat_1SLOT;
        instHandle->xmtObj.bMuteON = FALSE;
        instHandle->xmtObj.paused = FALSE;
        instHandle->xmtObj.flush = FALSE;
        instHandle->xmtObj.isTempPacketValid = FALSE;
        instHandle->xmtObj.enableHwFifo = FALSE;
        instHandle->xmtObj.gblErrCbk = NULL;
        instHandle->xmtObj.userDataBufferSize = 0x00;
        instHandle->xmtObj.loopJobBuffer = NULL;
        instHandle->xmtObj.loopJobLength = 0x00;
        instHandle->xmtObj.nextLinkParamSetToBeUpdated = 0x00;
        instHandle->xmtObj.loopjobUpdatedinParamset = FALSE;
        instHandle->xmtObj.roundedWordWidth = 0x00;
        instHandle->rcvObj.mode = (Uint16)IOM_INPUT;
        instHandle->rcvObj.devHandle = NULL;
        instHandle->rcvObj.cbFxn   = NULL;
        instHandle->rcvObj.cbArg   = NULL;
        instHandle->rcvObj.edmaHandle = NULL;
        instHandle->rcvObj.edmaEventQue = Mcbsp_RXEVENTQUE;
        instHandle->rcvObj.edmaCallback = NULL;
        instHandle->rcvObj.xferChan = 0x00;
        instHandle->rcvObj.tcc = 0x00;
        instHandle->rcvObj.tempPacket = NULL;
        instHandle->rcvObj.submitCount = 0x00;
        instHandle->rcvObj.dataFormat = Mcbsp_BufferFormat_1SLOT;
        instHandle->rcvObj.bMuteON = FALSE;
        instHandle->rcvObj.paused = FALSE;
        instHandle->rcvObj.flush = FALSE;
        instHandle->rcvObj.isTempPacketValid = FALSE;
        instHandle->rcvObj.enableHwFifo = FALSE;
        instHandle->rcvObj.gblErrCbk = NULL;
        instHandle->rcvObj.userDataBufferSize = 0x00;
        instHandle->rcvObj.loopJobBuffer = NULL;
        instHandle->rcvObj.loopJobLength = 0x00;
        instHandle->rcvObj.nextLinkParamSetToBeUpdated = 0x00;
        instHandle->rcvObj.loopjobUpdatedinParamset = FALSE;
        instHandle->rcvObj.roundedWordWidth = 0x00;

#ifdef Mcbsp_LOOPJOB_ENABLE
        /* driver is compiled in loop Job mode                                */
        instHandle->loopJobMode = TRUE;

#if 0
        if (TRUE ==  instHandle->pscPwrmEnable)
        {
            /* power management not supported for loop job mode               */
            status = IOM_EBADMODE;
        }
#endif
#else
        instHandle->loopJobMode = FALSE;
#endif

        if ((IOM_COMPLETED == status) &&
            ((FALSE == instHandle->prcmPwrmEnable) ||
             (TRUE == instHandle->loopJobMode)))
        {
#ifdef BIOS_PWRM_ENABLE
            /* power on using bios PWRM API                                   */
            status = (Int32)PWRM_setDependency(
                                (PWRM_Resource)instHandle->hwInfo.pwrmLpscId);
#if 0
            /* power on using PRCM API                                        */
            /* This is hard coded for McBSP1                                  */
            status = Prcm_ModuleClkCtrl(
                         instHandle->hwInfo.prcmInstance,
                         (Uint32)instHandle->hwInfo.pwrmPrcmId,
                         TRUE);
#endif
#else
            if (0 == instHandle->instNum)
            {
                Prcm_ModuleClkCtrl(CM_FCLKEN1_CORE, EN_MCBSP1_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
                Prcm_ModuleClkCtrl(CM_ICLKEN1_CORE, EN_MCBSP1_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            }

            if (1 == instHandle->instNum)
            {
                Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP2_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
                Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP2_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            }

            if (2 == instHandle->instNum)
            {
                Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP4_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
                Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP4_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            }

            instHandle->prcmPwrmEnable = TRUE;
#endif
        }

        for (count = 0; count < Mcbsp_MAXLINKCNT; count++)
        {
            instHandle->xmtObj.pramTbl[count] = 0x00;
            instHandle->rcvObj.pramTbl[count] = 0x00;
            instHandle->xmtObj.pramTblAddr[count] = 0x00;
            instHandle->rcvObj.pramTblAddr[count] = 0x00;
        }

        /* set the status of the channel to closed                            */
        instHandle->xmtObj.chanState = Mcbsp_DriverState_CLOSED;
        instHandle->rcvObj.chanState = Mcbsp_DriverState_CLOSED;

        /* create the pending and floating queue for the TX channel           */
        QUE_new(&(instHandle->xmtObj.queuePendingList));
        QUE_new(&(instHandle->xmtObj.queueFloatingList));

        /* create the pending and floating queue for the RX channel           */
        QUE_new(&(instHandle->rcvObj.queuePendingList));
        QUE_new(&(instHandle->rcvObj.queueFloatingList));

        /* power on the module                                                */
/** Kayjin        status = Mcbsp_localLpscOn(instHandle);					***/

        if (IOM_COMPLETED == status)
        {
            /* Reset the McBSP Transmitter, receiver and SRGR before          *
             * configuration                                                  */
            ctrlMask = ((Mcbsp_SpcrCtrl_TX_DISABLE |
                         Mcbsp_SpcrCtrl_RX_DISABLE) |
                       (Mcbsp_SpcrCtrl_SRG_DISABLE |
                        Mcbsp_SpcrCtrl_FSYNC_DISABLE));

            Mcbsp_localResetCtrl(instHandle,ctrlMask);

            /* copy the configuration for the sample rate generator and config*
             * the emulation mode and DLB mode settings                       */

            /* Configure the McBSP with user supplied parameters              */
            instHandle->srgrConfig = *(params->srgSetup);

            instHandle->clkStpMode = params->clkStpMode;

            /* reset the Mcbsp                                                */
            instHandle->hwInfo.regs->SPCR1 = 0x00u;
            instHandle->hwInfo.regs->SPCR2 = 0x00u;

            /* set the DLB mode settings                                      */
            instHandle->hwInfo.regs->XCCR &= (~CSL_MCBSP_XCCR_DLB_MASK);
            instHandle->hwInfo.regs->XCCR |=
                (params->dlbMode << CSL_MCBSP_XCCR_DLB_SHIFT);

            /* disable the transmitter                                        */
            instHandle->hwInfo.regs->XCCR |= (CSL_MCBSP_XCCR_XDISABLE_MASK);

            /* disable the receiver                                           */
            instHandle->hwInfo.regs->RCCR |= (CSL_MCBSP_RCCR_RDISABLE_MASK);

            /* set the emulation state                                        */
            instHandle->hwInfo.regs->SPCR2 &= (~(CSL_MCBSP_SPCR2_SOFT_MASK |
                                                CSL_MCBSP_SPCR2_FREE_MASK));
            instHandle->hwInfo.regs->SPCR2 |=
                (params->emulationMode << CSL_MCBSP_SPCR2_SOFT_SHIFT);

/** Kayjin            status = Mcbsp_localLpscOff(instHandle);				***/
        }

#ifndef Mcbsp_LOOPJOB_ENABLE
        /* create the SWI used to handle the FIFO emptying during the non     *
         * loop job mode                                                      */
        if (IOM_COMPLETED == status)
        {
            /* create the swi to handle the TX FIFO empty case handling       */
            swiParams.priority = Mcbsp_SWI_PRIORITY;
            swiParams.fxn = (SWI_Fxn)&Mcbsp_swiTxFifo;
            swiParams.arg0 = (Arg)&instHandle->xmtObj;
            swiParams.arg1 = (Arg)instHandle;

            SWI_new(&instHandle->swiObject,&swiParams);
        }
#endif

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
        if ((IOM_COMPLETED == status) && (TRUE == instHandle->pscPwrmEnable))
        {
            /* register the notify function for the PWRM events               */
            status = McbspRegisterNotification(instHandle);
        }

        if (IOM_COMPLETED != status)
        {
            *devp = NULL;
        }
        else
#endif
        {
            *devp = instHandle;

            /* set the status of the driver to created                        */
            instHandle->devState = Mcbsp_DriverState_CREATED;
        }
    }
    return status;
}

/**
 *  \brief  Creates a communication channel in specified mode to communicate
 *          data between the application and the McBSP device instance. This
 *          function sets the required hardware configurations for the data
 *          transactions.it returns configured channel handle to application.
 *          which will be used in all further transactions with the channel.
 *
 *          This function is called in response to a GIO_create call.
 *
 *          Pre-requisites:
 *          1.  Valid chanParams structure
 *              This takes much information pertaining to mcbsp channel
 *              configuration such as how many slots are used for this
 *              channel what are they communication parameters,clock setting etc
 *          2.  Valid device pointer
 *
 * \note    Please refer to Appendix A : IOM interface of spru616
 *          DSP/BIOS Driver Developer's Guide for details on
 *          mcbspMdCreateChan parameters.
 *
 * \param   chanp        [IN]     Channel Handler
 * \param   devp         [IN]     Device pointer
 * \param   name         [IN]     Mcbsp Instance name like MCBSP0
 * \param   mode         [IN]     channel  mode -> input or output
 * \param   chanParams   [IN]     channel parameters from user
 * \param   cbFxn        [IN]     callback function pointer
 * \param   cbArg        [IN]     callback function Arguments
 *
 * \return  IOM_COMPLETED     if successful
 *          IOM_EBADIO        if not successful
 *          IOM_EALLOC            "
 *          IOM_EBADARGS      if passed invalid chanParams structure
 */
static Int mcbspMdCreateChan(Ptr             *chanp,
                             Ptr              devp,
                             String           name,
                             Int              mode,
                             Ptr              chanParams,
                             IOM_TiomCallback cbFxn,
                             Ptr              cbArg)
{
    Mcbsp_Object        *instHandle = NULL;
    Mcbsp_ChannelObj    *chanHandle = NULL;
    Mcbsp_ChanParams    *chanparam  = NULL;
    Uint32               hwiKey     = 0;

    //Int32                retVal     = IOM_COMPLETED;
    Int32                status     = IOM_COMPLETED;

    /* Kayjin to add WUGEN											**/
    volatile Uint32* pWUGENReg0 = (volatile Uint32*) IVA_WUGEN_MEVTCLR0;
    volatile Uint32* pWUGENReg2 = (volatile Uint32*) IVA_WUGEN_MEVTCLR2;

    // Unused variable
    (void)name;

/* Begin parameter checking                                                   */
#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
    if (((NULL == chanParams)
        || (NULL == cbFxn)
        || (NULL == cbArg)
        || (NULL == devp))
        || ((IOM_INPUT != mode) && (IOM_OUTPUT != mode)))
    {
        status = IOM_EBADARGS;
    }
    else
    {
#endif
        chanparam = (Mcbsp_ChanParams *)chanParams;
#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
        if (NULL == chanparam->edmaHandle)
        {
            status = IOM_EBADARGS;
        }
    }
#endif  /* PSP_DISABLE_INPUT_PARAMETER_CHECK */
/* End parameter checking                                                     */

    if (IOM_COMPLETED == status)
    {
        instHandle = (Mcbsp_Object *)devp;

        if (IOM_COMPLETED == status)
        {
            /* get the pointer to the required channel structure              */
            if (IOM_INPUT == mode)
            {
                chanHandle = &instHandle->rcvObj;
            }
            else
            {
                chanHandle = &instHandle->xmtObj;
            }

            /* critical section starts                                        */
            hwiKey = (Uint32)_disable_interrupts();

            /* we will check if the current requested channel is availablehere*
             * protect the channel status so that multiple threads requesting *
             * same channel do not cause a corruption                         */
            if (Mcbsp_DriverState_CLOSED == chanHandle->chanState)
            {
                chanHandle->chanState = Mcbsp_DriverState_OPENED;
            }
            else
            {
                /* Requested channel is already taken hence we will set status*
                 * as invalid                                                 */
                status = IOM_EBADMODE;
            }
            /* critical section ends                                          */
            _restore_interrupts(hwiKey);
        }

        if (IOM_COMPLETED == status)
        {
            chanHandle->mode = (Uint16)mode;
            chanHandle->devHandle = instHandle;


            /* Assign the respective callback function                        */
            chanHandle->cbFxn = cbFxn;
            chanHandle->cbArg = (Arg)cbArg;

            /* This is used in EDMA mode to populate paramsets in PINGPONG    */
            chanHandle->nextLinkParamSetToBeUpdated = 0;

            /* Initialize Mute parameter                                      */
            chanHandle->bMuteON = FALSE;

            chanHandle->isTempPacketValid = FALSE;

            /* Counter that counts outstanding requests of this channel       */
            chanHandle->submitCount = 0;

            /* Global error callback registered to channel                    */
            chanHandle->gblErrCbk = chanparam->gblCbk;

            /* copy the edma event queue details                              */
            chanHandle->edmaEventQue = chanparam->edmaEventQue;

            /* store the EDMA3 module handle                                  */
            chanHandle->edmaHandle = chanparam->edmaHandle;

            /* configure the FIFO                                             */
            chanHandle->enableHwFifo = chanparam->enableHwFifo;

            /* copy the user settings in to the channel object                */
            chanHandle->chanConfig = *(chanparam->chanConfig);
            chanHandle->clkSetup   = *(chanparam->clkSetup);
            chanHandle->multiChanCtrl = *(chanparam->multiChanCtrl);

        }

        if (IOM_COMPLETED == status)
        {
            /* configure the actual wordwidth to be used                      */
            switch (chanparam->wordWidth)
            {
                case Mcbsp_WordLength_8:
                    chanHandle->roundedWordWidth = 1u;
                    break;
                case Mcbsp_WordLength_12:
                case Mcbsp_WordLength_16:
                    chanHandle->roundedWordWidth = 2u;
                    break;
                case Mcbsp_WordLength_20:
                case Mcbsp_WordLength_24:
                    chanHandle->roundedWordWidth = 3u;
                    break;
                case Mcbsp_WordLength_32:
                default:
                    chanHandle->roundedWordWidth = 4u;
                    break;
            }

#ifdef  Mcbsp_LOOPJOB_ENABLE
            /* Configure the loop job for the user specified buffer if given  */
            if (NULL == chanparam->userLoopJobBuffer)
            {
                if (IOM_INPUT == chanHandle->mode)
                {
                    chanHandle->loopJobBuffer =
                        Mcbsp_loopDstBuf[instHandle->instNum].scratchBuffer;
                }
                else
                {
                    chanHandle->loopJobBuffer =
                        Mcbsp_loopSrcBuf[instHandle->instNum].scratchBuffer;
                }
                chanHandle->loopJobLength = chanHandle->roundedWordWidth;
            }
            else
            {
                /* Apps has preference on the loopjob buffer & lets use it    */
                chanHandle->loopJobBuffer = chanparam->userLoopJobBuffer;
                chanHandle->userLoopJobLength = chanparam->userLoopJobLength;

                /* user loopJob is being used                                 */
                chanHandle->userLoopJob = TRUE;
                if (chanHandle->roundedWordWidth >
                        chanparam->userLoopJobLength)
                {
                    /* not enough loopjob buffer has been provided  we        *
                     * should have aleast loopbuffer for 1 sync event         */
                    status = IOM_EBADARGS;
                }
            }
#endif
        }

        if (Mcbsp_DevMode_McBSP == instHandle->mode)
        {
            if (IOM_INPUT == chanHandle->mode)
            {
                /* Assign the Channel ID and TCC                              */
                chanHandle->xferChan     = instHandle->hwInfo.edmaRxEventNum;
                chanHandle->tcc          = instHandle->hwInfo.edmaRxEventNum;
                chanHandle->edmaCallback =
                    (EDMA3_RM_TccCallback)&Mcbsp_localEdmaCallback;

                mcbspRegisterInt(
                    chanHandle,
                    chanparam->hwiNumber,
                    (Uint16)instHandle->hwInfo.cpuRxEventNum,
                    (ECM_Fxn)&mcbspGblRcvIsr);

                *pWUGENReg0 = 1<<(instHandle->hwInfo.cpuRxEventNum);
                *pWUGENReg2 = 1<<(instHandle->hwInfo.edmaRxEventNum);
            }
            else
            {
                /* Assign the Channel ID and TCC                              */
                chanHandle->xferChan     = instHandle->hwInfo.edmaTxEventNum;
                chanHandle->tcc          = instHandle->hwInfo.edmaTxEventNum;
                chanHandle->edmaCallback =
                    (EDMA3_RM_TccCallback)&Mcbsp_localEdmaCallback;

                mcbspRegisterInt(
                    chanHandle,
                    chanparam->hwiNumber,
                    (Uint16)instHandle->hwInfo.cpuTxEventNum,
                    (ECM_Fxn)&mcbspGblXmtIsr);

                *pWUGENReg0 = 1<<(instHandle->hwInfo.cpuTxEventNum);
                *pWUGENReg2 = 1<<(instHandle->hwInfo.edmaTxEventNum);
            }

            if (IOM_COMPLETED == status)
            {
                mcbspConfigureFifo(&(instHandle->hwInfo),
                    chanHandle,
                    chanHandle->enableHwFifo);
            }

            if (IOM_INPUT == chanHandle->mode)
            {
                /* configure the RX section                                   */
                status = Mcbsp_localConfigureRcvChannel(instHandle,chanparam);
            }
            else
            {
                /* configure the TX section                                   */
                status = Mcbsp_localConfigureXmtChannel(instHandle,chanparam);
            }

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
            if ((IOM_COMPLETED == status) &&
                (TRUE == instHandle->pscPwrmEnable) &&
                (FALSE == instHandle->pwrmInfo.constraintRegistered) &&
                (Mcbsp_SrgClk_CLKCPU == instHandle->srgrConfig.srgInputClkMode))
            {
                /* register constraints only in internal clock is used        */
                status = McbspCalculateConstraints(instHandle,chanHandle);

                if (IOM_COMPLETED == status)
                {
                    /* set the variable to allow contraints to be registered  *
                     * only once                                              */
                    instHandle->pwrmInfo.constraintRegistered = TRUE;
                }
            }
#endif
            if (IOM_COMPLETED == status)
            {
                /* configure the sample rate generator                        */
                status = Mcbsp_localConfigureSrgr(instHandle,chanHandle);
            }

            if (IOM_COMPLETED == status)
            {
                /* enable the internal sample rate generator if required      */
                if (((IOM_INPUT == chanHandle->mode) &&
                    (TRUE == instHandle->rxSrgEnable)) ||
                    ((IOM_OUTPUT == chanHandle->mode) &&
                    (TRUE == instHandle->txSrgEnable)))
                {
                    /* enable the sample rate generator                       */
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_SRG_ENABLE);
                    instHandle->srgEnabled = TRUE;

                    /* wait for the 2CLKG clock cycles.But the minumum time   *
                     * provided by the bios is 1ms.                           */
                    TSK_sleep(1u);
                }

                /* clear the XSYNCERR (to be done only if TX is used)         */
                if (IOM_OUTPUT == chanHandle->mode)
                {
                    /* Enable the TX section                                  */
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_ENABLE);

                    /* wait for 2 CLKR or CLX cycles.But the minumum time     *
                     * provided by the bios is 1ms.                           */
                    TSK_sleep(1u);

                    /* Disable the TX section to clear any XYNCERR            */
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_DISABLE);
                }

                /* complete the EDMA setup for the driver                     */
                status =  Mcbsp_localSetupEdmaDuringOpen(chanHandle);
            }

#ifdef Mcbsp_LOOPJOB_ENABLE
            if (IOM_COMPLETED == status)
            {

                /* configure the FIFO for the specific channel                        */
                if (TRUE == chanHandle->enableHwFifo)
                {
                    /* Disable and enable the FIFO so that the events are             *
                     * generated to the Mcbsp for the first time                      */
                    mcbspConfigureFifo(&instHandle->hwInfo,chanHandle,FALSE);
                    mcbspConfigureFifo(&instHandle->hwInfo,chanHandle,TRUE);
                }

                /* Enable the McBSP hardware                                   */
                if (IOM_INPUT == chanHandle->mode)
                {
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_RX_ENABLE);
                }
                else
                {
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_ENABLE);
                }

                /* enable the EDMA transfer for the channel so that it is     *
                 * ready to transfer the data as soon as the state machine is *
                 * enabled                                                    */
                EDMA3_DRV_enableTransfer(
                    chanHandle->edmaHandle,
                    chanHandle->xferChan,
                    EDMA3_DRV_TRIG_MODE_EVENT);

               /* Start the McBSP hardware                                   */
                if (IOM_INPUT == chanHandle->mode)
                {
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_RX_START);
                }
                else
                {
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_START);
                }

                if (IOM_INPUT == chanHandle->mode)
                {
                    if (TRUE == instHandle->rxFsgEnable)
                    {
                        /* enable the sample rate generator                   */
                        Mcbsp_localResetCtrl(
                            instHandle,
                            Mcbsp_SpcrCtrl_FSYNC_ENABLE);
                    }
                }
                else
                {
                    if (TRUE == instHandle->txFsgEnable)
                    {
                        /* enable the sample rate generator                   */
                        Mcbsp_localResetCtrl(
                            instHandle,
                            Mcbsp_SpcrCtrl_FSYNC_ENABLE);
                    }
                }

                /* State machine stop status                                  */
                if (IOM_INPUT == chanHandle->mode)
                {
                    instHandle->stopSmFsRcv = FALSE;
                }
                else
                {
                    instHandle->stopSmFsXmt = FALSE;
                }
            }
#endif
        }

    }

    if (IOM_COMPLETED != status )
    {
        /* enter critical section                                             */
        hwiKey = (Uint32)_disable_interrupts();

        if (NULL != chanHandle)
        {
            /* set the status of the channel to closed                        */
            chanHandle->mode = (Uint16)Mcbsp_DriverState_CLOSED;
        }

        /* Leave critical section                                             */
        _restore_interrupts(hwiKey);

        /* channel opening for transaction has failed                         */
        chanHandle = NULL;
    }
    else
    {
        *chanp = chanHandle;
    }

    return status;
}

/**
 * \brief   frees a channel and all it's associated resources.
 *
 *          Frees the EDMA resources including EDMA master channel and link
 *          PaRAM entries held by the channel.it also unregisters all the
 *          interrupt handlers.
 *
 * \param   chanp  [IN]       Handle to the channel
 *
 * \return  IOM_COMPLETED     if successful
 *          IOM_ABORTED       if not successful
 *
 * \enter   chanp       should be non NULL and valid pointer,
 *
 * \leave   Not implemented
 */
static Int mcbspMdDeleteChan(Ptr chanp)
{
    Mcbsp_Object      *instHandle = NULL;
    Mcbsp_ChannelObj  *chanHandle = NULL;
    Uint8              linkCnt    = 0;
    Uint32             timeOut    = 0x00;
    Bool               pscPwrOn   = FALSE;
    Int32              status     = IOM_COMPLETED;
    Int32              retVal     = IOM_COMPLETED;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
    if (NULL == chanp)
    {
        status = IOM_EBADARGS;
    }
    else
    {
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */
        chanHandle = (Mcbsp_ChannelObj *)chanp;
#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
        if (Mcbsp_DriverState_OPENED != chanHandle->chanState)
        {
            status = IOM_EBADMODE;
        }
        else
        {
            if (NULL == chanHandle->devHandle)
            {
                status = IOM_EBADARGS;
            }
            else
            {
#endif
                instHandle = (Mcbsp_Object *) chanHandle->devHandle;
#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
                if (NULL == instHandle)
                {
                    status = IOM_EBADARGS;
                }
            }
        }
    }
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */

    if (IOM_COMPLETED == status)
    {
        /* power on the module                                                */
        status = Mcbsp_localLpscOn(instHandle);

        if (IOM_COMPLETED == status)
        {
            pscPwrOn = TRUE;

            /* reset the channel              .                               */
            Mcbsp_localAbortReset(chanHandle);

            /* Disable the current transfer to make sure that there is no     *
             * running EDMA transfer taking place                             */
            status = EDMA3_DRV_disableTransfer(
                         chanHandle->edmaHandle,
                         chanHandle->xferChan,
                         EDMA3_DRV_TRIG_MODE_EVENT);

            /* Disable current EDMA transfer                                  */
            if (IOM_COMPLETED == status)
            {
                if (IOM_OUTPUT == chanHandle->mode)
                {
                    timeOut = instHandle->retryCount;

                    /* Poll for TXEMPTY                                       */
                    while ((CSL_MCBSP_SPCR2_XEMPTY_MASK ==
                        (CSL_MCBSP_SPCR2_XEMPTY_MASK & instHandle->hwInfo.regs->SPCR2))
                        && (0 != timeOut))
                    {
                        timeOut--;
                    }
                }

                /* Reset McBSP before freeing the edma channels               */
                if (IOM_INPUT == chanHandle->mode)
                {
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_RX_DISABLE);
                }
                else
                {
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_DISABLE);
                }

                /* Free Edma channels                                         */
                status = EDMA3_DRV_freeChannel(
                             chanHandle->edmaHandle,
                             chanHandle->xferChan);

                if (IOM_COMPLETED ==  status)
                {
                    /* free the EDMA PaRAM entries used for Ping pong buffer  */
                    for (linkCnt = 0; linkCnt < Mcbsp_MAXLINKCNT; linkCnt++)
                    {
                        status = EDMA3_DRV_freeChannel(
                                     chanHandle->edmaHandle,
                                     chanHandle->pramTbl[linkCnt]);
                        if (IOM_COMPLETED != status)
                        {
                            break;
                        }
                    }
                }

                if (IOM_COMPLETED == status)
                {
                    /* Set the state of the channel as closed                 */
                    chanHandle->chanState = Mcbsp_DriverState_CLOSED;

                    /* Set the Muted state to FALSE                           */
                    chanHandle->bMuteON   = FALSE;

                    /* Set the Paused state to FALSE                          */
                    chanHandle->paused    = FALSE;
                }

                /* Unregister the interrupts registered for the err/data ready*/
                if (IOM_INPUT == chanHandle->mode)
                {
                    msbspUnRegisterInt((Uint16)instHandle->hwInfo.cpuRxEventNum);
                }
                else
                {
                    msbspUnRegisterInt((Uint16)instHandle->hwInfo.cpuTxEventNum);
                }
            }
        }

        if (TRUE == pscPwrOn)
        {
            retVal = Mcbsp_localLpscOff(instHandle);
        }
    }

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
    if ((TRUE == instHandle->pscPwrmEnable) &&
        (NULL != instHandle) &&
        (Mcbsp_DriverState_CLOSED == instHandle->xmtObj.chanState) &&
        (Mcbsp_DriverState_CLOSED == instHandle->rcvObj.chanState))
    {
        status = McbspUnregisterConstraints(instHandle);
        instHandle->pwrmInfo.constraintRegistered = FALSE;
    }
    else
    {
        status = IOM_EBADMODE;
    }
#endif

    if (IOM_COMPLETED == status)
    {
        status = retVal;
    }
    return (status);
}

/**
 * \brief   Closes the McBSP device instance from use.
 *          This API is called by the application when it no longer requires the
 *          device instance. Note that all the channels have to be closed for
 *          the device instance to be deleted sucessfully.
 *
 * \param   devp    [IN]  Handle to the device created.
 *
 * \return  IOM_COMPLETED     if successful
 *          IOM_ABORTED       if not successful
 *
 * \enter   devp       should be non NULL and valid pointer,
 *
 * \leave   Not implemented
 */
static Int mcbspMdUnBindDev(Ptr devp)
{
    Mcbsp_Object    *instHandle = NULL;
    Int32            status     = IOM_COMPLETED;
    Uint32           hwiKey     = 0;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
    if (NULL == devp)
    {
        status = IOM_EBADARGS;
    }
    else
    {
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */

        instHandle = (Mcbsp_Object *)devp;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
        if ((CSL_MCBSP_PER_CNT <= instHandle->instNum)                  ||
            (Mcbsp_DriverState_CLOSED != instHandle->xmtObj.chanState)  ||
            (Mcbsp_DriverState_CLOSED != instHandle->rcvObj.chanState))
        {
            status = IOM_EBADARGS;
        }
    }
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */

    if (IOM_COMPLETED == status)
    {
        /* critical section starts                                            */
        hwiKey = (Uint32)_disable_interrupts();

        status = Mcbsp_localLpscOn(instHandle);

        if (IOM_COMPLETED == status)
        {
            /* reset the Mcbsp                                                */
            instHandle->hwInfo.regs->SPCR1 = 0x00u;
            instHandle->hwInfo.regs->SPCR2 = 0x00u;

            /* power off the module                                           */
            status = Mcbsp_localLpscOff(instHandle);
        }

        /* Mark driver state as deleted and module as not in use              */
        instHandle->devState = Mcbsp_DriverState_DELETED;
        Mcbsp_module.inUse[instHandle->instNum] = FALSE;

        /* update the user params to the instance object                      */
        instHandle->instNum = -1;

        /* critical section ends                                              */
        _restore_interrupts(hwiKey);

        if ((FALSE == instHandle->prcmPwrmEnable) ||
             (TRUE == instHandle->loopJobMode))
        {
#ifdef BIOS_PWRM_ENABLE
            /* power on using bios PWRM API                                   */
            status = (Int32)PWRM_releaseDependency(
                                (PWRM_Resource)instHandle->hwInfo.pwrmLpscId);
#if 0
            /* power on using PRCM API                                         	*/
            /* This is hard coded for McBSP1									*/
            status = Prcm_ModuleClkCtrl(
                         instHandle->hwInfo.prcmInstance,
                         (Uint32)instHandle->hwInfo.pwrmPrcmId,
                         TRUE);
#endif
#else
            /* We are going to hard-code for mcbsp1								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN1_CORE, EN_MCBSP1_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            Prcm_ModuleClkCtrl(CM_ICLKEN1_CORE, EN_MCBSP1_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);

            /* We are going to hard-code for mcbsp2								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP2_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP2_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);

            /* We are going to hard-code for mcbsp4								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP4_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP4_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);
#endif
        }

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
        if ((IOM_COMPLETED == status) && (TRUE == instHandle->pscPwrmEnable))
        {
            /* register the notify function for the PWRM events               */
            status = McbspUnregisterNotification(instHandle);
        }
#endif
        /* Intialize the mcbsp driver to default values                       */
        memset(instHandle,0x00,sizeof(Mcbsp_Object));
    }
    return status;
}

/**
 * \brief   Submit a I/O packet to a channel for processing
 *
 *  The GIO layer calls this function to cause the mini-driver
 *  to process the IOM_Packet for read/write/flush/abort operations.
 *
 * \param   chanp         [IN]    Pointer to channel
 * \param   ioPacket      [IN]    Pointer to packet to be submitted
 *
 * \return  IOM_COMPLETED, if packet is fully processed
 *          IOM_ABORTED,   if packet is aborted
 *          IOM_PENDING,   if packet is not fully processed
 *          IOM_EBADIO     in case of an error in processing
 *
 * \enter   chanp       should be non NULL and valid pointer,
 *          ioPacket    should be non NULL and valid pointer
 *
 * \leave   Not implemented
 */
static Int mcbspMdSubmitChan(Ptr chanp, IOM_Packet *const ioPacket)
{
    Mcbsp_Object          *instHandle = NULL;
    Mcbsp_ChannelObj      *chanHandle = NULL;
    Bool                   pscPwrOn   = FALSE;
    Int32                  status     = IOM_COMPLETED;

    static Bool            primeTx    = FALSE;
    Uns                    ticks;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
    if ((NULL == chanp) || ( NULL == ioPacket))
    {
        status = IOM_EBADARGS;
    }
    else
    {
#endif
        chanHandle = (Mcbsp_ChannelObj *)chanp;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK

        if ((NULL == chanHandle) || (NULL == chanHandle->devHandle))
        {
            status = IOM_EBADARGS;
        }
        else
        {
#endif
            instHandle = (Mcbsp_Object *)chanHandle->devHandle;
            if (primeTx == TRUE) {
                /* disable the receiver                                               */
                //instHandle->hwInfo.regs->SPCR1 &= (~CSL_MCBSP_SPCR1_RRST_MASK );
                instHandle->hwInfo.regs->RCCR |= (~CSL_MCBSP_RCCR_RDISABLE_MASK);

                /* enable the transmitter                                             */
                //instHandle->hwInfo.regs->SPCR2 |= CSL_MCBSP_SPCR2_XRST_MASK;
                ticks = CLK_countspms()/CLK_getprd();
                TSK_sleep(ticks);
                instHandle->hwInfo.regs->XCCR &= (~CSL_MCBSP_XCCR_XDISABLE_MASK);

                primeTx = FALSE;
            }

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
            if (NULL == instHandle)
            {
                status = IOM_EBADARGS;
            }
            else
            {
                if (NULL == ioPacket->addr)
                {
                    if ((IOM_READ == ioPacket->cmd) ||
                        (IOM_WRITE == ioPacket->cmd))
                    {
                        status = IOM_EBADARGS;
                    }
                }
            }
        }
    }
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */

    if (IOM_COMPLETED == status)
    {
        /* process the command sent by the application                        */
        switch (ioPacket->cmd)
        {
            case IOM_READ:
            case IOM_WRITE:
                if (TRUE != chanHandle->flush)
                {
                    status = Mcbsp_localLpscOn(instHandle);

                    if (IOM_COMPLETED == status)
                    {
                        pscPwrOn = TRUE;
                        status = mcbspSubmitReq(instHandle,chanHandle,ioPacket);
                    }
                }
                else
                {
                    status = IOM_EBADIO;
                }
                break;
            case IOM_FLUSH:
                /* Flush command has been issued need to abort the receive    *
                 * channel packets and complete the TX packets normally       */
                chanHandle->flush = TRUE;
                Mcbsp_localAbortReset(chanHandle);
                chanHandle->flush = FALSE;
                break;
            case IOM_ABORT:
                Mcbsp_localAbortReset(chanHandle);
                break;
            default:
                status = IOM_EBADIO;
                break;
        }
    }

    if ((TRUE == pscPwrOn) && (IOM_PENDING != status))
    {
        /* reduce the dependency count for the module                         */
        Mcbsp_localLpscOff(instHandle);
    }

    return status;
}

/**
 * \brief   Implements the IOCTLS for McBSP IOM driver.
 *
 *          mdControlChan() implements recieves IOCTL commands from the
 *          GIO layer and executes them accordingly.
 *
 * \param   chanp  [IN]    Pointer to channel
 * \param   cmd    [IN]    specific IOCTL command
 * \param   arg    [IN]    arguments required for specific commands
 *
 * \return  IOM_COMPLETED, if command is executed correctly
 *          IOM_ABORTED,   if command returns error during execution
 *          IOM_ENOTIMPL,  if command is not supported
 *          IOM_EBADARGS   if args are not correct
 */
static Int mcbspMdControlChan(Ptr chanp, Uns cmd, Ptr arg)
{
    Mcbsp_ChannelObj    *chanHandle = NULL;
    Mcbsp_Object        *instHandle = NULL;
    Int32                status     = IOM_COMPLETED;
    Int32                retVal     = IOM_COMPLETED;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
    if (NULL == chanp)
    {
        status = IOM_EBADARGS;
    }
    else
    {
#endif /* PSP_DISABLE_INPUT_PARAMETER_CHECK */

        chanHandle = (Mcbsp_ChannelObj *)chanp;

#ifndef PSP_DISABLE_INPUT_PARAMETER_CHECK
        if (NULL == chanHandle->devHandle)
        {
            status = IOM_EBADARGS;
        }
    }
#endif

    if (IOM_COMPLETED == status)
    {
        instHandle = (Mcbsp_Object *)chanHandle->devHandle;
        status = Mcbsp_localLpscOn(instHandle);

        if (IOM_COMPLETED == status)
        {
            /* call the function to process the IOCTL command                 */
            status =  Mcbsp_localSubmitIoctl(
                            chanHandle,
                            (Mcbsp_IOCTL)cmd,
                            arg,
                            NULL);

            /* reduce the dependency count for the module                     */
            retVal = Mcbsp_localLpscOff(instHandle);

            if (IOM_COMPLETED == status)
            {
                status = retVal;
            }
        }
    }
    return (status);
}


/*============================================================================*/
/*                          LOCAL FUNCTION DEFINTIONS                         */
/*============================================================================*/

/**
 *  \brief   Submit a I/O packet to a channel for processing
 *
 *   This function is called with the IOM_READ or IOM_WRITE command to process
 *   the IOP. the function handles in the MCBSP mode.In case
 *   that the driver is currently idle it sets up packet for transfer. in case
 *   that the driver is currently processing a packet then we will Queue up the
 *   current packet and exit.
 *
 * \param   instHandle  [IN]  pointer to the instance object
 * \param   chanHandle  [IN] handle to the channel
 * \param   ioPacket    [IN] pointer to IOM Packet.
 *
 * \return  IOM_PENDING in case the packet is sucesfully processed
 *          IOM_EBADIO  in case of any error.
 */
static Int32 mcbspSubmitReq(Mcbsp_Object     *instHandle,
                            Mcbsp_ChannelObj *chanHandle,
                            IOM_Packet       *ioPacket)
{
    Uint32        hwiKey     = 0;
    Int32         status     = IOM_COMPLETED;

    assert((NULL != chanHandle) && (NULL != ioPacket) && (NULL != instHandle));
    assert((0 != ioPacket->size) && (ioPacket->size < Mcbsp_MAX_PACKET_SIZE));

    /* clean the buffers if the cache operation mode is enabled               */
    if (TRUE == instHandle->enablecache)
    {
        if (IOM_INPUT == chanHandle->mode)
        {
            /* Cleans and invalidate the D Cache in MVA way                   */
            BCACHE_wbInv(ioPacket->addr,ioPacket->size,TRUE);
        }
        else
        {
            /* clean the D Cache in MVA way                                   */
            BCACHE_wb(ioPacket->addr,ioPacket->size,TRUE);
        }
    }

    /*==========================MCBSP MODE ===================================*/
    if (Mcbsp_DevMode_McBSP == instHandle->mode)
    {
        /* We are entering the crictical section because the current active   *
         * IOP is being check and it can become NULL at any point. hence we   *
         * will protect this code from the interupt handler                   */
        hwiKey = (Uint32)_disable_interrupts();

#ifdef Mcbsp_LOOPJOB_ENABLE
        if ((Mcbsp_MAXLINKCNT <= chanHandle->submitCount) ||
#else
        /* We now have 3 packets loaded in the EDMA                           */
        if (((Mcbsp_MAXLINKCNT + 1) <= chanHandle->submitCount) ||
#endif
            (TRUE == chanHandle->paused)||
            (Mcbsp_DriverState_PWRM_SUSPEND == instHandle->devState))
        {
            /* There are enough packets programmed in the EDMA or if the MCBSP*
             * is issued a pause command,hence queue packet in to the pending *
             * queue                                                          */
            chanHandle->submitCount++;

            QUE_put(&chanHandle->queuePendingList,(Ptr)ioPacket);

            /* critical section ends                                          */
            _restore_interrupts(hwiKey);
        }
        else
        {
            /* Either one of the paramset or Both the paramsets are free      */

            /* increment the submit count                                     */
            chanHandle->submitCount++;

            QUE_put(&(chanHandle->queueFloatingList),ioPacket);

            Mcbsp_localLoadPktToEdma(chanHandle,ioPacket);

            /* exit the critical section code                                 */
            _restore_interrupts(hwiKey);

        } /* Pending queue empty or not*/
    }
    if (IOM_COMPLETED == status)
    {
        status = IOM_PENDING;
    }
    return (status);
}

/**
 * \brief   Function to unregister interrupt handler for channel
 *
 *          This function unregisters the interrupt handler for the channel by
 *          disabling the evnt for the channel.
 *
 * \param   evt          [IN]  interrupt event number
 *
 * \return  None
 */
static Void msbspUnRegisterInt(Uint16 evt)
{
    Uint32   hwiKey = 0;

    /* disable the interrupts                                                 */
    hwiKey = (Uint32)_disable_interrupts();

    /* Disable the event                                                      */
    ECM_disableEvent(evt);

    /* Enabling the interrupts                                                */
    _restore_interrupts(hwiKey);
}

/**
 * \brief   McBSP SPCR configuration function
 *
 *  This Function is used to set/reset specific bit of SPCR as specified in
 *  the given mask.
 *
 * \param   instHandle   [IN]  pointer to the mcbsp instance object.
 * \param   selectMask   [IN]  the SPCR control mask
 *
 * \return  None
 */
Void Mcbsp_localResetCtrl(Mcbsp_Object *instHandle, Uint32 selectMask)
{
  Uns ticks;

    assert((NULL != instHandle) && (NULL != instHandle->hwInfo.regs));

    /* Configuring SPCR for Frame sync generator enable/disable               */
    if (0u != (selectMask & Mcbsp_SpcrCtrl_FSYNC_DISABLE))
    {
        instHandle->hwInfo.regs->SPCR2 &= (~CSL_MCBSP_SPCR2_FRST_MASK );
    }

    if (0 != (selectMask & Mcbsp_SpcrCtrl_RX_DISABLE))
    {
        instHandle->hwInfo.regs->SPCR1 &= (~CSL_MCBSP_SPCR1_RRST_MASK );
    }

    /* start the sample rate generator                                        */
    if (0u != (selectMask & Mcbsp_SpcrCtrl_SRG_ENABLE))
    {
        instHandle->hwInfo.regs->SPCR2 |=  CSL_MCBSP_SPCR2_GRST_MASK;
    }

    /* Configuring SPCR for Transmit enable/disable                           */
    if (0 != (selectMask & Mcbsp_SpcrCtrl_TX_DISABLE))
    {
        instHandle->hwInfo.regs->SPCR2 &= (~CSL_MCBSP_SPCR2_XRST_MASK);
   }

    /* Configuring SPCR for transmit section enable                           */
    if (0 != (selectMask & Mcbsp_SpcrCtrl_TX_ENABLE))
    {
        instHandle->hwInfo.regs->SPCR2 |= CSL_MCBSP_SPCR2_XRST_MASK;
    }

    /* Configuring SPCR for transmit section start                            */
    if (0 != (selectMask & Mcbsp_SpcrCtrl_TX_START))
    {
        /* The wait added here to match up what was done for ALSA */
        ticks = CLK_countspms()/CLK_getprd();
        TSK_sleep(ticks);

        /* enable the transmitter                                             */
        instHandle->hwInfo.regs->XCCR &= (~CSL_MCBSP_XCCR_XDISABLE_MASK);
    }

    /* Configuring SPCR for Receive section enable                            */
    if (0 != (selectMask & Mcbsp_SpcrCtrl_RX_ENABLE))
    {
        instHandle->hwInfo.regs->SPCR1 |= CSL_MCBSP_SPCR1_RRST_MASK ;

        /* The wait added here to match up what was done for ALSA             */
        ticks = CLK_countspms()/CLK_getprd();
        TSK_sleep(ticks);

        /* enable the receiver                                                */
        instHandle->hwInfo.regs->RCCR &= (~CSL_MCBSP_RCCR_RDISABLE_MASK);
    }

    /* Configuring SPCR for Receive section start                             */
    if (0 != (selectMask & Mcbsp_SpcrCtrl_RX_START))
    {
        /* The wait added here to match up what was done for ALSA             */
        ticks = CLK_countspms()/CLK_getprd();
        TSK_sleep(ticks);

        /* enable the receiver                                                */
        instHandle->hwInfo.regs->RCCR &= (~CSL_MCBSP_RCCR_RDISABLE_MASK);
    }

    /* Set the FRST bit to 1 to start the internal frame sync generator       */
    if (0u != (selectMask & Mcbsp_SpcrCtrl_FSYNC_ENABLE))
    {
        instHandle->hwInfo.regs->SPCR2 |=  CSL_MCBSP_SPCR2_FRST_MASK;
    }

    /* Configuring SPCR for sample rate generator enable/disable              */
    if (0u != (selectMask & Mcbsp_SpcrCtrl_SRG_DISABLE))
    {
        instHandle->hwInfo.regs->SPCR2 &=  (~CSL_MCBSP_SPCR2_GRST_MASK);
    }
}

/**
 *  \brief   This function completes the current pending request and then
 *           invokes the application registered callback.
 *
 *  \param   chanHandle   [IN]  Handle to the channel
 *
 *  \return  None
 *
 *  \enter   chanHandle  is a valid non null pointer
 *
 *  \leave   Not implemented
 */
Void Mcbsp_localCompleteCurrentIo(Mcbsp_ChannelObj *chanHandle)
{
//    Mcbsp_Object *instHandle  = NULL;
    IOM_Packet   *ioPacket    = NULL;

    assert(NULL != chanHandle);
    assert(NULL != chanHandle->devHandle);

//    instHandle = (Mcbsp_Object *)chanHandle->devHandle;

    if (TRUE == chanHandle->isTempPacketValid)
    {
        ioPacket = chanHandle->tempPacket;

        chanHandle->tempPacket = NULL;
        chanHandle->isTempPacketValid = FALSE;
    }
    else
    {
        /* SPI mode hence the dataPacket contains the packet                  */
        ioPacket = chanHandle->dataPacket;

        chanHandle->dataPacket = NULL;
    }

    /* reduce the dependency count                                            */
//    Mcbsp_localLpscOff(instHandle);

    if (NULL != ioPacket)
    {
        /* call the completion callback function registered with us           *
         * during channel creation                                            */
        if ((NULL != chanHandle->cbFxn) && (NULL != chanHandle->cbArg))
        {
            /*  Application callback                                          */
            (*chanHandle->cbFxn)((Ptr)chanHandle->cbArg,ioPacket);
        }
    }

    chanHandle->currentError = IOM_COMPLETED;
    chanHandle->userDataBufferSize = 0;
}

/**
 * \brief     This function configures the transmit section of the mcbsp
 *            sync properties.
 *
 * \param     instHandle  [IN] pointer to the instance object.
 * \param     params      [IN] User supplied channel parameters
 *
 * \return    IOM_EBADARGS  if configuration fails.
 *            IOM_COMPLETED if configuration is sucessful.
 */
Int32 Mcbsp_localConfigureXmtChannel(Mcbsp_Object     *instHandle,
                                     Mcbsp_ChanParams *params)
{
    Uint32  tempVal = 0x00;
    Int32   status  = IOM_COMPLETED;

    assert((NULL != instHandle) && (NULL != params));

    /* configure the transmit section                                         */
    /* configure the receive interrupt setting                                */
    instHandle->hwInfo.regs->SPCR2 &= (~CSL_MCBSP_SPCR2_XINTM_MASK);
    instHandle->hwInfo.regs->SPCR2 |=
        (params->chanConfig->intMode << CSL_MCBSP_SPCR2_XINTM_SHIFT);

    /* New update from Sandeep */
#if 0
    /* configure the DX enabler */
    instHandle->hwInfo.regs->SPCR1 &= (~CSL_MCBSP_SPCR1_DXENA_MASK);
    instHandle->hwInfo.regs->SPCR1 |=
        (params->chanConfig->dxState << CSL_MCBSP_SPCR1_DXENA_SHIFT);
#endif

    /* configure the receive control register settings                        */
    instHandle->hwInfo.regs->XCR2 &= (~CSL_MCBSP_XCR2_XPHASE_MASK);
    instHandle->hwInfo.regs->XCR2 |=
        (params->chanConfig->phaseNum << CSL_MCBSP_XCR2_XPHASE_SHIFT);

    /* configure the frame length for single and dual phase frames            */
    instHandle->hwInfo.regs->XCR1 &= (~CSL_MCBSP_XCR1_XFRLEN1_MASK);
    instHandle->hwInfo.regs->XCR1 |=
        ((params->chanConfig->frmLen1 - 1u) << CSL_MCBSP_XCR1_XFRLEN1_SHIFT);

    /* configure the word length of the single and dual phase frames          */
    switch (params->chanConfig->wrdLen1)
    {
        case Mcbsp_WordLength_8:
            tempVal = 0u;
            break;
        case Mcbsp_WordLength_12:
            tempVal = 1u;
            break;
        case Mcbsp_WordLength_16:
            tempVal = 2u;
            break;
        case Mcbsp_WordLength_20:
            tempVal = 3u;
            break;
        case Mcbsp_WordLength_24:
            tempVal = 4u;
            break;
        case Mcbsp_WordLength_32:
            tempVal = 5u;
            break;
        default:
            /* wordlength is not supported by the driver                      */
            status = IOM_EBADARGS;
            break;
    }

    instHandle->hwInfo.regs->XCR1 &= (~CSL_MCBSP_XCR1_XWDLEN1_MASK);
    instHandle->hwInfo.regs->XCR1 |=
        (tempVal << CSL_MCBSP_XCR1_XWDLEN1_SHIFT);

    if (Mcbsp_Phase_DUAL == params->chanConfig->phaseNum)
    {

        instHandle->hwInfo.regs->XCR2 &= (~CSL_MCBSP_XCR2_XFRLEN2_MASK);
        instHandle->hwInfo.regs->XCR2 |=
            ((params->chanConfig->frmLen2 -1u) << CSL_MCBSP_XCR2_XFRLEN2_SHIFT);

        /* configure the word length of the single and dual phase frames      */
        switch (params->chanConfig->wrdLen2)
        {
            case Mcbsp_WordLength_8:
                tempVal = 0u;
                break;
            case Mcbsp_WordLength_12:
                tempVal = 1u;
                break;
            case Mcbsp_WordLength_16:

                tempVal = 2u;
                break;
            case Mcbsp_WordLength_20:
                tempVal = 3u;
                break;
            case Mcbsp_WordLength_24:
                tempVal = 4u;
                break;
            case Mcbsp_WordLength_32:
                tempVal = 5u;
                break;
            default:
                /* wordlength is not supported by the driver                  */
                status = IOM_EBADARGS;
                break;
        }

        instHandle->hwInfo.regs->XCR2 &= (~CSL_MCBSP_XCR2_XWDLEN2_MASK);
        instHandle->hwInfo.regs->XCR2 |=
            (tempVal << CSL_MCBSP_XCR2_XWDLEN2_SHIFT);
    }

    /* set the bit reverse settings                                           */
    instHandle->hwInfo.regs->XCR2 &= (~CSL_MCBSP_XCR2_XREVERSE_MASK);
    instHandle->hwInfo.regs->XCR2 |=
        (params->chanConfig->bitReversal << CSL_MCBSP_XCR2_XREVERSE_SHIFT);

    /* configure the data delay                                               */
    instHandle->hwInfo.regs->XCR2 &= (~CSL_MCBSP_XCR2_XDATDLY_MASK);
    instHandle->hwInfo.regs->XCR2 |=
        (params->chanConfig->dataDelay << CSL_MCBSP_XCR2_XDATDLY_SHIFT);

    /* configure the clock polarity                                           */
    if (Mcbsp_ClkPol_RISING_EDGE == params->clkSetup->clkPolarity)
    {
        /* clock data sampled on rising edge                                  */
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_CLKXP_MASK);
    }
    else
    {
        /* clock data sampled on falling edge                                 */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_CLKXP_MASK);
    }

    /* configure the frame sync polarity                                      */
    if (Mcbsp_FsPol_ACTIVE_HIGH == params->clkSetup->frmSyncPolarity)
    {
        /* frame sync polarity is active high                                 */
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_FSXP_MASK);
    }
    else
    {
        /* frame sync polarity is active low                                  */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_FSXP_MASK);
    }

    /* check if the frame sync generator is to be enabled for this TX section */
    if (Mcbsp_FsClkMode_EXTERNAL == params->clkSetup->frmSyncMode)
    {
        /* External frame sync to be used                                     */
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_FSXM_MASK);

        /* frame sync generator needs to be disabled                          */
        instHandle->txFsgEnable = FALSE;
    }
    else
    {
        /* internal frame sync to be used                                     */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_FSXM_MASK);

        /* New update from Sandeep */
        /* The FRST and GRST has to be cleared to configure the FSGM bit.
         * By clearing the FRST and GRST bit, the sample rate generator and the
         * frame sync generator will be kept in the reset mode.
         * Ref IR SDOCM00077966
         */
        tempVal = instHandle->hwInfo.regs->SPCR2;
        instHandle->hwInfo.regs->SPCR2 &= ~(CSL_MCBSP_SPCR2_FRST_MASK |
                                                     CSL_MCBSP_SPCR2_GRST_MASK );

        /* could be internal or configured for DXR to XSR copy                */
        if (Mcbsp_FsClkMode_INTERNAL == params->clkSetup->frmSyncMode)
        {
            /* set the FSGM bit in the SRGR register                          */
            instHandle->hwInfo.regs->SRGR2 |= (CSL_MCBSP_SRGR2_FSGM_MASK);

            /* frame sync generator needs to be enabled                       */
            instHandle->txFsgEnable = TRUE;
        }
        else
        {   /* DXR to XSR copy generates frame sync                           */
            /* reset the FSGM bit in the SRGR register                        */
            instHandle->hwInfo.regs->SRGR2 &= (~CSL_MCBSP_SRGR2_FSGM_MASK);

            /* frame sync generator needs to be disabled                      */
            instHandle->txFsgEnable = FALSE;
        }
        /* New update from Sandeep */
        /* The SPCR register values are reverted back once the FSGM bit is set
         */
        instHandle->hwInfo.regs->SPCR2 = tempVal;
    }

    /* configure the clock mode (external or internal)                        */
    if (Mcbsp_TxRxClkMode_EXTERNAL == params->clkSetup->clkMode)
    {
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_CLKXM_MASK);

        if (TRUE == instHandle->txFsgEnable)
        {
            /* frame sync generator is using the internal clock hence need to *
             * enable the sample rate generator                               */
            instHandle->txSrgEnable = TRUE;
        }
        else
        {
            /* dont require to enable the sample rate generator               */
            instHandle->txSrgEnable = FALSE;
        }
    }
    else
    {
        /* external mode clock enabled                                        */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_CLKXM_MASK);

        /* sample rate generator is enabled                                   */
        instHandle->txSrgEnable = TRUE;
    }

    return (status);
}


/**
 * \brief     This function configures the receives section of the mcbsp.
 *
 * \param     instHandle  [IN] pointer to the instance object.
 * \param     params      [IN] User supplied channel parameters
 *
 * \return    IOM_EBADARGS  if configuration fails.
 *            IOM_COMPLETED if confioguration is sucessful.
 */
Int32 Mcbsp_localConfigureRcvChannel(Mcbsp_Object     *instHandle,
                                     Mcbsp_ChanParams *params)
{
    Uint32  tempVal = 0x00;
    Int32   status  = IOM_COMPLETED;

    assert((NULL != instHandle) && (NULL != params));

    /* configure the receive section                                          */
    /* configure the receive interrupt setting                                */
    instHandle->hwInfo.regs->SPCR1 &= (~CSL_MCBSP_SPCR1_RINTM_MASK);
    instHandle->hwInfo.regs->SPCR1 |=
        (params->chanConfig->intMode << CSL_MCBSP_SPCR1_RINTM_SHIFT);

    /* New update from Sandeep */
    /* configure the Receive sign-extension and justification                 */
    instHandle->hwInfo.regs->SPCR1 &= (~CSL_MCBSP_SPCR1_RJUST_MASK);
    instHandle->hwInfo.regs->SPCR1 |=
        (params->chanConfig->rjust << CSL_MCBSP_SPCR1_RJUST_SHIFT);

    /* configure the receive control register settings                        */
    instHandle->hwInfo.regs->RCR2 &= (~CSL_MCBSP_RCR2_RPHASE_MASK);
    instHandle->hwInfo.regs->RCR2 |=
        (params->chanConfig->phaseNum << CSL_MCBSP_RCR2_RPHASE_SHIFT);

    /* configure the frame length for single and dual phase frames            */
    instHandle->hwInfo.regs->RCR1 &= (~CSL_MCBSP_RCR1_RFRLEN1_MASK);
    instHandle->hwInfo.regs->RCR1 |=
        ((params->chanConfig->frmLen1 - 1u) << CSL_MCBSP_RCR1_RFRLEN1_SHIFT);

    /* configure the word length of the single and dual phase frames          */
    switch (params->chanConfig->wrdLen1)
    {
        case Mcbsp_WordLength_8:
            tempVal = 0u;
            break;
        case Mcbsp_WordLength_12:
            tempVal = 1u;
            break;
        case Mcbsp_WordLength_16:
            tempVal = 2u;
            break;
        case Mcbsp_WordLength_20:
            tempVal = 3u;
            break;
        case Mcbsp_WordLength_24:
            tempVal = 4u;
            break;
        case Mcbsp_WordLength_32:
            tempVal = 5u;
            break;
        default:
            /* wordlength is not supported by the driver                      */
            status = IOM_EBADARGS;
            break;
    }


    instHandle->hwInfo.regs->RCR1 &= (~CSL_MCBSP_RCR1_RWDLEN1_MASK);
    instHandle->hwInfo.regs->RCR1 |=
        (tempVal << CSL_MCBSP_RCR1_RWDLEN1_SHIFT);

    if (Mcbsp_Phase_DUAL == params->chanConfig->phaseNum)
    {
        instHandle->hwInfo.regs->RCR2 &= (~CSL_MCBSP_RCR2_RFRLEN2_MASK);
        instHandle->hwInfo.regs->RCR2 |=
        ((params->chanConfig->frmLen2 - 1u) << CSL_MCBSP_RCR2_RFRLEN2_SHIFT);

        /* configure the word length of the single and dual phase frames      */
        switch (params->chanConfig->wrdLen2)
        {
            case Mcbsp_WordLength_8:
                tempVal = 0u;
                break;
            case Mcbsp_WordLength_12:
                tempVal = 1u;
                break;
            case Mcbsp_WordLength_16:
                tempVal = 2u;
                break;
            case Mcbsp_WordLength_20:
                tempVal = 3u;
                break;
            case Mcbsp_WordLength_24:
                tempVal = 4u;
                break;
            case Mcbsp_WordLength_32:
                tempVal = 5u;
                break;
            default:
                /* wordlength is not supported by the driver                      */
                status = IOM_EBADARGS;
                break;
        }

        instHandle->hwInfo.regs->RCR2 &= (~CSL_MCBSP_RCR2_RWDLEN2_MASK);
        instHandle->hwInfo.regs->RCR2 |=
            (tempVal << CSL_MCBSP_RCR2_RWDLEN2_SHIFT);
    }

    /* set the bit reverse settings                                           */
    instHandle->hwInfo.regs->RCR2 &= (~CSL_MCBSP_RCR2_RREVERSE_MASK);
    instHandle->hwInfo.regs->RCR2 |=
        (params->chanConfig->bitReversal << CSL_MCBSP_RCR2_RREVERSE_SHIFT);

    /* configure the data delay                                               */
    instHandle->hwInfo.regs->RCR2 &= (~CSL_MCBSP_RCR2_RDATDLY_MASK);
    instHandle->hwInfo.regs->RCR2 |=
        (params->chanConfig->dataDelay << CSL_MCBSP_RCR2_RDATDLY_SHIFT);

    /* configure the clock polarity                                           */
    if (Mcbsp_ClkPol_RISING_EDGE == params->clkSetup->clkPolarity)
    {
        /* New update from Sandeep */
        /* clock data sampled on rising edge                                  */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_CLKRP_MASK);
    }
    else
    {
        /* New update from Sandeep */
    	/* clock data sampled on falling edge                                 */
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_CLKRP_MASK);
    }

    /* configure the frame sync polarity                                      */
    if (Mcbsp_FsPol_ACTIVE_HIGH == params->clkSetup->frmSyncPolarity)
    {
        /* frame sync polarity is active high                                 */
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_FSRP_MASK);
    }
    else
    {
        /* frame sync polarity is active low                                  */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_FSRP_MASK);
    }


    /* check if the frame sync generator is to be enabled for this section    */
    if (Mcbsp_FsClkMode_INTERNAL == params->clkSetup->frmSyncMode)
    {
        /* set the frame sync generation mode                                 */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_FSRM_MASK);

        /* frame sync generator needs to be enabled                           */
        instHandle->rxFsgEnable = TRUE;
    }
    else
    {
        /* reset the frame sync generation mode                               */
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_FSRM_MASK);

        /* frame sync generator needs to be disabled                          */
        instHandle->rxFsgEnable = FALSE;
    }

    /* configure the clock mode (external or internal)                        */
    if (Mcbsp_TxRxClkMode_EXTERNAL == params->clkSetup->clkMode)
    {
        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_CLKRM_MASK);

        if (TRUE == instHandle->rxFsgEnable)
        {
            /* frame sync generator is using the internal clock hence need to *
             * enable the sample rate generator                               */
            instHandle->rxSrgEnable = TRUE;
        }
        else
        {
            /* dont require to enable the sample rate generator               */
            instHandle->rxSrgEnable = FALSE;
        }
    }
    else
    {
        /* external mode clock                                                */
        instHandle->hwInfo.regs->PCR |= (CSL_MCBSP_PCR_CLKRM_MASK);

        /* sample rate generator is enabled                                   */
        instHandle->rxSrgEnable = TRUE;
    }
    return (status);
}

/**
 * \brief     This function configures the sample rate generator and frame
 *            sync properties.
 *
 * \param     instHandle  [IN] pointer to the instance object.
 * \param     chanHandle  [IN] Handle to the channel.
 *
 * \return    IOM_EBADARGS  if calculation fails.
 *            IOM_COMPLETED if calculation is sucessful.
 */
Int32 Mcbsp_localConfigureSrgr(Mcbsp_Object     *instHandle,
                               Mcbsp_ChannelObj *chanHandle)
{
    Bool             srgrConfig  = FALSE;
    Uint32           clkgDiv     = 0x00;
    Uint32           noOfBits    = 0x00;
    Uint32           framePeriod = 0x00;
    Bool             sclkme      = 0x00;
    Bool             clksm       = 0x00;
    Int32            status      = IOM_COMPLETED;
    Uint32           hwiKey      = 0x00;
#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
    Uns              setpoint    = 0x00;
    PWRM_Domain      domain      = PWRM_CPU;
    PWRM_Status      retVal      = PWRM_SOK;
#endif

    assert((NULL != instHandle) && (NULL != chanHandle));

    /* check if the SRGR is not configured already                            */
    hwiKey = (Uint32)_disable_interrupts();

    if (FALSE == instHandle->srgConfigured)
    {
        /* set the status that SRGR is configured                             */
        instHandle->srgConfigured = TRUE;

        _restore_interrupts(hwiKey);

        /* check if this channel requires the SRGR to be enabled              */
        if (IOM_INPUT == chanHandle->mode)
        {
            if (TRUE == instHandle->rxSrgEnable)
            {
                /* Need to configure the SRGR                                 */
                srgrConfig = TRUE;
            }
            /* New update from Sandeep */
            else
            {
            	instHandle->srgConfigured = FALSE;
            }
        }
        else
        {
            if (TRUE == instHandle->txSrgEnable)
            {
                /* Need to configure the SRGR                                 */
                srgrConfig = TRUE;
            }
            /* New update from Sandeep */
            else
            {
            	instHandle->srgConfigured = FALSE;
            }
        }
    }
    else
    {
        _restore_interrupts(hwiKey);
    }

    if (TRUE == srgrConfig)
    {
        /* set the input clock selection for Sample rate generator            */
        switch (instHandle->srgrConfig.srgInputClkMode)
        {
            /* Input clock is CLKS                                            */
            case Mcbsp_SrgClk_CLKS:
                sclkme = 0x00;
                clksm = 0x00;

                /* set the CLKS polarity (only if CLKS is used)               */
                instHandle->hwInfo.regs->SRGR2 &= (~CSL_MCBSP_SRGR2_CLKSP_MASK);
                instHandle->hwInfo.regs->SRGR2 |=
                    (instHandle->srgrConfig.clksPolarity
                        << CSL_MCBSP_SRGR2_CLKSP_SHIFT);

                /* set the GSYNC option                                       */
                instHandle->hwInfo.regs->SRGR2 &= (~CSL_MCBSP_SRGR2_GSYNC_MASK);
                instHandle->hwInfo.regs->SRGR2 |=
                    (instHandle->srgrConfig.gSync
                        << CSL_MCBSP_SRGR2_GSYNC_SHIFT);
                break;
            /* Input clock for SRG is from CPU clock                          */
            case Mcbsp_SrgClk_CLKCPU:
                sclkme = 0x00;
                clksm = 0x01u;
                break;
            /* Input clock is from receive pin                                */
            case Mcbsp_SrgClk_CLKR:
                sclkme = 0x01u;
                clksm = 0x00;
                break;
            /* Input clock is from transmit pin                               */
            case Mcbsp_SrgClk_CLKX:
                sclkme = 0x01u;
                clksm = 0x01u;
                break;
            default:
                status = IOM_EBADARGS;
                break;
        }

        instHandle->hwInfo.regs->PCR &= (~CSL_MCBSP_PCR_SCLKME_MASK);
        instHandle->hwInfo.regs->PCR |= (sclkme << CSL_MCBSP_PCR_SCLKME_SHIFT);

        instHandle->hwInfo.regs->SRGR2 &= (~CSL_MCBSP_SRGR2_CLKSM_MASK);
        instHandle->hwInfo.regs->SRGR2 |= (clksm << CSL_MCBSP_SRGR2_CLKSM_SHIFT);

        /* calculate and set the CLKG div values                              */
        if (Mcbsp_Phase_DUAL == chanHandle->chanConfig.phaseNum)
        {
            noOfBits = ((chanHandle->chanConfig.wrdLen1
                            * chanHandle->chanConfig.frmLen1)
                        + (chanHandle->chanConfig.wrdLen2
                            * chanHandle->chanConfig.frmLen2));
        }
        else
        {
            noOfBits  = (chanHandle->chanConfig.wrdLen1
                            * chanHandle->chanConfig.frmLen1);
        }

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
        if ((TRUE == instHandle->pscPwrmEnable) &&
            (Mcbsp_pllDomain_NONE != instHandle->pwrmInfo.pllDomain) &&
            (Mcbsp_SrgClk_CLKCPU == instHandle->srgrConfig.srgInputClkMode))
        {
            /* check what clock domain is supported by the device             */
            if (Mcbsp_pllDomain_0 == instHandle->pwrmInfo.pllDomain)
            {
                domain = PWRM_CPU;
            }
            else
            {
                domain = PWRM_PER;
            }

            retVal = PWRM_getCurrentSetpoint(domain,&setpoint);

            if (PWRM_SOK == retVal)
            {
                status = McbspCalculateClockSettings(
                                instHandle,
                                chanHandle,
                                setpoint,
                                &clkgDiv,
                                &framePeriod);
            }

        }
        else
        {
#endif
            clkgDiv = ((instHandle->srgrConfig.srgrInputFreq/
                        (chanHandle->clkSetup.samplingRate
                        * noOfBits)) - 1u);

            framePeriod = (noOfBits - 1u);

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
        }
#endif
        if ((0xFF < clkgDiv) || (0xFFF < framePeriod) ||
            (IOM_COMPLETED != status))
        {
            /* The CLKGDIV or frame period value has exceeded the limit       */
            status = IOM_EBADARGS;
        }
        else
        {
            /* set the value of the CLKGDIV                                   */
            instHandle->hwInfo.regs->SRGR1 &= (~CSL_MCBSP_SRGR1_CLKGDV_MASK);
            instHandle->hwInfo.regs->SRGR1 |= clkgDiv;

            /* set the value of the frame period                              */
            instHandle->hwInfo.regs->SRGR2 &= (~CSL_MCBSP_SRGR2_FPER_MASK);
            instHandle->hwInfo.regs->SRGR2 |=
                (framePeriod << CSL_MCBSP_SRGR2_FPER_SHIFT);

            /* set the frame width                                            */
            instHandle->hwInfo.regs->SRGR1 &= (~CSL_MCBSP_SRGR1_FWID_MASK);
            instHandle->hwInfo.regs->SRGR1 |=
                (instHandle->srgrConfig.srgFrmPulseWidth <<
                    CSL_MCBSP_SRGR1_FWID_SHIFT);
        }
    }

    return (status);
}

/**
 * \brief   McBSP Tx ISR function
 *
 *          This Function is the interrupt service routine for the Mcbsp TX
 *          event.
 *
 * \param   chanHandle  [IN]  Handle to the channel
 *
 * \return  None
 */
static Void mcbspGblXmtIsr(Mcbsp_ChannelObj *chanHandle)
{
    Mcbsp_Object     *instHandle = NULL;

    assert(NULL != chanHandle);
    assert(NULL != chanHandle->devHandle);

    instHandle = (Mcbsp_Object *)chanHandle->devHandle;
    assert(NULL != instHandle);

    /* Only DMA mode of operation is supported by the Mcbsp driver hence we   *
     * will only check if there is an sync error and notify the application   */
    if (CSL_MCBSP_SPCR2_XSYNCERR_MASK ==
        (CSL_MCBSP_SPCR2_XSYNCERR_MASK & instHandle->hwInfo.regs->SPCR2))
    {
        /* call the application registered global callback function           */
        if (NULL != chanHandle->gblErrCbk)
        {
            (*chanHandle->gblErrCbk)((Uint32)chanHandle,
                instHandle->hwInfo.regs->SPCR2,
                NULL);
        }
    }
}

/**
 * \brief   McBSP Rx ISR function
 *
 *          This Function is the interrupt service routine for the Mcbsp RX
 *          event.
 *
 * \param   chanHandle  [IN]  Handle to the channel
 *
 * \return  None
 */
static Void mcbspGblRcvIsr(Mcbsp_ChannelObj *chanHandle)
{
    Mcbsp_Object     *instHandle = NULL;

    assert(NULL != chanHandle);
    assert(NULL != chanHandle->devHandle);

    instHandle = (Mcbsp_Object *)chanHandle->devHandle;
    assert(NULL != instHandle);

    /* Only DMA mode of operation is supported by the Mcbsp driver hence we   *
     * will only check if there is an sync error and notify the application   */
    if (CSL_MCBSP_SPCR1_RSYNCERR_MASK ==
        (CSL_MCBSP_SPCR1_RSYNCERR_MASK & instHandle->hwInfo.regs->SPCR1))
    {
        /* call the application registered global callback function           */
        if (NULL != chanHandle->gblErrCbk)
        {
            (*chanHandle->gblErrCbk)((Uint32)chanHandle,
                instHandle->hwInfo.regs->SPCR1,
                NULL);
        }
    }
}

/**
 * \brief   Abort the queued up requests.
 *
 *          This commands aborts all the pending IO requests and returns them
 *          to the application. The current state of the IO request will be set
 *          to ABORTED.
 *
 * \param   chanHandle   [IN]   Handle to the channel whose requests are to be
 *                              aborted
 *
 * \return  None
 */
Void Mcbsp_localAbortReset(Mcbsp_ChannelObj *chanHandle)
{
    Mcbsp_Object     *instHandle   = NULL;
    IOM_Packet       *ioPacket     = NULL;
    Uint32            hwiKey       = 0x00;
    Uint32            timeoutCount = 0x00;

    assert(NULL != chanHandle);
    assert(NULL != chanHandle->devHandle);

    instHandle = chanHandle->devHandle;
    assert(NULL != instHandle);

    /* initialize the retry count value                                       */
    timeoutCount = instHandle->retryCount;

    Mcbsp_localLpscOn(instHandle);

    /* critical section starts !                                              */
    hwiKey = (Uint32)_disable_interrupts();

    if (Mcbsp_DevMode_McBSP == instHandle->mode)
    {
        /* Disable the EDMA transfer for the current transfer channel         */
        EDMA3_DRV_disableTransfer(
            chanHandle->edmaHandle,
            chanHandle->xferChan,
            EDMA3_DRV_TRIG_MODE_EVENT);

        if (IOM_OUTPUT == chanHandle->mode)
        {
            /* Wait for the TX to be empty                                    */
            while ((CSL_MCBSP_SPCR2_XEMPTY_MASK ==
                (CSL_MCBSP_SPCR2_XEMPTY_MASK & instHandle->hwInfo.regs->SPCR2)) &&
                (0 != timeoutCount))
            {
                timeoutCount--;
            }
        }

        /* Stop the McBSP instance                                            */
        if (IOM_INPUT == chanHandle->mode)
        {
            Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_RX_DISABLE);
        }
        else
        {
            Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_DISABLE);
        }


        /* Empty the floating queue. Aborted request is currently enqueued    *
         * in the floating queue. Dequeue the floating request in EDMA        *
         * param table and return the actual transfer element count           */
        while (TRUE !=  QUE_empty(&chanHandle->queueFloatingList))
        {
            ioPacket = QUE_get(&chanHandle->queueFloatingList);

            if (NULL != ioPacket)
            {
                /* if FLUSH cmd called for INPUT channel then status is equal *
                 * to FLUSHED otherwise status is equal to ABORTED            */
                if ((TRUE == chanHandle->flush) &&
                    (IOM_INPUT == chanHandle->mode))
                {
                    ioPacket->status = IOM_FLUSHED;
                }
                else
                {
                    ioPacket->status = IOM_ABORTED;
                }

                /* reduce the dependency count                                */
                Mcbsp_localLpscOff(instHandle);


                if ((NULL !=  chanHandle->cbFxn) && (NULL != chanHandle->cbArg))
                {
                    /*  Application callback                                  */
                    (*chanHandle->cbFxn)((Ptr)chanHandle->cbArg,ioPacket);
                }
                /* Decrement the submit count                                 */
                chanHandle->submitCount--;
            }
        }

        /* Empty the pending queue                                            */
        while (TRUE != QUE_empty(&chanHandle->queuePendingList))
        {
            ioPacket = QUE_get(&chanHandle->queuePendingList);

            if (NULL != ioPacket)
            {
                /* if FLUSH cmd called for INPUT channel then status is equal *
                 * to IOM_FLUSHED otherwise status is equal to IOM_ABORTED    */
                if ((TRUE == chanHandle->flush) &&
                    (IOM_INPUT == chanHandle->mode))
                {
                    ioPacket->status = IOM_FLUSHED;
                }
                else
                {
                    ioPacket->status = IOM_ABORTED;
                }

                /* reduce the dependency count                                */
                Mcbsp_localLpscOff(instHandle);

                if ((NULL != chanHandle->cbFxn) && (NULL != chanHandle->cbArg))
                {
                    /*  Application callback                                  */
                    (*chanHandle->cbFxn)((Ptr)chanHandle->cbArg,ioPacket);
                }

                /* Decrement the submit count                                 */
                chanHandle->submitCount--;
            }
        }

        /* As EDMA is disabled it might have thrown an error and set error bit*
         * Clear the error bit to enable the transfer again                   */
        EDMA3_DRV_clearErrorBits(chanHandle->edmaHandle,chanHandle->xferChan);

        /* Configure the EDMA channel and EDMA param tables with              *
         * intialization configuration as they are configured at the          *
         * create time.                                                       */
        Mcbsp_localEdmaChanPaRAMsetup(chanHandle);

#ifdef Mcbsp_LOOPJOB_ENABLE

		/* configure the FIFO for the specific channel                */
		if (TRUE == chanHandle->enableHwFifo)
		{
		    /* Disable and enable the FIFO so that the events are     *
		     * generated to the Mcbsp for the first time              */
		    mcbspConfigureFifo(&instHandle->hwInfo,chanHandle,FALSE);
		    mcbspConfigureFifo(&instHandle->hwInfo,chanHandle,TRUE);
		}

        /* Enable the EDMA transfer to start the Loop job running             */
        EDMA3_DRV_enableTransfer(
            chanHandle->edmaHandle,
            chanHandle->xferChan,
            EDMA3_DRV_TRIG_MODE_EVENT);

        chanHandle->loopjobUpdatedinParamset = TRUE;
        chanHandle->nextLinkParamSetToBeUpdated = 0;


        /* stop the Mcbsp channel                                             */
        if (IOM_INPUT == chanHandle->mode)
        {
                Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_RX_ENABLE);
        }
        else
        {
			Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_ENABLE);
		}
#endif
    }
    /* critical section ends                                                  */
    _restore_interrupts(hwiKey);

    Mcbsp_localLpscOff(instHandle);
}
/**
 *
 * \brief   This function configures the Hardware FIFO
 *
 * \param   hMcbsp       [IN] pointer to the Mcbsp Hardware information struct
 * \param   chanHandle   [IN] handle to the channel
 * \param   enableHwFifo [IN] option to Enable or disbale the FIFO
 *
 * \return  None
 *
 * \enter   hMcbsp      is a valid non null pointer
 *          chanHandle  is a valid non null pointer
 *
 * \leave   Not implemented
 * Kayjin modified for DM3730 to configure THRSH REGs
 *
 */
static Void mcbspConfigureFifo(Mcbsp_HwInfo        *hMcbsp,
                               Mcbsp_ChannelObj    *chanHandle,
                               Bool                 enableHwFifo)
{
    /* validate the input parameters                                          */
    assert((NULL != hMcbsp) && (NULL != chanHandle));

    /* check if the HW FIFO usage is requested by the user for this channel   */
    if (TRUE == enableHwFifo)
    {
        if (IOM_INPUT == chanHandle->mode)
        {
            /* configure the receive channel                                  */
            /* enable the FIFO now by setting the "THRSH1 REG"                */
            hMcbsp->regs->THRSH1 =
                (Mcbsp_FRAME_LENGTH	<< CSL_MCBSP_THRSH1_RTHRES_SHIFT);
        }
        else
        {
            /* configure the transmit channel                                 */
             /* enable the FIFO now by setting the "THRSH2 REG"               */
            hMcbsp->regs->THRSH2 =
                (Mcbsp_FRAME_LENGTH << CSL_MCBSP_THRSH2_XTHRES_SHIFT);
        }
    }
    else
    {
        /* FIFO needs to be disabled                                          */
        if (IOM_INPUT == chanHandle->mode)
        {
            /* disable the FIFO now by resetting the "WENA" bit               */
            hMcbsp->regs->THRSH1 =
                (0	<< CSL_MCBSP_THRSH1_RTHRES_SHIFT);
        }
        else
        {
            /* disable the FIFO now by resetting the "WENA" bit               */
            hMcbsp->regs->THRSH2 =
            	(0 << CSL_MCBSP_THRSH2_XTHRES_SHIFT);
        }
    }
}

/**
 * \brief   registers interrupt handler for the channel.
 *
 * \param   chanp        [IN]    Pointer to channel
 * \param   intNum       [IN]    Hwi number for this event
 * \param   evt          [IN]    event number
 * \param   intIsr       [IN]    function pointer
 *
 * \return  None
 */
static Int32 mcbspRegisterInt(Ptr chanp,Uint32 intNum,Uint16 evt,ECM_Fxn intIsr)
{
    ECM_Attrs            ecmAttrs   = ECM_ATTRS;
    Uint32               hwiKey     = 0;
    Int32                status     = IOM_COMPLETED;

    assert((NULL != chanp) && (NULL != intIsr));

    hwiKey = (Uint32)_disable_interrupts();

    /*Enable the interrupt later                                              */
    ecmAttrs.unmask = 0u;

    /* setup arg such that isr will get 'chan' as arg                         */
    ecmAttrs.arg    = (Arg)chanp;

    /* Plug the ISR with interrupt (event) number                             */
    ECM_dispatchPlug(evt,intIsr, &ecmAttrs);

    /* Enabling the HWI_ID where corresponding ECM group is configured        */
    C64_enableIER(1u << intNum);

    ECM_enableEvent(evt);

    /* Enabling the interrupts                                                */
    _restore_interrupts(hwiKey);

    return status;
}

/**
 * \brief   This is going to complete the current request and abort
 *          all other reqest.
 *
 * \param   chanHandle   [IN]   Channel handle
 *
 * \return  None
 *
 * \enter   chanHandle  is a valid non null pointer
 *
 * \leave   Not implemented
 */
Void Mcbsp_localCancelAndAbortAllIo(Mcbsp_ChannelObj *chanHandle)
{
    assert(NULL != chanHandle);

    chanHandle->submitCount--;
    chanHandle->tempPacket->status = IOM_ABORTED;

    Mcbsp_localCompleteCurrentIo(chanHandle);

    Mcbsp_localAbortReset(chanHandle);
}

#ifndef Mcbsp_LOOPJOB_ENABLE
/**
 *  \brief  This function waits for the FIFO to be emptied(if enabled) and the
 *          TX empty bit to be set so that the TX section could be disabled when
 *          there is no data present.
 *
 *  \param  arg0   [IN]  Handle to the TX channel
 *  \param  arg1   [IN]  unused
 *
 *  \return None
 *
 *  \enter  arg0 is a valid non null pointer
 *
 *  \leave  Not implemented
 */
static Void Mcbsp_swiTxFifo(Arg arg0,Arg arg1)
{
    Mcbsp_ChannelObj  *chanHandle = NULL;
    Mcbsp_Object      *instHandle = NULL;
    Uint32             timeOut    = 0x00;

    chanHandle = (Mcbsp_ChannelObj *)arg0;
    instHandle = (Mcbsp_Object *)arg1;

    assert(NULL != chanHandle);
    assert(NULL != instHandle);

    /* update the timeout value from the instance handle                      */
    timeOut = instHandle->retryCount;

    /* we have come here means that the Mcbsp has got an callback but it      *
     * did have any more packet to load Hence here we will wait for the       *
     * FIFO to become empty (if FIFO is enabled) else wait for the TX to      *
     * become empty.then we will disable the TX section                       */
    if (TRUE == chanHandle->enableHwFifo)
    {
#if 0 /* Kayjin for DM3730 */
        while ((0 != (instHandle->hwInfo.fifoRegs->WFIFOSTS &
                       CSL_BFIFO_WFIFOSTS_WLVL_MASK))
                && (0 != timeOut))
        {
            /* reduce the timeout count and check if the FIFO is empty    */
            timeOut--;
        }
#endif
    }

    /* reinitialize the retry count                                       */
    timeOut = instHandle->retryCount;

    while ((CSL_MCBSP_SPCR2_XEMPTY_MASK ==
            (instHandle->hwInfo.regs->SPCR2 & CSL_MCBSP_SPCR2_XEMPTY_MASK))
           && (0 != timeOut))
    {
        /* reduce the retry count and check if the TX has completed           *
         * transmitting all the bytes                                         */
        timeOut--;
    }

    /* we need to stop the frame sycn generator now.But also need to check    *
     * if  1.The frame sycn generator is actually started By TX.              *
     *     2.The RX is  not feeding of the sample rate generator              */
    if ((TRUE == instHandle->txFsgEnable) &&
        ((TRUE != instHandle->rxFsgEnable)
            || (TRUE == instHandle->stopSmFsRcv)))
    {
        /* Now we can disable the frame sync generator                        */
        Mcbsp_localResetCtrl(
            (Mcbsp_Object *)chanHandle->devHandle,
            Mcbsp_SpcrCtrl_FSYNC_DISABLE);
        instHandle->fsgEnabled = FALSE;
    }
    /* Stop the TX section                                                    */
    Mcbsp_localResetCtrl(
        (Mcbsp_Object *)chanHandle->devHandle,
        Mcbsp_SpcrCtrl_TX_DISABLE);

    /* Transmit state machine is stopped                                      */
    instHandle->stopSmFsXmt = TRUE;

    /* clear the error bits in the EDMA(as this is the last packet)           */
    EDMA3_DRV_clearErrorBits(
        chanHandle->edmaHandle,
        chanHandle->xferChan);

    /* complete the IOP now and call the callback to the stream               */
    chanHandle->tempPacket = QUE_get(&(chanHandle->queueFloatingList));

    /* Decrement the submit count for the IOpackets                           */
    chanHandle->submitCount--;

    chanHandle->isTempPacketValid = TRUE;
    chanHandle->tempPacket->status = chanHandle->currentError;
    Mcbsp_localCompleteCurrentIo(chanHandle);

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
    /* check if the driver is waiting to go to sleep or process DVFS event    *
     * (only if the application has requested PWRM support)                   */
    if (TRUE == instHandle->pscPwrmEnable)
    {
        Mcbsp_localHandlePwrmEvent(instHandle,chanHandle);
    }
#endif /* #ifdef BIOS_PWRM_ENABLE  */
}
#endif /* #ifndef Mcbsp_LOOPJOB_ENABLE */

/**
 * \brief  This function toggles the index of the edma params
 *
 * \param  index [IN]  pointer to current index
 *
 * \return None
 *
 * \enter  index is a valid non null pointer
 *
 * \leave  Not implemented
 */
Void Mcbsp_localGetNextIndex(Uint32 *index)
{
    assert(NULL != index);
    *index = (((*index) + 1u) & 0x01u);
}

/**
 *  \brief  This function loads the packets to the actual EDMA paramset.
 *
 *  \param  chanHandle [IN]  Handle to channel.
 *  \param  ioPacket   [IN]  pointer to the ioPacket
 *
 *  \return None
 *
 *  \enter  Not implemented
 *
 *  \leave  Not implemented
 */
Void Mcbsp_localLoadPktToEdma(Mcbsp_ChannelObj *chanHandle,IOM_Packet *ioPacket)
{
#ifndef Mcbsp_LOOPJOB_ENABLE
    Mcbsp_Object     *instHandle  = NULL;
#endif
    Int32             status      = IOM_COMPLETED;

    assert(NULL != chanHandle);
    assert(NULL != ioPacket);

#ifndef Mcbsp_LOOPJOB_ENABLE
    instHandle = (Mcbsp_Object *)chanHandle->devHandle;
    assert(NULL != instHandle);
#endif

    chanHandle->currentDataSize = (Uint16)ioPacket->size;
    chanHandle->userDataBufferSize = (Uint32)ioPacket->size;

#ifdef Mcbsp_LOOPJOB_ENABLE
    if (Mcbsp_MAXLINKCNT == chanHandle->submitCount)
    {
#else
    /* The second and the third packet will go the link paramsets             */
    if (Mcbsp_MAXLINKCNT <= chanHandle->submitCount)
    {
#endif
        /* Though we have to post to param set directly from here,            *
         * there will be differene between first such packet and              *
         * second packet. As we have control here we are second packet        *
         * and first packet has not yet returned (or corresponding            *
         * edma callback has not been called.For second packet, we            *
         * will be updating the second param set, which is currently          *
         * hosting loopjob parameter. Hence increment the index to            *
         * point second paramset and since we are moving out loopjob          *
         * from both param sets, the loopjobUpdatedinParamset is reset        */
        chanHandle->loopjobUpdatedinParamset = FALSE;
        Mcbsp_localGetNextIndex(
            &chanHandle->nextLinkParamSetToBeUpdated);
    }

    /* Now update the data packet to the link params. The paramset to         *
     * be updated is decidec by the "nextLinkParamSetToBeUpdated"             */
    if (IOM_COMPLETED != Mcbsp_localUpdtDtPktToLnkPrms(chanHandle,ioPacket))
    {
        status = IOM_EBADIO;
    }

    if ((1u == chanHandle->submitCount) && (IOM_COMPLETED == status))
    {
#ifdef Mcbsp_LOOPJOB_ENABLE
        /* if at all this is the very first packet, then one param set        *
         * has loop job loaded , self linked and active with the main         *
         * xfer channel param. other param set is ready loaded (just          *
         * now)and has link param set as the one having loopjob (this         *
         * is to ensure that if at all we are not getting any more            *
         * packets loopjob be will taken over). Now we have to link           *
         * the floating / newly loaded param set to xfer channel.             */
        if (IOM_COMPLETED != EDMA3_DRV_linkChannel(
                                 chanHandle->edmaHandle,
                                 chanHandle->xferChan,
                                 chanHandle->pramTbl[chanHandle->nextLinkParamSetToBeUpdated]))
        {
            status = IOM_EBADIO;
        }
#else
        /* configure the FIFO for the specific channel                        */
        if (TRUE == chanHandle->enableHwFifo)
        {
            /* Disable and enable the FIFO so that the events are             *
             * generated to the Mcbsp for the first time                      */
            mcbspConfigureFifo(&instHandle->hwInfo,chanHandle,FALSE);
            mcbspConfigureFifo(&instHandle->hwInfo,chanHandle,TRUE);
        }

        /* enable the EDMA transfer for the channel so that it is             *
         * ready to transfer the data as soon as the state machine is         *
         * enabled                                                            */
        EDMA3_DRV_enableTransfer(
            chanHandle->edmaHandle,
            chanHandle->xferChan,
            EDMA3_DRV_TRIG_MODE_EVENT);

        /* Start the McBSP hardware                                           */
        if (IOM_INPUT == chanHandle->mode)
        {
            instHandle->stopSmFsRcv = FALSE;
            Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_RX_ENABLE);
        }
        else
        {
            instHandle->stopSmFsXmt = FALSE;
            Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_ENABLE);
        }

        if (((IOM_INPUT == chanHandle->mode) &&
            (TRUE == instHandle->rxFsgEnable)) ||
            ((IOM_OUTPUT == chanHandle->mode) &&
             (TRUE == instHandle->txFsgEnable)))
        {
            /* enable the sample rate generator                               */
            Mcbsp_localResetCtrl(
                instHandle,
                Mcbsp_SpcrCtrl_FSYNC_ENABLE);
        }
#endif /* Mcbsp_LOOPJOB_ENABLE */
    }
}

/*============================================================================*/
/*                             PWRM FUNCTIONS                                 */
/*============================================================================*/

/**
 *  \brief    Mcbsp_localLpscOn
 *
 *            This function is invoked to power on the Mcbsp peripheral and
 *            configure it according to the operating mode. Note that this
 *            function uses different functions to power ON the module,
 *            depending on whether PWRM is enabled in this driver or NOT
 *
 *  \param    instHandle  [IN] Handle to the device instance
 *
 *  \return   IOM_COMPLETED in case of success, an ERROR code in case of error.
 *
 *  \enter    instHandle    must be a valid pointer
 *
 *  \leave    Not Implemented.
 */
Int32 Mcbsp_localLpscOn(Mcbsp_Object *instHandle)
{
    Int32  status = IOM_COMPLETED;

    assert(NULL != instHandle);

    if (TRUE != instHandle->prcmPwrmEnable)
    {
#ifdef BIOS_PWRM_ENABLE
        /* power on using bios PWRM API                                       */
        status = (Int32)PWRM_setDependency(
                            (PWRM_Resource)instHandle->hwInfo.pwrmLpscId);
#if 0
            /* power on using PRCM API                                         	*/
            /* This is hard coded for McBSP1									*/
            status = Prcm_ModuleClkCtrl(
                         instHandle->hwInfo.prcmInstance,
                         (Uint32)instHandle->hwInfo.pwrmPrcmId,
                         TRUE);
#endif
#else
        if (0 == instHandle->instNum)
        {
            /* We are going to hard-code for mcbsp1								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN1_CORE, EN_MCBSP1_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            Prcm_ModuleClkCtrl(CM_ICLKEN1_CORE, EN_MCBSP1_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);
        }

        if (1 == instHandle->instNum)
        {
            /* We are going to hard-code for mcbsp2								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP2_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP2_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);
         }

        if (2 == instHandle->instNum)
        {
            /* We are going to hard-code for mcbsp4								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP4_FCLK_BIT, Prcm_moduleClkCtrl_ENABLE);
            Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP4_ICLK_BIT, Prcm_moduleClkCtrl_ENABLE);
        }

        instHandle->prcmPwrmEnable = TRUE;
#endif
    }
    return (status);
}

/**
 * \brief    Mcbsp_localLpscOff
 *
 *           This function is invoked to power off the module.
 *
 * \param    instHandle  [IN] Handle to the device instance
 *
 * \return   IOM_COMPLETED in case of success, an ERROR code in case of error.
 *
 * \enter    instHandle must be a valid channel object
 *           No pending IO
 *
 * \leave    Not Implemented.
 */
Int32 Mcbsp_localLpscOff(Mcbsp_Object *instHandle)
{
    Int32  status = IOM_COMPLETED;

    assert(NULL != instHandle);

    if (TRUE == instHandle->prcmPwrmEnable)
    {
#ifdef BIOS_PWRM_ENABLE
        /* power off  using PWRM API                                          */
        status = (Int32)PWRM_releaseDependency(
                    (PWRM_Resource)instHandle->hwInfo.pwrmLpscId);
#if 0
            /* power on using PRCM API                                         	*/
            /* This is hard coded for McBSP1									*/
            status = Prcm_ModuleClkCtrl(
                         instHandle->hwInfo.prcmInstance,
                         (Uint32)instHandle->hwInfo.pwrmPrcmId,
                         TRUE);
#endif
#else
        if (0 == instHandle->instNum)
        {
            /* We are going to hard-code for mcbsp1								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN1_CORE, EN_MCBSP1_FCLK_BIT, Prcm_moduleClkCtrl_DISABLE);
       	    Prcm_ModuleClkCtrl(CM_ICLKEN1_CORE, EN_MCBSP1_ICLK_BIT, Prcm_moduleClkCtrl_DISABLE);
        }

        if (1 == instHandle->instNum)
        {
            /* We are going to hard-code for mcbsp2								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP2_FCLK_BIT, Prcm_moduleClkCtrl_DISABLE);
       	    Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP2_ICLK_BIT, Prcm_moduleClkCtrl_DISABLE);
        }

        if (2 == instHandle->instNum)
        {
            /* We are going to hard-code for mcbsp4								*/
            Prcm_ModuleClkCtrl(CM_FCLKEN_PER, EN_MCBSP4_FCLK_BIT, Prcm_moduleClkCtrl_DISABLE);
       	    Prcm_ModuleClkCtrl(CM_ICLKEN_PER, EN_MCBSP4_ICLK_BIT, Prcm_moduleClkCtrl_DISABLE);
        }

        instHandle->prcmPwrmEnable = FALSE;
#endif
    }
    return (status);
}

#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
/**
 * \brief     This function restores the driver to the original state that is it
 *            resumes the normal operation of the driver by picking the IOPs
 *            from the pending queue and putting it to the active IOP.
 *
 * \param     instHandle [IN] Handle to the device instance object
 *
 * \enter     instHandle is a valid non NULL pointer
 *
 * \leave     Not impplemented
 */
static Int32 McbspLoadPendedIops(Mcbsp_ChannelObj *chanHandle)
{
    IOM_Packet           *ioPacket        = NULL;
    Int32                 status          = IOM_COMPLETED;
    Uint32                count           = 0x00;
    Uint32                hwiKey          = 0x00;
    Uint32                tempSubmitCount = 0x00;

    assert(NULL != chanHandle);

    /* reset the submit count to indicate we are loading the first packet     */
    hwiKey = (Uint32)_disable_interrupts();

    tempSubmitCount = chanHandle->submitCount;
    chanHandle->submitCount = 0x00;

    for (count = 0; count < (Mcbsp_MAXLINKCNT + 1); count++)
    {
        /* start the critical section                                         */
        if (TRUE == QUE_empty(&chanHandle->queuePendingList))
        {
            break;
        }
        else
        {
            chanHandle->submitCount++;

            /* get the packet out of the pending queue and load it to the     *
             * active Queue                                                   */
            ioPacket = QUE_get(&(chanHandle->queuePendingList));

            assert(NULL != ioPacket);

            /* put the packet in to the active queue                          */
            QUE_put(&(chanHandle->queueFloatingList),(Ptr)ioPacket);

            /* load the packet to the edma                                    */
            Mcbsp_localLoadPktToEdma(chanHandle,ioPacket);
        }
    }

    /* restore the submit count                                               */
    chanHandle->submitCount = tempSubmitCount;
    _restore_interrupts(hwiKey);

    return status;
}

/**
 * \brief     This function registers with the BIOS PWRM module for all the
 *            possible power management events.
 *
 * \param     instHandle [IN] Handle to the driver instance object
 *
 * \return    None
 *
 * \enter     instHandle is a NON null valid pointer
 *
 * \leave     Not implemented
 */
static Int32 McbspRegisterNotification(Mcbsp_Object *instHandle)
{
    Uint32       eventCnt = 0x00;
    PWRM_Status  retVal   = PWRM_SOK;
    Int32        status   = IOM_COMPLETED;

    assert(NULL != instHandle);

    /* register for the events to be notified by the driver                   */
    for (eventCnt = 0; eventCnt < Mcbsp_MAX_PWRM_EVENTS; eventCnt++)
    {
        /* register the notify function for the PWRM event                    */
        retVal = PWRM_registerNotify(
                    (PWRM_Event)eventCnt,
                    NULL,
                    &McbspNotifyCallback,
                    (Arg)instHandle,
                    &(instHandle->pwrmInfo.notifyHandle[eventCnt]),
                    &(instHandle->pwrmInfo.delayedCompletionFxn[eventCnt]));

        if (PWRM_SOK != retVal)
        {
            status = IOM_EBADARGS;
            break;
        }
    }
    return (status);
}

/**
 * \brief     This function is the notify function called by the BIOS whenever
 *            a power event occurs in the system.This function handles the
 *            power events and calls the appropriate functions to process the
 *            same.
 *
 * \param     eventType [IN] power event type
 * \param     eventArg1 [IN] event-specific argument
 * \param     eventArg2 [IN] event-specific argument
 * \param     clientArg [IN] argument passed by the PWRM module(usually
 *                          (supplied during the registration of the notifyFxn)
 *
 * \return    None
 *
 * \enter     eventType is a valid event
 *
 * \leave     Not implemented
 */
static PWRM_NotifyResponse McbspNotifyCallback(PWRM_Event    eventType,
                                               Arg           eventArg1,
                                               Arg           eventArg2,
                                               Arg           clientArg)
{
    Mcbsp_Object          *instHandle = NULL;
    PWRM_NotifyResponse    pwrmStatus = PWRM_NOTIFYDONE;
    PWRM_Status            status     = PWRM_SOK;

    assert(NULL != clientArg);

    eventArg1 = eventArg1;
    eventArg2 = eventArg2;

    instHandle = (Mcbsp_Object *)clientArg;
    assert(NULL != instHandle);

    /* check the event notified by the PWRM module and process accordingly    */
    switch (eventType)
    {
        case PWRM_GOINGTOSLEEP:
        case PWRM_GOINGTOSTANDBY:
        case PWRM_GOINGTODEEPSLEEP:
            pwrmStatus = McbspSuspendCurrentIops(instHandle,eventType);
            break;
        case PWRM_PENDING_CPU_SETPOINTCHANGE:
        case PWRM_PENDING_PER_SETPOINTCHANGE:
            /* CPU or PERIPHERAL domain V/F setpoint or a sleep command is    *
             * issued hence we will complete the current IO and suspend all   *
             * the pending IOs                                                */
            if (((PWRM_PENDING_CPU_SETPOINTCHANGE == eventType) &&
                     (Mcbsp_pllDomain_0 == instHandle->pwrmInfo.pllDomain)) ||
                ((PWRM_PENDING_PER_SETPOINTCHANGE == eventType) &&
                     (Mcbsp_pllDomain_1 == instHandle->pwrmInfo.pllDomain)))
            {
                pwrmStatus = McbspSuspendCurrentIops(instHandle,eventType);
            }
            break;
        case PWRM_DONE_CPU_SETPOINTCHANGE:
        case PWRM_DONE_PER_SETPOINTCHANGE:
            /* The pending CPU setpoint change has now been made.Hence now we *
             * will recalculate the new settings for this instance and then   *
             * restore the pending state of the driver                        */
            if (((PWRM_DONE_CPU_SETPOINTCHANGE == eventType) &&
                     (Mcbsp_pllDomain_0 == instHandle->pwrmInfo.pllDomain)) ||
               ((PWRM_DONE_PER_SETPOINTCHANGE == eventType) &&
                      (Mcbsp_pllDomain_1 == instHandle->pwrmInfo.pllDomain)))
            {
                pwrmStatus = McbspConfigureNewVfSettings(
                                 instHandle,
                                 (Uns)eventArg1,
                                 (Uns)eventArg2);
            }

            if (PWRM_NOTIFYDONE == pwrmStatus)
            {
                McbspLoadPendedIops(&instHandle->xmtObj);
                McbspLoadPendedIops(&instHandle->rcvObj);
                instHandle->devState = Mcbsp_DriverState_CREATED;
            }
            break;
        case PWRM_AWAKEFROMSLEEP:
        case PWRM_AWAKEFROMDEEPSLEEP:
            /* restore the dependency count                                   */
            while (instHandle->pwrmInfo.dependencyCount > 0)
            {
                status = PWRM_setDependency(
                             (PWRM_Resource)instHandle->hwInfo.pwrmLpscId);

                if (PWRM_SOK != status)
                {
                    break;
                }
                instHandle->pwrmInfo.dependencyCount--;
            }

            if (PWRM_SOK == status)
            {
                McbspLoadPendedIops(&instHandle->xmtObj);
                McbspLoadPendedIops(&instHandle->rcvObj);
                instHandle->devState = Mcbsp_DriverState_CREATED;
            }
            else
            {
                pwrmStatus = PWRM_NOTIFYNOTDONE;
            }
            break;
        case PWRM_AWAKEFROMSTANDBY:
            McbspLoadPendedIops(&instHandle->xmtObj);
            McbspLoadPendedIops(&instHandle->rcvObj);
            instHandle->devState = Mcbsp_DriverState_CREATED;
            break;
        default:
            break;
    }
    return (pwrmStatus);
}

/**
 * \brief     This function calculates the constraints for a device at all
 *            the setpoints supported for the SOC. it then registers the
 *            appropriate contraints for each of the setpoints not supported.
 *
 * \param     instHandle [IN] Handle to the driver instance object
 *
 * \return    IOM_COMPLETED in case of success
 *            IOM_EBADMODE  in case of error
 *
 * \enter     instHandle is a NON null valid pointer
 */
static Int32 McbspCalculateConstraints(Mcbsp_Object     *instHandle,
                                       Mcbsp_ChannelObj *chanHandle)
{
    Uns               numberSetpoints = 0x00;
    Uns               setpoint        = 0x00;
    Uint32            clkgDiv         = 0x00;
    Uint32            framePeriod     = 0x00;
    Uint32            mask            = 0x00;
    Int32             status          = IOM_COMPLETED;
    PWRM_Constraint   constraint      = PWRM_DISALLOWEDSLEEPSTATE_MASK;
    PWRM_Status       retVal          = PWRM_SOK;
    PWRM_Domain       domain          = PWRM_CPU;

    assert((NULL != instHandle) && (NULL != chanHandle));

    /* check what clock domain is supported by the device                     */
    if (Mcbsp_pllDomain_0 == instHandle->pwrmInfo.pllDomain)
    {
        domain = PWRM_CPU;
        constraint = PWRM_DISALLOWED_CPU_SETPOINT_MASK;
    }
    else if (Mcbsp_pllDomain_1 == instHandle->pwrmInfo.pllDomain)
    {
        domain = PWRM_PER;
        constraint = PWRM_DISALLOWED_PER_SETPOINT_MASK;
    }
    else
    {
        /* do nothing                                                         */
    }

    /* get the number of supported setpoint info for this instance            */
    retVal = PWRM_getNumSetpoints(domain, &numberSetpoints);

    if (PWRM_SOK == retVal)
    {
        /* calculate the contraints for the required bus frequency            */
        for (setpoint = 0x00; setpoint < numberSetpoints; setpoint++)
        {
            status = McbspCalculateClockSettings(
                          instHandle,
                          chanHandle,
                          setpoint,
                          &clkgDiv,
                          &framePeriod);

            if (IOM_COMPLETED == status)
            {
                /* successfully calculated now check if the prescaler is OK   */
                if ((0xFF < clkgDiv) || (0xFFF < framePeriod))
                {
                    /* register the constraint for this setpoint              */
                    mask |= (1u << setpoint);
                }
            }
            else
            {
                retVal = (Uns)PWRM_EINVALIDVALUE;
                break;
            }
        }

        /* check if this instance has already registered some constraint      *
         * if so then unregister those before registering the new             *
         * contraints(for CPU or PER event's unsupported DVFS only)           */
        if (PWRM_SOK == retVal)
        {
            if (NULL != instHandle->pwrmInfo.constraintHandle[constraint - 1])
            {
                retVal = PWRM_unregisterConstraint(
                              instHandle->pwrmInfo.constraintHandle[constraint - 1]);
            }

            /* register the new set of constraints (only if any contraints    *
             * are present)                                                   */
            if ((PWRM_SOK == retVal) && (0 != mask))
            {
                /* store the value of the constraint mask being registered    */
                instHandle->pwrmInfo.constraintMask = mask;

                retVal = PWRM_registerConstraint(
                             constraint,
                             mask,
                             &instHandle->pwrmInfo.constraintHandle[constraint - 1]);
            }
        }
    }

    if (PWRM_SOK != retVal)
    {
        /* failed in calculating the constraints                              */
        status = IOM_EBADMODE;
    }
    return (status);
}

/**
 * \brief      This function calculates clock settings for the new DVFS set
 *             point.
 *
 * \param      instHandle [IN] Handle to the device instance object
 * \param      eventArg1  [IN] previous set point.
 * \param      eventArg2  [IN] current set point
 *
 * \return     PWRM_SOK incase that the prescaler values for the new set point
 *             are calculated successfully else an appropriate error code is
 *             returned
 *
 * \enter      instHandle is a valid non NULL pointer
 *
 * \leave      Not imeplemented
 */
static PWRM_NotifyResponse McbspConfigureNewVfSettings(Mcbsp_Object *instHandle,
                                                       Uns           eventArg1,
                                                       Uns           eventArg2)
{
    Mcbsp_ChannelObj    *chanHandle  = NULL;
    PWRM_NotifyResponse  retVal      = PWRM_NOTIFYDONE;
    Int32                status      = IOM_COMPLETED;

    /* This function will calculate the settings for the new set point and    *
     * also indicate if this is a valid setpoint                              */
    assert(NULL != instHandle);

    /* check if both the set points are not the same                          */
    if (eventArg1 != eventArg2)
    {
        if (Mcbsp_DriverState_OPENED == instHandle->rcvObj.chanState)
        {
            chanHandle = &instHandle->rcvObj;
        }
        else if (Mcbsp_DriverState_OPENED == instHandle->xmtObj.chanState)
        {
            chanHandle = &instHandle->xmtObj;
        }
        else
        {
            /* do nothing                                                     */
        }

        assert(NULL != chanHandle);

        if (IOM_COMPLETED == Mcbsp_localLpscOn(instHandle))
        {
            /* reset the flag so that the SRGR can be reconfigured for new    *
             * settings                                                       */
            instHandle->srgConfigured = FALSE;

            /* stop the sample rate generator                                 */
            if (TRUE == instHandle->srgEnabled)
            {
                Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_SRG_DISABLE);

                status = Mcbsp_localConfigureSrgr(instHandle,chanHandle);

                if (IOM_COMPLETED == status)
                {
                    /* enable the sample rate generator                       */
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_SRG_ENABLE);

                    /* wait for the 2CLKG clock cycles.But the minumum time   *
                     * provided by the bios is 1ms.                           */
                    TSK_sleep(1u);
                }

                /* clear the XSYNCERR (to be done only if TX is used)         */
                if (IOM_OUTPUT == chanHandle->mode)
                {
                    /* Enable the TX section                                  */
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_ENABLE);

                    /* wait for 2 CLKR or CLX cycles.But the minumum time     *
                     * provided by the bios is 1ms.                           */
                    TSK_sleep(1u);

                    /* Disable the TX section to clear any XYNCERR            */
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_TX_DISABLE);
                }
            }
            status |= Mcbsp_localLpscOff(instHandle);
        }
    }

    if (IOM_COMPLETED != status)
    {
        retVal = PWRM_NOTIFYERROR;
    }
    return (retVal);
}

/**
 * \brief     This function calculates the required prescaler values
 *            for the required ouput frequency for a given setpoint.
 *
 * \param     instHandle [IN]  Handle to the driver instance object
 * \param     setpoint   [IN]  setpoint for which the prescaler is calculated.
 * \param     opFreq     [IN]  desired module IO frequency
 * \param     prescale   [OUT] calculated prescaler value.
 *
 * \return    IOM_COMPLETED if the calculation is completed.
 *            IOM_EBADMODE  if the calcualtion failed for some reason.
 *
 * \enter     instHandle is a valid non null pointer
 *            prescalar is a non null pointer.
 *
 * \leave     Not imeplemented
 *
 * \note      Please note that the calculated prescaler values are not validated
 *            in this function.
 */
static Int32 McbspCalculateClockSettings(Mcbsp_Object      *instHandle,
                                         Mcbsp_ChannelObj  *chanHandle,
                                         Uint32             setpoint,
                                         Uint32            *clkgDiv,
                                         Uint32            *framePeriod)
{
    PWRM_Status     retVal          = PWRM_SOK;
    PWRM_Domain     domain          = PWRM_CPU;
    Uns             numberSetpoints = 0x00;
    Uns             frequency       = 0x00;
    Uns             voltage         = 0x00;
    Uint32          sysclkDiv       = 0x00;
    Uint32          noOfBits        = 0x00;
    Int32           status          = IOM_COMPLETED;

    assert((NULL != instHandle) && (NULL != clkgDiv) &&
        (NULL != framePeriod) && (NULL != chanHandle));

    /* check what clock domain is supported by the device                     */
    if (Mcbsp_pllDomain_0 == instHandle->pwrmInfo.pllDomain)
    {
        domain = PWRM_CPU;
    }
    else if (Mcbsp_pllDomain_1 == instHandle->pwrmInfo.pllDomain)
    {
        domain = PWRM_PER;

    }
    else
    {
        /* do nothing                                                         */
    }

    /* get the number of supported setpoint info for this instance            */
    retVal = PWRM_getNumSetpoints(domain, &numberSetpoints);

    if (PWRM_SOK == retVal)
    {
        /* calculate the sysclk divisor value                                 */
        retVal = PWRM_getSetpointInfo(
                     domain,
                     (numberSetpoints - 1u),
                     &frequency,
                     &voltage);

        if (PWRM_SOK == retVal)
        {
            /* calculate the sysclk divisor value                             */
            sysclkDiv = ((frequency * 1000u)
                             / instHandle->hwInfo.moduleClkFreq);

            /* get the information about the current set point                */
            retVal = PWRM_getSetpointInfo(
                         domain,
                         setpoint,
                         &frequency,
                         &voltage);

            if (PWRM_SOK == retVal)
            {
                /* calculate and set the CLKG div values                      */
                if (Mcbsp_Phase_DUAL == chanHandle->chanConfig.phaseNum)
                {
                    noOfBits = ((chanHandle->chanConfig.wrdLen1
                                    * chanHandle->chanConfig.frmLen1)
                                + (chanHandle->chanConfig.wrdLen2
                                    * chanHandle->chanConfig.frmLen2));
                }
                else
                {
                    noOfBits  = (chanHandle->chanConfig.wrdLen1
                                    * chanHandle->chanConfig.frmLen1);
                }

                (*clkgDiv) = ((((frequency * 1000u)/sysclkDiv)/
                                (chanHandle->clkSetup.samplingRate * noOfBits))
                                - 1u);

                (*framePeriod) = (noOfBits - 1u);
            }
        }
    }

    if (PWRM_SOK != retVal)
    {
        status = IOM_EBADMODE;
    }
    return (status);
}

/**
 * \brief    This function configures the driver so that the current IOP can be
 *           completed and then supends all the remaining IOPs.
 *
 * \param    instHandle [IN] Handle to the device instance object
 * \param    eventType  [IN] event which has caused this notification call to
 *                           driver
 *
 * \return   PWRM_NOTIFYDONE    if no IOPs are pending
 *           PWRM_NOTIFYNOTDONE if IOPs are pending in the driver.
 */
static PWRM_NotifyResponse McbspSuspendCurrentIops(Mcbsp_Object  *instHandle,
                                                   PWRM_Event     eventType)
{
    Uint32               hwiKey = 0x00;
    PWRM_NotifyResponse  status = PWRM_NOTIFYDONE;

    assert(NULL != instHandle);

    hwiKey = (Uint32)_disable_interrupts();

    if ((TRUE == QUE_empty(&instHandle->xmtObj.queueFloatingList)) &&
        (TRUE == QUE_empty(&instHandle->rcvObj.queueFloatingList)))
    {
        /* No IOP is pending currently. we can return the status of the       *
         * notify call as sucess                                              */
        instHandle->devState = Mcbsp_DriverState_PWRM_SUSPEND;
        status = PWRM_NOTIFYDONE;
    }
    else
    {
        /* There are some IOPs pending,hence we will complete processing      *
         * of this request in the ISR context(task in case of polled mode)    *
         * by calling the delayed completion function                         */
        status = PWRM_NOTIFYNOTDONE;
        instHandle->pwrmInfo.ioSuspend = TRUE;
        instHandle->pwrmInfo.pwrmEvent = eventType;
    }

    _restore_interrupts(hwiKey);
    return (status);
}


/**
 * \brief     This function unregisters all the power management events
 *            which were previously registered by the driver instance.
 *
 * \param     instHandle [IN] Handle to the driver instance object
 *
 * \return    IOM_COMPLETED in case of success
 *            IOM_EBADARGS  in case of error
 *
 * \enter     instHandle is a NON null valid pointer
 *
 * \leave     Not implemented
 */
static Int32 McbspUnregisterNotification(Mcbsp_Object *instHandle)
{
    Uint32       eventCnt = 0x00;
    PWRM_Status  retVal   = PWRM_SOK;
    Int32        status   = IOM_COMPLETED;

    assert(NULL != instHandle);

    for (eventCnt = 0; eventCnt < Mcbsp_MAX_PWRM_EVENTS; eventCnt++)
    {
        /* register the notify function for the PWRM event                    */
        if (NULL != (instHandle->pwrmInfo.notifyHandle[eventCnt]))
        {
            retVal = PWRM_unregisterNotify(
                        (instHandle->pwrmInfo.notifyHandle[eventCnt]));

            if (PWRM_SOK != retVal)
            {
                status = IOM_EBADARGS;
                break;
            }
            else
            {
                instHandle->pwrmInfo.notifyHandle[eventCnt] = 0x00;
            }
        }
    }
    return (status);
}

/**
 * \brief     This function unregisters all the constraints for the device
 *            instance.
 *
 * \param     instHandle [IN]  Handle to the driver instance object
 *
 * \return    IOM_COMPLETED in case of success
 *            IOM_EBADMODE in case of failure
 *
 * \enter     instHandle is a valid non null pointer
 *
 * \leave     Not implemented
 */
static Int32 McbspUnregisterConstraints(Mcbsp_Object *instHandle)
{
    Int32        status = IOM_COMPLETED;
    PWRM_Status  retVal = PWRM_SOK;
    Uint32       count  = 0x00;

    assert(NULL != instHandle);

    for (count = 0; count < Mcbsp_MAX_PWRM_CONSTRAINTS; count++)
    {
        if (NULL != instHandle->pwrmInfo.constraintHandle[count])
        {
            retVal = PWRM_unregisterConstraint(
                         instHandle->pwrmInfo.constraintHandle[count]);

            if (PWRM_SOK != retVal)
            {
                status = IOM_EBADMODE;
                break;
            }
            else
            {
                instHandle->pwrmInfo.constraintHandle[count] = 0x00;
            }
        }
    }
    return (status);
}

#endif /* #ifdef BIOS_PWRM_ENABLE */


/**
 * \brief   Function to modify the sample rate generator configuration
 *
 * \param   chanHandle  [IN]  Handle to the channel
 * \param   arg         [IN]  pointer to the srg config setup
 *
 * \return  None
 */
Int32 Mcbsp_localModifySampleRate(Mcbsp_ChannelObj *chanHandle,Ptr arg)
{
    Mcbsp_Object    *instHandle = NULL;
    Mcbsp_ClkSetup  *clkConfig  = NULL;
    Int32            status     = IOM_COMPLETED;

    assert((NULL != arg) && (NULL != chanHandle));

    instHandle = chanHandle->devHandle;
    assert(NULL != instHandle);

    clkConfig = (Mcbsp_ClkSetup *)arg;

    /* check if the Frame sync clock is generated by the module               */
    if (TRUE == instHandle->srgConfigured)
    {
        /* Configure the McBSP with user supplied parameters                  */
        chanHandle->clkSetup = *(clkConfig);
#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
        /* unregister the old constraints                                     */
        status = McbspUnregisterConstraints(instHandle);

        /* calculate the new constraints                                      */
        if (IOM_COMPLETED == status)
        {
            status = McbspCalculateConstraints(instHandle,chanHandle);

            if (IOM_COMPLETED == status)
            {
#endif
                instHandle->srgConfigured = FALSE;

                /* stop the sample rate generator                             */
                if (TRUE == instHandle->srgEnabled)
                {
                    Mcbsp_localResetCtrl(instHandle,Mcbsp_SpcrCtrl_SRG_DISABLE);

                    status = Mcbsp_localConfigureSrgr(instHandle,chanHandle);

                    if (IOM_COMPLETED == status)
                    {
                        /* enable the sample rate generator                   */
                        Mcbsp_localResetCtrl(
                            instHandle,
                            Mcbsp_SpcrCtrl_SRG_ENABLE);

                        /* wait for the 2CLKG clock cycles.But the minumum    *
                         * time provided by the bios is 1ms.                  */
                        TSK_sleep(1u);
                    }

                    /* clear the XSYNCERR (to be done only if TX is used)     */
                    if (IOM_OUTPUT == chanHandle->mode)
                    {
                        /* Enable the TX section                              */
                        Mcbsp_localResetCtrl(
                            instHandle,
                            Mcbsp_SpcrCtrl_TX_ENABLE);

                        /* wait for 2 CLKR or CLX cycles.But the minumum time *
                         * provided by the bios is 1ms.                       */
                        TSK_sleep(1u);

                        /* Disable the TX section to clear any XYNCERR        */
                        Mcbsp_localResetCtrl(
                            instHandle,
                            Mcbsp_SpcrCtrl_TX_DISABLE);
                    }
                }
#if defined(BIOS_PWRM_ENABLE) && !defined(Mcbsp_LOOPJOB_ENABLE)
            }
        }
#endif
    }
    else
    {
        status = IOM_EBADMODE;
        /* New update from Sandeep */
        if (IOM_INPUT == chanHandle->mode)
        {
            if (TRUE != instHandle->rxSrgEnable)
            {
	            /* Since this mode does not use the sample rate generator, so
	             * there is no point in changing the sample rate. So return
	             * with success*/
				status	= IOM_COMPLETED;
            }
        }
        else
        {
            if (TRUE != instHandle->txSrgEnable)
            {
	            /* Since this mode does not use the sample rate generator, so
	             * there is no point in changing the sample rate. So return
	             * with success*/
	            status	= IOM_COMPLETED;
            }
        }
    }
    return (status);
}

/* ========================================================================== */
/*                             END OF FILE                                    */
/* ========================================================================== */

