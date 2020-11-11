/* ========================================================================== */
/**
 *  @file   afeinstru.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afeinstru.c
 *
 *  @brief  Implements AFE's debug instrumentation functionality
 *          - PCM Tracing
 *          - Pattern Generation
 *          - Statistics
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

/* --- Standard C headers --- */
#include <stdio.h>
#include <string.h>

/* --- XDC Types Header --- */
#include <xdc/std.h>

/* --- Types Package Header --- */
#include <ti/mas/types/types.h>

/* --- AFE Headers --- */
#include <src/afeinstru.h>
#include <src/hal.h>
#include <src/afedbg.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 0 //TODO
/* ============ globAfePcmTrace ========== */
/**
 *  @brief  AFE's global PCM Trace structure
 */
static AFE_PcmTraceMsg globAfePcmTrace;
#endif

/* ============ globAfePcmPattern ========== */
/**
 *  @brief  AFE's global PCM pattern structure
 */
AFE_PcmPattern globAfePcmPattern;


UInt16 AFE_getStats(AFE_Object *obj, AFE_Stats *stats)
{
  AFE_IsrTimerStats * isrStats = &(obj->isrStats);
  stats->lastIsr    = isrStats->tIsr;
  stats->lastdtIsr  = isrStats->dtIsr;
  stats->dtIsrMin   = isrStats->dtIsrMin;
  stats->dtIsrMax   = isrStats->dtIsrMax;
  stats->dtIsrErrP  = isrStats->dtIsrErrP;
  stats->dtIsrErrN  = isrStats->dtIsrErrN;  
  stats->frames     = isrStats->isrCnt;
  return AFE_STATUS_OK;
}


#if 0 //TODO
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
)
{
  Uint32 size = length * sizeof(linSample);

  /* Copy the linear samples buffer to data */
  memcpy (&globAfePcmTrace.data[0], buf, length * sizeof(linSample));

  /* Fill the PCM packet trace header */
  globAfePcmTrace.hdr.pktLen_streamId = 
    (Uint16)(((size << MGB_PCMPKT_TRACE_LENGTH_SHIFT) & MGB_PCMPKT_TRACE_LENGTH_MASK) | 
    ((streamId << MGB_PCMPKT_TRACE_STREAM_ID_SHIFT) & MGB_PCMPKT_TRACE_STREAM_ID_MASK));

  /* Increment the packet number and populate it in the header */
  *pkt_num += 1;
  globAfePcmTrace.hdr.pkt_num = *pkt_num;

  /* Send this message to AFEMGR */
  size = length * sizeof(linSample) + sizeof(mgbHeader_t) 
    + sizeof(struct MGB_DMS_PCMPKT_TRACE_HDR);

  /* Fill message header */
  globAfePcmTrace.header.chnum = (((DPR_PKT_NORMAL_PRIORITY_MSG << 11) & 0xF800)|
    ((chan_num+AFE_FIRSTCHAN) & 0x07FF));
  globAfePcmTrace.header.msg_id = MGB_DM_PCMPKT_TRACE;
  globAfePcmTrace.header.trans_id = 0;
  globAfePcmTrace.header.length = size;

  /* Send the packet to AFEMGR */
//  AFE_sendMessage (obj, (mgb_t *)&globAfePcmTrace, size);
}
#endif

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
void AFE_generatePcmPattern (AFE_Object *obj, linSample *buf, Int length)
{
    FIX_COMPIL_WARNING(obj);
    FIX_COMPIL_WARNING(buf);
    FIX_COMPIL_WARNING(length);
#if 0
  Int i;
  Int       index;

  /* Get readIndex to local value */
  index = globAfePcmPattern.readIndex;

  /* Copy circularly PCM pattern from global PCM pattern buffer */
  for (i = 0; i < length; i++) {
    buf[i] = globAfePcmPattern.buf[index++];
    if (index >= globAfePcmPattern.length) {
      index = 0;
    }
  }

  /* Copy back the index to global PCM pattern structure */
  globAfePcmPattern.readIndex = index;
#endif
}

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
void AFE_readRegister (AFE_Object *obj, mgb_t *msg)
{
  struct MGB_MDS_AFE_PERH_CMD_GET_REG *afe_get_reg;
  Uint32 regAddr;
  Uint32 regVal;

  /* Get local values */
  afe_get_reg = (struct MGB_MDS_AFE_PERH_CMD_GET_REG *)
    &msg->msg_body.afe_perh_ctrl.u.get_reg;

  regAddr = (((Uint32)afe_get_reg->addr_msw) << 16) | ((Uint32)afe_get_reg->addr_lsw);
  regVal = *((volatile Uint32 *)regAddr);

  /* Send register value to SRM debug pipe */
  sprintf (AFE_dbgStringBuf, 
    "%s ADDRESS = 0x%08X\n\t"
    "%s VALUE   = 0x%08X\n",
    str_info, regAddr,
    str_info, regVal);

//  AFE_sendDebugMessage (obj, AFE_dbgStringBuf);     TODO provide some mechanism to inform the debug message

  return;
}

