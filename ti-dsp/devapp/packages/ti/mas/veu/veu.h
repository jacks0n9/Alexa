#ifndef _VEU_H   /*_VEU_H */
#define _VEU_H   /*_VEU_H     */

/* ========================================================================== */
/**
 *  @file   veu.h
 *
 *  path    /dsps_gtmas/ti/mas/veu/veu.h
 *
 *  @brief  Voice Quality Enhancer Unit API and Data Definitions
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 1999-2009
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/* System level header files */
#include <ti/mas/types/types.h>

/* Memory management definitions */
#include <ti/mas/util/ecomem.h>

/* debug */
#include <ti/mas/util/debug.h>

/** @defgroup VEU Voice Enhancement Unit
 *
 *  The following figure shows the flow of the signals in and out of VEU.
 *  @image html veu_image2.JPG
 *
 */

/** @ingroup VEU */
/* @{ */

/**
 * @defgroup VeuErrorCodes Return Error Codes 
 */
/** @ingroup VeuErrorCodes */
/* @{ */

/**
 *  @def  veu_NOMEMORY
 *        VEU return code -- Memory is not properly allocated for VEU.
 */
#define veu_NOMEMORY  -1

/**
 *  @def  veu_NOERR
 *        VEU return code -- VEU operation/configuration is successful.
 */
#define veu_NOERR     0

/**
 *  @def  veu_ERR_BAD_INST
 *        VEU return code -- Instantiation/de-instantiation of VEU is not 
 *                           accomplished.
 */
#define veu_ERR_BAD_INST     1

/**
 *  @def  veu_ERR_BAD_PARAM
 *        VEU return code -- Configuration parameters for VEU are invalid.
 */
#define veu_ERR_BAD_PARAM     2

/**
 *  @def  veu_ERR_BAD_BUILD
 *        VEU return code -- Wideband operation is requested in a narrowband build. 
 */
#define veu_ERR_BAD_BUILD     3

/* @} */ /* ingroup VeuErrorCodes */


/**
 * @defgroup VeuControlCodes VEU Control Codes 
 * The valid control codes to do run-time control via veuControl(). They can be 
 * assigned to veuControl_s.ctl_code. 
 */
/** @ingroup VeuControlCodes */
/* @{ */

/**
 *  @def  veu_CTL_MODE          
 *        VEU control code 0 -- to enable/disable VEU operational modes.
 *        Corresponding control parameter: veuControl_s.mode_ctl.
 */
#define  veu_CTL_MODE            0  
/**
 *  @def  veu_CTL_ANR_LEVEL
 *        VEU control code 1 -- to channge ANR level.
 *        Corresponding control parameter: veuControl_s.anr_level.
 */
#define  veu_CTL_ANR_LEVEL       1  
/**
 *  @def  veu_CTL_ALC_SEND_LEVEL
 *        VEU control code 2 -- to change ALC send path target level.
 *        Corresponding control parameter: veuControl_s.alc_level.
 */
#define  veu_CTL_ALC_SEND_LEVEL  2  
/**
 *  @def  veu_CTL_ALC_RECV_LEVEL
 *        VEU control code 3 -- to change ALC receive path target level.
 *        Corresponding control parameter: veuControl_s.alc_level.
 */
#define  veu_CTL_ALC_RECV_LEVEL  3
/**
 *  @def  veu_CTL_WAEPL
 *        VEU control code 4 -- to change WAEPL.
 *        Corresponding control parameter: veuControl_s.waepl.
 */
#define  veu_CTL_WAEPL          4
/**
 *  @def  veu_CTL_BULK_DELAY
 *        VEU control code 5 -- to change bulk delay.
 *        Corresponding control parameter: veuControl_s.bulk_delay.
 */
#define  veu_CTL_BULK_DELAY     5
/**
 *  @def  veu_CTL_FGC_SEND_GAIN
 *        VEU control code 6 -- to change FGC send path gain.
 *        Corresponding control parameter: veuControl_s.fgc_gain.
 */
