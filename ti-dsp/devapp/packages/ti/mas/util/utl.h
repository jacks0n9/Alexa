/* ========================================================================== */
/**
 *  @file   utl.h
 *
 *  path    /dsps_gtmas/ti/mas/util/utl.h
 *
 *  @brief  Header file for commonly used inlined utility functions and prototypes.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef _UTL_H
#define _UTL_H

#if (defined(utl_Cm) || defined(gnu_targets_MVArm9) || defined(gnu_targets_arm_GCArmv6) || defined(gnu_targets_arm_GCArmv7A) || defined(microsoft_targets_arm_WinCE) || defined(microsoft_targets_VC98) || defined(ti_targets_arm_Arm11) || defined(ti_targets_arm_Arm11_big_endian))
/**
   definition utl_Cm is to globally indicate that 
   all APIs are supposed to be C models, this will need to be defined
   by user of utl if desired to use C model of utl. This is required because of
   inlined API implementations. Note util's pacakage.bld itself also defines this,
   and is required in general because there could be utl API implementations that are not
   inlined. */
#define utlDotSat_Cm
#endif

/** @defgroup  UTL
 * This module contains commonly shared utility functions.
 * The utility functions are target independent and endian agonistics.  
 */

/** @ingroup UTL */
/* @{ */

#include <ti/mas/types/types.h>
#include <ti/mas/fract/fract.h>
#include <ti/mas/util/cga.h>
#include <ti/mas/util/src/utilNumPort.h>

/*! 
 * @brief Q-format of values in DB. 
*/   
#define utl_DB_Q  4

/*! 
 * @brief Q-format of values in linear. 
*/  
#define utl_LIN_Q 0

/**
 * @fn LFract utldB2LinPower(Fract PdB)
 * @brief Function to convert dB to linear power.          
 *
 *
 * @param[in]  PdB  Input in utl_DB_Q format 
 *
 * @retval     Plin in utl_LIN_Q format    
 *
 * @remark     With PdB in Q4 format, the valid input range is [-84.25 (-1348 in Q4), 9.0 (144 in Q4)].
 *             For any value outside the range, results are unpredictable.
 *       
 */
LFract utldB2LinPower(Fract PdB);

/**
 * @fn Fract  utlLin2dBPower(LFract Plin)
 * @brief Function to convert linear power to dB.          
 *
 *
 * @param[in]  Plin   Input linear power in utl_LIN_Q.
 *
 * @retval     PdB in Q4 utl_DB_Q.
 *
 * @remark  The range of the input in Q0 is [0,2^31]. The linear value 1 is 
 *          always added to prevent unpredictable results.
 */

Fract  utlLin2dBPower(LFract Plin);

/*!  @def utlAbs(num)
      @brief Utility for the obtaining the 
	         absolute value of a number.
 */
#define utlAbs(num) ((num)>0?(num):(-(num)))

/*!  @def utlNbytes2Ntints(numbytes)
      @brief Utility for bytes to tints conversion. 
 */
#define utlNbytes2Ntints(numbytes) \
   ((((numbytes) * 8) + TYP_TINT_SIZE - 1) / TYP_TINT_SIZE)

/*!  @def utlNtints2Nbytes(numtints)
      @brief Utility for tints to bytes conversion. 
 */
#define utlNtints2Nbytes(numtints) \
   (((numtints) * TYP_TINT_SIZE) / 8)

/*!  @def utlNbytes2NtwordsCeil(numbytes)
      @brief Utility for converting bytes into MAU units.
 */
#define utlNbytes2NtwordsCeil(numbytes) \
   (((numbytes) + ((TYP_TWORD_SIZE/8) - 1)) / (TYP_TWORD_SIZE/8))

/*!  @def utlNbytes2NtwordsFloor(numbytes)
      @brief Utility for converting MAU units into bytes.
 */
#define utlNbytes2NtwordsFloor(numbytes) \
   ((numbytes) / (TYP_TWORD_SIZE/8))

/*!  @def utlNtwords2Nbytes(numtwords)
      @brief Utility for converting MAU units into bytes.
 */
