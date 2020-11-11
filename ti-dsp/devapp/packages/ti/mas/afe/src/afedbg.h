/* ========================================================================== */
/**
 *  @file   afedbg.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afedbg.h
 *
 *  @brief  AFE debug macros
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __AFEDBG_H__
#define __AFEDBG_H__

#ifdef __cplusplus
extern "C" {
#endif

extern char *str_info;
extern char *str_error;
extern char *str_success;

/* If DBGLEV is not defined then make it 0 */
#ifndef DBGLEV
#define DBGLEV 0
#endif

/* Debug level 1 ==> Print all debug messages to log */
#if (DBGLEV==1)
#define DBGLOG1(a) LOG_printf a;
#define DBGLOG2(a) LOG_printf a;
#define DBGLOG3(a) LOG_printf a;

/* Debug level 2 ==> Print critical and semicritical messages to log */
#elif (DBGLEV==2)
#define DBGLOG1(a)
#define DBGLOG2(a) LOG_printf a;
#define DBGLOG3(a) LOG_printf a;

/* Debug level 3 ==> Print critical messages to log */
#elif (DBGLEV==3)
#define DBGLOG1(a)
#define DBGLOG2(a)
#define DBGLOG3(a) LOG_printf a;

/* No debugging enabled */
#else
#define DBGLOG1(a)
#define DBGLOG2(a)
#define DBGLOG3(a)
#endif
/* End of debug macros */

/* Profile macros */
#ifdef PROFILE
#define PROFSTART(a) STS_set(&(a), (CLK_gethtime() * CLK_cpuCyclesPerHtime()));
#define PROFSTOP(a) STS_delta(&(a), (CLK_gethtime() * CLK_cpuCyclesPerHtime()));
#else
#define PROFSTART(a)
#define PROFSTOP(a)
#endif  /* PROFILE */
 
#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* __AFEDBG_H__ */