#define  veu_CTL_FGC_SEND_GAIN  6
/**
 *  @def  veu_CTL_FGC_RECV_GAIN
 *        VEU control code 7 -- to change FGC receive path gain.
 *        Corresponding control parameter: veuControl_s.fgc_gain.
 */
#define  veu_CTL_FGC_RECV_GAIN  7

/* @} */ /* ingroup */


/**
 * @defgroup VeuModeControlMask VEU Mode Control Bit Masks
 * @brief Bit masks that can be used to enable/disable VEU operational modes,  
 *        e.g. ALC, ANR, FGC, through veuControl() at run time or through 
 *        veuOpen() at configuration time.
 *
 *    - For run-time control, these bit masks can be used to construct 
 *      veuModeControl_s.mask and veuModeControl_s.value to enable/disable 
 *      certain mode.
 *      For example, set mask to veu_CTLM_VEU | veu_CTLM_ALC_SEND to enable/
 *      disable VEU and send path ALC without affecting any other mode. Then
 *      set value to veu_CTLM_VEU | 0 to enable VEU and disable send path ALC.
 * 
 *    - For configuration time initialization, these bit masks can be used to
 *      construct veuConfigParam_s.config_bitfield to indicate which modes are
 *      to be enabled and which to be disabled.
 *      For example, set config_bitfield to veu_CTLM_VEU | veu_CTLM_ALC_SEND to
 *      enable VEU and send path ALC, but disabling all other modes.
 */
/** @ingroup VeuModeControlMask */
/* @{ */
#define veu_CTLM_VEU          0x0001   /**< Enable/disables VEU:
                                            bit value is 1: VEU is enabled; 
                                            bit value is 0: VEU is disabled.
                                            Note: this bit does not enable or
                                            disable any individual operation 
                                            mode, which has to be enabled or
                                            disabled by the corresponding bit.*/
#define veu_CTLM_ANR_SEND     0x0002   /**< Enables/disables Noise Reduction
                                            (NR) in the SEND direction. 
                                            bit value is 1: NR is enabled; 
                                            bit value is 0: NR is disabled.   */
#define veu_CTLM_ANR_RECV     0x0004   /**< Enables/disables Noise Reduction
                                            (NR) in the RECEIVE direction. 
                                            bit value is 1: NR is enabled. 
                                            bit value is 0: NR is disabled.   */
#define veu_CTLM_ALC_HLC_SEND 0x0008   /**< Enables/disables High Level Control 
                                            (HLC) in the SEND direction. 
                                            bit value is 1: HLC is enabled. 
                                            bit value is 0: HLC is disabled.  */
#define veu_CTLM_ALC_FGC_SEND 0x0010   /**< Enables/disables Fix Gain Control 
                                            (FGC) in the SEND direction. 
                                            bit value is 1: FGC is enabled; 
                                            bit value is 0: FGC is disabled.  */
#define veu_CTLM_ALC_SEND     0x0020   /**< Enables/disables Automatic Level 
                                            Control (ALC) in the SEND direction. 
                                            bit value is 1: ALC is enabled; 
                                            bit value is 0: ALC is disabled.  */
#define veu_CTLM_ALC_HLC_RECV 0x0040   /**< Enables/disables High Level Control 
                                            (HLC) in the RECEIVE direction. 
                                            bit value is 1: HLC is enabled; 
                                            bit value is 0: HLC is disabled.  */
#define veu_CTLM_ALC_FGC_RECV 0x0080   /**< Enables/disables Fix Gain Control 
                                            (FGC) in the RECEIVE direction.         
                                            bit value is 1: FGC is enabled; 
                                            bit value is 0: FGC is disabled.  */
#define veu_CTLM_ALC_RECV     0x0100   /**< Enables/disables Automatic Level 
                                            Control (ALC) in RECEIVE direction. 
                                            bit value is 1: ALC is enabled; 
                                            bit value is 0: ALC is disabled.  */
#define veu_CTLM_AEC          0x0200   /**< Enables/disables Acoustic Echo Control 
                                            bit value is 1: AEC is enabled; 
                                            bit value is 0: AEC is disabled.  */
