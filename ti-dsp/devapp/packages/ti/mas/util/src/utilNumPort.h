/**
 *  @file   utilNumPort.h
 *  @brief  Contains processor specific port file includes for portability.
 *
 *  (C) Copyright 2007, Texas Instruments, Inc.
 */
#ifndef _UTILNUMPORT_H
#define _UTILNUMPORT_H

#ifdef ti_targets_C54
#define _UTILNUMPORT_C54 1
#else
#define _UTILNUMPORT_C54 0
#endif

#ifdef ti_targets_C54_far
#define _UTILNUMPORT_C54_FAR 1
#else
#define _UTILNUMPORT_C54_FAR 0
#endif

#ifdef ti_targets_C55
#define _UTILNUMPORT_C55 1
#else
#define _UTILNUMPORT_C55 0
#endif

#ifdef ti_targets_C55_large
#define _UTILNUMPORT_C55L 1
#else
#define _UTILNUMPORT_C55L 0
#endif

#ifdef ti_targets_C64
#define _UTILNUMPORT_C64 1
#else
#define _UTILNUMPORT_C64 0
#endif

#ifdef ti_targets_C64_big_endian
#define _UTILNUMPORT_C64_BIG_ENDIAN 1
#else
#define _UTILNUMPORT_C64_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C674
#define _UTILNUMPORT_C674 1
#else
#define _UTILNUMPORT_C674 0
#endif

#ifdef ti_targets_C674_big_endian
#define _UTILNUMPORT_C674_BIG_ENDIAN 1
#else
#define _UTILNUMPORT_C674_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C66
#define _UTILNUMPORT_C66 1
#else
#define _UTILNUMPORT_C66 0
#endif

#ifdef ti_targets_C66_big_endian
#define _UTILNUMPORT_C66_BIG_ENDIAN 1
#else
#define _UTILNUMPORT_C66_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C64P
#define _UTILNUMPORT_C64P 1
#else
#define _UTILNUMPORT_C64P 0
#endif

#ifdef ti_targets_C64P_big_endian
#define _UTILNUMPORT_C64P_BIG_ENDIAN 1
#else
#define _UTILNUMPORT_C64P_BIG_ENDIAN 0
#endif

#ifdef gnu_targets_MVArm9
#define _UTILNUMPORT_MVARM9 1
#else
#define _UTILNUMPORT_MVARM9 0
#endif

#ifdef ti_targets_arm_Arm11
#define _UTILNUMPORT_ARM_ARM11 1
#else
#define _UTILNUMPORT_ARM_ARM11 0
#endif

#ifdef ti_targets_arm_Arm11_big_endian
#define _UTILNUMPORT_ARM_ARM11_BIG_ENDIAN 1
#else
#define _UTILNUMPORT_ARM_ARM11_BIG_ENDIAN 0
#endif

#ifdef gnu_targets_arm_GCArmv6
#define _UTILNUMPORT_ARM_GCARMV6 1
#else
#define _UTILNUMPORT_ARM_GCARMV6 0
#endif

#ifdef gnu_targets_arm_GCArmv7A
#define _UTILNUMPORT_ARM_GCARMV7A 1
#else
#define _UTILNUMPORT_ARM_GCARMV7A 0
#endif

#ifdef microsoft_targets_arm_WinCE
#define _UTILNUMPORT_WINCE 1
#else
#define _UTILNUMPORT_WINCE 0
#endif

#ifdef microsoft_targets_VC98
#define _UTILNUMPORT_VC98 1
#else
#define _UTILNUMPORT_VC98 0
#endif


#if _UTILNUMPORT_C54 || _UTILNUMPORT_C54_FAR 
#include <ti/mas/util/src/c54/utilNumPort.h>
#elif _UTILNUMPORT_C55 || _UTILNUMPORT_C55L
#include <ti/mas/util/src/c55/utilNumPort.h>
#elif _UTILNUMPORT_C64 || _UTILNUMPORT_C64_BIG_ENDIAN || _UTILNUMPORT_C674 || _UTILNUMPORT_C674_BIG_ENDIAN || _UTILNUMPORT_C64P || _UTILNUMPORT_C64P_BIG_ENDIAN || _UTILNUMPORT_C66 || _UTILNUMPORT_C66_BIG_ENDIAN 
#include <ti/mas/util/src/c64/utilNumPort.h>
#elif _UTILNUMPORT_MVARM9
#elif _UTILNUMPORT_WINCE
#elif _UTILNUMPORT_VC98
#elif _UTILNUMPORT_ARM_GCARMV6
#elif _UTILNUMPORT_ARM_GCARMV7A
#elif _UTILNUMPORT_ARM_ARM11
#elif _UTILNUMPORT_ARM_ARM11_BIG_ENDIAN
#else
#error invalid target
#endif

#endif                          /* _UTILNUMPORT_H */

/* Nothing past this point */
