/* ========================================================================== */
/**
 *  @file   afedevalg.h
 *
 *  path   /dsps_gtmas/ti/mas/afe/src/afedevalg.h
 *
 *  @brief  AFEDEV resource allocation api
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef __AFEDEVALG_H__
#define __AFEDEVALG_H__

/* -- Standard C Headers -- */

/* -- DSP/BIOS Headers -- */
#include <std.h>

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- AFE Local Headers -- */
#include "afe.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AFEDEV_MAX_NUM_BUFFERS (10)

Int AFEDEV_getNumBuffers (Void);

Int AFEDEV_getSizes(AFE_CreateParams *createParams, IALG_MemRec memTab[]);

Void* AFEDEV_create(AFE_CreateParams *createParams, const IALG_MemRec memTab[]);

Int AFEDEV_delete(Void *afedevObj);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* __AFEDEVALG_H__ */
