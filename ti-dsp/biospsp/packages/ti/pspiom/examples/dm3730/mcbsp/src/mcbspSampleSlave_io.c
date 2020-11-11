/*
 * mcbspSampleSlave_io.c
 *
 * This file contains the test / demo code to demonstrate the McBSP driver
 * slave functionality on DSP/BIOS 5.
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
 * \file    mcbspSampleSlave_io.c
 *
 * \brief   McBSP evm to evm communication sample application
 *
 *          This file demonstrates the use of Mcbsp is by using an
 *          EVM to EVM communication setup.This file configures one of the evm
 *          in slave mode.
 *
 *          (C) Copyright 2009, Texas Instruments, Inc
 *
 * \author  PSP
 *
 * \version 0.1
 */

/* ========================================================================== */
/*                            INCLUDE FILES                                   */
/* ========================================================================== */

#include <std.h>
#include <stdio.h>
#include <sio.h>
#include <iom.h>
#include <log.h>
#include <tsk.h>
#include <string.h>
#include <ti/sdo/edma3/drv/edma3_drv.h>
#include <ti/pspiom/mcbsp/Mcbsp.h>
#include <ti/pspiom/cslr/cslr.h>
#include <ti/pspiom/cslr/tistdtypes.h>
//#define SEND_PATTERN

/* ========================================================================== */
/*                        IMPORTED VARIABLES                                  */
/* ========================================================================== */

extern LOG_Obj trace;
extern EDMA3_DRV_Handle hEdma[];

/* ========================================================================== */
/*                        EXTERNAL FUNCTIONS                                  */
/* ========================================================================== */

extern Int edma3init();

/* Pre-generated sine wave data, 16-bit signed samples */
/*static Int16 sinetable[48] = {
   0x0000, 0x10b4, 0x2120, 0x30fb, 0x3fff, 0x4dea, 0x5a81, 0x658b,
   0x6ed8, 0x763f, 0x7ba1, 0x7ee5, 0x7ffd, 0x7ee5, 0x7ba1, 0x763f,
   0x6ed8, 0x658b, 0x5a81, 0x4dea, 0x3fff, 0x30fb, 0x2120, 0x10b4,
   0x0000, 0xef4c, 0xdee0, 0xcf06, 0xc002, 0xb216, 0xa57f, 0x9a75,
   0x9128, 0x89c1, 0x845f, 0x811b, 0x8002, 0x811b, 0x845f, 0x89c1,
   0x9128, 0x9a76, 0xa57f, 0xb216, 0xc002, 0xcf06, 0xdee0, 0xef4c*//*
   0x0000, 0x10b4, 0x2120, 0x30fb, 0x3fff, 0x4dea, 0x5a81, 0x658b,
   0x6ed8, 0x763f, 0x7ba1, 0x7ee5, 0x7ffd, 0x7ee5, 0x7ba1, 0x763f,
   0x6ed8, 0x658b, 0x5a81, 0x4dea, 0x3fff, 0x30fb, 0x2120, 0x10b4,
   0x0000, 0xef4c, 0xdee0, 0xcf06, 0xc002, 0xb216, 0xa57f, 0x9a75,
   0x9128, 0x89c1, 0x845f, 0x811b, 0x8002, 0x811b, 0x845f, 0x89c1,
   0x9128, 0x9a76, 0xa57f, 0xb216, 0xc002, 0xcf06, 0xdee0, 0xef4c*/
//};

/* ========================================================================== */
/*                         MACRO DEFINITIONS                                  */
/* ========================================================================== */

#define NUM_BUFS              2             /* Max of 1 outstanding requests - PingPong  */
#define BUFALIGN              128           /* align buffers to 128 bytes     */
#define BUFLEN				  64
#define NUM_TX_CHANNELS      	2             /* Number of slots to be used     */
#define NUM_RX_CHANNELS			6
#define MAX_TX_CHANNELS			6
#define TX_BUFSIZE              (BUFLEN*NUM_TX_CHANNELS*2)         /* 1K of data transceive          */
#define RX_BUFSIZE				(BUFLEN*NUM_RX_CHANNELS*2)

#define DEFAULT_BITPERSAMPLE  16             /* number of bits per slot        */

