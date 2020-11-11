/* ========================================================================== */
/**
 *  @file   ecoxdaisxlat.h
 *
 *  path    /dsps_gtmas/ti/mas/util/ecoxdaisxlat.h
 *
 *  @brief  Utility APIs for tranlating between ECO and XDAIS buffers.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef _ECOXDAISXLAT_H
#define _ECOXDAISXLAT_H

#include <ti/mas/types/types.h>
#include <ti/mas/util/ecomem.h>
#include <ti/xdais/ialg.h>

/** @defgroup ECOXDAISXLAT 
 * This module contains utility APIs for tranlating between ECO and XDAIS buffers.
 */

/** @ingroup ECOXDAISXLAT */
/* @{ */
/**
 * @fn void ecoxdaisxlatEcoToXdais (tint nbufs, const ecomemBuffer_t *ecoBufsPtr, IALG_MemRec *xdaisMemTabPtr)
 * @brief  Function to convert ECO Memory buffers to XDAIS Memory Record.
 *
 * @param[in]  nbufs             Numbers of buffers in the ECO
 * @param[in]  ecoBufsPtr        Pointer to ECO buffers
 * @param[in]  xdaisMemTabPtr    Pointer to XDAIS Memory Record
 *
 */
void ecoxdaisxlatEcoToXdais (tint nbufs, const ecomemBuffer_t *ecoBufsPtr, 
                              IALG_MemRec *xdaisMemTabPtr);
/**
 * @fn void ecoxdaisxlatXdaisToEco (tint nbufs, const IALG_MemRec *xdaisMemTabPtr, ecomemBuffer_t *ecoBufsPtr)
 * @brief  Function to convert XDAIS Memory Record to ECO Memory buffers.
 *
 * @param[in]  nbufs             Numbers of buffers in the ECO
 * @param[in]  xdaisMemTabPtr    Pointer to XDAIS Memory Record
 * @param[in]  ecoBufsPtr        Pointer to ECO buffers
 *
 */
void ecoxdaisxlatXdaisToEco (tint nbufs, const IALG_MemRec *xdaisMemTabPtr, 
                             ecomemBuffer_t *ecoBufsPtr);
/* @} */ /* ingroup */
#endif /* _ECOXDAISXLAT_H */
/* Nothing past this point */