#define utlNtwords2Nbytes(numtwords) \
   (numtwords * (TYP_TWORD_SIZE/8))

/*!  @def utlNtwords2Nword32sCeil(numtwords)
      @brief Utility for converting MAU units to 32-bit words.
 */
#define utlNtwords2Nword32sCeil(numtwords) \
   (((numtwords) + ((32/TYP_TWORD_SIZE) - 1)) / (32/TYP_TWORD_SIZE))

/*!  @def utlNtwords2Nword32sFloor(numtwords)
      @brief Utility for converting MAU units to 32-bit words.
 */
#define utlNtwords2Nword32sFloor(numtwords) \
   (((numtwords) * TYP_TWORD_SIZE) / 32)

/*!  @def utlNword32s2Ntwords(numword32s)
      @brief Utility for converting 32-bit words to MAU units.
 */
#define utlNword32s2Ntwords(numword32s) \
   ((numword32s) * (32/TYP_TWORD_SIZE))


 /**
 * @fn Fract utlSin (Fract x)
 * @brief Sine Function.
 *
 * Calculates sine function. Input is in Q15 format in [-1,1).
 * It corresponds to [-pi,pi) range. The output is in Q14 format
 * in [-1,1] range.
 *
 * @param[in] x      input value in Q15. Range [-1,1).
 *
 * @remark Due to numerical error, the actual range of the output
 * is in [-16388/16384, 16388/16384] range, i.e
 * [-1.000244140625, 1.000244140625] range in Q14 format.
 *
 *****************************************************************************/
Fract utlSin (Fract x);

 /**
 * @fn LFract utlVecSum (Fract *x, tint Qx, tint n, tint Qres)
 * @brief Compute vector sum.
 *
 * Computes the sum of elements in a vector of fractional type.
 *
 * @param[in]     x         a pointer to input vector
 * @param[in]     Qx        Q-format for the vector elements
 * @param[in]     n         number of elements in the vector
 * @param[in]     Qres      Q-format of the result (must fit in LFract)
 *
 *  <b> Limitation </b> <BR>
 *   Let Qs be a Q-format of the sum and
 *   n <= 2^a. Then, <BR>
 *   <CODE> Qres <= Qs <= TYP_LFRACT_SIZE - TYP_FRACT_SIZE - a + Qx </CODE> <BR>
 *
 *   In other words, if the above inequality is
 *   not satisfied unpredictable result may occur.
 *
 *   @remark Qs is always selected as equal to Qx, i.e.
 *   Qs=Qx. Hence, 'a' must be <= then a
 *   difference between LFract and Fract sizes.
 *   In a case where LFract and Fract are the
 *   same, one should use the "maximum number of
 *   bits that are used within the elements of x"
 *   instead of Fract size.
 * 
 ****************************************************************************/
LFract utlVecSum (Fract *x, tint Qx, tint n, tint Qres);

 /*! Utility defined in utilmisc.s/c to avoid some compiler bug */
void utlDummy        (void);

 /**
 * @fn void utlScaleDown6dB (linSample *a, linSample *b, tint n)
 *
 * @brief  Scale an array of samples down 6dB.
 *
 * @param[in]   a     Pointer to the input array of linear samples
 * @param[in]   b     Pointer to the ouput (scaled) array of linear samples
 * @param[in]   n     Length of the array
 * 
 */
void utlScaleDown6dB (linSample *a, linSample *b, tint n);

/**
 * @fn  void utlScaleUp6dB (linSample *a, linSample *b, tint n)
 *
 * @brief  Scale an array of samples up 6dB.
 *
 * @param[in]   a     Pointer to the input array of linear samples
 * @param[in]   b     Pointer to the ouput (scaled) array of linear samples
 * @param[in]   n     Length of the array
 * 
 * @remark  No overflow check is performed.  This routine assumes 
 *                  the data will not overflow.
 * 
 *****************************************************************************/
