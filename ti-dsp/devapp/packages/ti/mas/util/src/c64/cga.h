#ifndef _CGA_C64_H
#define _CGA_C64_H
/* ========================================================================== */
/**
 *  @file   src/c64/cga.h
 *
 *  path    /dsps_gtmas/ti/mas/util/src/c64/cga.h
 *
 *  @brief  Code Generation Assistance header file for c64x.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/** @ingroup CGA */
/* @{ */
/** @brief restrict qualifier.
    @remark See compiler user manual for more information */
#define cga_RESTRICT restrict

/** @brief Loop Information Macros
*/
#define cga_PRAGMA(x) _Pragma(#x)
#define cga_LOOP_COUNT_INFO(min, max, multiple, minOften, maxOften) \
        cga_PRAGMA(PROB_ITERATE(minOften, maxOften)) \
        cga_PRAGMA(MUST_ITERATE(min, max, multiple))

/** @brief Used to express alignment assumptions */
#define cga_PTR_IS_ALIGNED(ptr, pow2) \
  _nassert(((unsigned int) (ptr) & (pow2)-1) == 0);
/* @} */
#endif
/* Nothing past this point */
