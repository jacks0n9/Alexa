/* ========================================================================== */
/**
 *  @file   tdminit.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/c64P/Leo/tdmdma/tdminit.h
 *
 *  @brief  Header file for TDM driver code
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __TDMINIT_H__
#define __TDMINIT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -- DSP/BIOS Headers -- */
#include <std.h>
#include <log.h>

/* -- CSL DAVINCI Headers -- */
#include <csl_error.h>
#include <csl_types.h>
#include <csl_tdm.h>

/* -- HAL Header -- */
#include "src/c64P/Leo/tdmdma/tdmloc.h"

/* ============ trace ============= */
/**
 *  @name   trace
 *
 *  @brief   trace LOG_Obj used to do LOG_printf
 *
 */
extern LOG_Obj trace;

/* ============ aspHandle ============== */
extern CSL_TdmHandle tdmHandle;        /**< Global TDM Handle */
extern CSL_TdmObj    tdmObj;           /**< Global TDM object */

/* ============ halTdmOpen() ========== */
/**
 *  @brief  Opens TDM and assigns global TDM handle
 *
 *  @return CSL_Status code
 *
 *  @sa     None
 *
 */
CSL_Status halTdmOpen (void);

/* ============ halTdmReset() ========== */
/**
 *  @brief  Resets the TDM
 *
 *  @param[in]  aspHandle  Pointer to TDM object structure
 *
 *  @return CSL_Status code
 *
 *  @sa     None
 *
 */
CSL_Status halTdmReset (CSL_TdmHandle handle);

/* ============ halTdmInit() ========== */
/**
 *  @brief  Initializes TDM
 *
 *  @param[in]  tdm   Pointer to halTdmTdm_t structure
 *
 *  @return CSL_Status code
 *
 *  @sa     None
 *
 */
CSL_Status halTdmInit (CSL_TdmHandle handle, halTdmMcBsp_t *tdm);

/* ============ halTdmEnable() ========== */
/**
 *  @brief  Enables the TDM
 *
 *  @param[in]  aspHandle  Pointer to TDM object structure
 *
 *  @return CSL_Status code
 *
 *  @sa     None
 *
 */
CSL_Status halTdmEnable (CSL_TdmHandle handle);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* __TDMINIT_H__ */
