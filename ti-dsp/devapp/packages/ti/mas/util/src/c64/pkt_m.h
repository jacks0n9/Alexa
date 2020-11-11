#ifndef __PKT_M_C64_H
#define __PKT_M_C64_H
/******************************************************************************
 * FILE PURPOSE: C64X platform specific macros and definitions
 ******************************************************************************
 * FILE NAME: pkt_m.h  
 *
 * DESCRIPTION: This contains 8 bit byte in 16 bit word manipulation via
 *              macros.  It is important to check size-optimization quality
 *              of compiler, and tune these macros to minimize size given
 *              existing alignments.  It might be smaller to test proper
 *              alignment with an if, and directly assign it than have
 *              optimizer optimize shifting and masking.
 *
 * (C) Copyright 2002, TELOGY Networks, Inc.
 *****************************************************************************/
#include <ti/mas/types/types.h>

/******************************************************************************
 * FUNCTION PURPOSE: Read n bit value from 8 bit word (macro version)
 ******************************************************************************
 * DESCRIPTION: copies n bit value from an word. Assumes nothing
 * tuint pktReadnbits_m (
 *    tword *base,      - Base of byte array
 *    tuint byteOffset, - Byte offset to read
 *    tuint nbits);     - Number of bits to read
 * 
 *****************************************************************************/
static inline tword pktReadnbits_m (tword *base, tuint byteOffset, tint nbits) 
{
  char *src = (char *)base;
  char wvalue = *(src + byteOffset);
  tuint readByte = (tuint)(wvalue & 0xFF);
  tword temp_bit;

  if (nbits > 0)
	temp_bit = readByte >> nbits; 
  else
	temp_bit = readByte << (-nbits); 
  return (temp_bit);
} /* pktReadnbits_m */

/******************************************************************************
 * FUNCTION PURPOSE: Read 8 bit value from 16 bit word (macro version)
 ******************************************************************************
 * DESCRIPTION: Returns 8 bit value from 16 bit word.  Assumes nothing.
 * 
 * tuint pktRead8bits_m (
 *    tword *base,       - Base of byte array
 *    tuint byteOffset); - Byte offset to read
 * 
 *****************************************************************************/
static inline tuint pktRead8bits_m (tword *base, tuint byteOffset) 
{
  char *src = (char *)base;
  char wvalue = *(src + byteOffset);
  tuint readByte = (tuint)(wvalue & 0xFF);
  return readByte;
} /* pktRead8bits_m */

/******************************************************************************
 * FUNCTION PURPOSE: Write 8 bit value into 16 bit word (macro version)
 ******************************************************************************
 * DESCRIPTION: Writes 8 bit value into 16 bit word; nothing assumed.
 * 
 * void pktWrite8bits_m (
 *    tword *base,      - Base of byte array
 *    tuint byteOffset, - byte offset to write
 *    tuint val)        - Byte in low 8 bits of val
 * 
 *****************************************************************************/
static inline void pktWrite8bits_m (tword *base, tuint byteOffset, tuint val)
{
  char *wptr = ((char *)base + byteOffset);
  *wptr = (char)(val & 0xFF);
} /* pktWrite8bits_m */

/******************************************************************************
 * FUNCTION PURPOSE: Write 16 bit value into 16 bit word (macro version)
 ******************************************************************************
 * DESCRIPTION: Writes 16 bit value into 16 bit word.  No assumptions
 * 
 * void pktWrite16bits_m (
 *    tword *base,      - Base of byte array
 *    tuint byteOffset, - byte offset to write; assumed to be even
 *    tuint val)        - 16 bit val
 * 
 *****************************************************************************/
static inline void pktWrite16bits_m (tword *base, tuint byteOffset, tuint val) 
{
  char *wptr = ((char *)base + byteOffset);

  /* Shift/mask is endian-portable, but look out for stupid compilers */
  wptr[0] = (char)(val>>8);
  wptr[1] = (char)(val & 0xff);

} /* pktWrite16bits_m */

