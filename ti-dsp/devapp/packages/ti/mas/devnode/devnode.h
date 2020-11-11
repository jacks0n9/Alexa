/* ========================================================================== */
/**
 *  @file   ti/mas/devnode/devnode.h
 *
 *  @brief  The IALG definitions for DEVNODE
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
 *  @defgroup   ti_mas_devnode_DEVNODE   DEVNODE Interface 
 *
 *  This is the DEVNODE Interface 
 */

#ifndef ti_mas_DEVNODE_H_
#define ti_mas_DEVNODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/mas/devnode/idevnode.h>

#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/visa.h>
#include <ti/sdo/ce/skel.h>
#include <ti/xdais/xdas.h>

/** @ingroup    ti_mas_devnode_DEVNODE */
/*@{*/

#define DEVNODE_EOK      IDEVNODE_EOK      /**< @copydoc IDEVNODE_EOK */
#define DEVNODE_EFAIL    IDEVNODE_EFAIL    /**< @copydoc IDEVNODE_EFAIL */
#define DEVNODE_ERUNTIME IDEVNODE_ERUNTIME /**< @copydoc IDEVNODE_ERUNTIME */

#define DEVNODE_MAX_IO_BUFFERS   IDEVNODE_MAX_IO_BUFFERS

/**
 *  @brief      Opaque handle to a DEVNODE codec.
 */
typedef VISA_Handle DEVNODE_Handle;

/**
 *  @brief      This structure defines the parameters necessary to create an
 *              instance of a Scale object.
 */
typedef struct IDEVNODE_Params DEVNODE_Params;

extern DEVNODE_Params DEVNODE_PARAMS;   /**< Default instance parameters. */

/**
 * @copydoc IDEVNODE_InArgs
 */
typedef IDEVNODE_InArgs           DEVNODE_InArgs;

/**
 * @copydoc IDEVNODE_OutArgs
 */
typedef IDEVNODE_OutArgs          DEVNODE_OutArgs;

/**
 * @copydoc IDEVNODE_Ctrl_InArgs
 */
typedef IDEVNODE_Cmd               DEVNODE_Cmd;

/**
 * @copydoc IDEVNODE_Ctrl_InArgs
 */
typedef IDEVNODE_Ctrl_InArgs              DEVNODE_Ctrl_InArgs;

/**
 * @copydoc IDEVNODE_Ctrl_OutArgs
 */
typedef IDEVNODE_Ctrl_OutArgs              DEVNODE_Ctrl_OutArgs;

/** @cond INTERNAL */

extern SKEL_Fxns DEVNODE_SKEL;

extern IDEVNODE_Fxns DEVNODE_STUBS;

/** @endcond */
/**
 *  @brief      Create an instance of a DEVNODE algorithm.
 *
 *  Instance handles must not be concurrently accessed by multiple threads;
 *  each thread must either obtain its own handle (via DEVNODE_create()) or
 *  explicitly serialize access to a shared handle.
 *
 *  @param[in]  e       Handle to an opened engine.
 *  @param[in]  name    String identifier of the scale algorithm
 *                      to create.
 *  @param[in]  params  Creation parameters.
 *
 *  @retval     NULL            An error has occurred.
 *  @retval     non-NULL        The handle to the newly created image decoder
 *                              instance.
 *
 *  @remark     Depending on the configuration of the engine opened, this
 *              call may create a local or remote instance of the algorithm.
 *
 *  @sa         Engine_open()
 */
extern DEVNODE_Handle DEVNODE_create(Engine_Handle e, String name,
    DEVNODE_Params *params);


