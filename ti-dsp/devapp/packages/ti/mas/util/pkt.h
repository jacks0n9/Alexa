/* ========================================================================== */
/**
 *  @file   pkt.h
 *
 *  path    /dsps_gtmas/ti/mas/util/pkt.h
 *
 *  @brief  Contains definitions and macros that deal with packing/unpacking.
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */
#ifndef __PKT_H__
#define __PKT_H__

/** @defgroup PKT 
 * This module contains definitions and macros that deal with packing/unpacking.
 * This module allows user to write portable code for all the targets supported by this module.
 *
 */

/** @ingroup PKT */
/* @{ */
#include <ti/mas/util/src/pkt_m.h>
/**
 * @brief Pack bytes into words
 *
 * Packs bytes (which are stored one-byte-per word in the least
 * significant 8 bits of the words) into words in big-endian
 * format (meaning most-significant bytes are stored in the
 * least significant bits of the words).
 *
 * @param[in]  bytes         Source bytes 
 * @param[out] words         Destination packed words
 * @param[in]  nbytes        Number of bytes to pack
 * @param[in]  offset        Number of bytes in 'words' to skip over
 *                         
 */
void pktPackBytesIntoWords (tword *bytes, tword *words, tint nbytes, tint offset);

/**
 * @brief Unpack words into bytes.
 *
 * Unpack words containing one or more bytes into individual bytes.
 * The most significant bits of word go into the lowest indexed
 * bytes (big endian format)
 *
 * @param[in]  words         Source packed words 
 * @param[out] bytes         Destination unpacked bytes
 * @param[in]  nbytes        Number of bytes to unpack
 * @param[in]  offset        Offset into first word to start unpacking
 *                         
 */
void pktUnpackWordsIntoBytes (tword *words, tword *bytes, tint nbytes, tint offset);

/**
 * @brief pack bytes into tuint(Min 16-bits).
 *
 * This utility is used to pack bytes from a data buffer into an array of tuint( Min 16-bits ).
 *
 * @param[in]  src           Pointer to dataBuffer with source data bytes
 * @param[out] dest          Pointer to byte array to pack data bytes to
 * @param[in]  nbytes        Number of bytes to pack
 *                         
 */
void pktPackWordToTuint (tword *src, tuint *dest, tint nbytes);

/**
 * @brief Repack words into words
 *
 * Copies words from source to destination starting at the 'offset' byte within the first word.
 *
 * @param[in]  src           Pointer to source packed words 
 * @param[out] dst           Pointer to destination packed words
 * @param[in]  sskip         Number of bytes in source to skip
 * @param[in]  nbytes        Number of bytes to copy
 * @param[in]  skip          Number of bytes in dst to skip over
 *                           In 24 bit word | 0 | 1 | 2 |
 *                         
 *****************************************************************************/
void pktRepackWordsIntoWords (tword *src, tword *dst, tint sskip, tint nbytes, 
                              tint skip);

/**
 * @brief Read n bit value from 8 bit word (function version).
 *
 * Copies n bit value from an word. Assumes the reading from MSB to LSB.
 *
 * @param[in]  base           Base of byte array
 * @param[in]  byteOffset     Byte offset to read
 * @param[in]  nbits          Number of bits to read
 *                         
 */
tword pktReadnbits (tword *base, tuint byteOffset, tint nbits);
/**
 * @brief Read 8 bit value from 16 bit word (function version).
 *
 * Returns 8 bit value from 16 bit word.  Assumes nothing.
 *
 * @param[in]  base           Base of byte array
 * @param[in]  byteOffset     Byte offset to read
 *                         
 */
tuint pktRead8bits (tword *base, tuint byteOffset);
/**
 * @brief Read 16 bit value from 16 bit word (function version).
 *
 * Returns 16 bit value from 16 bit word. 
 *
 * @param[in]  base           Base of byte array
 * @param[in]  byteOffset     Byte offset to read
 *                         
 */
tuint pktRead16bits (tword *base, tuint byteOffset);
/**
 * @brief Read 32 bit value from 16 bit word Array (function version).
 *
 * Returns Read 32 bit value from 16 bit words. No alignment assumed. 
 *
 * @param[in]  base           Base of byte array
 * @param[in]  byteOffset     Byte offset to read
 *                         
 */
tulong pktRead32bits (tword *base, tuint byteOffset);
/**
 * @brief Write 8 bit value into 16 bit word (function version).
 *
 * Writes 8 bit value into 16 bit word. No alignment assumed. 
 *
 * @param[in]  base           Base of byte array
 * @param[in]  byteOffset     Byte offset to read
 * @param[in]  val            Byte in low 8 bits to write                       
 */
void pktWrite8bits (tword *base, tuint byteOffset, tuint val);

/**
 * @brief Write 16 bit value into 16 bit word (function version).
 *
 * Writes 16 bit value into 16 bit word. No alignment assumed. 
 *
 * @param[in]  base           Base of byte array
 * @param[in]  byteOffset     Byte offset to read
 * @param[in]  val            16 bit value to write                       
 */
void pktWrite16bits (tword *base, tuint byteOffset, tuint val);
/**
 * @brief Write 32 bit value into 16 bit word Array (function version).
 *
 * Writes 32 bit value into 16 bit word Array. No alignment assumed. 
 *
 * @param[in]  base           Base of byte array
 * @param[in]  byteOffset     Byte offset to read
 * @param[in]  val            32 bit value to write                       
 */
void pktWrite32bits (tword *base, tuint byteOffset, tulong val);
/**
 * @brief Logical @b OR operation in a endian portable form.
 *
 * Performs OR operation in an endian protable way.The value to be ORed is
 * specified assuming big endian archtecture.The function will take care byte swapping for little endian architectures. 
 * 
 * @param[in]  src            Base of byte array
 * @param[in]  byteOffset     Byte offset to for OR operation
 * @param[in]  val            16 value to be ORed
 * 
 */
void pktOR16bits (tword *src, tuint byteOffset, tuint val);
/**
 * @brief Logical @b AND operation in a endian portable form.
 *
 * Performs AND operation in an endian protable way.The value to be ANDed is
 * specified assuming big endian architecture.The function will take care byte swapping for little endian architectures. 
 * 
 * @param[in]  src            Base of byte array
 * @param[in]  byteOffset     Byte offset to for AND operation
 * @param[in]  val            16 value to be ANDed
 * 
 */
void pktAND16bits (tword *src, tuint byteOffset, tuint val);
/* @} */ /* ingroup */
#endif /* __PKT_H__ */

/* nothing past this point */
