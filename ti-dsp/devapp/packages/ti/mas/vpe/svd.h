/******************************************************************************
 * FILE PURPOSE: Simple Voice Activity Detection (VAD) module
 *               
 ******************************************************************************
 * FILE NAME: svd.h  
 *
 * DESCRIPTION: Implements a (very) simple Voice Activity Detector (VAD) based 
 *              almost solely on the SNL module.
 *
 * TABS: NONE
 *
 * REVISION HISTORY:
 *
 * (C) Copyright 2009, Texas Instruments, Inc.
 *****************************************************************************/
#ifndef _SVD_H
#define _SVD_H

/**
 *  @file   svd.h
 *  @brief  Contains external APIs for the SVD module.
 *
 *  (C) Copyright 2009, Texas Instruments, Inc.
 */

/** 
 *  @defgroup SVD Simple Voice activity Detector (SVD)
 *
 *  @brief SVD .
 *
 */
/** @ingroup SVD */
/* @{ */

#include <ti/mas/types/types.h>              /* DSP types            */

/**
 *  @name SVD error codes
 *
 */
enum{
  svd_NOERR    = 0,  /**< success, no error   */
  svd_ERR      = 1,  /**< failed              */
  svd_BADPARAM = 2   /**< Returned from function with wrong parameters */       
};                            

/**
 *  @name SVD decision codes
 *
 */
enum{
  svd_VOICE_UNDEFINED = -1,  /**< VAD decision undefined */
  svd_VOICE_INACTIVE  = 0,   /**< VAD decision NO VOICE  */
  svd_VOICE_ACTIVE    = 1    /**< VAD decision VOICE     */
};                           

/**
 *  @name SVD sampling rate factor
 *  \remark These numbers are used as multipliers, so can't be changed to other values.
 *
 */
enum{
  svd_SRATE_8K  = 1,         /**< sampling rate is 8kHz  */
  svd_SRATE_16K = 2          /**< sampling rate is 16kHz */
};                           

/**
 *  @def  svd_ENV_Q
 *        The Fract Q format for the adaptive envelope threshold returned by
 *        svdGetLevels().  	 
 */
#define svd_ENV_Q               3

/**
 *  @def  svd_PWR_Q
 *        The Fract Q format for the speech and noise power level returned by
 *        svdGetLevels().  	 
 */
#define svd_PWR_Q               0

/**
 *  @def  svd_INVALID_POW_DB
 *        The initialization value for the signal and noise dB value.  It's not a 
 *        valid result for either signal or noise level estimation.  
 *  @sa 
 */
#define svd_INVALID_POW_DB      0x7F

/**
 *  @brief Obtain from SVD the instance memory requirements. 
 *  \remark This must be the first function to be called before calling any other SVD APIs
 *            
 *  @param[out]   instsize  The size of the instance
 *
 *  @return           \link SVD::svd_NOERR     svd_NOERR,   \endlink \n
 *                    \link SVD::svd_ERR       svd_ERR      \endlink: if instsize==NULL 
 */
tint svdGetSizes (tint *instsize);

/**
 *  @brief  Initialize simple voice activity detector
 *  \remark This function is called after svdGetSizes(), and initializes the instance. 
 *
 *  @param[out]       *inst      Pointer to the SVD instance
 *  @param[in]        samp_rate  \link SVD::svd_SRATE_FACTOR_8K  svd_SRATE_FACTOR_8K  \endlink \n
 *                               \link SVD::svd_SRATE_FACTOR_16K svd_SRATE_FACTOR_16K \endlink \n 
 *
 *  @return           \link SVD::svd_NOERR     svd_NOERR,   \endlink  \n
 *                    \link SVD::svd_ERR       svd_ERR,     \endlink :if inst==NULL \n 
 *                    \link SVD::svd_BADPARAM  svd_BADPARAM \endlink :if samp_rate is not one of 
 *                                                                       svd_SRATE_FACTOR_8K
 *                                                                    or svd_SRATE_FACTOR_16K
 *
 */
tint svdInit (void *inst, tint samp_rate);

/**
 *  @brief  Simple VAD detection process
 *  \remark This is the function that processes the signal and makes the Voice/Noise decision.  The 
 *          function should be called every frame. The frame size must be at least 1.5ms (12 for 8kHz, 
 *          24 for 16kHz).
 *
 *  @param[in, out]   *inst     Pointer to the instance
 *  @param[in]        *samples  Pointer to samples
 *  @param[in]        nSamps    Number of samples, or frame size
 *
 *  @return           \link SVD::svd_NOERR     svd_NOERR,   \endlink  \n
 *                    \link SVD::svd_ERR       svd_ERR,     \endlink  \n 
 */
tint svdProcess (void *inst, linSample *samples, tuint nSamps);

/**
 *  @brief  Get Voice decision
 *  \remark This is the function to call when a Voice/Noise decision is needed.  It can be called 
 *          as needed.
 *
 *  @param[in]   *inst    Pointer to the instance
 *                        
 *  @return      \link SVD::svd_VOICE_UNDEFINED svd_VOICE_UNDEFINED \endlink \n
 *               \link SVD::svd_VOICE_INACTIVE  svd_VOICE_INACTIVE  \endlink \n 
 *               \link SVD::svd_VOICE_ACTIVE    svd_VOICE_ACTIVE    \endlink \n 
 */
tint svdGetDecision (void *inst);

/**
 *  @brief  Get speech and noise levels
 *  \remark This is the function to be called whenever the levels are needed.  
 *          It can be called every frame or when requested.
 *
 *  @param[in]   *inst    Pointer to the instance
 *  @param[out]  *spchdB  pointer to returned speech power level in Q0 dB
 *  @param[out]  *noisedB pointer to returned noise  power level in Q0 dB
 *  @param[out]  *thresh  pointer to returned adaptive threshold for envelope 
 *                        in Q\link SVD::svd_ENV_Q svd_ENV_Q \endlink linear amplitude.
 * 
 *  @return           \link SVD::svd_NOERR     svd_NOERR   \endlink
 */

tint svdGetLevels (void *inst, Fract *spchdB, Fract *noisedB, UFract *thresh);

/* @} */ /* ingroup SVD */

#endif

/* Nothing past this point */
