#ifndef _HALLOC_H
#define _HALLOC_H
/******************************************************************************
 * FILE PURPOSE: Local Hardware Abstraction Layer definitions and structures
 ******************************************************************************
 * FILE NAME: halloc.h
 *
 * DESCRIPTION: Contains definitions and structures restricted in scope to
 *              HAL.
 *
 * TABS: None
 * 
 * Copywrite (C) 2002 Telogy Networks, Inc.
 *
 *****************************************************************************/
#include "ti/mas/types/types.h"
#include "src/hal.h"

/*****************************************************************************
 * Definition: The HAL context structure. There is a single instance of this
 *             structure defined globally.  It should maintain as small a 
 *             foot print as possible.
 *****************************************************************************/
typedef struct {
  tint nTdmPorts;   /* The number of tdmPorts enabled                       */
  void **tdmPorts;  /* Array of pointers (size nTdmPorts) to port instances */
} halContext_t;
extern halContext_t *halContext;

/******************************************************************************
 * Definition: Internal call table used only for linking the hal initialization
 *             functions to the code.
 ******************************************************************************/
typedef struct {
  tuint (*halTdmGetHeapSize)   (halNewConfig_t *cfg);  
  tint  (*halTdmGetDmaBufInfo) (tint port, halNewConfig_t *cfg, tuint *align, tuint *size, tint tdmType);
  void *(*halTdmAllocAndInit)  (tint port, void *base, tuint *used, tuint size, halNewConfig_t *newCfg, tuint *dmaTx, tuint dmaTxSize, tuint *dmaRx, tuint dmaRxSize, halReturn_t *ret);
  halReturn_t (*halTdmInitStart) (void *tdmp);
  void (*halTdmResourceInit)   (void);
  halReturn_t (*halTdmStop)    (void);
} halInitCtable_t;                              
extern halInitCtable_t halInitCtable;

/*  Prototypes */
/* halinit.c */
halReturn_t halGetSizes (tint *nbufs, halMemBuffer_t **bufs, halNewConfig_t *cfg);

halReturn_t halNew (tint nbufs, halMemBuffer_t *bufs, halNewConfig_t *cfg);

halReturn_t halControl (halControl_t *hal_ctrl) ;

#endif /* _HALLOC_H */

