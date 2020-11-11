/* ========================================================================== */
/**
 *  @file   afemem.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afestack.h
 *
 *  @brief  AFE's memory record stack header file
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __AFEMEM_H__
#define __AFEMEM_H__

/* -- RTSC Type Header -- */
#include <xdc/std.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============ AFE_initMemStack ========== */
/**
 *  @brief      Initialie memory stack
 *  @param[in]  segid Memory segment id to be used by stack data structure
 *  @return     None
 *  @sa         None
 */
Void AFE_initMemStack (Int segid);

/* ============ AFE_callocAndPush ========== */
/**
 *  @brief      Allocate memory block and push block attributes onto memory stack
 *  @param[in]  segid Memory segment identifier
 *  @param[in]  size  Block size in MADUs
 *  @param[in]  align Block alignment
 *  @return     None
 *  @sa         None
 */
Void *AFE_callocAndPush (Int segid, Uns size, Uns align);

/* ============ AFE_popAndFree ========== */
/**
 *  @brief      Pops and free one element from the memory stack
 *  @return     Zero for success, -1 for failure
 *  @sa         None
 */
Int AFE_popAndFree (Void);

/* ============ AFE_popAndFreeAll ========== */
/**
 *  @brief      Pop and frees the entire memory stack
 *              TODO: Check if it needs to be protected by critical section
 *  @return     None
 *  @sa         None
 */
Void AFE_popAndFreeAll (Void);

/* ============ AFE_printMemStack ========== */
/**
 *  @brief      Prints the content of the memory stack
 *  @return     None
 *  @sa         None
 */
void AFE_printMemStack (Void);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* __AFEMEM_H__ */
