/******************************************************************************
 * FILE PURPOSE: Speech and Noise level estimation
 *               
 ******************************************************************************
 * FILE NAME: snl.h  
 *
 * DESCRIPTION: Implements active speech and noise level estimation for the purposes
 *              of voice doctor 
 *
 * TABS: NONE
 *
 * REVISION HISTORY:
 *
 * (C) Copyright 2008, Texas Instruments, Inc.
 *****************************************************************************/
#ifndef _SNL_H
#define _SNL_H

/**
 *  @file   snl.h
 *  @brief  Contains external APIs for the SNL module.
 *
 *  (C) Copyright 2008, Texas Instruments, Inc.
 */

/** 
 *  @defgroup SNL Siganl and Noise Level estimator (SNL)
 *
 *  @brief SNL estimates the signal level and background noise level of the incoming voice signal.
 *         SNL operates at 666.67Hz sampling rate and it's frame-based there is a 1.5 ms minimum  
 *         limitation for the frame size.  At each frame, a logic is used to decide whether the 
 *         current segment of signal is more voice like or more noise like to adjust internal parameters 
 *         of the SNL to update the noise and signal level.
 *         
 *
 */
/** @ingroup SNL */
/* @{ */

#include <ti/mas/types/types.h>              /* DSP types            */

/**
 *  @name SNL error codes
 *
 */

enum{
  snl_NOERR    = 0,  /**< success, no error   */
  snl_ERR      = 1,  /**< failed              */
  snl_BADPARAM = 2   /**< Returned from function with wrong parameters */       
};                            

/**
 *  @name SNL sampling rate factor
 *  \remark These numbers are used as multipliers, so can't be changed to other values.
 *
 */

enum{
  snl_SRATE_FACTOR_8K  = 1, /**< sampling rate is 8kHz  */
  snl_SRATE_FACTOR_16K = 2  /**< sampling rate is 16kHz */
};                           

/**
 *  @def  snl_ENV_Q
 *        The Fract Q format for the adaptive envelope threshold returned by
 *        snlGetLevels().  	 
 */

#define snl_ENV_Q               3
/**
 *  @def  snl_INVALID_POW_DB
 *        The initialization value for the signal and noise dB value.  It's not a 
 *        valid result for either signal or noise level estimation.  
 *  @sa 
 */

#define snl_INVALID_POW_DB      0x7F

/**
 *  @brief Obtain from SNL the memory requirements of an instance. 
 *  \remark This must be the first function to be called before calling any other SNL APIs
 *            
 *
 *  @param[out]   instsize  The size of the instance
 *
 *  @return           \link SNL::snl_NOERR     snl_NOERR,   \endlink \n
 *                    \link SNL::snl_ERR       snl_ERR      \endlink: if instsize==NULL 
 */

tint snlGetSizes (tint *instsize);

/**
 *  @brief  Initialize signal level estimator
 *  \remark This is the function to call after calling snlGetSizes(), only samp_rate is user configurable.
 *          After this function call, both signal and noise level are initialized to 
 *          \link SNL::snl_INVALID_POW_DB snl_INVALID_POW_DB,\endlink  which is not considered
 *          a valid result from SNL
 *
 *  @param[out]       *inst      Pointer to the SNL instance
 *  @param[in]        samp_rate  \link SNL::snl_SRATE_FACTOR_8K  snl_SRATE_FACTOR_8K  \endlink \n
 *                               \link SNL::snl_SRATE_FACTOR_16K snl_SRATE_FACTOR_16K \endlink \n 
 *
 *  @return           \link SNL::snl_NOERR     snl_NOERR,   \endlink  \n
 *                    \link SNL::snl_ERR       snl_ERR,     \endlink :if inst==NULL \n 
 *                    \link SNL::snl_BADPARAM  snl_BADPARAM \endlink :if samp_rate is not one of snl_SRATE_FACTOR_8K
 *                                                                    and snl_SRATE_FACTOR_16K
 *
 */

tint snlInit      (void *inst, tint samp_rate);

/**
 *  @brief  Signal and noise level estimator
 *  \remark This is the function that processes the signal and calculates various parameters needed for level
 *          estimation.  This function needs to be called every frame. There is a limitation of 1.5ms for the 
 *          frame size, which will equivalent to 12 for the nSamps parameter for 8kHz sampling rate and 24
 *          for the nSamps parameter for 16kHz sampling rate.
 *
 *  @param[in, out]   *inst     Pointer to the instance
 *  @param[in]        *samples  Pointer to samples
 *  @param[in]        nSamps    Number of samples
 *
 *  @return           \link SNL::snl_NOERR     snl_NOERR   \endlink
 */

tint snlProcess   (void *inst, linSample *samples, tuint nSamps);

/**
 *  @brief  Get speech and noise levels
 *  \remark This is the function to be called whenever the levels are needed.  It can be called every frame
 *          or when requested.
 *
 *  @param[in]   *inst    Pointer to the instance
 *  @param[out]  *spchdB  pointer to returned speech power level in Q0 dB
 *  @param[out]  *noisedB pointer to returned noise  power level in Q0 dB
 *  @param[out]  *thresh  pointer to returned adaptive threshold for envelope 
 *                        in Q\link SNL::snl_ENV_Q snl_ENV_Q \endlink linear amplitude.
 * 
 *  @return           \link SNL::snl_NOERR     snl_NOERR   \endlink
 */

tint snlGetLevels (void *inst, Fract *spchdB, Fract *noisedB, UFract *thresh);

/* @} */ /* ingroup SNL */

#endif

/* Nothing past this point */