void utlScaleUp6dB (linSample *a, linSample *b, tint n);

 /**
 * @fn void utlFractMemSet (Fract *a, Fract c, tint n)
 *
 * @brief Write a constant of type Fract to an array of Fract's.
 *
 * @param[in]   a     Pointer to the input array of Fract's
 * @param[in]   c     Constant to be written to the array
 * @param[in]   n     Length of the array
 *
 */
void utlFractMemSet (Fract *a, Fract c, tint n);

/**
 * @fn void utlLFractMemSet (LFract *a, LFract c, tint n)
 *
 * @brief Write a constant of type LFract to an array of LFract's.
 *
 * @param[in]   a     Pointer to the input array of LFract's
 * @param[in]   c     Constant to be written to the array
 * @param[in]   n     Length of the array
 *
 */
void utlLFractMemSet (LFract *a, LFract c, tint n);

/**
 * @fn void utlLsMemSet (linSample *a, linSample c, tint n)
 *
 * @brief Write a constant of type linSample to an array of linSamples.
 *
 * @param[in]   a     Pointer to the input array of linsamples
 * @param[in]   c     Constant to be written to the array
 * @param[in]   n     Length of the array
 *
 */
void utlLsMemSet (linSample *a, linSample c, tint n);
/**
 * @fn LFract    utlLFractVecMax (const LFract *x, tint x_len)
 *
 * @brief Compute the maximum value of vector of LFract's.
 *
 * @param[in]   x         Pointer to the vector of LFract's
 * @param[in]   x_len     Number of elements in the vector (> 0)
 *
 */
LFract    utlLFractVecMax (const LFract *x, tint x_len);

/**
 * @fn linSample utlLsVecMax     (const linSample *x, tint x_len);
 *
 * @brief Compute the maximum value of vector of linSamples.
 *
 * @param[in]   x         Pointer to the vector of linSamples
 * @param[in]   x_len     Number of elements in the vector (> 0)
 *
 */
linSample utlLsVecMax     (const linSample *x, tint x_len);

/**
 * @fn LFract    utlLVecSum      (const LFract *x, tint Qx, tint n, tint Qres)
 * @brief Compute (LFract) vector sum.
 *
 * Computes the sum of elements in a vector of long fractional type.
 *
 * @param[in]     x         a pointer to input vector
 * @param[in]     Qx        Q-format for the vector elements
 * @param[in]     n         number of elements in the vector
 * @param[in]     Qres      Q-format of the result (must fit in LFract)
 *
 *  <b> Limitation </b> <BR>
 *  Let Qs be a Q-format of the sum and n <= 2^a. 
 *  Also let the number of used bits in x[i] be l(x[i]). Then, <BR>
 *
 *  <CODE> Qres <= Qs <= TYP_LFRACT_SIZE - max{l(x[.])} + Qx - a </CODE>
 *
 *  In other words, if the above inequality is not satisfied unpredictable result may occur.
 *
 *  @remark Qs is always selected as equal to Qx, i.e. Qs=Qx.
 *  Hence, 'a' must be <= then a difference between LFract and the "maximum
 *  number of bits that are used within the elements of x."
 * 
 ****************************************************************************/

LFract    utlLVecSum      (const LFract *x, tint Qx, tint n, tint Qres);

/**
 * @fn tint utlLog2Up (tulong n)
 * 
 * @brief  <b>Upper</b> binary logarithm of integer number.
 *
 * @param[in] n Input value
 *
 */

static inline tint utlLog2Up (tulong n)
{
  tint i;

  if (n == 0) return(0);
  i = 0; n--;
  while (n != 0) {
    n >>= 1;
    i++;
  }
  return (i);
} /* utlLog2Up */

/**
 * @fn tint utlLog2Down (tulong n) 
 *
 * @brief  <b>Lower</b> binary logarithm of integer number.
 *
 * @param[in] n Input value
 *
 *****************************************************************************/

static inline tint utlLog2Down (tulong n)
{
  tint i;

  i = 0;
  while (n > 1) {
    n >>= 1;
    i++;
  }
  return (i);
} /* utlLog2Down */

/**
 * @fn void utlCopy (tword *inBuf, tword *outBuf, tint length)
 * @brief Copies elements from one buffer to another buffer.
 *
 * @param[in]   inBuf   address of source buffer
 * @param[out]  outBuf  address of destination buffer
 * @param[in]   length  length of data to be copied
 */

