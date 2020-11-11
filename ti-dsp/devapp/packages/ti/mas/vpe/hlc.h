/******************************************************************************
 * FILE PURPOSE:header file of HLC (high-level compensation)
 ******************************************************************************
 * FILE NAME:   hlc.h
 *
 * DESCRIPTION: Contains external definitions&functions prototypes for HLC
 *
 * (C) Copyright 2008, Texas Instruments, Inc.
 *****************************************************************************/
#ifndef _HLC_H
#define _HLC_H

/**
 *  @file   hlc.h
 *  @brief  Contains external APIs for the HLC module.
 *
 *  (C) Copyright 2008, Texas Instruments, Inc.
 */

/** 
 * @defgroup HLC High Level Compensation(HLC) 
 *
 * @brief HLC is designed to attenuate the incoming signal when the signal level exceeds
 *        certain pre-configured threshold to avoid possible distortion due to future flat-top saturation.
 *        HLC is a frame-based operation and there's a requirement of minimum 5ms frame size, 
 *        after HLC parameters are properly configured the signal level will be measured and compared 
 *        with the threshold, then an attenuation will be calculated and applied to the signal.\n
 *
 *        Application of attenuation to the signal should not be instantaneous. Rather
 *        HLC should gradually ramp in and eventually apply the complete attenuation. 
 *        The ramping in time constant is relatively low to ensure fast action so that 
 *        the signal level can be reduced before saturation actually occurs.  It is fixed 
 *        to 10ms/dB for now. Ramping in should be triggered only when the power exceeds thresh. 
 *        During this period, attenuation could increase or stay constant, based on the changes in signal power.\n
 *
 *        In the case the signal level goes below thresh, HLC attenuation should disengage, although 
 *        slowly, so as not to cause sudden changes in the signal level heard by the listener, especially during 
 *        short pauses in speech. The ramping out time constant is configurable between 10ms to 1000ms per dB 
 *        reduction in attenuation, specified in 10ms steps. Ramping out should be triggered only for powers below thresh. 
 *        During this period, attenuation should gradually reduce based on the setting of the ramp-out time period. 
 *
 */
 
/** @ingroup HLC */
/* @{ */

/* System level header files */
#include <ti/mas/types/types.h>            /* DSP types            */


/**
 *  @name HLC Function Return Value Definitions
 *  
 */

enum{
 hlc_NOERR    = 0,   /**< Returned from function without error */
 hlc_ERROR    = 1,   /**< Returned from function with error    */       
 hlc_BADPARAM = 2    /**< Returned from function with wrong parameters */       
};

 
/**
 *  @name HLC sampling rate factor
 *  \remark These numbers are used as multipliers, so can't be changed to other values.
 */

enum{
  hlc_SRATE_FACTOR_8K  = 1, /**< sampling rate is 8kHz  */
  hlc_SRATE_FACTOR_16K = 2  /**< sampling rate is 16kHz */
};                           


/**
 *  @name HLC configure bit fields
 *  
 */

enum{
  hlc_CFG_BIT_THRESH      = 0,  /**< Bit 0 for threshold                  */
  hlc_CFG_BIT_RAMP_OUT    = 1,  /**< Bit 1 for ramp out period            */
  hlc_CFG_BIT_POWER_TC    = 2,  /**< Bit 2 for power calculation constant */
  hlc_CFG_BIT_FRM_LEN     = 3,  /**< Bit 3 for frame length               */
  hlc_CFG_BIT_SRATE_FACT  = 4   /**< Bit 4 for sampling rate factor       */
};

/**
 *  @name HLC state
 *  
 */

enum{
  hlc_DISABLED = 0,       /**< HLC is disabled */
  hlc_ENABLED  = 1        /**< HLC is enabled  */
};

/**
 *  @name HLC power calculation constant 
 *
 */
enum{
  hlc_TC_PWR_4_MS  = 1,   /**< 4 msec time constant  */
  hlc_TC_PWR_8_MS  = 2,   /**< 8 msec time constant  */
  hlc_TC_PWR_16_MS = 3    /**< 16 msec time constant */
};

/** 
 * \brief HLC configure Structure
 *  
 * Contains parameters that can be chosen to be configured by user or using 
 * the default when initializing HLC
 *
 */

