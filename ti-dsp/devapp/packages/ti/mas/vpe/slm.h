/******************************************************************************
 * FILE PURPOSE:header file of SLM (signal limiter)
 ******************************************************************************
 * FILE NAME:   slm.h
 *
 * DESCRIPTION: Contains external definitions&functions prototypes for SLIM
 *
 * (C) Copyright 2008, Texas Instruments, Inc.
 *****************************************************************************/
#ifndef _SLM_H
#define _SLM_H

/**
 *  @file   slm.h
 *  @brief  Contains external APIs for the slm module.
 *
 *  (C) Copyright 2008, Texas Instruments, Inc.
 */

/**  
 * @defgroup SLM Signal Limiter(SLM)
 * 
 * @brief When voice signal is flat-top saturated, the resulting signal tends to have more nonlinear components. 
 *        Signal limiter limits the voice signal to reduce non-linearity caused by hard limiting.\n
 *
 *        There are five modes in signal limiter. Each mode has its own set of thresholds.  When the
 *        amplitude of the sample exceeds the threshold in the selected mode, the SLM starts to take
 *        effect. Since the SLM changes wave form, it will produce certain degree of distortion depending
 *        on the mode selected and the level of the signal. \n
 *
 *        The SLM is a frame-based operation. Though there's no limitation of the frame size, bigger frame size will 
 *        reduce overhead, thus result in better MIPS efficiency.
 *
 */
/** @ingroup SLM */
/* @{ */

/* System level header files */
#include <ti/mas/types/types.h>            /* DSP types */

/**
 *  @name SLM Mode Definitions
 *  
 */
enum{
  slm_DISABLE    =    0,      /**< Disable signal limiter */
  slm_MODE1      =    1,      /**< Signal limiter mode 1: Max -10.38 dBm0, Min -17.25 dBm0 */
  slm_MODE2      =    2,      /**< Signal limiter mode 2: Max  -8.25 dBm0, Min -15.34 dBm0 */  
  slm_MODE3      =    3,      /**< Signal limiter mode 3: Max  -6.44 dBm0, Min -13.43 dBm0 */
  slm_MODE4      =    4,      /**< Signal limiter mode 4: Max  -4.28 dBm0, Min -11.41 dBm0 */
  slm_MODE5      =    5,      /**< Signal limiter mode 5: Max  -2.17 dBm0, Min  -9.18 dBm0 */  
  slm_NUMOF_MODE = slm_MODE5  /**< Number of different SLM */ 
};

/**
 *  @name SLM Function Return Value Definitions
 *  
 */

enum{
  slm_NOERR    = 0,     /**< Functions returned without error */
  slm_ERROR    = 1,     /**< Functions returned with error, such as NULL point */
  slm_BADPARAM = 2      /**< Functions returned with bad parameters */  
};

/**
 *  @name SLM sampling rate factor
 *  \remark These numbers are used as table indeces, so can't be changed.
 */

enum{
  slm_SRATE_FACTOR_16K = 1, /**< sampling rate is 16kHz */
  slm_SRATE_FACTOR_8K  = 2  /**< sampling rate is 8kHz  */
};                           

/**
 *  @name SLM Configuration Structure Valid Bit-field
 *  
 */

enum{
  slm_CFG_BIT_SRATE      = 0     /**< Bit 0: sampling rate factor */
};

/** 
 * \brief SLM configure Structure
 *  
 * Contains parameters that can be chosen to be configured by user when initializing SLM
 *
 */
/* @{ */
typedef struct{
  tuint      valid_bf;            /**< Bit-fields indicating which parameters to configure \n
                                  \link SLM::slm_CFG_BIT_SRATE bitfield definition \endlink */
  tint       srate_factor;        /**< Sampling rate factor:\n
                                  \link SLM::slm_SRATE_FACTOR_8K slm_SRATE_FACTOR_8K   \endlink \n 
                                  \link SLM::slm_SRATE_FACTOR_16K slm_SRATE_FACTOR_16K \endlink */
} slmConfig_t;
/* @} */
/**
 *  @brief Function slmGetSizes() get the size of a SLM instance 
 *            
 *  \remark Function slmGetSizes() is the first function to be called.  
 *
 *  @param[in, out] instsize     Pointer to the size of a SLM instance .
 *
 *  @return           \link SLM::slm_NOERR     slm_NOERR,   \endlink \n
 *                    \link SLM::slm_ERROR     slm_ERROR    \endlink 
 */
tint slmGetSizes (tint *instsize);
    

/**
 *  @brief Function slmInit() initializes SLM  
 *            
 *  \remark Function slmInit() must be called after slmGetSizes(). After the
 *  function call, SLM is disabled.  
 *      
 *
 *  @param[in, out] inst    Pointer to a SLM instance structure
 *  @param[in]      cfg     Pointer to a configuration structure slmConfig_t
 *
 *  @return           \link SLM::slm_NOERR     slm_NOERR,   \endlink \n
 *                    \link SLM::slm_ERROR     slm_ERROR,   \endlink \n
 *                    \link SLM::slm_BADPARAM  slm_BADPARAM \endlink 
 *       
 */ 
tint slmInit (void *inst, slmConfig_t *cfg);

/**
 *  @brief Function slmControl() configure SLM with different types of limiter
 *            
 *  \remark Function slmControl() must be called after slmInit() to enable SLM by
 *  setting the mode to one of the operational modes.
 *  All internal parameters will be reset whenever there's a mode change 
 *   
 *
 *  @param[in, out]  inst     Pointer to a SLM instance structure.
 *  @param[in]       mode     one of \link SLM::slm_DISABLE slm modes \endlink.
 *
 *  @return           \link SLM::slm_NOERR     slm_NOERR,   \endlink \n
 *                    \link SLM::slm_ERROR     slm_ERROR,   \endlink \n
 *                    \link SLM::slm_BADPARAM  slm_BADPARAM \endlink 
 */
tint slmControl (void *inst, tint mode);

/**
 *  @brief This is the function that performs the signal attenuation
 *            
 *  \remark This function can only be called after slmInit() and slmControl()
 *  to make SLM take effect and it needs to be called every frame.  
 *   
 *
 *  @param[in]        inst            Pointer to a SLM instance structure.
 *  @param[in, out]   speechSamples   Pointer to the voice buffer
 *  @param[in]        frame_length    Number of samples processed
 *
 *  @return           \link SLM::slm_NOERR     slm_NOERR   \endlink
 *
 */

tint slmProcess (void *inst, void *speechSamples, tint frame_length);

/* @} */ /* ingroup SLM */
#endif

/* end of slm.h */