/* ============ AFE_writeRegister ========== */
/**
 *  @brief      Write peripheral register
 *  @param[in]  obj         Pointer to AFE_Object
 *  @param[in]  msg         Pointer to message buffer
 *  @return     None
 *  @sa         AFE_readRegister
 *
 */
void AFE_writeRegister (AFE_Object *obj, mgb_t *msg)
{
  struct MGB_MDS_AFE_PERH_CMD_SET_REG *afe_set_reg;
  Uint32 regAddr;
  Uint32 regOldVal;
  Uint32 regNewVal;
  Uint32 regMask;

  /* Get local values */
  afe_set_reg = (struct MGB_MDS_AFE_PERH_CMD_SET_REG *)&msg->msg_body.afe_perh_ctrl.u.set_reg;

  regAddr   = (((Uint32)afe_set_reg->addr_msw) << 16)  | ((Uint32)afe_set_reg->addr_lsw);
  regNewVal = (((Uint32)afe_set_reg->value_msw) << 16) | ((Uint32)afe_set_reg->value_lsw);
  regMask   = (((Uint32)afe_set_reg->mask_msw) << 16)  | ((Uint32)afe_set_reg->mask_lsw);

  regOldVal = *((volatile Uint32 *)regAddr);
  regNewVal = (regOldVal & ~regMask) | (regNewVal & regMask); 

  *((volatile Uint32 *)regAddr) = regNewVal; 

  /* Send success message to SRM debug pipe */
  sprintf (AFE_dbgStringBuf, "%s Wrote 0x%08X value to register 0x%08X",
    str_info, regNewVal, regAddr);
//  AFE_sendDebugMessage (obj, AFE_dbgStringBuf);

  return;
}

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
void AFE_readAspRegisters (AFE_Object *obj, mgb_t *msg)
{
  Uint32 SPCR, RCR, XCR, SRGR, PCR;

  msg = msg;  /* To avoid compiler warning */

  SPCR = *((volatile Uint32 *)0x01E02008);
  RCR  = *((volatile Uint32 *)0x01E0200C);
  XCR  = *((volatile Uint32 *)0x01E02010);
  SRGR = *((volatile Uint32 *)0x01E02014);
  PCR  = *((volatile Uint32 *)0x01E02024);

  /* Send the ASP register values to SRM debug pipe */
  sprintf (AFE_dbgStringBuf, 
    "%s SPCR = 0x%08X\n\t"
    "%s RCR  = 0x%08X\n\t"
    "%s XCR  = 0x%08X\n\t"
    "%s SRGR = 0x%08X\n\t"
    "%s PCR  = 0x%08X",
    str_info, SPCR, str_info, RCR,
    str_info, XCR, str_info, SRGR,
    str_info, PCR);

//  AFE_sendDebugMessage (obj, AFE_dbgStringBuf);
}

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
void AFE_writeAspRegister (AFE_Object *obj, mgb_t *msg)
{
  struct MGB_MDS_AFE_PERH_CMD_WRITE_ASP_REG *write_asp_reg;
  Uint32 addr;
  Uint32 curval;
  Uint32 newval;
  Uint32 mask;

  /* Get local values */
  write_asp_reg = &msg->msg_body.afe_perh_ctrl.u.write_asp_reg;

  addr    = (DM6446_ASP_BASE_ADDR + write_asp_reg->offset);
  newval  = (((Uint32)write_asp_reg->value_msw << 16) | ((Uint32)write_asp_reg->value_lsw));
  mask    = (((Uint32)write_asp_reg->mask_msw << 16)  | ((Uint32)write_asp_reg->mask_lsw));
  
  /* Read current register value */
  curval = *((volatile Uint32 *)addr);

  /* Compute new value to write */
  newval = ((curval & ~mask) | (newval & mask));

  /* Write to the specified register */
  *((volatile Uint32 *)addr) = newval;

  /* Send the value written to ASP register to SRM debug pipe */
  sprintf (AFE_dbgStringBuf, "%s Written %08X to ASP register %08X",
    str_info, newval, addr);

//  AFE_sendDebugMessage (obj, AFE_dbgStringBuf);
}

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
void AFE_readEdmaRegisters (AFE_Object *obj, mgb_t *msg)
{
  Uint32 CCCFG, EMR, EMCR, EMRH, EMCRH, QEMR, QEMCR, CCERR, CCERRCLR, EEVAL;

  msg = msg;  /* To avoid compiler warnings */

  /* Read the memory mapped EDMA registers */
  CCCFG    = *((volatile Uint32 *)0x01C00004);
  EMR      = *((volatile Uint32 *)0x01C00300);
  EMCR     = *((volatile Uint32 *)0x01C00304);
  EMRH     = *((volatile Uint32 *)0x01C00308);
  EMCRH    = *((volatile Uint32 *)0x01C0030C);
  QEMR     = *((volatile Uint32 *)0x01C00310);
  QEMCR    = *((volatile Uint32 *)0x01C00314);
  CCERR    = *((volatile Uint32 *)0x01C00318);
  CCERRCLR = *((volatile Uint32 *)0x01C0031C);
  EEVAL    = *((volatile Uint32 *)0x01C00320);

  /* Send current values of counters to SRM debug pipe */
  sprintf (AFE_dbgStringBuf, 
    "%s CCCFG     = 0x%08X\n\t"
    "%s EMR       = 0x%08X\n\t"
    "%s EMCR      = 0x%08X\n\t"
    "%s EMRH      = 0x%08X\n\t"
    "%s EMCRH     = 0x%08X\n\t"
    "%s QEMR      = 0x%08X\n\t"
    "%s QEMCR     = 0x%08X\n\t"
    "%s CCERR     = 0x%08X\n\t"
    "%s CCERRCLR  = 0x%08X\n\t"
    "%s EEVAL     = 0x%08X\n",
    str_info, CCCFG, str_info, EMR, 
    str_info, EMCR, str_info, EMRH,
    str_info, EMCRH, str_info, QEMR,
    str_info, QEMCR, str_info, CCERR, 
    str_info, CCERRCLR, str_info, EEVAL);

//  AFE_sendDebugMessage (obj, AFE_dbgStringBuf);
}

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
void AFE_readEdmaParam (AFE_Object *obj, mgb_t *msg)
{
  struct MGB_MDS_AFE_PERH_CMD_READ_EDMA_PARAM *afe_read_param;
  Uint32 paramRegVal[8];
  Uint32 paramAddr;
  Int i;

  /* Get local values */
  afe_read_param = (struct MGB_MDS_AFE_PERH_CMD_READ_EDMA_PARAM *)
    &msg->msg_body.afe_perh_ctrl.u.read_edma_param;

  /* Get the start address of the PARAM set */
  paramAddr = (DM6446_EDMA_PARAM_BASE_ADDR + 
    (Uint32)afe_read_param->param_set * DM6446_EDMA_PARAM_SET_SIZE);

  /* Read all 8 param register within the set */
  for (i = 0; i < DM6446_EDMA_PARAM_ELEMENTS_PER_SET; i++) {
    paramRegVal[i] = *((volatile Uint32 *)paramAddr);
    paramAddr += sizeof(Uint32);
  }

  /* Send the PARAM register values to SRM debug pipe */
  sprintf (AFE_dbgStringBuf, 
    "%s PRAM1 = 0x%08X\n\t"
    "%s PRAM2 = 0x%08X\n\t"
    "%s PRAM3 = 0x%08X\n\t"
    "%s PRAM4 = 0x%08X\n\t"
    "%s PRAM5 = 0x%08X\n\t"
    "%s PRAM6 = 0x%08X\n\t"
    "%s PRAM7 = 0x%08X\n\t"
    "%s PRAM8 = 0x%08X\n",
    str_info, paramRegVal[0], str_info, paramRegVal[1], 
    str_info, paramRegVal[2], str_info, paramRegVal[3],
    str_info, paramRegVal[4], str_info, paramRegVal[5],
    str_info, paramRegVal[6], str_info, paramRegVal[7]);

//  AFE_sendDebugMessage (obj, AFE_dbgStringBuf);
}

