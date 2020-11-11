/* ========================================================================== */
/**
 *  @file   ti/mas/devnode/idevnode.h
 *
 *  @brief  This header defines all types, constants, and functions
 *              shared by all implementations of the devnode
 *              algorithms.
 *
 *  ============================================================================
 */
/* --COPYRIGHT--,BSD
 * Copyright (c) 2008, Texas Instruments Incorporated
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

/**
 *  @defgroup   ti_mas_devnode_IDEVNODE   IDEVNODE Interface 
 *
 *  This is the DEVNODE XDM interface 
 */

#ifndef ti_mas_codecs_devnode_IDEVNODE_H_
#define ti_mas_codecs_devnode_IDEVNODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <xdc/std.h>
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>

/** @ingroup    ti_mas_devnode_IDEVNODE */
/*@{*/

/* Return codes */
#define IDEVNODE_EOK       0 /**< Success. */ 
#define IDEVNODE_EFAIL    -1 /**< Failure. */ 
#define IDEVNODE_ERUNTIME -2 /**< Runtime error */ 

#define IDEVNODE_MAX_IO_BUFFERS 12 /**< Maximum number of I/O buffers per stream */ 
#define IDEVNODE_MAX_STREAMS    1  /**< Maximum number of streams per DEVNODE handle */

typedef struct IDEVNODE_Params {
    Int32 size;                /* size of this structure, in bytes */
} IDEVNODE_Params;

/**
 *  @brief      This must be the first field of all IDEVNODE instance objects.
 */
typedef struct IDEVNODE_Obj {
    struct IDEVNODE_Fxns *fxns;
} IDEVNODE_Obj;

/**
 *  @brief      Opaque handle to an IDEVNODE object.
 */
typedef struct IDEVNODE_Obj *IDEVNODE_Handle;

/**
 *  @brief      Stream descriptor.
 */
typedef struct IDEVNODE_StreamDesc {
    UInt32  maxBufId;    /**< This is largest buffer index present in the buf[] array plus 1. 
                              Note: if buf[0], buf[3] and buf[6] are present then maxBufId = 6+1 = 7
                              User application should populate this value */
    UInt8   *buf[IDEVNODE_MAX_IO_BUFFERS];     /**< Array of buffer pointers - active channels
                                                   within the stream are represented by valid 
                                                   buffer pointers, unused channels
                                                   represented by NULL pointers. Each index
                                                   into the array represents a buffer with a 
                                                   specific purpose within a stream - capture,
                                                   playout, debug, etc. The semantics of the 
                                                   buffer indices for various usage scenarios 
                                                   are defined in header files for actual 
                                                   DEVNODE algorithm implementations. */
    UInt32  bufSize[IDEVNODE_MAX_IO_BUFFERS]; /**< Size of each buffer in bytes.*/

} IDEVNODE_StreamDesc;

/**
 *  @brief      Input arguments to ithe DEVNODE process routine
 */
typedef struct IDEVNODE_InArgs {
    UInt32           size;           /**< @sizeField */
    UInt32           timeStamp;      /**< timestamp when this structure was filled */
    Uint32           numStreams;     /**< max num of streams for a particular implementation */
    Uint32           v2pOffset;      /**< Relation between physical and virtual addresses for issueStream */
    IDEVNODE_StreamDesc *issueStream[IDEVNODE_MAX_STREAMS]; 
                                     /**< Array of issued stream descriptors, unused streams will
                                          be represented by NULL pointers. An index into this
                                          array (0 to numStreams -1) represents the stream ID
                                          for a stream. */
} IDEVNODE_InArgs;

/**
 *  @brief      Output arguments to the DEVNODE process routine
 */
typedef struct IDEVNODE_OutArgs {
    UInt32           size;            
                          /**< @sizeField */
    UInt32           timeStamp;     
                          /**< timestamp when this struct was filled */
    UInt32           timeToAlgo;     
                          /**< time elapsed in traversing CE and transport (DSP Link) layer  */
    Uint32           numStreams;     
                          /**< max num of streams for a particular implementation */
    IDEVNODE_StreamDesc *reclaimedStream[IDEVNODE_MAX_STREAMS]; 
                          /**< Array of reclaimed stream descriptors, unused streams will
                               be represented by NULL pointers. An index into this
                               array (0 to numStreams -1) represents the stream ID
                               for a stream. */
    Int32            extendedInfo; /**< any extra information from the algo process() call 
                                        complementing return value */
    
} IDEVNODE_OutArgs;

/**
 *  @brief      Input arguments to the DEVNODE control routine
 */
typedef struct IDEVNODE_Ctrl_InArgs {
    XDAS_Int32             size;
    XDAS_Int32             messageSize;
    XDAS_UInt8            *message;
} IDEVNODE_Ctrl_InArgs;

/**
 *  @brief      Output arguments to the DEVNODE control routine
 */
typedef struct IDEVNODE_Ctrl_OutArgs {
    XDAS_Int32             size;
    XDAS_Int32             messageSize;
    XDAS_UInt8            *message;
} IDEVNODE_Ctrl_OutArgs;

/**
 *  @brief      Defines the control commands for the IDEVNODE module.
 *
 *  @remarks    This ID can be extended in IMOD interface for
 *              additional controls.
 *
 *  @sa         XDM_CmdId
 *
 *  @sa         IDEVNODE_Fxns::control()
 */
typedef  IALG_Cmd IDEVNODE_Cmd;

/**
 *  @brief      IDEVNODE interface table
 */
typedef struct IDEVNODE_Fxns{
    IALG_Fxns   ialg;
    Int32 (*process)(IDEVNODE_Handle handle, IDEVNODE_InArgs *inArgs, IDEVNODE_OutArgs *outArgs);
    Int32 (*control)(IDEVNODE_Handle handle, IDEVNODE_Cmd cmd, IDEVNODE_Ctrl_InArgs *inArgs, IDEVNODE_Ctrl_OutArgs *outArgs);
} IDEVNODE_Fxns;

/*@}*/  /* ingroup */

#ifdef __cplusplus
}
#endif

#endif

