/* ========================================================================== */
/**
 *  @file   afeutl.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afeutl.h
 *
 *  @brief  AFE's bit-field manipulation macros
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __AFEUTL_H__
#define __AFEUTL_H__

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/**
 * @brief Main macros for accessing configuration bit-fields
 *        input parameter a is tuint containing bit-field
 *        b is the bit offset (counting from 0) within bit-field
 *        c is the number of bits used bu that parameter.
 *        x is the new value of the parameter that is packed
 *        in this bit-field.
 */
#define UTL_GET_BITFIELD(a,b,c)               \
  (((a)>>(b)) & ((1U<<(c))-1))

/**
 * @brief The following macro enforces strict setting to prevent overflow
 *        into other bits, would cost program space for additional 
 *        protection
 */
#define UTL_SET_BITFIELD(a,x,b,c)             \
  (a) &= ~(((1U<<(c))-1)<<(b)),               \
  (a) |= (((tuint)(x) & ((1U<<(c))-1))<<(b))

/**
 * @brief Macros above for tulong (32-bit) bit-fields
 *
 */
#define UTL_GET_BITFIELD_TULONG(a,b,c)        \
  (((a)>>(b)) & (((tulong)1<<(c))-1))

#define UTL_SET_BITFIELD_TULONG(a,x,b,c)      \
  (a) &= ~((((tulong)1<<(c))-1)<<(b)),        \
  (a) |= (((tulong)(x) & (((tulong)1<<(c))-1))<<(b))

#endif /* __AFEUTL_H__ */



