/* ========================================================================== */
/**
 *  @file   wng.h
 *
 *  path    /dsps_gtmas/ti/mas/util/wng.h
 *
 *  @brief  Header file for White noise generator.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */
#ifndef _WNG_H
#define _WNG_H


/** @defgroup WNG 
* This module contains function and definition for generating White Noise.
*/

/** @ingroup WNG */
/* @{ */
/* System level header files */
#include <ti/mas/types/types.h>                      /* DSP types            */

/**
 * @brief Initialize noise generator.
 *
 * @param[in]   noise_level     noise level in dBm0 [0,-70] range
 * @param[in]   seed            initial seed
 * @param[out]  mean            a pointer to mean value for noise that will
 *                              be needed for wngGenNoiseSample()
 * @param[out]  rng_state       a pointer to generator state that will be
 *                              needed for wngGenNoiseSample()
 *
 */
void wngInit(tint noise_level, tuint seed, tint *mean, tuint *rng_state);

/**
 * @brief Generate new noise sample.
 *
 * @b Description : @b <BR>
 * The random number generator used:<BR>
 * <code> x = (x * WNG_COEF + 1)mod(2^16) </code> <BR>
 * where:<BR>
 * x is unsigned number <BR>
 * 
 * <code> y = (Signed) x </code> <BR>
 *
 * Transformation to the white noise with the specified level:<BR>
 * <code> z = y*alpha </code> <BR>
 * where the level is based on the fact that the sine wave with
 * the amplitude 32768 produces signal level of +3dBm0.
 * The random number generator generates number in range 
 * (0:2^16-1).
 *
 *
 * @param[in]   mean        mean value, controls signal power
 *                           (obtained from wngInit())
 * @param[in]   rng_state   pointer to generator state that needs to be
 *                           saved in between calls. (first state obtained
 *                           in wngInit())
 *
 *
 */
linSample wngGenNoiseSample (tint mean, tuint *rng_state);

/**
 * @brief Generate new noise frame.
 *
 * @b Description : @b <BR>
 * The random number generator is described in wngGenNoiseSample().
 *
 * @param[in]   mean         mean value, controls signal power
 *                           (obtained from wngInit())
 * @param[in]   rng_state    pointer to generator state that needs to be
 *                           saved in between calls. (first state obtained
 *                           in wngInit())
 * @param[in]   n            frame length in samples
 * @param[in]   frame        a pointer to output frame buffer
 *
 * @remark  Frame length must be greater than 0. If not, unpredictable results would occur.
 */
void      wngGenNoiseFrame  (tint mean, tuint *rng_state, tint n,
                             linSample *frame);

/* @} */ /* ingroup */
#endif  /* _WNG_H */
/* nothing past this point */
