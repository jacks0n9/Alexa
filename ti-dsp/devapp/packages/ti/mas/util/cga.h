/* ========================================================================== */
/**
 *  @file   cga.h
 *
 *  path    /dsps_gtmas/ti/mas/util/cga.h
 *
 *  @brief  Code Generation Assistance header file.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef _CGA_H
#define _CGA_H

/** @defgroup  CGA 
 * This module contains macros for assisting code generation. 
 * The macros helps in writing portable code that can be used across the platform.
 * 
 */

/** @ingroup CGA */
/* @{ */

#ifdef ti_targets_C54
#define _CGAPORT_C54 1
#else
#define _CGAPORT_C54 0
#endif

#ifdef ti_targets_C54_far
#define _CGAPORT_C54f 1
#else
#define _CGAPORT_C54f 0
#endif

#ifdef ti_targets_C55 
#define _CGAPORT_C55 1
#else
#define _CGAPORT_C55 0
#endif

#ifdef ti_targets_C55_large
#define _CGAPORT_C55L 1
#else
#define _CGAPORT_C55L 0
#endif

#ifdef ti_targets_C64
#define _CGAPORT_C64 1
#else
#define _CGAPORT_C64 0
#endif

#ifdef ti_targets_C64_big_endian
#define _CGAPORT_C64_BIG_ENDIAN 1
#else
#define _CGAPORT_C64_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C674
#define _CGAPORT_C674 1
#else
#define _CGAPORT_C674 0
#endif

#ifdef ti_targets_C674_big_endian
#define _CGAPORT_C674_BIG_ENDIAN 1
#else
#define _CGAPORT_C674_BIG_ENDIAN 0
#endif


#ifdef ti_targets_C64P
#define _CGAPORT_C64P 1
#else
#define _CGAPORT_C64P 0
#endif

#ifdef ti_targets_C64P_big_endian
#define _CGAPORT_C64P_BIG_ENDIAN 1
#else
#define _CGAPORT_C64P_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C66
#define _CGAPORT_C66 1
#else
#define _CGAPORT_C66 0
#endif

#ifdef ti_targets_C66_big_endian
#define _CGAPORT_C66_BIG_ENDIAN 1
#else
#define _CGAPORT_C66_BIG_ENDIAN 0
#endif

#ifdef gnu_targets_MVArm9
#define _CGAPORT_MVARM9 1
#else
#define _CGAPORT_MVARM9 0
#endif

#ifdef gnu_targets_arm_GCArmv6
#define _CGAPORT_ARM_GCARMV6 1
#else
#define _CGAPORT_ARM_GCARMV6 0
#endif

#ifdef gnu_targets_arm_GCArmv7A
#define _CGAPORT_ARM_GCARMV7A 1
#else
#define _CGAPORT_ARM_GCARMV7A 0
#endif


#ifdef ti_targets_arm_Arm11
#define _CGAPORT_ARM_ARM11 1
#else
#define _CGAPORT_ARM_ARM11 0
#endif

#ifdef ti_targets_arm_Arm11_big_endian
#define _CGAPORT_ARM_ARM11_BIG_ENDIAN 1
#else
#define _CGAPORT_ARM_ARM11_BIG_ENDIAN 0
#endif


#ifdef microsoft_targets_arm_WinCE
#define _CGAPORT_WINCE 1
#else
#define _CGAPORT_WINCE 0
#endif

#ifdef microsoft_targets_VC98
#define _CGAPORT_VC98 1
#else
#define _CGAPORT_VC98 0
#endif

#if _CGAPORT_C54 || _CGAPORT_C54f
#include <ti/mas/util/src/c54/cga.h>
#elif _CGAPORT_C55 || _CGAPORT_C55L
#include <ti/mas/util/src/c55/cga.h>
#elif _CGAPORT_C64 || _CGAPORT_C64_BIG_ENDIAN || _CGAPORT_C674 || _CGAPORT_C674_BIG_ENDIAN || _CGAPORT_C64P || _CGAPORT_C64P_BIG_ENDIAN || _CGAPORT_C66 || _CGAPORT_C66_BIG_ENDIAN 
#include <ti/mas/util/src/c64/cga.h>
#elif _CGAPORT_MVARM9 || _CGAPORT_ARM_GCARMV6 || _CGAPORT_ARM_GCARMV7A || _CGAPORT_ARM_ARM11 || _CGAPORT_ARM_ARM11_BIG_ENDIAN
#include <ti/mas/util/src/arm/cga.h>
#elif _CGAPORT_WINCE
#include <ti/mas/util/src/arm/cga.h>
#elif _CGAPORT_VC98
#include <ti/mas/util/src/arm/cga.h>
#else
#error invalid target
#endif
/* @} */
#endif /* _CGAPORT_H */

/* Nothing past this point */



