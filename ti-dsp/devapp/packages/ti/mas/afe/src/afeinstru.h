/* ========================================================================== */
/**
 *  @file   afeinstru.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afeinstru.c
 *
 *  @brief  Header file for AFE's debug instrumentation
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/* -- Standard XDC Types Header */
#include <xdc/std.h>

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- AFE Local Headers -- */
#include <afe.h>
#include <src/afeloc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============ Peripheral debugging macros / defines ============= */
#define DM6446_ASP_BASE_ADDR                (0x01E02000UL)
#define DM6446_EDMA_BASE_ADDR               (0x01C00000UL)
#define DM6446_EDMA_PARAM_BASE_ADDR         (0x01C04000UL)
#define DM6446_EDMA_PARAM_ELEMENTS_PER_SET  (8)     /**< Number of elements in PARAM set */
#define DM6446_EDMA_PARAM_SET_SIZE          (32)    /**< Size of PARAM set in bytes */

/* ============ AFE_PcmTraceMsg ========== */
/**
 *  @brief  AFE's PCM Trace Message Structure
 *          This is used by AFE to send PCM stream coming from 
 *          AIC/TDM or going to AIC/TDM
 */
#define AFE_MAX_SAMPLE_BUFSIZE      (160)   /**< Corresponding to 10ms 16kHz */

#if 0 //TODO
typedef struct AFE_PcmTraceMsg {
  mgbHeader_t header;
  struct MGB_DMS_PCMPKT_TRACE_HDR hdr;
  linSample data[AFE_MAX_SAMPLE_BUFSIZE];
} AFE_PcmTraceMsg;
#endif

/* ============ AFE_PcmPattern ========== */
/**
 *  @brief  AFE's PCM pattern generation structure
 *          This is used by AFE to send PCM pattern towards TDM or MSP
 */
#define AFE_MAX_PCM_PATTERN_BUF_SIZE  (128) /**< Maximum size of the PCM pattern than can repeat */
typedef struct AFE_PcmPattern {
  Int32       length;
  Int32       readIndex;
  linSample   buf[AFE_MAX_PCM_PATTERN_BUF_SIZE];
} AFE_PcmPattern;

/* ============ globAfePcmPattern ========== */
/**
 *  @brief  AFE's global PCM pattern structure
 */
extern AFE_PcmPattern globAfePcmPattern;

/* ============ AFE_pcmTrace() ========== */
/**
 *  @brief      AFE's PCM trace routine
 *
 *  @param[in]      obj       Pointer to AFE_Object structure
 *  @param[in]      buf       Pointer linear PCM sample buffer
 *  @param[in]      length    Length of the linear PCM sample buffer
 *  @param[in]      streamId  PCM stream ID to be used
 *  @param[in,out]  pkt_num   PCM packet number to be incremented and used
 *  @param[in]      chan_num  Channel number to be used to send PCM trace
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void AFE_pcmTrace (
  AFE_Object  *obj,
  linSample   *buf,
  tuint       length,
  tuint       streamId,
  tuint       *pkt_num,
  tuint       chan_num
);

/* ============ AFE_generatePcmPattern() ========== */
/**
 *  @brief      Copies PCM pattern from the global PCM pattern buffer 
 *              to a given linear sample buffer
 *
 *  @param[in]      obj     Pointer to AFE_Object structure
 *  @param[in,out]  buf     Pointer linear PCM sample buffer to be filled
 *  @param[in]      length  Length of the linear PCM sample buffer to be filled
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void AFE_generatePcmPattern (AFE_Object *obj, linSample *buf, Int length);

UInt16 AFE_getStats(AFE_Object *obj, AFE_Stats *stats);


#if 0 //TODO
/* ============ AFE_readRegister ========== */
/**
 *  @brief      Read one memory mapped peripheral register
 *  @param[in]  obj         Pointer to AFE_Object
 *  @param[in]  msg         Pointer to message buffer
 *  @return     None
 *  @sa         AFE_writeRegister
 *
 */
void AFE_readRegister (AFE_Object *obj, mgb_t *msg);

/* ============ AFE_writeRegister ========== */
/**
 *  @brief      Write peripheral register
 *  @param[in]  obj         Pointer to AFE_Object
 *  @param[in]  msg         Pointer to message buffer
 *  @return     None
 *  @sa         AFE_readRegister
 *
 */
void AFE_writeRegister (AFE_Object *obj, mgb_t *msg);

/* ============ AFE_readAspRegisters ========== */
/**
 *  @brief      This API reads all the ASP registers except DRR and DXR
 *
 *              NOTE: This is a debug API, added for ASP debugging
 *              The proper implementation should be done in a target 
 *              specific file and CSL should be used read the ASP
 *              registers
 *
 *  @param[in]  obj   Pointer to AFE_Object
 *  @param[in]  msg   Pointer to message buffer
 *  @return     None
 *  @sa         AFE_writeAspRegister
 *
 */
void AFE_readAspRegisters (AFE_Object *obj, mgb_t *msg);

/* ============ AFE_writeAspRegister ========== */
/**
 *  @brief      Write to specific ASP register specified by offset
 *
 *              NOTE: This is a debug API, added for ASP debugging
 *              The proper implementation should be done in a target 
 *              specific file and CSL should be used to write to ASP
 *              register
 *
 *  @param[in]  obj   Pointer to AFE_Object
 *  @param[in]  msg   Pointer to message structure
 *  @return     None
 *  @sa         AFE_readAspRegisters
 *
 */
void AFE_writeAspRegister (AFE_Object *obj, mgb_t *msg);

/* ============ AFE_readEdmaRegisters ========== */
/**
 *  @brief      Read EDMA register
 *
 *              NOTE: This is a debug API added for EDMA debugging
 *              The proper implementation this API should be moved to 
 *              target platform specific file and CSL should be 
 *              used to access EDMA register
 *
 *  @param[in]  obj         Pointer to AFE_Object
 *  @param[in]  msg         Pointer to message buffer
 *  @return     None
 *  @sa         AFE_readEdmaParam
 *
 */
void AFE_readEdmaRegisters (AFE_Object *obj, mgb_t *msg);

/* ============ AFE_readEdmaParam ========== */
/**
 *  @brief      Read memory mapped EDMA PARAM registers
 *
 *              NOTE: This is a debug API added for EDMA debugging
 *              The proper implementation of this API should be moved to 
 *              target specific file and CSL should be used to read EDMA PARAM 
 *              registers
 *
 *  @param[in]  obj         Pointer to AFE_Object
 *  @param[in]  msg         Pointer to message structure
 *  @return     None
 *  @sa         AFE_readEdmaRegisters
 *
 */
void AFE_readEdmaParam (AFE_Object *obj, mgb_t *msg);

/* ============ AFE_restartAspEdma ========== */
/**
 *  @brief      Restart the ASP/EDMA
 *  @param[in]  obj       Pointer to AFE_Object structure
 *  @param[in]  msg       Pointer to message structure
 *  @return     None
 *  @sa         None
 *
 */
void AFE_restartAspEdma (AFE_Object *obj, mgb_t *msg);
#endif

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
