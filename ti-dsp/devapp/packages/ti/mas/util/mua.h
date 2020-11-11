/* ========================================================================== */
/**
 *  @file   mua.h
 *
 *  path    /dsps_gtmas/ti/mas/util/mua.h
 *
 *  @brief  Header file for companding tables.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */
#ifndef _MUATBL_H
#define _MUATBL_H

/* System include files */
#include <ti/mas/types/types.h>                      /* DSP types            */
/** @defgroup MUA 
 * This module contains utility function for converting PCM samples to U/A-Law.
 * It also defines max allowed PCM value.
 */

/** @ingroup MUA */
/* @{ */
/**
 @brief Max absolute value of a PCM sample 
 */
#define mua_TBL_MAX_PCM_VAL     32632   
/**
 @brief a-Law Silence value.
 * 
 * Equivalent to -8 Linear value
 */
#define mua_TBL_ALAW_SILENCE    0x55    
/**
 @brief u-law Silence value.
 * 
 * Equivalent to 0 Linear value
 */
#define mua_TBL_MULAW_SILENCE   0x7F    

/**
 * @brief a-law Tables 
 */
extern const tint muaTblAlaw[];
/**
 * @brief u-law Tables 
 */
extern const tint muaTblUlaw[];

/**
 * @brief Compress a frame of linear sample using u-law.
 *
 * @param[in]   length         length of samples to be compressed
 * @param[in]   xin            Pointer to input frame in Q0 format
 * @param[out]  xout           Pointer to output PCM samples
 *
 */
void muaTblUlawCmpr (tint length, linSample *xin, tint *xout);

/**
 * @brief Compress a frame of linear sample using A-law.
 *
 * @param[in]   length         length of samples to be compressed
 * @param[in]   xin            Pointer to input frame in Q0 format
 * @param[out]  xout           Pointer to output PCM samples
 *
 */
void muaTblAlawCmpr (tint length, linSample *xin, tint *xout);

/* @} */ /* ingroup */
#endif /* _MUATBL_H */
/* nothing after this point */
