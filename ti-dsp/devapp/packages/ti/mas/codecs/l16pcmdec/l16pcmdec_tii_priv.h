/*******************************************************************************
*-----------------------------------------------------------------------------*
*                     TEXAS INSTRUMENTS INC                                   *
*                     Multimedia Codecs Group                                 *
*                                                                             *
*-----------------------------------------------------------------------------*
*                                                                             *
* (C) Copyright 2004  Texas Instruments Inc.  All rights reserved.            *
*                                                                             *
* Exclusive property of the Multimedia Codecs Group, Texas Instruments Inc.   *
* Any handling, use, disclosure, reproduction, duplication, transmission      *
* or storage of any part of this work by any means is subject to restrictions *
* and prior written permission set forth.                                     *
*                                                                             *
* This copyright notice, restricted rights legend, or other proprietary       *
* markings must be reproduced without modification in any authorized copies   *
* of any part of this work. Removal or modification of any part of this       *
* notice is prohibited.                                                       *
*                                                                             *
*******************************************************************************/
/******************************************************************************
 * FILE PURPOSE: L16PCM Unit Local Definitions
 ******************************************************************************
 * FILE NAME   : l16pcmdec_tii_priv.h
 *
 * DESCRIPTION : This is the header file for the Codec API functions.
 *
 ******************************************************************************/

#ifndef L16PCMDEC_TII_PRIV_
#define L16PCMDEC_TII_PRIV_

/* Interface headers */
#include "ti/xdais/dm/isphdec1.h"

/* Some useful constants */
#define NBUFS_DEC 1

/******************************************************************************
 * DATA DEFINITION: Instance data for a L16PCM VCA module.
 ******************************************************************************
 * DESCRIPTION: This structure specifies the state of an instance of a L16PCM
 * VCA.
 *****************************************************************************/

typedef struct
{
  ISPHDEC1_Fxns  *fxns;
} L16PCMDEC_TII_Obj;

#define L16PCMDEC_BYTESWAP(a) ((((a) & 0xFF) << 8) | (((a) >> 8) & 0xFF))

#endif /* L16PCMDEC_TII_PRIV_ */

/* nothing past this point */