static inline void utlCopy (tword *inBuf, tword *outBuf, tint length)
{
  int i;
  for(i = 0; i < length; i++) {
    outBuf[i] = inBuf[i];
  }
}

/**
* @fn void utlCopyNonZeroLen (tword *inBuf, tword *outBuf, tint length)
* @brief Copies elements from one buffer to another buffer. 
*
* @param[in]   inBuf   address of source buffer
* @param[out]  outBuf  address of destination buffer
* @param[in]   length  length of data to be copied, must be non-zero
*
* @remark Length must be non-zero.
*/

static inline void utlCopyNonZeroLen (tword *inBuf, tword *outBuf, tint length)
{
  int i;

  /* Specify compiler that minimum loop count is 1 so it does not create
     extra instructions to check loop count before the loop execution */
  cga_LOOP_COUNT_INFO(1,,,,)
  for(i = 0; i < length; i++) {
    outBuf[i] = inBuf[i];
  }
}

/**
 *  @fn void utlCopyBits (word *src,word *dest,tint src_offset,tint dest_offset,
 *                          tint num_bits)
 *
 *  @brief ...
 *
 *  @param[in]  src          ...
 *  @param[out] dest         ...
 *  @param[in]  src_offset   ...
 *  @param[in]  dest_offset  ...
 *  @param[in]  num_bits     ...
 */
void utlCopyBits ( tword *src, tword *dest, tint src_offset, tint dest_offset,
                    tint num_bits);

/**
 * @fn LFract utlDot (Fract *x, tint Qx, Fract *y, tint Qy, tint n, tint Qs, tint Qres)
 * @brief Computes a dot product of 2 Fract vectors
 *
 * <code>Qres <= Qs <= TYP_LFRACT_SIZE - a - 1 - Bx - By </code> <BR>
 * where:<BR>
 *  <b>a</b> is from n <= 2^a,<BR>
 *  <b>Bx</b> is from |x| < 2^Bx, <BR>
 *  <b>By</b> from |y| < 2^By<BR>
 *
 *  @param[in]    x           a pointer to the first vector
 *  @param[in]    Qx          Q-format for elements of x
 *  @param[in]    y           a pointer to the second vector
 *  @param[in]    Qy          Q-format for elements of y
 *  @param[in]    n           length of the vectors
 *  @param[in]    Qs          Q-format for the sum of products
 *  @param[in]    Qres        Q-format for the result
 *
 *  @remark This API does not guarantee availability of guard bits for intermediate
 *        overflow accumulation in sum of products. The reason for providing
 *        Qs input is so that user ensures that inner product does not overflow.
 */

/**
 *  @fn    void utlBlockAdd (tuint *dst, tuint *src, tuint shift, tuint size)
 *
 *  @brief Calculates dst = dst + (src << shift)  for a arbitrary long 
 *              of length size 16 bits.  Max shift: 15 bits.
 *
 *  @param[out]  dst    A pointer to result array
 *  @param[in]   src    A pointer to operand array
 *  @param[in]   shift  Shift value
 *  @param[in]   size   Size of the array
 *
 *  @remark      dst and src cannot be same.
 */
void utlBlockAdd (tuint *dst, tuint *src, tuint shift, tuint size);

/**
 *  @fn    void utlBlockMult (tuint *dst, tuint *src, tuint k, tuint size)
 *
 *  @brief Calculates dst = src * k for a arbitrary long of length size 16 bits. 
 *
 *  @param[out]  dst    A pointer to result array
 *  @param[in]   src    A pointer to operand array
 *  @param[in]   k      Number to be mult by src array
 *  @param[in]   size   Size of the array
 *
 *  @remark      dst and src cannot be same.
 */
void utlBlockMult (tuint *dst, tuint *src, tuint k, tuint size);