/******************************************************************************
 * FUNCTION PURPOSE: Read 16 bit value from 16 bit word (macro version)
 ******************************************************************************
 * DESCRIPTION: Returns 16 bit value from 16 bit word.  No assumptions.
 * 
 * tuint pktRead16bits_m (
 *    tword *base,       - Base of byte array
 *    tuint byteOffset); - Byte offset to read, assumed to be even
 * 
 *****************************************************************************/
static inline tuint pktRead16bits_m (tword *base, tuint byteOffset) 
{
  char *wptr = ((char *)base + byteOffset);
  tuint ret;

  /* Shift/mask is endian-portable, but look out for stupid compilers */
  ret = (((tuint)wptr[0]) << 8) | (wptr[1] & 0xFF);

  return ret;
} /* pktRead16bits_m */

/******************************************************************************
 * FUNCTION PURPOSE: Write 32 bit value into 16 bit words (macro version)
 ******************************************************************************
 * DESCRIPTION: Writes 32 bit value into 16 bit word; No
 *              alignment assumed
 * 
 * void pktWrite32bits_m (
 *    tword *base,      - Base of byte array
 *    tuint byteOffset, - byte offset to write; assumed to be even.
 *    tulong val)       - 32 bit val
 * 
 *****************************************************************************/
static inline void pktWrite32bits_m (tword *base, tuint byteOffset, tulong val) 
{
  /* Shift/mask is endian-portable, but look out for stupid compilers */
  pktWrite16bits_m (base, byteOffset, (tuint)(val>>16));
  pktWrite16bits_m (base, byteOffset+2, (tuint)(val&0xffff));

} /* pktWrite32bits_m */

/******************************************************************************
 * FUNCTION PURPOSE: Read 32 bit value from 16 bit words (macro version)
 ******************************************************************************
 * DESCRIPTION: Read 32 bit value from 16 bit words; No
 *              alignment assumed
 * 
 * tulong pktRead32bits_m (
 *    tword *base,      - Base of byte array
 *    tuint byteOffset) - byte offset to write; assumed to be even.
 * 
 *****************************************************************************/
static inline tulong pktRead32bits_m (tword *base, tuint byteOffset) 
{
  tulong ret;

  /* Shift/mask is endian-portable, but look out for stupid compilers */
  ret = (((tulong)pktRead16bits_m (base, byteOffset)) << 16);
  ret |= (tulong)pktRead16bits_m (base, byteOffset + 2);

  return ret;
} /* pktRead32bits_m */

/*********************************************************************************
 * FUNCTION PURPOSE: OR operation in a endian portable form
 *********************************************************************************
 * DESCRIPTION: Performs OR operation in an endian protable way.The value to be 
 *              ORed is specified assuming big endian archtecture.The function will 
 *              take care byte swapping for little endian architectures 
 * 
 * void pktOR16bits(
 *    tword *src,      - Base of byte array 
 *    tint  byteOffset - byte offset to for OR operation
 *    tint  val        - value to be ORed 
 * 
 *********************************************************************************/
static inline void pktOR16bits_m (tword *src, tuint byteOffset, tuint val)
{
  tuint existing_val = pktRead16bits_m (src, byteOffset);
  existing_val |= val;
  pktWrite16bits_m (src, byteOffset, existing_val);
} /* pktOR16bits_m */

/*********************************************************************************
 * FUNCTION PURPOSE: AND operation in a endian portable form
 *********************************************************************************
 * DESCRIPTION: Performs AND operation in an endian protable way.The value to be 
 *              ANDed is specified assuming big endian archtecture.The function will 
 *              take care byte swapping for little endian architectures 
 * 
 * void pktAND16bits(
 *    linSample *src,      - Base of byte array 
 *    tint      byteOffset - byte offset to for AND operation
 *    tint      val        - value to be ANDed 
 * 
 *********************************************************************************/
static inline void pktAND16bits_m (tword *src, tuint byteOffset, tuint val)
{
  tuint existing_val = pktRead16bits_m (src, byteOffset);
  existing_val &= val;
  pktWrite16bits_m (src, byteOffset, existing_val);
} /* pktAND16bits_m */

#endif /* __PKT_M_C64_H */

/* nothing past this point */
