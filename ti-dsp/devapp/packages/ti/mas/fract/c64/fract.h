/* ========================================================================== */
/**
 *  @file   
 *
 *  path    /dsps_gtmas/ti/mas/fract/c64/fract.h
 *
 *  @brief  Contains macros for manipulation of fractional types.
 *          This is platform specific version. It is automatically
 *          included from fract\fract.h master file. Do NOT attempt
 *          to include this file directly.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================*/

/** @ingroup FRACT */
/* @{ */

/* Since we use types we have to include them */
#include <ti/mas/types/types.h>

/* Let's define the minimum and maximum Fract/LFract values */
#define frct_UFRACT_MIN   0
#define frct_UFRACT_MAX   ((UFract)(~0))
#define frct_ULFRACT_MIN  0
#define frct_ULFRACT_MAX  ((ULFract)(~0))

/* Mask that extracts Fract from LFract, e.g. 0x0000FFFF on 16-bit DSP CPU's */
#define frct_FRACT_MASK   (((LFract)1<<TYP_FRACT_PREC)-1)
/* Maximum positive fractional number is 0x7FFF on 16-bit DSP CPU's */
#define frct_MAX          ((Fract)((~0)^(1U<<(TYP_FRACT_PREC-1))))
/* Minimum negative fractional number is 0x8000 on 16-bit DSP CPU's */
#define frct_MIN          ((Fract)(1<<(TYP_FRACT_PREC-1)))
/* Maximum positive long fractional number is 0x7FFFFFFF on 16-bit DSP CPU's */
#define frct_LFRCT_MAX    ((LFract)((~0U)^(1U<<(TYP_LFRACT_PREC-1))))
/* Minimum negative long fractional number is 0x80000000 on 16-bit DSP CPU's */
#define frct_LFRCT_MIN    ((LFract)(1<<(TYP_LFRACT_PREC-1)))

/* Macros and functions for fixed point arithmetic */

/**
 * @brief Find an absolute value of a fractional number.
 *
 * @remark May result in OVERFLOW if |x| is outside the range of positive numbers.
 * For example, frctAbs(-32768) is out of range on 16-bit CPU's.
 * If you are concerned with overflows, you must use either
 * frctAbsFS() or frctAbsLFS() macros.
 *
 * @param[in] x  input number
 *
 */
#define frctAbs(x)          (((x)>=0)?(x):(-(x)))

/**
 * @brief Saturate fractional number to [-32767,+32767] range.
 * @param[in]  f   input number (Fract)
 */
#define frctSaturateF(f)  ((Fract)(((Fract)(f)>frct_MIN)?(f):(frct_MIN | 0x1)))

/**
 * @brief Saturate LFract number to +/- frct_LFRCT_MAX range.
 * 
 * @param[in]  lf   input number (LFract)
 */
#define frctSaturateLF(lf)        \
    ((LFract)(((lf)>frct_LFRCT_MIN)?(lf):(frct_LFRCT_MIN | 0x1)))

/**
 * @brief Saturation macro for Q0 format. Results in [-32768,32767] range.
 * 
 * @param[in]  lf   input number (LFract)
 */
#define frctSaturate0(lf)         \
    ((Fract)((frctAbs(lf)>(frct_MAX))?(((lf)>0)?(frct_MAX):(frct_MIN)):(lf)))

/**
 * @brief Saturation macro for Q0 format. Results in [0,2^31-1] range.
 * 
 * @param[in]  ulf   input number (ULFract)
 */
#define frctSaturateULF2LF0(ulf)  \
    ((LFract)(((ULFract)(ulf)>(ULFract)(frct_LFRCT_MAX))?(frct_LFRCT_MAX):(ulf)))

/**
 * @brief Saturate ULFract number to [0,frct_UFRACT_MAX] range.
 *
 * @param[in]  ulf   input number (ULFract)
 *
 */
#define frctSaturateULF2UF(ulf)  \
    ((UFract)(((ULFract)(ulf)>(ULFract)(frct_UFRACT_MAX))?(frct_UFRACT_MAX):(ulf)))

/**
 * @brief Find an absolute value of a Fract number and saturate.
 *
 * @remark This function will properly saturate the result if necessary. For example,
 *         frctAbsFS(-32768) will become +32767 on 16-bit CPU's.
 *
 * @param[in]  x   input number (Fract)
 *
 */
