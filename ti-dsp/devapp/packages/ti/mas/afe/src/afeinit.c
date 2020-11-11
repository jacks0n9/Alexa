/* ========================================================================== */
/**
 *  @file   afeinit.c
 *
 *  path    /dsps_gtmas/ti/mas/afe/src/afeinit.c
 *
 *  @brief  Perform AFE global structure initialization after download
 *
 *          This files contains all functions necessary to initialize the
 *          AFE after download state.  This process includes 
 *          initializing AFE, HAL
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
#include <stdlib.h>
#include <string.h>

/* -- AFE Header -- */
#include <ti/mas/afe/afe.h>

/* -- DSP/BIOS Headers -- */

/* -- Types Header -- */
#include <ti/mas/types/types.h>

/* -- Other pkges Headers -- */

/* -- AFE Local Headers -- */
#include <src/afeloc.h>
#include <src/afeswcfg.h>
#ifdef AER
#include <src/ecodrv/afeaeri.h>
#include <src/ecodrv/afeagc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* ============ AFE_init ========== */
/**
 *  @brief      Initializes AFE_inst and afeContext global data structures
 *
 *  @param[in]  obj    Pointer to AFE_Object
 *
 *  @return     None
 *
 *  @sa         None
 *
 */
void AFE_init (AFE_Object *obj)
{
    FIX_COMPIL_WARNING(obj);

}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

/* Nothing beyond this point */

