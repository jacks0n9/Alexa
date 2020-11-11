/* ========================================================================== */
/**
 *  @file   afehal.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/ecodrv/afehal.h
 *
 *  @brief  Implements HAL initialization functionality
 *
 *          This file contains functions that are used to initialize
 *          the HAL. They are called only once at system startup
 *          and so can be overlayed with an uninitialized data section
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __AFEHAL_H__
#define __AFEHAL_H__

#include <afe.h>
#include <src/afeloc.h>

#ifdef __cplusplus
extern "C" {
#endif

void AFE_subRateFcn (void *handle, tuint ts, Uint32 ticks);

/* ============ AFE_newHal ========== */
/**
 *  @brief      Initializes HAL (Hardware Abstraction Layer)
 *
 *              HAL memory is allocated and HAL is configured
 *
 *  @param[in]    afeObj    Pointer to AFE_Object
 *
 *  @param[out]   revId     Chip revision ID
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
AFE_Error AFE_newHal (AFE_Object *afeObj, tuint *revId);

/* ============ AFE_initHal ========== */
/**
 *  @brief      Initializes HAL and configures memory and cache
 *
 *  @param[in]  afeObj    Pointer to AFE_Object
 *
 *  @return     AFE Error Code
 *
 *  @sa         None
 *
 */
AFE_Error AFE_initHal (AFE_Object *afeObj);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* __AFEHAL_H__ */

/* Nothing beyond this line */