static inline Fract frctAbsFS (Fract x)
{
  return(frctAbs(frctSaturateF(x)));
} /* frctAbsFS */

/**
 * @brief Find an absolute value of a LFract number and saturate.
 *
 * @remark This function will properly saturate the result if necessary. 
 * For example, frctAbsLFS(frct_LFRCT_MIN) will become frct_LFRCT_MAX.
 *
 * @param[in]  x   input number (LFract)
 *
 */
static inline LFract frctAbsLFS (LFract x)
{
  return(frctAbs(frctSaturateLF(x)));
} /* frctAbsLFS */

/**
 * @brief Adjust Q-format of a fractional number. ('delta' version)
 *
 * @remark Works for both Fract and LFract types. Output type is the same as input type.
 * If conversion is necessary, one should use F2LF and LF2F macros.
 *
 * @param[in]    x     input number (Fract)
 * @param[in]    dQ    amount of change for the Q-format (>0: left shift,
 *                <0: right shift, 0: no change).
 *
 */
#define frctAdjust_dQ(x,dQ)       (((dQ)>0)?((x)<<(dQ)):((x)>>(-(dQ))))

/**
 * @brief Adjust Q-format of a fractional number. ('absolute' version)
 *
 * @remark Works for both Fract and LFract types. Output type is the same as input type.
 * If conversion is necessary, one should use F2LF and LF2F macros.
 *
 * @param[in]   x      input number [Fract]
 * @param[in]   inQ    Q-format of the input number
 * @param[in]   outQ   Q-format of the result Must fit within the input number type.
 *
 */
#define frctAdjustQ(x,inQ,outQ)   frctAdjust_dQ((x),((outQ)-(inQ)))

/**
 * @brief Convert Fract to LFract and Adjust Q-format.
 *
 * @remark Output type is always LFract.
 *
 * @param[in]   f      input number (Fract)
 * @param[in]   Qf     Q-format of the input number. Must fit within the Fract.
 * @param[in]   Qlf    Q-format of the result. Must fit within the LFract.
 *
 */
#define frctF2LF(f,Qf,Qlf)        frctAdjustQ(((LFract)(f)),(Qf),(Qlf))

/**
 * @brief Convert LFract to Fract and Adjust Q-format.
 * 
 * @remark If the input number cannot fit into Fract, the result is unpredictable.
 *
 * @param[in]   lf      input number (LFract)
 * @param[in]   Qlf    Q-format of the input number. Must fit within the LFract.
 * @param[in]   Qf     Q-format of the result. Must fit within the Fract.
 *
 *****************************************************************************/
#define frctLF2F(lf,Qlf,Qf)       \
    ((Fract)(frctAdjustQ(((LFract)(lf)),(Qlf),(Qf)) & frct_FRACT_MASK))

/**
 * @brief Convert LFract to Fract, Adjust Q-format & saturate.
 *
 * @remark If the input number cannot fit into Fract, the result
 * is saturated to maximum or minimum Fract.
 *
 * @param[in]   lf      input number (LFract)
 * @param[in]   Qlf    Q-format of the input number. Must fit within the LFract.
 * @param[in]   Qf     Q-format of the result. Must fit within the Fract.
 *
 *****************************************************************************/

#define frctLF2FS(lf,Qlf,Qf)      frctSaturate0(frctAdjustQ((lf),(Qlf),(Qf)))

/**
 * @brief Convert ULFract to LFract, Adjust Q-format & saturate.
 *
 * @remark If the input number cannot fit into LFract, the result is saturated to maximum LFract. 
 * If the input number is not of ULFract type, results are unpredictable.
 *
 * @param[in]   ulf      input number (ULFract)
 * @param[in]   Qulf    Q-format of the input number. Must fit within the ULFract.
 * @param[in]   Qlf     Q-format of the result. Must fit within the LFract.
 *
 */
#define frctULF2LFS(ulf,Qulf,Qlf)   \
    frctSaturateULF2LF0(frctAdjustQ((ULFract)(ulf),(Qulf),(Qlf)))

/**
 * @brief Divide fractional number by positive power of 2.
 *
 * @remark Works only for POSITIVE powers of 2. One should use frctMulbyP2()
 *          for NEGATIVE powers of 2. This function does not support true
 *          fractional division. (i.e. 5(Qx)/2=2(Qx) no matter the Qa & Qres).
 *
 * @param[in]   a      first operand (Fract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   p2     second operand is 2^(p2)
 * @param[in]   Qres   Q-format of the result (must fit in Fract).
 *
 *****************************************************************************/
