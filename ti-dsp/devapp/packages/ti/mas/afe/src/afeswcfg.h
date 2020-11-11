/* ========================================================================== */
/**
 *  @file   afeswcfg.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afeswcfg.h
 *
 *  @brief  AFE's compile time software configuration parameters
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __AFESWCFG_H__
#define __AFESWCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AFE_MAXPKTSIZE          (256)

#ifdef GG_AICLEO
#define AFE_MAXTDMTIMESLOTS     (3)
#else
#endif
#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* __AFESWCFG_H__ */
