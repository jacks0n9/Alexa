/* ========================================================================== */
/**
 *  @file   png.h
 *
 *  path    /dsps_gtmas/ti/mas/util/png.h
 *
 *  @brief  Header file for pink noise generator.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */
#ifndef _PNG_H
#define _PNG_H
/** @defgroup PNG 
 * This module contains function and definition for generating Pink Noise.
 */

/** @ingroup PNG */
/* @{ */
#include <ti/mas/types/types.h>

/** 
 * @brief Length of vector to store filter state. 
 **/
#define png_STATE_VEC_LEN 2  

/**
 * @brief Generate pink noise. 
 *
 * @b Description: @b <BR> 
 *        Generate "cnt" samples of pink noise in output frame.  
 *
 * @param[in]       filter_state     last time input x[n-1] and output y[n-1]. 
 *                                   i.e.<code> filter_state[0]=x[n-1] and filter_state[1]=y[n-1] </code>
 * @param[in]       cnt              number of samples to generate
 * @param[in,out]   buf              a pointer to input/output sample frame
 *
 * @remark count must be non-zero and multiple of 4
 *
 */
void pngGenNoiseFrame (Fract *filter_state, tint cnt, tint *buf);
/* @} */ /* ingroup */
#endif  /* _PNG_H */
/* nothing past this point */