#define veu_CTLM_ALE          0x0400   /**< Enables/disables Level Enhancement. 
                                            It is available only in RECEIVE 
                                            direction. For ALE to be enabled, 
                                            ALC in the RECEIVE direction has to 
                                            be enabled. 
                                            bit value is 1: ALE is enabled; 
                                            bit value is 0: ALE is disabled.  */
#define veu_CTLM_ALC_AGGRESS  0x0800   /**< Specifies whether ALC is Aggressive 
                                            or not.
,                                           bit value is 1: aggressive;
                                            bit value is 0: not aggressive.   */
#define veu_CTLM_TONE         0x1000  /**<  Specifies if tone is present. 
                                            bit value is 1: tone present; 
                                            bit value is 0: tone not present. */ 
#define veu_CTLM_WB_SEND      0x2000  /**<  Specifies wide band option in SEND 
                                            path. 
                                            bit value is 1: wide band;
                                            bit value is 0: narrow band.      */ 
#define veu_CTLM_WB_RECV      0x4000  /**<  Specifies wide band option in 
                                            RECEIVE path. 
                                            bit value is 1: wide band;
                                            bit value is 0: narrow band.      */ 
/* @} */ /* ingroup */

/**
 * @defgroup VeuValidFrameSizes VEU Valid Frame Sizes
 * @brief Valid frame sizes supported by VEU in units of 125-usec periods. VEU
 *        supports both 5/10 msec and 5.5/11 msec operations. Currently, VEU
 *        only supports up to 10msec/11msec frames. 
*/
/** @ingroup VeuValidFrameSizes */
/* @{ */
#define veu_INFRAME_MINLENGTH      40  /**< Minimum for 5/10msec frames: 5 ms  */
#define veu_INFRAME_MAXLENGTH      80  /**< Maximum for 5/10msec frames: 10 ms */
#define veu_INFRAME_LENGTHINC      40  /**< Increment for 5/10msec frames: 5ms */
#define veu_INFRAME_MINLENGTH_5P5  44  /**< Minimum for 5.5/11msec frames: 5.5 ms  */
#define veu_INFRAME_MAXLENGTH_5P5  88  /**< Maximum for 5.5/11msec frames: 11 ms   */  
#define veu_INFRAME_LENGTHINC_5P5  44  /**< Increment for 5.5/11msec frames: 5.5ms */
/* @} */ /* ingroup */

/**
 * @brief Specification of VEU Mode Control structure. 
 * This structure is used to enable/disable various VEU operational modes 
 * through veuControl() according to control code veu_CTL_MODE.


 */
typedef struct veuModeControl_s{
  tuint bit_masks;       /**< specifies which modes are to be enabled/disabled,
                              constructed by VeuModeControlMask.              */
  tuint bit_values;      /**< specifies the new value of each bit indicated by 
                              the bit masks of veuModeControl_s:mask.         */
} veuModeControl_t;

/**
 * @brief Specification of VEU Control structure Object.
 * This structure contains parameters required for calls to veuControl().
 */
typedef struct veuControl_s{
  /** @brief Control code specifying the veu configuration to be controlled. 
             Refer @ref VeuControlCodes for valid control codes.              */
  tint  ctl_code;      
  union {
    /** @brief VEU mode control, which applies when ctl_code is veu_CTL_MODE. 
               The modes to be enabled/disabled by this control code are 
               defined by VeuModeControlMask.                                 */
    veuModeControl_t  mode_ctl;    

    /** @brief Adaptive noise reduction level, which applies when ctl_code is 
               veu_CTL_ANR_LEVEL. This parameter takes only 4
               possible values: 6, 12, 18, 24. If it is given any other value, 
               it will be set to default 12. The unit of this parameter is dB.*/
    tint  anr_level;   

    /** @brief Automatic level control target level, which applies when ctl_code 
               is veu_CTL_ALC_SEND_LEVEL or veu_CTL_ALC_RECV_LEVEL. 
               This parameter takes only 6 possible values: 
               -9, -12, -15, -18, -21, -24. If given any other value, it will
               be set to -15. The unit of this parameter is dBm0. */
    tint  alc_level;   

    /** @brief Bulk delay, which applies when ctl_code is veu_CTL_BULK_DELAY. 
               This parameter takes value from 0 to 270, in steps of 18. If it
               is given a value above 270, it will be set to 270. If the value 
               is not a multiple of 18, it will be rounded to next multiple of  
               18. The recommended value is 160msec. The unit is msec.        */

    tint  bulk_delay;  

    /** @brief Worst Acoustic Echo Path Loss, which applies when ctl_code is 
               veu_CTL_WAEPL. This parameter takes value from 24 to 45, 
               in steps of 3. If the given value is smaller than 24, it will be 
               set to 24. If the given value is larger than 45, it will be set 
               to 45. If the value is not a multiple of 3, it will be rounded
               to next multiple of 3. The unit is dB. Recommended is 36dB.    */
    tint  waepl;       

    /** @brief Fixed gain control, applies when ctl_code is veu_CTL_FGC_SEND_GAIN
               or veu_CTL_FGC_RECV_GAIN. This parameter takes only 9 possible 
               values, from -12 to 12 in steps of 3. If it is given any other
               value, it will be set to default 0. The unit is dB.            */
    tint  fgc_gain;    

  } u;
} veuControl_t;