static inline LFract utlDot (Fract *x, tint Qx, Fract *y, tint Qy, tint n,
                             tint Qs, tint Qres)
{
  int i;
  LFract sum = 0; 

  for (i = 0; i < n; i++)
    sum += frctMul (x[i], Qx, y[i], Qy, Qs);    /* Qp = Qs */

  return(frctAdjustQ(sum,Qs,Qres));
} /* utlDot */

#ifdef utlDotSat_Cm /* C model below */
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
 *	  n > 0 (i.e loop count is non-zero)
 *    n <= 256 (because 8-bit guard)
 *
 *****************************************************************************/

/* type of minimum 40-bits */
typedef struct {
  tint g;  /* enforce 8-bit guard, we don't have a minimum 8-bit type,
              so using mininmum 16-bit type i.e tint */
  LFract x;
} tint40_t;

static inline void utl_add_tint40 (tint40_t *sum, tint40_t *a, tint40_t *b)
{
  tint carry = 0;
  LFract t;

  t = a->x + b->x;
  
  if (((a->x ^ b->x) & frct_LFRCT_MIN) == 0) { /* a->x and b->x of same sign? */
    if ((t ^ a->x) & frct_LFRCT_MIN) { /* result overflowed? */
      if (a->x < 0) {
	    carry = -1;
	  }
	  else {
        carry = 1;
	  }
    }
  }
  sum->x = t;
  sum->g = a->g + b->g + carry;
}

/* Shift left with saturation, assumption is 32 > shift >= 0 */
static inline LFract utl_sshl_tint40 (tint40_t *ptr, tuint shift)
{
  
  LFract temp;
  
  /* shift could be too large, check first to see if ptr->g is already overflowed */
  if (ptr->g > 0) {
    return (frct_LFRCT_MAX);
  }
  else {
    if (ptr->g < -1) {
	  return (frct_LFRCT_MIN);
    }
  }

  temp = (ptr->g << shift) + (ptr->x >> (TYP_LFRACT_SIZE-shift));

  if (temp > 0) {
    return(frct_LFRCT_MAX);
  } 
  else {
    if (temp < -1) {
      return(frct_LFRCT_MIN);
	}
    else {
      return(ptr->x << shift);
    }
  }
}

/* Shift right with saturation, assumption is 32 > shift >= 0 */
static inline LFract utl_sshr_tint40 (tint40_t *ptr, tuint shift)
{
  tint temp;
 
  temp = ptr->g >> shift;

  if (temp > 0) {
    return(frct_LFRCT_MAX);
  } 
  else {
    if (temp < -1) {
      return(frct_LFRCT_MIN);
	}
    else {
      return((ptr->x >> shift) + ((LFract)ptr->g << (TYP_LFRACT_SIZE-shift)));
    }
  }
}

