/*****************************************************************************
 * FILE PURPOSE: HAL Call table setup
 *****************************************************************************
 * FILE NAME: hal.c
 *
 * DESCRIPTION: Sets up the HAL call table for Leo
 *
 * TABS: None
 *
 * Copywrite (C) 2007, Texas Instrument Inc.
 *
 *****************************************************************************/
/* -- Standard C Headers -- */
#include <stdlib.h>
#include "src/hal.h"
#include "src/halloc.h"

#include "src/c64P/DM3730/tdm/tdm.h"

halApi_t hal = {
  halGetSizes,              /* halGetSizes         */
  halNew,                   /* halNew              */
  halControl,               /* halControl          */
  NULL,                     /* halTdmInit          */
  halTdmControl             /* halTdmControl       */
};

/*****************************************************************************
 * Declaration of the intialization call table used by halinit. This table
 * can often be overlayed with the utopia tx buffer since it is used
 * only once.
 ****************************************************************************/
halInitCtable_t halInitCtable = {
  halTdmGetHeapSize,        /* halTdmGetHeapSize   */
  halTdmGetDmaBufInfo,      /* halTdmGetDmaBufInfo */
  halTdmAllocAndInit,       /* halTdmAllocAndInit  */
  halTdmInitStart,          /* halTdmInitStart     */
  NULL,                     /* halTdmResourceInit  */
  halTdmStop                /* halTdmStop          */
};

halContext_t *halContext;

/* Nothing beyond this line */