/**
 * @brief Specification of Size configuration structure Object.
 * This structure defined as void.  All prior variables are in context. 
 */
typedef struct veuSizeConfig_s{
  tint  max_frame_size;     /**< max Frame size */
} veuSizeConfig_t;

/**
 * @brief Specification of Creation configuration structure Object.
 * This structure contains parameters needed at the creation of an VEU instance.
 */
typedef struct veuNewConfig_s{
  void *handle;            /**< system handle used by veuContext_s:exception */
} veuNewConfig_t;

/**
 * @brief Specification of VEU configuration structure Object.



 * This structure contained parameters needed to open an instance of VEU.
 *
 */
typedef struct veuConfig_s{
  tint  config_bitfield; /**< Configuration bitfield that enables/disables VEU
                              operation modes. Meaning of each bit is the same
                              as that explained in @ref VeuModeControlMask. */
  tint  frame_size;      /**< Frame size in 125-usec periods. Valid frame
                              sizes are specified by \ref VeuValidFrameSizes. */ 
  tint  anr_level;       /**< refer to veuControl_s.anr_level                 */
  tint  alc_send_level;  /**< refer to veuControl_s.alc_level                 */
  tint  alc_recv_level;  /**< refer to veuControl_s.alc_level                 */
  tint  bulk_delay;      /**< refer to veuControl_s.bulk_delay                */
  Fract waepl;           /**< refer to veuControl_s.waepl                     */
  Fract fgc_send_gain;   /**< refer to veuControl_s.fgc_gain                  */
  Fract fgc_recv_gain;   /**< refer to veuControl_s.fgc_gain                  */
} veuConfig_t;

/**
 * @brief Specification of VEU statistics Object
 * This structure contained parameters that indicate VEU performance. send/recv 
 *              avg signal energy, send/recv noise energy and send/recv path gain.
 */
typedef struct veuStats_s{
  /** @brief Gain that is being applied by the VEU to Send signal*/
  Fract  send_gain;            
  /** @brief Noise Energy in the SEND Direction */
  Fract  send_noise_enr;       
  /** @brief Average Energy in the SEND Direction */ 
  Fract  send_avg_enr;        
  /** @brief Gain that is being applied by the VEU to Send signal*/   
  Fract  recv_gain;           
  /** @brief Noise Energy in the RECV Direction */ 
  Fract  recv_noise_enr;       
  /** @brief Average Energy in the RECV Direction */ 
  Fract  recv_avg_enr;         
} veuStats_t;


/**
 * @brief Specification of VEU context Structure Object
 * This structure contained information shared amongst VEU instances.
 */
