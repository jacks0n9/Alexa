/* ========================================================================== */
/**
 *  @file   afehwcfg.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afehwcfg.c
 *
 *  @brief  Implements HW_CONFIG message processing functionality
 *
 *          This file contains functions that are used to initialize
 *          the HAL. They are called only once at system startup
 *          and so can be overlayed with an uninitialized data section
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */
/* -- Standard C Headers -- */
#include <xdc/std.h>

#include <stdio.h>
#include <string.h>

/* -- DSP/BIOS Headers -- */
#include <std.h>
#include <tsk.h>

/* -- Types Headers -- */
#include <ti/mas/types/types.h>

/* -- AFE Headers -- */
#include <afe.h>
#include <src/afeloc.h>
#include <src/afedbg.h>
#include <src/afeswcfg.h>
#include <src/afedevloc.h>

/* -- HAL Header -- */
#include <src/hal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============ AFE_processHwConfig ========== */
/**
 *  @brief      Process hardware configuration message from AFEMGR
 *
 *              If the hardware configuration message supplied is valid this
 *              function configures DSP and peripherals through HAL APIs and
 *              sends READY_INDICATION. If any configuration parameters are
 *              invalid then an error is reported.
 *
 *              NOTE: This HW config only processes things that are required
 *              for Davinci IPP. Non-supported parameters are ignored.
 *              Supported parameters are validated and error indication sent
 *              if they are not valid.
 *
 *  @return     AFE_STATUS_OK or appropriate error code
 *
 *  @sa         None
 *
 */
UInt16 AFE_processHwConfig (AFE_Object *obj, AFE_HwConfig *hw_config)
{
  halReturn_t       halReturn;
  halControl_t      hal_control;
  halTdmControl_t   halTdmControl;

  /* Update number of Rx TDM timeslots */
  obj->maxRxTdmTimeslots = hw_config->num_rx_timeslots;

  /* Update number of Tx TDM timeslots */
  obj->maxTxTdmTimeslots = hw_config->num_tx_timeslots;

  /* Make sure we got the same Rx sample rate as given during AFE creation */
  if (obj->maxRxSampleRate < (hw_config->rx_sampling_frequency)) {
    return AFE_STATUS_INVALID_HW_CONFIG;
  }
  obj->rxSampleRate = hw_config->rx_sampling_frequency;
  obj->rxSamplesPerTdmIsr  = (tuint)((AFE_TDMISRINTERVALMS) * (hw_config->rx_sampling_frequency)/1000);

  /* Make sure we got the same Tx sample rate as given during AFE creation */
  if (obj->maxTxSampleRate < (hw_config->tx_sampling_frequency)) {
    return AFE_STATUS_INVALID_HW_CONFIG;
  }
  obj->txSampleRate = hw_config->tx_sampling_frequency;
  obj->txSamplesPerTdmIsr  = (tuint)((AFE_TDMISRINTERVALMS) * (hw_config->tx_sampling_frequency)/1000);

  /* Start the primary PCM serial port */
  halTdmControl.mode = HAL_TDM_TDM_CONFIG;

  /* Assign the selected port */
  halTdmControl.tdmCtrl.wordSize         = 16;
  halTdmControl.tdmCtrl.rxSubFrameRate   = (tuint)((AFE_TDMISRINTERVALMS) * (hw_config->rx_sampling_frequency)/1000);
  halTdmControl.tdmCtrl.rxClocksPerFrame = hw_config->num_rx_timeslots;
  halTdmControl.tdmCtrl.rxSamplingFreq   = hw_config->rx_sampling_frequency;
  halTdmControl.tdmCtrl.txSubFrameRate   = (tuint)((AFE_TDMISRINTERVALMS) * (hw_config->tx_sampling_frequency)/1000);
  halTdmControl.tdmCtrl.txClocksPerFrame = hw_config->num_tx_timeslots;
  halTdmControl.tdmCtrl.txSamplingFreq   = hw_config->tx_sampling_frequency;

  /* Configure the serial port with the run time configuration information */
  halReturn = (*obj->halApi->halTdmControl) (obj->halIdPcm, &halTdmControl);
  if (halReturn != HAL_SUCCESS) {
    return AFE_STATUS_TDM_INIT_FAILED;
  }

  /* Startup the serial port */
  hal_control.periph = HAL_TDM;
  hal_control.op = HAL_PORT_ENABLE;
  hal_control.portHandle = obj->halIdPcm;
  halReturn = (*obj->halApi->halControl) (&hal_control);
  if (halReturn != HAL_SUCCESS) {
    return AFE_STATUS_TDM_START_FAILED;
  }
  return AFE_STATUS_OK;
}

UInt16 AFE_shutDown (AFE_Object *obj)
{
  int timeSlot;

  halTdmControl_t  halTdmControl;
  halReturn_t      retval;
  halControl_t     hal_control;

    halTdmControl.mode = HAL_TDM_TIMESLOT_CONFIG;
    for (timeSlot = 0; timeSlot < obj->maxNumberChannels; timeSlot++) {
      halTdmControl.txTimeslot    = timeSlot;
      halTdmControl.rxTimeslot    = timeSlot;
      halTdmControl.toTdmEnable   = FALSE;
      halTdmControl.fromTdmEnable = FALSE;
      retval = (*obj->halApi->halTdmControl) (obj->halIdPcm, &halTdmControl);
      if (retval != HAL_SUCCESS) {
        return AFE_STATUS_TDM_STOP_FAILED;
      }
    }

    /* Startup the serial port */
    hal_control.periph = HAL_TDM;
    hal_control.op = HAL_PORT_DISABLE;
    hal_control.portHandle = obj->halIdPcm;
    retval = (*obj->halApi->halControl) (&hal_control);
    if (retval != HAL_SUCCESS) {
      return AFE_STATUS_TDM_STOP_FAILED;
    }

    return AFE_STATUS_OK;
}
#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

/* Nothing after this point */
