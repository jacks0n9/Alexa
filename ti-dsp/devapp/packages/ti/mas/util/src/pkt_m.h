/******************************************************************************
 * FILE PURPOSE: To include the header file specific to platforms.
 ******************************************************************************
 * FILE NAME:   pkt_m.h
 *
 * DESCRIPTION: Based on compiler switch, include platform specific header file.
 *
 * (C) Copyright 2006, Texas Instruments, Inc.
 *****************************************************************************/

#ifndef _PKT_M_H
#define _PKT_M_H

#ifdef ti_targets_C54
#define _PKT_M_C54 1
#else
#define _PKT_M_C54 0
#endif

#ifdef ti_targets_C54_far
#define _PKT_M_C54_FAR 1
#else
#define _PKT_M_C54_FAR 0
#endif

#ifdef ti_targets_C55
#define _PKT_M_C55 1
#else
#define _PKT_M_C55 0
#endif

#ifdef ti_targets_C55_large
#define _PKT_M_C55L 1
#else
#define _PKT_M_C55L 0
#endif

#ifdef ti_targets_C64
#define _PKT_M_C64 1
#else
#define _PKT_M_C64 0
#endif

#ifdef ti_targets_C64_big_endian
#define _PKT_M_C64_BIG_ENDIAN 1
#else
#define _PKT_M_C64_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C674
#define _PKT_M_C674 1
#else
#define _PKT_M_C674 0
#endif

#ifdef ti_targets_C674_big_endian
#define _PKT_M_C674_BIG_ENDIAN 1
#else
#define _PKT_M_C674_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C66
#define _PKT_M_C66 1
#else
#define _PKT_M_C66 0
#endif

#ifdef ti_targets_C66_big_endian
#define _PKT_M_C66_BIG_ENDIAN 1
#else
#define _PKT_M_C66_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C64P
#define _PKT_M_C64P 1
#else
#define _PKT_M_C64P 0
#endif

#ifdef ti_targets_C64P_big_endian
#define _PKT_M_C64P_BIG_ENDIAN 1
#else
#define _PKT_M_C64P_BIG_ENDIAN 0
#endif

#ifdef gnu_targets_MVArm9
#define _PKT_M_MVARM9 1
#else
#define _PKT_M_MVARM9 0
#endif

#ifdef gnu_targets_arm_GCArmv6
#define _PKT_M_ARM_GCARMV6 1
#else
#define _PKT_M_ARM_GCARMV6 0
#endif

#ifdef gnu_targets_arm_GCArmv7A
#define _PKT_M_ARM_GCARMV7A 1
#else
#define _PKT_M_ARM_GCARMV7A 0
#endif

#ifdef ti_targets_arm_Arm11
#define _PKT_M_ARM_ARM11 1
#else
#define _PKT_M_ARM_ARM11 0
#endif

#ifdef ti_targets_arm_Arm11_big_endian
#define _PKT_M_ARM_ARM11_BIG_ENDIAN 1
#else
#define _PKT_M_ARM_ARM11_BIG_ENDIAN 0
#endif

#ifdef microsoft_targets_arm_WinCE
#define _PKT_M_WINCE 1
#else
#define _PKT_M_WINCE 0
#endif

#ifdef microsoft_targets_VC98
#define _PKT_M_VC98 1
#else
#define _PKT_M_VC98 0
#endif


#if (_PKT_M_C54_FAR || _PKT_M_C54)
#include <ti/mas/util/src/c54/pkt_m.h>
#elif (_PKT_M_C55 || _PKT_M_C55L)
#include <ti/mas/util/src/c55/pkt_m.h>
#elif (_PKT_M_C64 || _PKT_M_C64_BIG_ENDIAN || _PKT_M_C674 || _PKT_M_C674_BIG_ENDIAN ||_PKT_M_C64P || _PKT_M_C64P_BIG_ENDIAN || _PKT_M_C66 || _PKT_M_C66_BIG_ENDIAN )
#include <ti/mas/util/src/c64/pkt_m.h>
#elif (_PKT_M_MVARM9 || _PKT_M_ARM_GCARMV6 || _PKT_M_ARM_GCARMV7A || _PKT_M_WINCE || _PKT_M_VC98 || _PKT_M_ARM_ARM11 || _PKT_M_ARM_ARM11_BIG_ENDIAN)
#include <ti/mas/util/src/arm/pkt_m.h>
#else
#error invalid target
#endif

#endif /* _PKT_M_H */

/* nothing past this point */
