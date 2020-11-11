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

#include <c6x.h>


/* -- AFE Headers -- */
#include <afe.h>
#include <src/afedbg.h>
#include <src/afeloc.h>
#include <src/afeswcfg.h>
#include <src/afedevloc.h>
/* -- TDM Headers -- */
#include "tdmloc.h"
#include "evtLog.h"
#include "control.h"
#include "dataLog.h"

/* ========================================================================== */
/*                        IMPORTED VARIABLES                                  */
/* ========================================================================== */

extern SEM_Handle gSemMicDataReady;
extern SEM_Handle gSemSpeakDataDone;

/* Handles for the TX channels                            */
extern SIO_Handle mcbspOutHandle;

/* Handles for the RX channels                            */
extern SIO_Handle mcbspInHandle;

/* ========================================================================== */
/*                         MACRO DEFINITIONS                                  */
/* ========================================================================== */

/* ========================================================================== */
/*                          LOCAL VARIABLES                                   */
/* ========================================================================== */

// Global counter of frames playback with audio (instead of silence)
u32 gSpkSoundOut = 0;

/* ============ spkSoundOutCount() ========== */
/**
 *  @brief      This function counts the number of frames with audio out
 *
 *  @param[in]  pFram  pointer to the frame being sent for playback
 *
 *  @return     None
 *
 */
void spkSoundOutCount( tSpeakFrame * pFrm)
{
    int n = pFrm->numSample * pFrm->numCh;
    SAMPLE * pS = pFrm->samples;
    while( n-- > 0 ) {
        if(*pS++) {
            gSpkSoundOut++;
            break;
        }
    }

}

/* ============ halTdmIsr() ========== */
/**
 *  @brief      TX DMA interrupt service routine
 *
 *              Sorts interleved data from DMA Ping/Pong buffer.
 *
 *  @param[in]  vtdm  Void pointer to halTdmMcBsp_t structure
 *
 *  @return     None
 *
 *  @sa         None
 *
 */

void halTxTdmIsr (void *vtdm)
{
    static int rxUnderCount = 0;
    static int wasUnderflow = 0;

    Ptr             xmt;
    tuint           bufSize;

    tSpeakFrame * pFrm = (tSpeakFrame * ) pgSpeakBuf[gCurrentBufferIndex];
    SAMPLE * pSampleIn = pFrm->samples;
    SAMPLE * pSampleOut;
    int status;
    int ch, i;


    /* Reclaim empty buffer from the output stream to be reused */
    status = SIO_reclaim(mcbspOutHandle, (Ptr *)&xmt, NULL);
    if (status < 0)
    {
        EVT_LOG("TDMISR: Error reclaiming empty buffer from the streams %x %d", *(Uint32 *)xmt, status);
        return;
    }

    EVT_LOG_INCREMENT_COUNT;
    // Check for underflow
    if(pFrm->metaData & SPEAK_FRAME_META_CONSUMED){
        pSampleOut = (SAMPLE *) xmt;
        for(i = 0; i < (SPEAKER_TARGET_RATE * FRAME_LEN_IN_MS / 1000*AFE_NUM_TX_CHANNELS); i++){
            *pSampleOut++ = 0;
        }
        // COunt it
        rxUnderCount++;
        if(!wasUnderflow){
            wasUnderflow = 1;
            EVT_LOG("RxISR Under %d\n", rxUnderCount);
        }
    }
    else {
        spkSoundOutCount(pFrm);
        for (ch = 0; ch < AFE_NUM_TX_CHANNELS; ch++) {
            if(ch >= pFrm->numCh){
                // Not enough channels restart
                pSampleIn = pFrm->samples;
            }
            pSampleOut = (SAMPLE *) xmt + ch;
            for(i = 0; i < (SPEAKER_TARGET_RATE * FRAME_LEN_IN_MS / 1000); i++){
                // Write the samples to the ISR buffer and interleaving channels
                *pSampleOut = *pSampleIn++;
                pSampleOut += AFE_NUM_TX_CHANNELS;
            }
        }
        // Mark this frame as read
        pFrm->metaData |= SPEAK_FRAME_META_CONSUMED;
        wasUnderflow = 0;
    }

    // Speaker data is done
    if(gSemSpeakDataDone){
        SEM_postBinary( gSemSpeakDataDone);
    }

    // This happens at the exact moment the ping pong buffer is switching...
    if (gSentinelFrameNumber == pFrm->seqNum) {
        StoreSentinelTimestamp(gSentinelSampleOffset);
    } 

    /* Issue full buffer to the output stream                             */
    bufSize = (SPEAKER_TARGET_RATE * FRAME_LEN_IN_MS / 1000) * AFE_NUM_TX_CHANNELS; /* = size of Tx subFrame in number of samples */
    bufSize = bufSize * 2;                         /* = size of Tx subFrame in number of bytes   */

    status = SIO_issue(mcbspOutHandle, xmt, bufSize, NULL);
    if (status != IOM_COMPLETED)
    {
        EVT_LOG("TDMISR: Error issuing transmit buffer to the stream. Error = %d\n", status);
    }
}

/* ============ halRxTdmIsr() ========== */
/**
 *  @brief      RX DMA interrupt service routine
 *
 *              Sorts interleved data from DMA Ping/Pong buffer.
 *
 *  @param[in]  rxTdm  Void pointer to rx halTdmMcBsp_t structure
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void halRxTdmIsr (void *vtdm)
{
    Ptr             rcv;
    tuint           bufSize;

    tMicFrame * pFrm = (tMicFrame * ) pgMicroBuf[gCurrentBufferIndex];
    SAMPLE * pSampleOut = pFrm->samples;
    SAMPLE * pSampleR;
    int status;
    int ch, i;

    const int validMicSequence[] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0};


    /* Reclaim full buffer from the input stream */
    // This is to reclaim buffers from the HW mcbsp
    status = SIO_reclaim(mcbspInHandle, (Ptr *)&rcv, NULL);
    if (status < 0)
    {
        EVT_LOG("TDMISR: Error reclaiming empty buffer from the streams %x %d", *(Uint32 *)rcv, status);
        return;
    }


    // Copy the data from the ISR buffer to our working buffer
    // Deinterleave the data at the same time
    for (ch = 0; ch < AFE_NUM_MIC_CH; ch++) {
        pSampleR = (SAMPLE *) rcv + validMicSequence[ch];
        i =  MIC_TARGET_RATE * FRAME_LEN_IN_MS / 1000;
        while(i--){
            *pSampleOut++ = *pSampleR;
            pSampleR += AFE_NUM_MIC_CH;
        }
    }

    // Microphone data is ready
    if(gSemMicDataReady){
        SEM_postBinary( gSemMicDataReady);
    }

    /* Issue an empty buffer to the input stream                          */
    bufSize = (MIC_TARGET_RATE * FRAME_LEN_IN_MS / 1000) * AFE_NUM_MIC_CH; /* = size of Rx subFrame in number of samples */
    bufSize = bufSize * 2;                         /* = size of Rx subFrame in number of bytes   */

    status = SIO_issue(mcbspInHandle, rcv, bufSize, NULL);
    if (status != IOM_COMPLETED)
    {
        EVT_LOG("TDMISR: Error issuing receive buffer to the stream. Error = %d\n", status);
    }
}


/* Nothing past this line */
