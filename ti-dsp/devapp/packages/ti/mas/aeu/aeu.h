/* ========================================================================== */
/**
 *  @file   ti/mas/aeu/aeu.h
 *
 *  @brief  The Control Interface definitions for Audio Enhancement Unit
 *
 *  ============================================================================
 */
/* --COPYRIGHT--,BSD
 * Copyright (c) 2009, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/**
 *  @defgroup   ti_mas_aeu_AEU    AEU Control/Message defines
 *
 *  This deines all the messages and controls for the AEU.
 */

#ifndef _AEU_H
#define _AEU_H

#include <xdc/std.h>

#include "ti/mas/eco/eco.h"

#include "ti/mas/fract/fract.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    ti_mas_aeu_AEU */
/*@{*/

/** @brief AEU Message IDs for request control messages received from user application */
#define AEU_SVD_ERROR_BASE 0x1000
#define AEU_SNL_ERROR_BASE 0x2000
#define AEU_SLM_ERROR_BASE 0x3000
#define AEU_VEU_ERROR_BASE 0x4000

#define AEU_ERR_NOAEUINST             0x0001
#define AEU_ERR_MAX_SVD_INST_EXCEEDED 0x0002
#define AEU_ERR_MAX_SNL_INST_EXCEEDED 0x0003
#define AEU_ERR_MAX_SLM_INST_EXCEEDED 0x0004
#define AEU_ERR_MAX_VEU_INST_EXCEEDED 0x0005
#define AEU_ERR_NO_ALG_SPECIFIED      0x0006
#define AEU_ERR_WRONG_SVD_ALG_ID      0x0007
#define AEU_ERR_WRONG_SNL_ALG_ID      0x0008
#define AEU_ERR_WRONG_SLM_ALG_ID      0x0009
#define AEU_ERR_WRONG_VEU_ALG_ID      0x000A
#define AEU_ERR_INVALID_ALG_ID        0x000B
/*
 *  ======== AEU API =====
 */
/**
 *  @brief  Enumeration for sampling rates
 */
typedef enum AEU_SampleRate{
    AEU_SRATE_8K  = 1,                   /** 8kHz  */
    AEU_SRATE_16K = 2                    /** 16kHz */
}AEU_SampleRate;

/**
 *  @brief AEU create parameters, will affect resource required during creation time.
 */
typedef struct AEU_SvdConfig {
    AEU_SampleRate samp_rate;    
} AEU_SvdConfig;

/**
 *  @brief AEU create parameters, will affect resource required during creation time.
 */
typedef struct AEU_SnlConfig {
    AEU_SampleRate samp_rate;    
} AEU_SnlConfig;

/**
 *  @brief AEU create parameters, will affect resource required during creation time.
 */
typedef struct AEU_SlmConfig {
    AEU_SampleRate samp_rate;   
} AEU_SlmConfig;

/**
 *  @brief VEU create parameters, will affect resource required during creation time.
 */
typedef struct AEU_VeuConfig {
    AEU_SampleRate samp_rate;   
    Int16          frame_size; /*in ms : 10ms supported */
    Uint16         config_bitfield;
    #define  AEU_ENABLE_VEU           0x0001
    #define  AEU_ENABLE_ALC_SEND      0x0020
    #define  AEU_ENABLE_ALC_RECV      0x0100
    Int16          alc_send_level;
    Int16          alc_recv_level;
} AEU_VeuConfig;

/**
 *  @brief This params is used to get resource required to create an
 *         AEU instance and associated algorithms
 */
typedef struct AEU_BasicParams {
    Uint16           num_svd;    /**< Number of SVD instances in this AEU. Will create SVD from AEU algId (0) to (num_svd-1) */
    #define  AEU_MAX_NUM_SVD_INST 4
    Uint16           num_snl;    /**< Number of SNL instances in this AEU. Will create SNL from AEU algId (num_svd) to (num_svd+num_snl-1) */
    #define  AEU_MAX_NUM_SNL_INST 4
    Uint16           num_slm;    /**< Number of SLM instances in this AEU. Will create SLM from AEU algId (num_svd+num_snl) to (num_svd+num_snl+num_slm-1) */
    #define  AEU_MAX_NUM_SLM_INST 4
    Uint16           num_veu;    /**< Number of VEU instances in this AEU. Will create VEU from AEU algId (num_svd+num_snl+num_slm), +1 for Tx and Rx side respectively to (num_svd+num_snl+num_slm+num_veu-1), +1 */
    #define  AEU_MAX_NUM_VEU_INST 1
    AEU_SvdConfig    svd_config; /**< SVD configuration parameters */
    AEU_SnlConfig    snl_config; /**< SNL configuration parameters */
    AEU_SlmConfig    slm_config; /**< SLM configuration parameters */
    AEU_VeuConfig    veu_config; /**< VEU configuration parameters */
} AEU_BasicParams;

/**
 * @brief  AEU module's Parameters needed during instance creation
 */