static inline Fract  frctDivbyP2 (Fract a, tint Qa, tint p2, tint Qres)
{
  Fract y;
   /* r = a / 2^(p2) */
  y = frctAdjustQ(a,Qa,Qres-p2);
  return(y);
} /* frctDivbyP2 */

/**
 * @brief Divide long fractional number by positive power of 2.
 *
 * @remark Works only for POSITIVE powers of 2. One should use frctMulLFbyP2()
 *         for NEGATIVE powers of 2. This function does not support true
 *         fractional division. (i.e. 5(Qx)/2=2(Qx) no matter the Qa & Qres).
 *
 * @param[in]   a      first operand (LFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   p2     second operand is 2^(p2)
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */

static inline LFract frctDivLFbyP2 (LFract a, tint Qa, tint p2, tint Qres)
{
  LFract y;

  /* r = a / 2^(p2) */
  y = frctAdjustQ(a,Qa,Qres-p2);
  return(y);
} /* frctDivLFbyP2 */

/**
 * @brief Divide ULFract number by positive power of 2.
 *
 * @remark Works only for POSITIVE powers of 2. One should use frctMulULFbyP2()
 *         for NEGATIVE powers of 2. This function does not support true
 *         fractional division. (i.e. 5(Qx)/2=2(Qx) no matter the Qa & Qres).
 *
 * @param[in]   a      first operand (ULFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   p2     second operand is 2^(p2)
 * @param[in]   Qres   Q-format of the result (must fit in ULFract).
 *
 *
 *****************************************************************************/
static inline ULFract frctDivULFbyP2 (ULFract a, tint Qa, tint p2, tint Qres)
{
  ULFract y;

  /* r = a / 2^(p2) */
  y = frctAdjustQ(a,Qa,Qres-p2);
  return(y);
} /* frctDivULFbyP2 */

/**
 * @brief Multiply fractional number by positive power of 2.
 *
 * @remark Works only for POSITIVE powers of 2. One should use frctDivbyP2()
 *         for NEGATIVE powers of 2.
 *
 * @param[in]   a      first operand (Fract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   p2     second operand is 2^(p2)
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */
static inline LFract frctMulbyP2 (Fract a, tint Qa, tint p2, tint Qres)
{
  LFract y;

  /* r = a * 2^(p2) */
  y = frctAdjustQ(a,Qa,Qres+p2);
  return(y);
} /* frctMulbyP2 */

/**
 * @brief Multiply long fractional number by positive power of 2.
 *
 * @remark Works only for POSITIVE powers of 2. One should use frctDivLFbyP2()
 *         for NEGATIVE powers of 2.
 *
 * @param[in]   a      first operand (LFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   p2     second operand is 2^(p2)
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */
static inline LFract frctMulLFbyP2 (LFract a, tint Qa, tint p2, tint Qres)
{
  LFract y;

  /* r = a * 2^(p2) */
  y = frctAdjustQ(a,Qa,Qres+p2);
  return(y);
} /* frctMulLFbyP2 */

/**
 * @brief Multiply ULFract number by positive power of 2.
 *
 * @remark Works only for POSITIVE powers of 2. One should use frctDivULFbyP2()
 *         for NEGATIVE powers of 2.
 *
 * @param[in]   a      first operand (ULFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   p2     second operand is 2^(p2)
 * @param[in]   Qres   Q-format of the result (must fit in ULFract).
 *
 *
 *****************************************************************************/
static inline ULFract frctMulULFbyP2 (ULFract a, tint Qa, tint p2, tint Qres)
{
  ULFract y;

  /* r = a * 2^(p2) */
  y = frctAdjustQ(a,Qa,Qres+p2);
  return(y);
} /* frctMulULFbyP2 */

/**
 * @brief Multiply two fractional numbers.
 *
 * @param[in]   a      first operand (Fract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   b      second operand (Fract)
 * @param[in]   Qb     Q-format of the second operand.
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */
static inline LFract frctMul (Fract a, tint Qa, Fract b, tint Qb, tint Qres)
{
  tint Qr;
  LFract r, y;

  r = (LFract)a * b;        /* portable but may be slow in some cases! */
  Qr = Qa + Qb;
  y = frctAdjustQ(r,Qr,Qres);
  return(y);
} /* frctMul */

