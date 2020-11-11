/* ========================================================================== */
/**
 *  @file   ti/mas/eco/ieco.h
 *
 *  @brief  This header defines all types, constants, and functions
 *              shared by all implementations of the eco
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
 *  @defgroup   ti_mas_eco_IECO   IECO Interface 
 *
 *  This is the ECO XDM interface 
 */

#ifndef ti_mas_eco_IECO_H_
#define ti_mas_eco_IECO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <xdc/std.h>
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>

/** @ingroup    ti_mas_eco_IECO */
/*@{*/

/* Return codes */
#define IECO_EOK       0 /**< Success. */ 
#define IECO_EFAIL    -1 /**< Failure. */ 
#define IECO_ERUNTIME -2 /**< Runtime error */ 

#define IECO_MAX_IO_BUFFERS 10 /**< Maximum number of I/O buffers per stream */ 
#define IECO_MAX_STREAMS    4  /**< Maximum number of streams per ECO handle */

/**
 *  @brief      This must be the first field of all IECO instance objects.
 */
typedef struct IECO_Obj {
    struct IECO_Fxns *fxns;
} IECO_Obj;

/**
 *  @brief      Opaque handle to an IECO object.
 */
typedef struct IECO_Obj *IECO_Handle;


typedef struct IECO_Arg {
    XDAS_Int32 size;
} IECO_Arg;

typedef struct IECO_Params {
    XDAS_Int32  size;                /* size of this structure, in bytes */
    XDAS_Int32  (*ecoCallOutFn)(IECO_Handle handle, IECO_Arg *inArg, IECO_Arg *outArg);
    IECO_Handle ecoCallOutHandle;
} IECO_Params;

/**
 *  @brief      Stream descriptor.
 */
typedef struct IECO_StreamDesc {
    UInt8   *buf[IECO_MAX_IO_BUFFERS];     /**< Array of buffer pointers - active channels
                                                   within the stream are represented by valid 
                                                   buffer pointers, unused channels
                                                   represented by NULL pointers. The semantics of the 
                                                   buffer indices for various usage scenarios 
                                                   are defined in header files for actual 
                                                   ECO algorithm implementations. */
    UInt32  bufSize[IECO_MAX_IO_BUFFERS]; /**< Size of each buffer in bytes.*/

} IECO_StreamDesc;

/**
 *  @brief      Input arguments to the ECO process routine
 */
typedef struct IECO_InArgs {
    UInt32           size;           /**< size of this structure */
    IECO_StreamDesc  *inStream[IECO_MAX_STREAMS]; 
                                     /**< Array of issued stream descriptors, unused streams will
                                          be represented by NULL pointers.*/
    Int32            message;        /**< Any message to be passed during algo process() */
} IECO_InArgs;

/**
 *  @brief      Output arguments to the ECO process routine
 */
typedef struct IECO_OutArgs {
    UInt32           size;           /**< size of this structure */
    IECO_StreamDesc  *outStream[IECO_MAX_STREAMS]; 
                                     /**< Array of out stream descriptors, unused streams will
                                          be represented by NULL pointers.*/
    Int32            *status;         /**< any extra information from the algo process() call 
                                          complementing return value */
    
} IECO_OutArgs;

typedef struct IECO_DynamicParams{
    XDAS_Int32       size;
    XDAS_Int32       bufSize;
    XDAS_Int8        *buf;
} IECO_DynamicParams;

/**
 *  @brief      This structure defines the ECO status that can be
 *              obtained during control() calls.
 *
 *  @extensibleStruct
 *
 *  @sa         IECO_Fxns::control()
 */
typedef struct IECO_Status{
    XDAS_Int32       size;
    XDAS_Int32       bufSize;
    XDAS_Int8        *buf;
} IECO_Status;

/**
 *  @brief      Defines the control commands for the IECO module.
 *
 *  @remarks    This ID can be extended in IMOD interface for
 *              additional controls.
 *
 *  @sa         XDM_CmdId
 *
 *  @sa         IECO_Fxns::control()
 */
typedef  IALG_Cmd IECO_Cmd;

#define XDM_GET_PERF_INFO (0xF000)

/**
 *  @brief      IECO interface table
 */
typedef struct IECO_Fxns{
    IALG_Fxns        ialg;
    XDAS_Int32       (*process)(IECO_Handle handle, IECO_InArgs *inArgs, IECO_OutArgs *outArgs);
    XDAS_Int32       (*control)(IECO_Handle handle, IECO_Cmd cmd, IECO_DynamicParams *params, IECO_Status *status);
} IECO_Fxns;

/*@}*/  /* ingroup */

#ifdef __cplusplus
}
#endif

#endif