static inline LFract utlDotSat (Fract *x, tint Qx, Fract *y, tint Qy, tint n,
                                tint Qres)
{
  tint i, diffQ;
  tint40_t lsum = {0,0};
  tint40_t prod = {0,0};
  
  cga_LOOP_COUNT_INFO(1,,,,)
  for (i = 0; i < n; i++) {
    prod.x = frctMul (x[i], Qx, y[i], Qy, (Qx+Qy));
    utl_add_tint40(&lsum, &lsum, &prod);
  }

  diffQ = (Qx+Qy-Qres);

  if (diffQ > 0) {
    return(utl_sshr_tint40(&lsum, diffQ));
  }
  else {
    return(utl_sshl_tint40(&lsum, -diffQ));
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
 * Assumptions
 *
 *	  n > 0 (i.e loop count is non-zero)
 *    Inputs x and y are aligned to best alignment for the processor
 *    n <= 256 (because 8-bit guard may be used in accumulation)
 *    Results not guaranteed when any 4 consecutive inputs are -32768 (negative max)
 *
 *  This API is created to utilize more MACs per cycle on some architectures
 *  like 64x. On 64x, this implementation when data inputs are cached, will result in 4
 *  MACs per cycle. The implementation (in src/64x/utilNumPort.h uses
 *  the _dotp2 intrinsic which tranlstes to DOTP2 instruction which does not
 *  have guard in h/w for the sum of the two products it is doing, in one
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
 * Note: C model implementation below does not care about assumptions above,
 *       because it simply defaults to utlDotSat
 *****************************************************************************/
static inline LFract utlDotSatAlignedInp (Fract *x, tint Qx, Fract *y, 
                         tint Qy, tint n, tint Qres)
{
  return(utlDotSat(x, Qx, y, Qy, n, Qres));
}
#endif

/*!
 *
 * @brief  Main macro for accessing configuration bit fields
 *
 * Returns bitfield of size <b>c</b> bits at <b>b</b> offset from input param <b>a</b> of size tuint.     
 *
 * @param[in]  a     input parameter of size tuint containing bit field
 * @param[in]  b     bit offset within bit field
 * @param[in]  c     number of bits used by that parameter
 */
#define utlGetBitfield(a,b,c)    (((a)>>(b)) & ((1U<<(c))-1))

/**
 *
 * @brief  Main macro for accessing configuration bit fields
 *
 * Writes bitfield <b>x</b> of size <b>c</b> bits at <b>b</b> offset from input param <b>a</b> of size tuint.     
 *
 * @param[in]   a    input parameter of size tuint containing bit field
 * @param[in]   x    new value of parameter that is required to be packed in this bit field 
 * @param[in]   b    bit offset within bit field
 * @param[in]   c    number of bits used by that parameter
 *
 * @remark This macro enforces strict setting to prevent overflow into other bits cost program space for additional protection */
#define utlSetBitfield(a,x,b,c)  (a) &= ~(((1U<<(c))-1)<<(b)), \
                                   (a) |= (((tuint)(x) & ((1U<<(c))-1))<<(b))

/** 
 *
 * @brief  Main macro for accessing configuration bit fields
 *
 * Returns bitfield of size <b>c</b> bits at <b>b</b> offset from input param <b>a</b> of size tulong.     
 *
 * @param[in]   a    input parameter of size tulong containing bit field
 * @param[in]   b    bit offset within bit field
 * @param[in]   c    number of bits used by that parameter
 */
#define utlGetBitfieldTulong(a,b,c)  (((a)>>(b)) & (((tulong)1<<(c))-1))

/**
 * @brief  Main macro for accessing configuration bit fields
 *
 * Writes bitfield <b>x</b> of size <b>c</b> bits at <b>b</b> offset from input param <b>a</b> of size tulong.     
 *
 * @param[in]   a    input parameter of size tulong containing bit field
 * @param[in]   x    new value of parameter that is required to be packed in this bit field 
 * @param[in]   b    bit offset within bit field
 * @param[in]   c    number of bits used by that parameter
 *
 * @remark This macro enforces strict setting to prevent overflow into other bits cost program space for additional protection */
#define utlSetBitfieldTulong(a,x,b,c)  (a) &= ~((((tulong)1<<(c))-1)<<(b)), \
                                          (a) |= (((tulong)(x) & (((tulong)1<<(c))-1))<<(b))

/*! 
 * @brief Assert-type macro that checks the size of structures.
 *
 * This assertion controls the compilation process. If the postulate about
 * the structure size is not TRUE the compilation would result in error.
 * Otherwise, it would proceed.
 *
 * @param[in]  postulate A boolean expression that postulates size of a structure
 *
 * @retval `<compilation error>` - if postulate is FALSE 
 * @retval `<compilation proceeds>` - if postulate is TRUE
 *
 * <b>Typical Usage:</b><BR>
 * <code>
 * ... <arbitrary C code>....<BR>
 *
 * utlCheckSize(sizeof(xxxInst_t) <= MAX_BUF_SIZE); <BR>
 *
 * ... <code that may crash when sizeof(xxxInst_t) > MAX_BUF_SIZE> <BR>
 * </code>
 */
#define utlCheckSize(postulate)                                         \
   do {                                                                 \
       typedef struct {                                                 \
         tword	NegativeSizeIfPostulateFalse[((int)(postulate))*2 - 1];	\
       } PostulateCheck_t;                                              \
   }                                                                    \
   while (0)
/* @} */ /* ingroup */
#endif /* _UTL_H */

/* nothing past this point */