typedef struct AEU_Params {
    ECO_Params      params; /**< First Element of the derived class */
    AEU_BasicParams  basicParams;
} AEU_Params;


typedef struct AEU_slmControl {
    Int16      mode;
} AEU_slmControl;

typedef struct AEU_veuModeControl{
  Uint16 mask;          /**< tells which bits are to be changed.             */
  Uint16 value;         /**< tells new value of each bit indicated by mask.  */
} AEU_veuModeControl;

typedef struct AEU_veuControl {
    Int16      ctl_code;
    union 
    {
      AEU_veuModeControl mode_control;
	    /** @brief Applies when ctl_code is veu_CTL_ALC_SEND_LEVEL, veu_CTL_ALC_RECV_LEVEL.Refer @ref VeuALCConfigParam for valid values, see veu.h */
      Int16  alc_level;   
    } u;
} AEU_veuControl;

typedef struct AEU_veuGetStats {
    Int16 reset;  /**< Get the stats and thern reset */        
} AEU_veuGetStats;

typedef struct AEU_svdDecision {
    Int16      decision;        /**< see svd_VOICE_xxx in svd.h */
} AEU_svdDecision;

typedef struct AEU_snlLevels {
    Int16      speechdB;        /**< speech power level in Q0 dB, see snl.h */  
    Int16      noisedB;         /**< noise  power level in Q0 dB, see snl.h */
    Uint16     threshold;       /**< adaptive threshold for envelope in snl_ENV_Q, see snl.h*/ 
} AEU_snlLevels;

typedef struct AEU_veuStats {
    /** @brief Gain that is being applied by the VEU to Send signal, see veu.h */
    Int16  send_gain;            
    /** @brief Noise Energy in the SEND Direction, see veu.h */
    Int16  send_noise_enr;       
    /** @brief Average Energy in the SEND Direction, see veu.h */ 
    Int16  send_avg_enr;        
    /** @brief Gain that is being applied by the VEU to Send signal, see veu.h */   
    Int16  recv_gain;           
    /** @brief Noise Energy in the RECV Direction, see veu.h */ 
    Int16  recv_noise_enr;       
    /** @brief Average Energy in the RECV Direction, see veu.h */ 
    Int32 recv_avg_enr;         
} AEU_veuStats;

/******************************************************************************
 * This information is when called to get AEU Performance Info
 *****************************************************************************/

typedef struct AEU_SvdPerfInfo {
  Int16  AeuVadDecision;     /**< VAD: -1: Undefine, 0: Voice Inactive, 1: Voice Active */
} AEU_SvdPerfInfo;

typedef struct AEU_AlcPerfInfo {
  Int16  AeuAlcGain;              /**< ALC Gain: in dB */
} AEU_AlcPerfInfo;

/**
 * @brief  AEU module's Control Parameters
 */
typedef struct AEU_ControlParams {
    Uint16               controlId;
    #define AEU_SLM_CONTROL      0x0001
    #define AEU_SVD_GET_DECISION 0x0002
    #define AEU_SNL_GET_LEVELS   0x0003
    #define AEU_VEU_CONTROL      0x0004
    #define AEU_VEU_GET_STATS    0x0005
    #define AEU_ALGO_RESET       0x0006
    Uint16               algId;
    union 
    {
      AEU_slmControl     slmControl;
      AEU_veuControl     veuControl;
      AEU_veuGetStats    veuGetStats;
    } ctrl;
} AEU_ControlParams;

/**
 * @brief  Union of AEU module's Response Parameters, such as stats.
 */
typedef struct AEU_ResponseParams {
    Uint16               responseId;
    union 
    {
      AEU_svdDecision    svdDecision;
      AEU_snlLevels      snlLevels;
      AEU_veuStats       veuStats;
    } resp;
} AEU_ResponseParams;

extern IECO_Fxns AEU_TI_IECO;


/**
 *  @brief   This enumeration defines the buffer map for AEU
             When calling ECO_process() these should be populated properly in
             the inArgs->inStream[].buf[i] array, or should be set to null.
 *
 */
typedef enum
{
  AEU_BUFF_IN,           /**< Input Buffer */
  AEU_BUFF_REF_IN,       /**< Input Buffer for Send side VEU */

  AEU_NUM_BUFF_IN       /**< Number of 'in' data buffers */
} AEU_BUFF_IN_MAP;

/**
 *  @brief   This enumeration defines the buffer map for AEU
             When calling ECO_process() these should be populated properly in
             the outArgs->outStream[].buf[i] array, or should be set to null.
 *
 */

typedef enum
{
  AEU_BUFF_OUT,             /**< Output Buffer */

  AEU_NUM_BUFF_OUT          /**< Number of 'out' data buffers */
} AEU_BUFF_OUT_MAP;

/* @} */ /* ingroup */

#ifdef __cplusplus
}
#endif

#endif /* (_AEU_H) */