typedef struct{
  tuint      valid_bf;            /**< Bit-fields indicating which parameters to configure \n
                                       \link HLC::hlc_CFG_BIT_THRESH bitfield definitions \endlink */
  Fract      thresh;              /**< Threshold (0.5 dBm0 units) in S14.1 format \n
                                      valid range: -96 ~ +6 corresponding to -48~+3 dBm0 \n
                                      default: 0 dBm0*/
  tint       ramp_out_period;     /**< Ramping out period in ms/dB \n
                                      valid range: 10~1000ms/dB, default 700ms/dB*/ 
  tint       power_tc;            /**< Power measurement time constant 
                                       valid numbers: \n
                                       \link HLC::hlc_TC_PWR_4_MS  hlc_TC_PWR_4_MS  \endlink \n
                                       \link HLC::hlc_TC_PWR_8_MS  hlc_TC_PWR_8_MS  \endlink \n
                                       \link HLC::hlc_TC_PWR_16_MS hlc_TC_PWR_16_MS \endlink 
                                  */
  tint       srate_factor;        /**< Sampling rate factor: \n
                                      \link HLC::hlc_SRATE_FACTOR_8K hlc_SRATE_FACTOR_8K \endlink \n
                                      \link HLC::hlc_SRATE_FACTOR_16K hlc_SRATE_FACTOR_16K \endlink \n 
                                  */
  tint       frm_len;             /**< Input signal frame length in samples, must be multiple of 5ms */
} hlcConfig_t;

/**
 *  @name HLC control code
 *  
 */

enum{
  hlc_CTL_DIS = 0,    /**< Disable HLC */
  hlc_CTL_ENA = 1,    /**< Enable HLC */
  hlc_CTL_THR = 2     /**< Set \link hlcConfig_t::thresh thresh\endlink in hlcConfig_t */
};


/** 
 * \brief  HLC Control Structure.
 * Contains control commands to change HLC parameters without 
 * resetting other parameters
 *
 */
typedef struct{
  tint    ctl_code;    /**< Set to one of hlc_CTL_XXX control codes, \n
                        \link HLC::hlc_CTL_DIS  Control Code \endlink */
  union {
    Fract   thresh;    /**< If ctl_code is \link HLC::hlc_CTL_THR hlc_CTL_THR \endlink, 
                            set the \link hlcConfig_t::thresh thresh\endlink in the HLC instance */
  } u;
} hlcControl_t;

/**
 * @name External APIs for HLC
 *
 */
/*@{ */
/**
 *  @brief To get the size of a HLC instance structure
 *            
 *  \remark Function hlcGetSizes() is the first function to be called.  
 *   
 *
 *  @param[in, out]   instsize        Pointer to the size of a HLC instance.
 *            
 *  @return           \link HLC::hlc_NOERR     hlc_NOERR,   \endlink \n
 *                    \link HLC::hlc_ERROR     hlc_ERROR,   \endlink \n
 *                    \link HLC::hlc_BADPARAM  hlc_BADPARAM \endlink 
 */ 
tint hlcGetSizes (tint *instsize); 

/**
 *  @brief Init HLC according to the user configurations.
 *            
 *  \remark Function hlcInit() needs be called after hlcGetSizes().  After
 *  this function call, HLC is disabled.  
 *   
 *
 *  @param[in, out]   inst         Pointer to a HLC instance structure.
 *  @param[in]        hlcCfg_info  Pointer to a configuration structure hlcConfig_t.
 *              
 *  @return           \link HLC::hlc_NOERR     hlc_NOERR,   \endlink \n
 *                    \link HLC::hlc_ERROR     hlc_ERROR,   \endlink \n
 *                    \link HLC::hlc_BADPARAM  hlc_BADPARAM \endlink 
 * 
 */

tint hlcInit(void *inst, hlcConfig_t *hlcCfg_info);

/**
 *  @brief Change parameters on the fly without re-initialzing all HLC parameters
 *            
 *  \remark Function hlcControl() must be called after hlcInit() to enable HLC.  Also
 *   it can be called anytime to reconfigure \link hlcConfig_t::thresh thresh\endlink 
 *   in the HLC instance. 
 *   
 *
 *  @param[in, out]   hlcInst     Pointer to a HLC instance.
 *  @param[in]        hlc_ctrl    Pointer to a control structure hlcControl_t.
 *             
 *  @return           \link HLC::hlc_NOERR     hlc_NOERR,   \endlink \n
 *                    \link HLC::hlc_ERROR     hlc_ERROR    \endlink 
 *                   
 */
tint hlcControl(void *hlcInst, hlcControl_t *hlc_ctrl);

/**
 *  @brief Calculate and apply attenuation when HLC is enabled
 *
 *  \remark This function is the function that processes the voice
 *  samples and needs to be called every frame.
 *
 *  @param[in]        hlcInst     Pointer to a HLC instance.
 *  @param[in, out]   samples     Pointer to the input/output signal buffer
 *             
 *  @return           \link HLC::hlc_NOERR     hlc_NOERR,   \endlink \n
 *                    \link HLC::hlc_ERROR     hlc_ERROR    \endlink 
 */
 
tint hlcProcess (void *hlcInst, void *samples);
/* @} */
/* @} */ /* ingroup HLC */
#endif

/* end of hlc.h */

