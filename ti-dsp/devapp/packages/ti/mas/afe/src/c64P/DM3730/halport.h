/******************************************************************************
 * FILE PURPOSE: HAL Portability Definitions (C64+ LEO version)
 ******************************************************************************
 * FILE NAME:   halport.h
 *
 * DESCRIPTION: Contains HAL portability definitions.
 *
 * TABS: NONE        
 *
 * (C) Copyright 2007, Texas Instrument Inc.
 *****************************************************************************/

#ifndef _HALPORT_H
#define _HALPORT_H

/* macro to even align internal carving of memory */
#define HAL_ALIGN_STRUCT(x)  (x = ((x)+7) & (~7))

#endif  /* _HALPORT_H */

/* nothing past this point */