/* ========================================================================== */
/*                          LOCAL VARIABLES                                   */
/* ========================================================================== */

/* Handles for the TX channels                            */
static SIO_Handle mcbspOutHandle = NULL;

/* Handles for the RX channels                            */
static SIO_Handle mcbspInHandle = NULL;

/* array to hold the pointer to the allocated buffers     */
Ptr buf[NUM_BUFS*2];

Uint32 pktCount = 0;

/**< settings to configure the TX or RX hardware sections                 */
Mcbsp_DataConfig mcbspChanConfig[2] =
{
		/* RX channel setup */
		{
    Mcbsp_Phase_SINGLE,
    Mcbsp_WordLength_16,
    Mcbsp_WordLength_16,    /* Dont care for single phase*/
    NUM_RX_CHANNELS,
    NUM_RX_CHANNELS,        /* Dont care                */
    Mcbsp_FrmSync_DETECT,
    Mcbsp_DataDelay_1_BIT,
    Mcbsp_BitReversal_DISABLE,
    Mcbsp_IntMode_ON_SYNCERR,
    Mcbsp_RxJust_RZF,    /* Dont care for TX         */
    Mcbsp_DxEna_OFF
		},
		/* TX channel setup */
		{
    Mcbsp_Phase_SINGLE,
    Mcbsp_WordLength_16,
    Mcbsp_WordLength_16,    /* Dont care for single phase*/
    NUM_TX_CHANNELS,
    NUM_TX_CHANNELS,        /* Dont care                */
    Mcbsp_FrmSync_DETECT,
    Mcbsp_DataDelay_1_BIT,
    Mcbsp_BitReversal_DISABLE,
    Mcbsp_IntMode_ON_SYNCERR,
    Mcbsp_RxJust_RZF,    /* Dont care for TX         */
    Mcbsp_DxEna_OFF
		}
};

/**< clock setup for the TX section                     */
Mcbsp_ClkSetup mcbspClkConfig[2] =
{
		/* RX clk setup */
		{
    Mcbsp_FsClkMode_EXTERNAL,
    16000,                   /* 96KHz                   */
    Mcbsp_TxRxClkMode_EXTERNAL,
    Mcbsp_FsPol_ACTIVE_HIGH,
    Mcbsp_ClkPol_RISING_EDGE
		},
		/* TX clk setup */
		{
    Mcbsp_FsClkMode_EXTERNAL,
    16000,                   /* 96KHz                   */
    Mcbsp_TxRxClkMode_EXTERNAL,
    Mcbsp_FsPol_ACTIVE_HIGH,
    Mcbsp_ClkPol_FALLING_EDGE
		}
};

/**< Multi channel setup                                                  */
Mcbsp_McrSetup mcbspMultiChanCtrl =
{
    Mcbsp_McmMode_ALL_CHAN_ENABLED_UNMASKED,
    Mcbsp_PartitionMode_CHAN_0_15,
    Mcbsp_PartitionMode_CHAN_16_31,
    Mcbsp_PartitionMode_2
};

Mcbsp_ChanParams mcbspChanparam =
{
		/* TX channel param	*/
    Mcbsp_WordLength_16,   /* wordlength configured    */
    NULL,                 /* loop job buffer internal */
    0,                    /* user loopjob length      */
    NULL,                 /* global error callback    */
    NULL,                 /* edma Handle              */
    1,                    /* EDMA event queue         */
    8,                    /* hwi number               */
    Mcbsp_BufferFormat_1SLOT,
    FALSE,                 /* FIFO mode enabled        */
    &mcbspChanConfig[1],     /* channel tx by-default configuration    */
    &mcbspClkConfig[1],      /* clock configuration      */
    &mcbspMultiChanCtrl,  /* multi channel control    */
    0x00,
    0x00,
    0x00,
    0x00
};

/* ========================================================================== */
/*                             LOCAL FUNCTIONS                                */
/* ========================================================================== */

static Void mcbspCreateStreams(Void);
static Void mcbspDriverPrime(Void);

/* ========================================================================== */
/*                           FUNCTION DEFINTIONS                              */
/* ========================================================================== */