/**
 * @brief Multiply a long fractional number by a fractional number.
 *
 * @param[in]   a      first operand (LFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   b      second operand (Fract)
 * @param[in]   Qb     Q-format of the second operand.
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */
static inline LFract frctMulLFbyF (LFract a, tint Qa, Fract b, tint Qb, tint Qres)
{
  tint Qr;
  LFract r, y;

  r = a * b;
  Qr = Qa + Qb;
  y = frctAdjustQ(r,Qr,Qres);
  return(y);
} /* frctMulLFbyF */

/**
 * @brief Multiply two long fractional numbers.
 *
 * @param[in]   a      first operand (LFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   b      second operand (LFract)
 * @param[in]   Qb     Q-format of the second operand.
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */
static inline LFract frctMulLFbyLF (LFract a, tint Qa,
                                    LFract b, tint Qb, tint Qres)
{
  tint Qr;
  LFract r, y;

  r = a * b;
  Qr = Qa + Qb;
  y = frctAdjustQ(r,Qr,Qres);
  return(y);
} /* frctMulLFbyLF */

/**
 * @brief Multiply two unsigned fractional numbers.
 *
 * @param[in]   a      first operand (UFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   b      second operand (UFract)
 * @param[in]   Qb     Q-format of the second operand.
 * @param[in]   Qres   Q-format of the result (must fit in ULFract).
 *
 */

static inline ULFract frctMulU (UFract a, tint Qa, UFract b, tint Qb, tint Qres)
{
  tint Qr;
  ULFract r, y;

  r = (ULFract)a * b;
  Qr = Qa + Qb;
  y = frctAdjustQ(r,Qr,Qres);
  return(y);
} /* frctMulU */

/**
 * @brief Multiply an ULFract number by an UFract number.
 *
 * @param[in]   a      first operand (ULFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   b      second operand (UFract)
 * @param[in]   Qb     Q-format of the second operand.
 * @param[in]   Qres   Q-format of the result (must fit in ULFract).
 *
 */
static inline ULFract frctMulULFbyUF (ULFract a, tint Qa,
                                      UFract b, tint Qb, tint Qres)
{
  tint Qr;
  ULFract r, y;

  r = a * b;
  Qr = Qa + Qb;
  y = frctAdjustQ(r,Qr,Qres);
  return(y);
} /* frctMulULFbyUF */

/**
 * @brief Multiply two unsigned long fractional numbers.
 *
 * @param[in]   a      first operand (ULFract)
 * @param[in]   Qa     Q-format of the first operand.
 * @param[in]   b      second operand (ULFract)
 * @param[in]   Qb     Q-format of the second operand.
 * @param[in]   Qres   Q-format of the result (must fit in ULFract).
 *
 */
static inline ULFract frctMulULFbyULF (ULFract a, tint Qa, 
                                       ULFract b, tint Qb, tint Qres)
{
  tint Qr;
  ULFract r, y;

  r = a * b;
  Qr = Qa + Qb;
  y = frctAdjustQ(r,Qr,Qres);
  return(y);
} /* frctMulULFbyULF */

/**
 * @brief Round a fractional number. (fast version)
 *
 * @param[in]   a      number to be rounded (Fract)
 * @param[in]   Qa     Q-format of the number.
 * @param[in]   Qres   Q-format of the result (must fit in Fract).
 *
 */
static inline Fract frctRoundF (Fract a, tint Qa, tint Qres)
{
  Fract x, y, one_half, mask;

  if (Qa > 0) {
    one_half = (Fract)(1 << (Qa - 1));  /* 0.5 in Qa */
    x = a + one_half;               /* a+0.5 in Qa */
    mask = (Fract)(~0 << Qa);           /* clean-up (floor) mask in Qa */
    x &= mask;                      /* floor(a+0.5) in Qa */
  }
  else
    x = a;
  y = frctAdjustQ (x, Qa, Qres);
  return(y);
} /* frctRoundF */

/**
 * @brief Round a long fractional number.
 *
 * @param[in]   a      number to be rounded (LFract)
 * @param[in]   Qa     Q-format of the number.
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */
static inline LFract frctRoundLF (LFract a, tint Qa, tint Qres)
{
  LFract x, y, one_half, mask;

  if (Qa > 0) {
    one_half = 1 << (Qa - 1);      /* 0.5 in Qa */
    x = a + one_half;               /* a+0.5 in Qa */
    mask = (~0 << Qa);             /* clean-up (floor) mask in Qa */
    x &= mask;                      /* floor(a+0.5) in Qa */
  }
  else
    x = a;
  y = frctAdjustQ (x, Qa, Qres);
  return(y);
} /* frctRoundLF */

