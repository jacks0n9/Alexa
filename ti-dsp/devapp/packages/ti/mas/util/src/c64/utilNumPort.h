/**
 *  @file   utilNumPort.h
 *  @brief  Contains 64x specific (optimized) implementation of numerical APIs
 *
 *  (C) Copyright 2007, Texas Instruments, Inc.
 */
#ifndef _UTILNUMPORT_C64_H
#define _UTILNUMPORT_C64_H

/******************************************************************************
* FILE PURPOSE: Portablity definitions for numerical operations
*******************************************************************************
* FILE NAME:   utilNumPort.h
*
* DESCRIPTION: Portability definitions for 64x, uses mostly intrinsics
*                                                        
* (C) Copyright 2007 Texas Instruments Inc.
******************************************************************************/

#include "c6x.h"
#include <ti/mas/types/types.h>
#include <ti/mas/fract/fract.h>
#include <ti/mas/util/cga.h>

#ifndef utlDotSat_Cm
/*****************************************************************************
 * FUNCTION PURPOSE: Computes a dot product of two Fract vectors with saturation
 *****************************************************************************  
 * DESCRIPTION: Qres <= TYP_LFRACT_SIZE - a - 1 - Bx - By
 *
 *              where a is from n <= 2^a,
 *                    Bx is from |x| < 2^Bx, By from |y| < 2^By
 *
 *  static inline LFract utlDotSat ( 
 *    Fract *x,         - a pointer to the first vector
 *    tint  Qx,         - Q-format for elements of x
 *    Fract *y,         - a pointer to the second vector
 *    tint  Qy,         - Q-format for elements of y
 *    tint  n,          - length of the vectors
 *    tint  Qres)       - Q-format for the result
 * 
 *   Same as utlDot (but no Qs input) but sum of products is guaranteed to 
 *   accumulate overflow up to 8 guard bits. The sum of products is not guaranteed
 *   to be saturated beyond 8 bits of guard i.e overflow beyond the guard
 *   should not be assumed. If overflow accumulation is within 8-bits, 
 *   end result will be saturated.
 *
 * Assumptions
 *
 *    n > 0 (i.e loop count is non-zero)
 *    n <= 256 (because 8-bit guard is used in accumulation)
 *
 *****************************************************************************/
static inline LFract utlDotSat (Fract *x, tint Qx, Fract *y, tint Qy, tint n,
                                tint Qres)
{
  int i, diffQ;
  long lsum = 0; /* long is 40-bits */

  cga_LOOP_COUNT_INFO(1,,,,)
  for (i = 0; i < n; i++) {
    lsum += (LFract) x[i] * y[i];
  }

  diffQ = (Qx+Qy-Qres);

  if (diffQ > 0) {
    lsum = lsum >> diffQ;
    return(_sat(lsum));
  }
  else {
    /* Note: _sat below is necessary because lsum is long (40-bit), not int (32-bit),
       and since it is left shift, if it was saturated at 32-bit, it will still be
       saturated after left shifting, but if it wasn't saturated at 32-bit, it could
       saturate after left shift */
    return(_sshl(_sat(lsum), -diffQ));
  }
}

/*****************************************************************************
 * FUNCTION PURPOSE: Computes a dot product of two Fract vectors with saturation
 *                   with some assumptions for optimization
 *****************************************************************************  
 * DESCRIPTION: Qres <= TYP_LFRACT_SIZE - a - 1 - Bx - By
 *
 *              where a is from n <= 2^a,
 *                    Bx is from |x| < 2^Bx, By from |y| < 2^By
 *
 *  static inline LFract utlDotSatAlignedInp ( 
 *    Fract *x,         - a pointer to the first vector
 *    tint  Qx,         - Q-format for elements of x
 *    Fract *y,         - a pointer to the second vector
 *    tint  Qy,         - Q-format for elements of y
 *    tint  n,          - length of the vectors
 *    tint  Qres)       - Q-format for the result
 * 
 * Same as utlDotSat but with additional restrictions on input as specified
 * below.
 *
 * Assumptions
 *
 *    n > 0 (i.e loop count is non-zero)
 *    Inputs x and y are 64-bit (8 bytes) aligned
 *    n <= 256 (because 8-bit max guard is assumed to be used in accumulation)
 *
 *
 *  Note: 
 *    
 *  This implementation when data inputs are cached, will result in 4
 *  MACs per cycle. The _dotp2 intrinsic uses DOTP2 instruction which does not
 *  have guard in h/w for the sum of  two products it is doing, in one
 *  case where inputs to this instruction are -32768 (neg max), it will
 *  produce wrong results because overflow will not correctly
 *  accumulate even though the accumulator itself is long (because
 *  dotp2 itself loses the guard).  This can happen for example if
 *  input is a saturated signal and energy is to be computed,  if there
 *  are two consecutive negative max samples but if input is simply 
 *  high but not saturated, the calculation will be o.k. The cure for
 *  this is to use _ldotp2 but this kills optimization as compiler has
 *  to do additional computation steps and results in only 2 MACs per
 *  cycle, which is achieved even with the regular utlDotSat above so
 *  there isn't any benefit of this specialized API, hence we don't use _ldotp2
 *
 *****************************************************************************/
static inline LFract utlDotSatAlignedInp (Fract *x, tint Qx, Fract *y, tint Qy, 
                                          tint n, tint Qres)
{
  int i, diffQ, rem;
  long lsum = 0; /* use long to accumulate overflow beyond 32-bit */

  /* Express alignment assumptions to compiler, this does not generate any code */
  _nassert((int) &(x[0]) % 8 == 0);
  _nassert((int) &(y[0]) % 8 == 0);

  /* Note: this code to compute remainder macs will get optimized out if
     n is a constant by the caller and is multiples of 4 */
  rem = (n & 3);
      
  /* optimized loop using intrinsics, Note: using sum = _sadd(sum, _dotp2(....)) does not
     yield optimum cycles compared to what is here, perhaps compiler cannot
     schedule well, even though _sadd is a native instruction. */
  cga_LOOP_COUNT_INFO(1,256,4,,)
  for (i = 0; i < (n - rem); i += 2) {
    lsum += _dotp2(_amem4(&x[i]), _amem4(&y[i]));
  }

  /* Note: this code to compute remainder macs will get optimized out if
     n is a constant by the caller and is multiples of 4 */
  for (i = (n-rem); i < n; i++) {
    lsum += (LFract) x[i] * y[i];
  }

  diffQ = (Qx+Qy-Qres);

  if (diffQ > 0) {
    lsum = lsum >> diffQ;
    return(_sat(lsum));
  }
  else {
    /* Note: _sat below is necessary because lsum is long (40-bit), not int (32-bit),
       and since it is left shift, if it was saturated at 32-bit, it will still be
       saturated after left shifting, but if it wasn't saturated at 32-bit, it could
       saturate after left shift */
    return(_sshl(_sat(lsum), -diffQ));
  }
}
#endif /* #ifndef utlDotSat_Cm */

#endif /* #ifndef _UTILNUMPORT_C64_H */

/* Nothing past this point */