/* ============ AFE_restartAspEdma ========== */
/**
 *  @brief      Restart the ASP/EDMA
 *  @param[in]  obj       Pointer to AFE_Object structure
 *  @param[in]  msg       Pointer to message structure
 *  @return     None
 *  @sa         None
 *
 */
void AFE_restartAspEdma (AFE_Object *obj, mgb_t *msg)
{
  struct MGB_MDS_AFE_PERH_CMD_RESTART_ASPEDMA *restart_aspedma;
  halReturn_t         ret;
  halDebugControl_t   halDebugControl;

  /* Get local values */
  restart_aspedma = (struct MGB_MDS_AFE_PERH_CMD_RESTART_ASPEDMA *)
    &msg->msg_body.afe_perh_ctrl.u.restart_aspedma;

  /* Issue hal debug control */
  halDebugControl.cmd = HAL_DEBUG_RESTART_ASPEDMA;
  halDebugControl.u.restartAspEdma.restartMask = restart_aspedma->restart_mask;
  ret = (*obj->afeContext->halApi->halDebugControl) (
    obj->afeContext->halIdPcm, &halDebugControl);

  if (ret != HAL_SUCCESS) {
    sprintf (AFE_dbgStringBuf, "%s AFE_restartAspEdma failed", str_error);
//    AFE_sendDebugMessage (obj, AFE_dbgStringBuf);
    return;
  }
}
#endif

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
