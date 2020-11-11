#ifndef _CONST_H
#define _CONST_H
/* ========================================================================== */
/**
 *  @file   const.h
 *
 *  path    /dsps_gtmas/ti/mas/types/const.h
 *
 *  @brief  This file contains basic DSP constant definitions.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2008
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/** @defgroup  CONST */

/** @ingroup CONST */
/* @{ */

/** Default voice bandwith sampling rate. 
 *
 * Voice Sampling Rate in Hz */
#define const_VOX_SAMPLING_RATE      8000    
/** Default voice bandwith sampling rate.
 *
 *  Max Voice Sampling Rate in Hz */
#define const_MAX_VOX_SAMPLING_RATE 16000    

/** Companding laws 8-bit A-law*/
#define const_COMP_ALAW_8     0       
#define const_COMP_FIRST      const_COMP_ALAW_8
/** Companding laws 8-bit u-law*/
#define const_COMP_MULAW_8    1       
/** n-bit linear, i.e. no companding */
#define const_COMP_LINEAR     2       
#define const_COMP_LAST       const_COMP_LINEAR

/* @} */ /* ingroup */

#endif /* _CONST_H */
/* nothing after this point */
