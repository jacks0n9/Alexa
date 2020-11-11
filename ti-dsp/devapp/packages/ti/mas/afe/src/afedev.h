#ifndef DAF_H
#define DAF_H

#include <xdc/std.h>
#include <stddef.h>     /* for size_t definition */
#include <dev.h>
#include <fxn.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  DAF_STREAM_USAGE_MONO_PLAYOUT = 0,   /**< Playout PCM data to speaker.*/

  DAF_STREAM_USAGE_MONO_NUM_CHANNELS_PLAYOUT   /**<  = 1 Number of voice/debug playout data channels supported */
} DAF_STREAM_USAGE_MONO_CHANNEL_MAP_PLAYOUT;

typedef enum
{
  DAF_STREAM_USAGE_MONO_CAPTURE = 0,       /**< Captured PCM data from microphone. */

  DAF_STREAM_USAGE_MONO_NUM_CHANNELS_CAPTURE   /**< = 1 Number of voice/debug data capture channels supported */
} DAF_STREAM_USAGE_MONO_CHANNEL_MAP_CAPTURE;

Void DAF_init (DEV_Attrs *afedevAttrs);

extern far DEV_Fxns DAF_FXNS;

/*
 *  ======== DEVNODE_Params ========
 */
typedef struct DAF_Params{    /* device parameters */
     UInt  maxFrameSize;      /* in msec */
     UInt  maxRxSamplingFreq; /* Rx sampling frequency in Hz */
     UInt  maxTxSamplingFreq; /* Tx sampling frequency in Hz */
     UInt  maxNumRxChannels;  /* number of rx TDM channels */
     UInt  maxNumTxChannels;  /* number of tx TDM channels */
     Void *afedevObj;         /* DAF Handle */
} DAF_Params;

typedef struct DAF_Frame{ /* Buffer Element exchanged with DAF */
     Ptr bufferAddress;   /* address of the buffer where data is exchanged */
     Int bufferSize;      /* in bytes */
} DAF_Frame;

typedef struct DAF_SioCtrl{  /* */
     Ptr ctrl;  /*  */
     Ptr resp;  /*  */
} DAF_SioCtrl;

extern DAF_Params DAF_PARAMS;           /* default parameter values */

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* DAF_H */