typedef struct veuContext_s{
  /** @brief For exception handling */
  dbgInfo_t exception;   /**< Function pointer used for exception handling. 
              Type dbgInfo_t is defined in debug.h of util package:
              typedef void (*dbgInfo_t)(void *, tuint, tuint, tuint, tuint*).
              When AER calls this function, it passes the following information 
              through function arguments (from left to right):
              - void * handle: AER instance handle provided in aerNewConfig_s;
              - tuint dbgMsg : debugging message, either dbg_WARNING or 
                               dbg_TRACE_INFO, defined in util\debug.h.
              - tuint dbgCode: debugging code.
                  - warning code when dbgMsg is dbg_WARNING. 
                    See \ref aer_warning_codes.
                  - data tracing code when dbgMsg is dbg_TRACE_INFO. Currently,
                    AER doesn't have data tracing.
              - tuint dbgSize: size of AER tracing data per call.
              - tuint *dbgData: location where AER tracing data is stored.    */

  /** @brief Maximum frame size in units of 125-usec periods. Valid frame
             sizes are specified by \ref VeuValidFrameSizes.*/                                          
  tint max_frame_size; 
} veuContext_t;

/* VEU context */
extern veuContext_t veuContext;

/** @name VEU (Embedded Communication Object) APIs
 *  
 */
/*@{*/

/**
 *  @brief This function obtains memory requirements for an instance of VEU.   
 *
 *  @param[in]   cfg     Pointer to a size configuration structure.
 *  @param[out]  nbufs   Number of memory buffers stored in this address.
 *  @param[out]  bufs    Address of buffer descriptors stored in this address.     
 *  @retval              Veu Error Codes         
 *                          - veu_NOERR: successful
 */
tint  veuGetSizes (tint *nbufs, ecomemBuffer_t **bufs, veuSizeConfig_t *cfg);

/**
 *  @brief This function creates an instance of VEU and initializes the memory 
 *         buffer pointers in the instance.  
 *  @remark Function veuNew() must be called before veuOpen()is called.
 *
 *  @param[in]     nbufs     Number of memory buffers
 *  @param[in]     bufs      Pointer to memory buffer descriptors
 *  @param[in]     cfg       Pointer to new instance configuration structure
 *  @param[in,out] veuInst   Pointer to VEU instance
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: veuInst is not NULL or nbufs is different
 *                                   from what is returned by veuGetSizes.
 *               - veu_NOMEMORY: memory buffer sizes are less than required by 
 *                               veuGetSizes.
 *  @pre  Pointer veuInst must be set to NULL before this function is called.
 *  @post Pointer veuInst will be set to point to the instance buffer described
 *        by bufs.
 */
tint  veuNew (void **veuInst, tint nbufs, ecomemBuffer_t *bufs, 
              veuNewConfig_t *cfg);

/**
 *  @brief This function initializes and configures an VEU instance.
 *
 *  @param[in]      cfg      Pointer to VEU Open Configuration Structure.
 *  @param[in,out]  veuInst  Pointer to VEU instance.
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: VEU is not in CLOSED state or frame_size 
 *                                   in veuConfig_t is invalid.
 *               - veu_ERR_BAD_BUILD: config_bitfield in veuConfig_t enables
 *                                    wideband operation, but the build is 
 *                                    narrow band.
 *               - veu_ERR_BAD_PARAM: parameters in veuConfig_t are invalid, but
 *                                    will be set to default.
 *
 *  @post The instance's state will be set to OPEN.  
 *
 */
tint  veuOpen (void *veuInst, veuConfig_t *cfg);

/**
 *  @brief This function controls the operations of an instance of VEU 
 *
 *  @remark This function should be called after VEU instance has  been 
 *          intialized with veuOpen().
 *
 *  @param[in]      ctl        Pointer to VEU control config structure.
 *  @param[in,out]  veuInst    Pointer to VEU instance.
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: VEU is not opened yet.
 *               - veu_ERR_BAD_BUILD: mode_ctl in veuControl_t enables
 *                                    wideband operation, but the build is 
 *                                    narrow band.
 *               - veu_ERR_BAD_PARAM: parameters in veuConfig_t are invalid, but
 *                                    will be set to default.
 */
tint  veuControl        (void *veuInst, veuControl_t *ctl);

/**
 * @brief  Routine which obtains VEU statistics 
 *         @param[in,out] veuInst          Pointer to VEU instance
 *         @param[in]     reset           Flag indicating whether to reset statistics
 *         @param[in]     stats            pointer to statistics report structure
 *  
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: VEU is not opened yet.
 */
