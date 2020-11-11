/* ========================================================================== */
/**
 *  @file   ti/mas/eco/eco.h
 *
 *  @brief  The IALG definitions for ECO
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
 *  @defgroup   ti_mas_eco_ECO   ECO Interface 
 *
 *  This is the ECO Interface 
 */

#ifndef ti_mas_ECO_H_
#define ti_mas_ECO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/mas/eco/ieco.h>

#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/visa.h>
#include <ti/sdo/ce/skel.h>
#include <ti/xdais/xdas.h>
#include <ti/xdais/dm/xdm.h>

/** @ingroup    ti_mas_eco_ECO */
/*@{*/

#define ECO_EOK      IECO_EOK      /**< @copydoc IECO_EOK */
#define ECO_EFAIL    IECO_EFAIL    /**< @copydoc IECO_EFAIL */
#define ECO_ERUNTIME IECO_ERUNTIME /**< @copydoc IECO_ERUNTIME */

#define ECO_MAX_IO_BUFFERS   IECO_MAX_IO_BUFFERS

/**
 *  @brief      Opaque handle to a ECO codec.
 */
typedef VISA_Handle ECO_Handle;

/**
 *  @brief      This structure defines the parameters necessary to create an
 *              instance of a Scale object.
 */
typedef struct IECO_Params ECO_Params;

extern ECO_Params ECO_PARAMS;   /**< Default instance parameters. */

/**
 * @copydoc IECO_InArgs
 */
typedef IECO_InArgs           ECO_InArgs;

/**
 * @copydoc IECO_OutArgs
 */
typedef IECO_OutArgs          ECO_OutArgs;

/**
 * @copydoc IECO_Ctrl_InArgs
 */
typedef IECO_Cmd              ECO_Cmd;

/**
 * @copydoc IECO_DynamicParams
 */
typedef IECO_DynamicParams    ECO_DynamicParams;

/**
 * @copydoc IECO_Status
 */
typedef IECO_Status           ECO_Status;

/** @cond INTERNAL */

extern SKEL_Fxns ECO_SKEL;

extern IECO_Fxns ECO_STUBS;

/** @endcond */
/**
 *  @brief      Create an instance of a ECO algorithm.
 *
 *  Instance handles must not be concurrently accessed by multiple threads;
 *  each thread must either obtain its own handle (via ECO_create()) or
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
extern ECO_Handle ECO_create(Engine_Handle e, String name,
    ECO_Params *params);


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
 *  @retval     #ECO_EOK          Success.
 *  @retval     #ECO_EFAIL        Failure.
 *  @retval     #ECO_ERUNTIME     Internal Runtime Error.
 *
 *  @remark     This is a blocking call, and will return after the data
 *              has been processed.
 *
 *  @remark     The buffers supplied to ECO_process() may have constraints
 *              put on them.  For example, in dual-processor, shared memory
 *              architectures, where the algorithm is running on a remote
 *              processor, the buffers may need to be physically contiguous.
 *              Additionally, the remote processor may place restrictions on
 *              buffer alignment.
 *
 *  @sa         ECO_create(), ECO_delete(), ECO_control()
 */
extern Int32 ECO_process(ECO_Handle handle, ECO_InArgs *inArgs, ECO_OutArgs *outArgs);

/*
 *  ======== ECO_control ========
 */
/**
 *  @brief      Execute the "control" method in this algorithm instance.
 *
 *  @param[in]  handle  Handle to a created algorithm instance.
 *  @param[in]  id      Command id for control operation.
 *  @param[in]  params  Runtime control parameters.
 *  @param[out] status  Runtime status.
 *
 *  @pre        @c handle is a valid (non-NULL) ECO algorithm handle
 *
 *  @retval     #ECO_EOK          Success.
 *  @retval     #ECO_EFAIL        Failure.
 *  @retval     #ECO_ERUNTIME     Internal Runtime Error.
 *
 *  @remark     This is a blocking call, and will return after the control
 *              command has been executed.
 *
 *  @sa         ECO_create(), ECO_delete()
 */
extern Int32 ECO_control(ECO_Handle handle, ECO_Cmd cmd, ECO_DynamicParams *params, ECO_Status *status);

/*
 *  ======== ECO_delete ========
 */
/**
 *  @brief      Delete the instance of a ECO algorithm.
 *
 *  @param[in]  handle  Handle to a created ECO instance.
 *
 *  @remark     Depending on the configuration of the engine opened, this
 *              call may delete a local or remote instance of the ECO
 *              algorithm.
 *
 *  @pre        @c handle is a valid (non-NULL) handle which is
 *              in the created state.
 *
 *  @post       All resources allocated as part of the ECO_create()
 *              operation (memory, DMA channels, etc.) are freed.
 *
 *  @sa         ECO_create()
 */
extern Void ECO_delete(ECO_Handle handle);

/*
 *  ======== ECO_getEcoHandle ========
 */
/**
 *  @brief      If the "real" instance is running on the remote processor, then
 *              it returns the instance handle in the remote processor context.
 *              The returned handle can be used by an application in the remote processor.
 *
 *  @param[in]  handle  Handle to a created ECO instance.
 *
 *  @remark     Depending on the configuration of the engine opened, this
 *              call returns instance handle in remote processor context.
 *
 *  @pre        @c handle is a valid (non-NULL) handle which is
 *              in the created state.
 *
 *  @post       None.
 *
 *  @sa         ECO_create()
 */
extern ECO_Handle ECO_getEcoHandle(ECO_Handle handle);

/*@}*/  /* ingroup */

#ifdef __cplusplus
}
#endif

#endif

