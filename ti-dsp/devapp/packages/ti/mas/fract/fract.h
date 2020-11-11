/* ========================================================================== */
/**
 *  @file   
 *
 *  path    /dsps_gtmas/ti/mas/fract/fract.h
 *
 *  @brief  Macros for manipulation of fractional types
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */
#ifndef _FRACT_H
#define _FRACT_H

/** @defgroup  FRACT */

/** @ingroup FRACT */
/* @{ */



#ifdef ti_targets_C54
#define _FRACT_C54 1
#else
#define _FRACT_C54 0
#endif

#ifdef ti_targets_C54_far
#define _FRACT_C54_FAR 1
#else
#define _FRACT_C54_FAR 0
#endif

#ifdef ti_targets_C55
#define _FRACT_C55 1
#else
#define _FRACT_C55 0
#endif

#ifdef ti_targets_C55_large
#define _FRACT_C55_LARGE 1
#else
#define _FRACT_C55_LARGE 0
#endif


#ifdef ti_targets_C64
#define _FRACT_C64 1
#else
#define _FRACT_C64 0
#endif

#ifdef ti_targets_C64_big_endian
#define _FRACT_C64_BIG_ENDIAN 1
#else
#define _FRACT_C64_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C674
#define _FRACT_C674 1
#else
#define _FRACT_C674 0
#endif

#ifdef ti_targets_C674_big_endian
#define _FRACT_C674_BIG_ENDIAN 1
#else
#define _FRACT_C674_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C64P
#define _FRACT_C64P 1
#else
#define _FRACT_C64P 0
#endif

#ifdef ti_targets_C64P_big_endian
#define _FRACT_C64P_BIG_ENDIAN 1
#else
#define _FRACT_C64P_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C66
#define _FRACT_C66 1
#else
#define _FRACT_C66 0
#endif

#ifdef ti_targets_C66_big_endian
#define _FRACT_C66_BIG_ENDIAN 1
#else
#define _FRACT_C66_BIG_ENDIAN 0
#endif

#ifdef gnu_targets_MVArm9
#define _FRACT_MVARM9 1
#else
#define _FRACT_MVARM9 0
#endif

#ifdef gnu_targets_arm_GCArmv6
#define _FRACT_ARM_GCARMV6 1
#else
#define _FRACT_ARM_GCARMV6 0
#endif

#ifdef gnu_targets_arm_GCArmv7A
#define _FRACT_ARM_GCARMV7A 1
#else
#define _FRACT_ARM_GCARMV7A 0
#endif

#ifdef ti_targets_arm_Arm11
#define _FRACT_ARM_ARM11 1
#else
#define _FRACT_ARM_ARM11 0
#endif


#ifdef ti_targets_arm_Arm11_big_endian
#define _FRACT_ARM_ARM11_BIG_ENDIAN 1
#else
#define _FRACT_ARM_ARM11_BIG_ENDIAN 0
#endif

#ifdef microsoft_targets_arm_WinCE
#define _FRACT_WINCE 1
#else
#define _FRACT_WINCE 0
#endif

#ifdef microsoft_targets_VC98
#define _FRACT_VC98 1
#else
#define _FRACT_VC98 0
#endif

#if _FRACT_C54
#include <ti/mas/fract/c54/fract.h>
#elif _FRACT_C54_FAR
#include <ti/mas/fract/c54/fract.h>
#elif _FRACT_C55
#include <ti/mas/fract/c55/fract.h>
#elif _FRACT_C55_LARGE
#include <ti/mas/fract/c55/fract.h>
#elif _FRACT_C64
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_C64_BIG_ENDIAN
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_C674
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_C674_BIG_ENDIAN
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_C64P
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_C64P_BIG_ENDIAN
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_C66
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_C66_BIG_ENDIAN
#include <ti/mas/fract/c64/fract.h>
#elif _FRACT_MVARM9
#include <ti/mas/fract/arm/fract.h>
#elif _FRACT_WINCE
#include <ti/mas/fract/arm/fract.h>
#elif _FRACT_VC98
#include <ti/mas/fract/arm/fract.h>
#elif _FRACT_ARM_GCARMV6
#include <ti/mas/fract/arm/fract.h>
#elif _FRACT_ARM_ARM11
#include <ti/mas/fract/arm/fract.h>
#elif _FRACT_ARM_ARM11_BIG_ENDIAN
#include <ti/mas/fract/arm/fract.h>
#elif _FRACT_ARM_GCARMV7A
#include <ti/mas/fract/arm/fract.h>
#else
#error invalid target
#endif
/* @} */ /* ingroup */
#endif
/* nothing past this point */
