/* ========================================================================== */
/**
 *  @file   tdm.h
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/c64P/Leo/tdmdma/tdm.h
 *
 *  @brief  Header file containing TDM DMA APIs
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __TDM_H__
#define __TDM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- HAL Headers -- */
#include <src/hal.h>

/* ============ halTdmGetHeapSize() ========== */
/**
 *  @brief      Returns the amount of heap memory required for TDM
 *
 *              Computes the amount of memory needed for every serial port 
 *              requested in bytes.
 *
 *  @param[in]  cfg    Pointer to halNewConfig_t structure
 *
 *  @return     HAL heap size
 *
 *  @sa         None
 *
 */
tuint halTdmGetHeapSize (halNewConfig_t *cfg);

/* ============ halTdmGetDmaBufInfo() ========== */
/**
 *  @brief      Returns the memory for DMA buffer requirements
 *
 *              The DMA requires buffers that are aligned on a power of two
 *              boundary that is strictly greater than the required buffer
 *              size. The same function is used for the Tx and Rx buffers, and 
 *              is defined to halTdmGetTxBufInfo and halTdmGetRxBufInfo in 
 *              tdm.h
 *
 *  @param[in]  port   TDM port number
 *  @param[in]  cfg    Pointer to halNewConfig_t structure
 *  @param[out] align  Alignment
 *  @param[out] size   Total size in bytes required for DMA double buffer
 *
 *  @return     HAL heap size
 *
 *  @sa         None
 *
 */
tint halTdmGetDmaBufInfo (
  tint              port,
  halNewConfig_t    *cfg,
  tuint             *align,
  tuint             *size,
  tint              tdmType
);

#define halTdmGetTxBufInfo halTdmGetDmaBufInfo
#define halTdmGetRxBufInfo halTdmGetDmaBufInfo

/* ============ halTdmAllocAndInit() ========== */
/**
 *  @brief      Allocate heap memory to TDM port and initializes data structures
 *
 *              Allocate memory for the TDM control structure. Returns a pointer
 *              to the tdm structure. The halPortId (handle) for the port is
 *              returned in the configuration structure.
 *
 *  @param[in]  port      HAL TDM port number, (indexed from 0)
 *  @param[in]  base      heap base address
 *  @param[in]  used      heap used - must be updated when heap is allocated
 *  @param[in]  size      heap size
 *  @param[in]  newCfg    TDM configuration structure
 *  @param[in]  dmaTx     DMA tx memory buffer address
 *  @param[in]  dmaTxSize Size of the DMA tx buffer
 *  @param[in]  dmaRx     DMA tx memory buffer address
 *  @param[in]  dmaRxSize Size of the DMA rx address
 *  @param[out] ret       Returns success or hal error code
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void *halTdmAllocAndInit (
  tint            port,
  void            *base,
  tuint           *used,
  tuint           size, 
  halNewConfig_t  *newCfg,
  tuint           *dmaTx, 
  tuint           dmaTxSize,
  tuint           *dmaRx,
  tuint           dmaRxSize,
  halReturn_t     *ret
);

/* ============ halTdmInitStart() ========== */
/**
 *  @brief      Init time startup of serial port
 *
 *              Enables the McBSP, DMA and DMA interrupts for a TDM port
 *
 *  @param[in]  pTdm    Pointer to halTdmMcbsp_t structure
 *
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t halTdmInitStart (void *pTdm);

/* ============ halTdmControl() ========== */
/**
 *  @brief      TDM control API
 *
 *              Configures TDM/DMA/Int system
 *
 *  @param[in]  halId   Hal identifier
 *
 *  @param[in]  ctl     Pointer to halTdmControl_t structure
 * 
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t halTdmControl (tint halId, halTdmControl_t *ctl);

/* ============ halTdmStop() ========== */
/**
 *  @brief      Stop serial port
 *
 *  @param[in]  None
 *
 *  @return     HAL error code
 *
 *  @sa         None
 *
 */
halReturn_t halTdmStop (void);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif  /* __TDM_H__ */