/* loopData to re-arrange tx EDMA buffers to 2 channels only */
void loopData(Int16 * destBuf, Int16 * srcBuf, Int16 num_dstCh, Int16 num_srcCh, Int16 bufLen)
{
    Int16       *dest = destBuf;
    Int16 		*src = srcBuf;
    Int16		bufCount = 0;
    Int16 		chCount = 0;

    for (bufCount = 0; bufCount < bufLen; bufCount++)
    {
    	for (chCount = 0; chCount < num_dstCh; chCount++)
    	{
        	*dest++ = *src++;
    	}
    	src = src + (num_srcCh-num_dstCh);
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
Void mcbspStartDemo(Void)
{
    Ptr                xmt       = NULL;
    Ptr                rcv       = NULL;
    Int32              nmadus0   = 0;

    /* create the streams required for the transactions                       */
    mcbspCreateStreams();

    /* prime the driver                                                       */
    mcbspDriverPrime();

    /* reclaim each packet and resubmit for "LOOP_COUNT" number of iteration  */
    /* Kayjin for (count = 0; count < LOOP_COUNT; count++)					***/
    while(1)
    {
#if 1
        /* Reclaim empty buffer from the input stream to be reused           */
        nmadus0 = SIO_reclaim(mcbspInHandle,(Ptr *)&rcv,NULL);

        if (nmadus0 < 0)
        {
            LOG_printf(&trace, "Error reclaiming empty buffer from the streams %x %d",
                *(Uint32 *)rcv,
                nmadus0);
        }
#endif
    	/* Reclaim empty buffer from the output stream to be reused           */
    	nmadus0 = SIO_reclaim(mcbspOutHandle,(Ptr *)&xmt,NULL);

        if (nmadus0 < 0)
        {
            LOG_printf(&trace, "Error reclaiming empty buffer from the streams %x %d",
                *(Uint32 *)xmt,
                nmadus0);
        }
        /* Copy the receive information to the transmit buffer */

        //memcpy(xmt, rcv, TX_BUFSIZE);
        loopData(xmt, rcv, NUM_TX_CHANNELS, NUM_RX_CHANNELS, BUFLEN);

#if 1
        /* submit the request for the receive data                           */
        if (IOM_COMPLETED != SIO_issue(mcbspInHandle,rcv, RX_BUFSIZE, NULL))
        {
            LOG_printf(&trace, "Error issuing receive buffer to the stream");
        }
#endif
        /* submit the request for the transmit data                           */
        if (IOM_COMPLETED != SIO_issue(mcbspOutHandle,xmt, TX_BUFSIZE, NULL))
        {
            LOG_printf(&trace, "Error issuing transmit buffer to the stream");
        }
        pktCount++;
    }
}


/*
 * \brief   Function to submit request the driver.Depending on the macro
 *          "NUM_BUFS" either the driver can be buffered with multiple requests
 *          or only one buffer by using the NUM_BUFS as 1.
 *
 * \param   None
 *
 * \return  None
 */
static Void mcbspDriverPrime(Void)
{
    Uint32 count = 0;

#if 1
    /* RX buffer */
    for (count = NUM_BUFS; count < (NUM_BUFS * 2); count++)
    {
        if (IOM_COMPLETED
            != SIO_issue(mcbspInHandle, (Ptr)buf[count], RX_BUFSIZE, NULL))
        {
            LOG_printf(&trace, "Issue to output stream failed.");
            SYS_abort("Issue to output stream failed\n");
        }
    }
#endif
    /* TX buffer */
    for (count = 0; count < NUM_BUFS; count++)
    {
        if (IOM_COMPLETED
            != SIO_issue(mcbspOutHandle, (Ptr)buf[count], TX_BUFSIZE, NULL))
        {
            LOG_printf(&trace, "Issue to output stream failed.");
            SYS_abort("Issue to output stream failed\n");
        }
    }
    LOG_printf(&trace, "Requests submitted to the Mcbsp driver.");
}


/*
 * \brief    Function to create the required streams for the reception of
 *           Mcbsp data.
 *
 * \params   None
 *
 * \return   None
 */
static Void mcbspCreateStreams(Void)
{
    SIO_Attrs      sioAttrs  = SIO_ATTRS;
    Uint32         count     = 0;

    sioAttrs.nbufs = NUM_BUFS;
    sioAttrs.align = BUFALIGN;
    sioAttrs.model = SIO_ISSUERECLAIM;

    /* TX channel setup	*/
    mcbspChanparam.chanConfig = &mcbspChanConfig[1];
    mcbspChanparam.clkSetup = &mcbspClkConfig[1];
    mcbspChanparam.edmaEventQue = 0;

    /* Create a TX channel for the transmission                               */
    mcbspOutHandle = SIO_create("/dioMcbspOUT", SIO_OUTPUT, TX_BUFSIZE, &sioAttrs);

    if (NULL == mcbspOutHandle)
    {
        LOG_printf(&trace, "Output Stream Creation Failed");
        SYS_abort("Output Stream Creation Failed\n");
    }
    
    sioAttrs.nbufs = NUM_BUFS;
    sioAttrs.align = BUFALIGN;
    sioAttrs.model = SIO_ISSUERECLAIM;

    /* RX channel setup	*/
    mcbspChanparam.chanConfig = &mcbspChanConfig[0];
    mcbspChanparam.clkSetup = &mcbspClkConfig[0];
    mcbspChanparam.edmaEventQue = 1;

    /* Create a RX channel for the transmission                               */
    mcbspInHandle = SIO_create("/dioMcbspIN", SIO_INPUT, RX_BUFSIZE, &sioAttrs);

    if (NULL == mcbspInHandle)
    {
        LOG_printf(&trace, "Input Stream Creation Failed");
        SYS_abort("Input Stream Creation Failed\n");
    }

    /* create the buffers required for the TX  operation                      */
    for (count = 0; count < (NUM_BUFS ); count++)
    {
        buf[count] = (Ptr)MEM_calloc(0, TX_BUFSIZE, BUFALIGN);

        if (NULL == buf[count])
        {
            LOG_printf(&trace, "MEM_calloc failed.");
            SYS_abort("MEM_calloc failed.\n");
        }
    }

    /* Fill the buffers with known data and transmit the same and check if the*
     * same pattern is received on the other EVM                              */
#ifdef SEND_PATTERN
    for (count = 0; count < (NUM_BUFS); count++)
    {

    	for (tempCount = 0; tempCount < BUFLEN; tempCount++)
        {
    			((Uint16 *)buf[count])[NUM_OF_TX_CHANNELS*tempCount] = 0xA0A0;
    			((Uint16 *)buf[count])[NUM_OF_TX_CHANNELS*tempCount+1] = 0x0A0A;
        }
/*#ifdef SEND_TONE
    	memcpy(buf[count], sinetable, TX_BUFSIZE);
#endif*/
    }
#endif


    /* create the buffers required for the RX  operation                      */
    for (count = NUM_BUFS; count < (NUM_BUFS * 2); count++)
    {
        buf[count] = (Ptr)MEM_calloc(0, RX_BUFSIZE, BUFALIGN);

        if (NULL == buf[count])
        {
            LOG_printf(&trace, "MEM_calloc failed.");
            SYS_abort("MEM_calloc failed.\n");
        }
    }
}

/*
 * \brief entry point function for the mcbsp demo task.
 *
 *       -> Intializes the EDMA driver
 *       -> creates the required channels for the demonstration of the MCBSP
 *          working in evm to evm communication mode.
 *
 * \params  None
 *
 * \return  None.
 */
Void mcbspDemoTask(Void)
{
    Int32    status = IOM_COMPLETED;

    /* initialise the edma library                                            */
    status = edma3init();

    if (EDMA3_DRV_SOK != status)
    {
        LOG_printf(&trace, "EDMA initialization failed");
    }
    else
    {
        LOG_printf(&trace, "EDMA initialized");

        /* update the edma handle to the channel parameters.Need to use       *
         * EDMA instance 0 only for Mcbsp 1                                   */
        mcbspChanparam.edmaHandle = hEdma[0];

        /* call the function to create the channel & demo the transceive      *
         * operation                                                          */
        mcbspStartDemo();

        LOG_printf(&trace, "Sample Application completed successfully...");
    }
}

/*============================================================================*/
/*                              END OF FILE                                   */
/*============================================================================*/