tint  veuGetStats  (void *veuInst, veuStats_t *stats, tbool reset);

/**
 * @brief  Relocates an instantiated VEU instance to another area of memory. 
 * 
 *    @param[in,out]    veuInst   Address of a pointer to VEU instance, *veuInst
 *                                should be the actual instance if instance 
 *                                (buffer 0) is not to be relocated, otherwise, 
 *                                relocated value is returned.
 *    @param[in]        nbufs     Max number of memory bufs
 *    @param[in]        bufs      Vector of buffer descriptors
 *    @param[in] relocBufBitMaskPtr  16-bit array of bit mask for buffers to 
 *                                relocate LSB is earlier buffer than MSB 
 *                                up to a max of nbufs bits.
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: parameter nbufs is different
 *                                   from what is returned by veuGetSizes.
 */
tint  veuRelocate (void **veuInst, tint nbufs, ecomemBuffer_t *bufs, 
                   tuint *relocBufBitMaskPtr);

/**
 *  @brief  This function processes downlink packets from the network.
 *   This is the Voice Quality Enhancer routine responsible for Noise Reduction, 
 *   Level control Listener and Enhancement in the SENDOUT direction.
 *   This routine also does signal power tracking functionality. The function 
 *   operates on RECVIN and SENDIN signal frames.
 *   The processed sendout samples pointed by the send_out pointer is passed as 
 *   the output of the routine.
 *   If the VEU is externally disabled via veuControl() this routine does not 
 *   do any processing.
 *  
 *  @param[in]      vsend_in          a pointer to downlink input packet.
 *  @param[in]      vrecv_in          a pointer to uplink input packet.
 *  @param[in]      vsend_out         a pointer to downlink output packet.
 *  @param[in]      comp_table_ptr    a pointer to a gain computation table.
 *  @param[in,out]  veuInst           a pointer to VEU instance. 
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: VEU is not opened yet.
 *  
 *  @remark   veuReceiveIn()should be called before veuSendIn().
 */
tint  veuSendIn     (void *veuInst, void *vsend_in, void *vrecv_in, 
                     void *vsend_out, const tint *comp_table_ptr);

/**
 *  @brief  This function processes uplink packets received from the network
 *  This is the Voice Quality Enhancer routine responsible for Noise Reduction, 
 *  Level control and Acoustic Echo Control in the RECVOUT direction. Function 
 *  veuSendIn() does all the processing related to calculation of the recv gain,
 *  veuReceiveIn() just applies the gain to the recvout samples. The processed 
 *  speech_sample samples pointed by the speech_sample pointer is passed as the 
 *  output of the routine. If the VEU is externally disabled via veucontrol() 
 *  this routine does not do any processing.
 *
 *  @param[in, out] speech_samples    a pointer to the received speech samples.
 *  @param[in,out]  veuInst           a pointer to VEU instance. 
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: VEU is not opened yet.
 *
 *  @remark   veuReceiveIn()should be called before veuSendIn(). 
 */
tint  veuReceiveIn (void *veuInst, void *speech_samples);

/**
 *  @brief This function closes the VEU instance identified by  veuInst. 
 *         It sets the state of VEU to CLOSED.  
 *
 *  @param[in,out]  veuInst
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 */
tint  veuClose (void *veuInst);

/**
 *  @brief This function deletes the VEU instance identified by veuInst.  
 *  @remark VEU must be closed prior to being deleted
 *
 *  @param[in,out]  veuInst  Pointer to VEU instance.
 *  @param[in]      nbufs    number of memory buffers.
 *  @param[in]      bufs     pointer to buffer descriptors   
 *  @retval   Veu Error Codes         
 *               - veu_NOERR: successful 
 *               - veu_ERR_BAD_INST: VEU is not closed yet.
 *
 *  @post Pointer veuInst will be set to NULL after this function is called.
 */
tint  veuDelete (void **veuInst, tint nbufs, ecomemBuffer_t *bufs);

/*@}*/ /* @name ECO APIs */

/* @} */ /* @ingroup VEU */

#endif /* _VEU_H */
/* nothing past this point */