/**
 *  @brief      Execute the "process" method in this instance of the algorithm.
 *
 *  @param[in]  handle  Handle to a created algorithm instance.
 *  @param[in]  inBuf   A buffer descriptor containing the input buffer.
 *  @param[out] outBuf  A buffer descriptor containing the output buffer.
 *  @param[in]  inArgs  Input Arguments.
 *  @param[out] outArgs Output Arguments.
 *
 *  @pre        @c handle is a valid (non-NULL) handle
 *              and the algorithm is in the created state.
 *
 *  @retval     #DEVNODE_EOK          Success.
 *  @retval     #DEVNODE_EFAIL        Failure.
 *  @retval     #DEVNODE_ERUNTIME     Internal Runtime Error.
 *
 *  @remark     This is a blocking call, and will return after the data
 *              has been processed.
 *
 *  @remark     The buffers supplied to DEVNODE_process() may have constraints
 *              put on them.  For example, in dual-processor, shared memory
 *              architectures, where the algorithm is running on a remote
 *              processor, the buffers may need to be physically contiguous.
 *              Additionally, the remote processor may place restrictions on
 *              buffer alignment.
 *
 *  @sa         DEVNODE_create(), DEVNODE_delete(), DEVNODE_control()
 */
extern Int32 DEVNODE_process(DEVNODE_Handle handle,
    DEVNODE_InArgs *inArgs, DEVNODE_OutArgs *outArgs);

/*
 *  ======== DEVNODE_control ========
 */
/**
 *  @brief      Execute the "control" method in this algorithm instance.
 *
 *  @param[in]  handle  Handle to a created algorithm instance.
 *  @param[in]  id      Command id for control operation.
 *  @param[in]  params  Runtime control parameters.
 *
 *  @pre        @c handle is a valid (non-NULL) DEVNODE algorithm handle
 *
 *  @retval     #DEVNODE_EOK          Success.
 *  @retval     #DEVNODE_EFAIL        Failure.
 *  @retval     #DEVNODE_ERUNTIME     Internal Runtime Error.
 *
 *  @remark     This is a blocking call, and will return after the control
 *              command has been executed.
 *
 *  @sa         DEVNODE_create(), DEVNODE_delete()
 */
extern Int32 DEVNODE_control(DEVNODE_Handle handle, DEVNODE_Cmd cmd, DEVNODE_Ctrl_InArgs *inArgs, DEVNODE_Ctrl_OutArgs *outArgs);

/*
 *  ======== DEVNODE_delete ========
 */
/**
 *  @brief      Delete the instance of a DEVNODE algorithm.
 *
 *  @param[in]  handle  Handle to a created DEVNODE instance.
 *
 *  @remark     Depending on the configuration of the engine opened, this
 *              call may delete a local or remote instance of the DEVNODE
 *              algorithm.
 *
 *  @pre        @c handle is a valid (non-NULL) handle which is
 *              in the created state.
 *
 *  @post       All resources allocated as part of the DEVNODE_create()
 *              operation (memory, DMA channels, etc.) are freed.
 *
 *  @sa         DEVNODE_create()
 */
extern Void DEVNODE_delete(DEVNODE_Handle handle);

/*@}*/  /* ingroup */

/* INTERNAKL TIMER FUNCTIONS */


typedef struct Timer_Obj {
    volatile Uint32     pid12;
    volatile Uint32     emumgt_clkspd;
    volatile Uint32     gpint_en;
    volatile Uint32     gpdir_dat;
    volatile Uint32     tim12;
    volatile Uint32     tim34;
    volatile Uint32     prd12;
    volatile Uint32     prd34;
    volatile Uint32     tcr;
    volatile Uint32     tgcr;
    volatile Uint32     wdtcr;
    volatile Uint32     tlgc;
    volatile Uint32     tlmr;
} Timer_Obj;

#ifdef DM6446
#define Timer_phys0 ((Timer_Obj *)0x01c21400)
#define Timer_phys1 ((Timer_Obj *)0x01c21800)
#define TIMER_FREQ  27  /* In MHz */
#endif
#ifdef LEO
#define Timer_phys0 ((Timer_Obj *)0x08086500)
#define Timer_phys1 ((Timer_Obj *)0x08086600)
#define TIMER_FREQ  100  /* In MHz */
#endif



#ifdef __cplusplus
}
#endif

#endif