/**
 * @brief Round a fractional number (slower for Fract numbers).
 *
 * @param[in]   a      number to be rounded (LFract)
 * @param[in]   Qa     Q-format of the number.
 * @param[in]   Qres   Q-format of the result (must fit in LFract).
 *
 */
#define frctRound(a,Qa,Qres)  frctRoundLF((LFract)(a), (Qa), (Qres))

/**
 * @brief Round an unsigned fractional number. (fast version)
 *
 * @param[in]   a      number to be rounded (UFract)
 * @param[in]   Qa     Q-format of the number.
 * @param[in]   Qres   Q-format of the result (must fit in UFract).
 */
static inline UFract frctRoundUF (UFract a, tint Qa, tint Qres)
{
  UFract x, y, one_half, mask;

  if (Qa > 0) {
    one_half = (UFract)(1 << (Qa - 1));   /* 0.5 in Qa */
    x = a + one_half;               /* a+0.5 in Qa */
    mask = (UFract)(~0 << Qa);            /* clean-up (floor) mask in Qa */
    x &= mask;                      /* floor(a+0.5) in Qa */
  }
  else
    x = a;
  y = frctAdjustQ (x, Qa, Qres);
  return(y);
} /* frctRoundUF */

/**
 * @brief Round an unsigned long fractional number.
 *
 * @param[in]   a      number to be rounded (ULFract)
 * @param[in]   Qa     Q-format of the number.
 * @param[in]   Qres   Q-format of the result (must fit in ULFract).
 *
 */
static inline ULFract frctRoundULF (ULFract a, tint Qa, tint Qres)
{
  ULFract x, y, one_half, mask;

  if (Qa > 0) {
    one_half = (ULFract)1 << (Qa - 1);      /* 0.5 in Qa */
    x = a + one_half;               /* a+0.5 in Qa */
    mask = ((ULFract)(~0 << Qa));             /* clean-up (floor) mask in Qa */
    x &= mask;                      /* floor(a+0.5) in Qa */
  }
  else
    x = a;
  y = frctAdjustQ (x, Qa, Qres);
  return(y);
} /* frctRoundLF */

/**
 * @brief Unsigned long saturated addition
 *
 * @remark Unsigned operation allows saturation to detect easily as seen 
 *              in code below, adition should only increase or keep value same.
 *
 * @param[in]   a      First Operand (ULFract)
 * @param[in]   b      Second Operand (ULFract)
 *
 */
/* Note: intrinsic below cannot be used because saturation will be to signed 
         as per h/w i.e. 0x7FFFFFFF, not 0xFFFFFFFF */
/*#define frctAddSaturateULF(a,b) _lsadd(a,b) */

static inline ULFract frctAddSaturateULF (ULFract a, ULFract b)
{
  ULFract tempULF = a;

  a += b;
  if (a < tempULF) {
    a = frct_ULFRACT_MAX;
  }
  return(a);
} /* frctAddSaturateULF */

/**
 * @brief Round an unsigned fractional number (slower for UFract's).
 *
 * @param[in]   a      number to be rounded (ULFract)
 * @param[in]   Qa     Q-format of the number.
 * @param[in]   Qres   Q-format of the result (must fit in ULFract).
 *
 */
#define frctRoundU(a,Qa,Qres)  frctRoundULF((ULFract)(a), (Qa), (Qres))

/**
 * @brief Convert float number into fractional.
 *
 * @param[in]   xf      Float number to be converted.
 * @param[in]   Qx      output Q-format (must fit into Fract).
 *
 */  
#define frctFloat2Fract(xf,Qx)  ((Fract)((xf)*(1<<Qx)+0.5f))

/**
 * @brief Convert float number into long fractional.
 *
 * @param[in]   xf      Float number to be converted.
 * @param[in]   Qx      output Q-format (must fit into LFract).
 *
 */
#define frctFloat2LFract(xf,Qx)  ((LFract)((xf)*((LFract)1<<Qx)+0.5f))
/* @} */ /* ingroup */
/* nothing past this point */
