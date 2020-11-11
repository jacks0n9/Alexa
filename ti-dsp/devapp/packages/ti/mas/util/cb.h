/* ========================================================================== */
/**
 *  @file   cb.h
 *
 *  path    /dsps_gtmas/ti/mas/util/cb.h
 *
 *  @brief  Header file for circular buffer management
 *
 *  ============================================================================
 *  Copyright (c) Texas Instruments Incorporated 2002-2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied or provided.
 *  ============================================================================
 */

#ifndef _CIRCBUF_H
#define _CIRCBUF_H

/** @defgroup  CB 
 * This modules contains function and macros for creating and managing @b circular @b buffer(CB).
 * Circular buffer is defined in terms of words. So size of the circular buffer 
 * in words is (number of elements in buffer times size of each element in words). Indexing is used to
 * take care of reading from and writing to the buffer. Any type can be used for the elements.
 */

/** @ingroup CB */
/* @{ */

/* System level header files */
#include <string.h>
#include <stddef.h>

/* ANSI C header files */
#include <ti/mas/types/types.h>                     

/**
 * @brief Circular buffer data structure
 */
typedef struct 
{
  tint buf_len;   /**< Number of elements allocated in the buffer */
  tint elm_size;  /**< Size (in words) of each element            */
  tword *base;    /**< Base address of the start of the buffer    */
  tint in;        /**< word index for in (write) operation        */
  tint out;       /**< word index for out (read) operation        */
  tint end;       /**< index for the last word of the buffer      */
} cbCircBuf_t; 

/**
 * @brief Add a single data item to a circbuf.
 *
 * @param[in]     q  Pointer to Circular buffer structure 
 * @param[in]     x  pointer to the input data item
 */

static inline void cbPush (cbCircBuf_t *q, const void *x)
{
  memcpy(&q->base[q->in], x, q->elm_size);
  q->in += q->elm_size;
  if ( q->in >= q->end ) 
    q->in = 0;  
} 
/**
 * 
 * @brief This routine adds a single integer element to a circbuf. 
 *
 * @param[in]     q  Pointer to Circular buffer structure 
 * @param[in]     x  pointer to the input data item
 *
 */

static inline void cbPushi (cbCircBuf_t *q, tint x)
{
  q->base[q->in++] = x;
  if ( q->in >= q->end ) 
    q->in = 0;  
} /*cbPushi */

/**
 * @brief This routine removes a single element from a circbuf. 
 * 
 * @param[in]  q  Pointer to Circular buffer structure 
 * 
 * @retval Pointer to the output data item.
 */

static inline void *cbPop (cbCircBuf_t *q)
{
  void *y;

  y = &q->base[q->out];
  q->out += q->elm_size;
  if ( q->out >= q->end )
    q->out = 0;
  return (y);
} /* cbPop */

/**
 * 
 * @brief Checks whether the circular buffer is empty or no
 *
 * @param[in]  q  Pointer to Circular buffer structure 
 *
 * @retval TRUE  Circular Buffer is empty
 * @retval FALSE Circular Buffer is not empty 
 ****************************************************************************/

static inline tbool cbEmpty (cbCircBuf_t *q)
{
  return (q->in == q->out);
} /* cbEmpty */

/**
 * @brief Set the offset between circular buffer's <b>in</b> and <b>out</b> index.
 *
 * @param[in]     q   Pointer to Circular buffer structure 
 * @param[in]     len offset to be set between <b>in</b> and <b>out</b> index of Circular buffer
 *          
 * @remark  The <b>len</b> is in terms of elements (not words!). The <b>in</b> index
 *              remains unchanged, while the <b>out</b> index is set <b>len</b> elements
 *              "behind" the <b>in</b> index. In this way the last len elements are
 *              retained in the buffer.
 ****************************************************************************/

static inline void cbSetSize (cbCircBuf_t *q, tint len)
{
  q->out = q->in - (len * q->elm_size);
  if ( q->out < 0 )
    q->out += q->buf_len * q->elm_size;
} /* cbSetSize */

/**
 * @brief Returns the number of elements in a circbuf.
 * 
 * @param[in]     q   Pointer to Circular buffer structure 
 */

static inline tint cbGetSize (cbCircBuf_t *q)
{
  tint a = (q->in - q->out)/q->elm_size;
  return (a >= 0 ? a : a+q->buf_len);
} /* cbGetSize */

/**
 * @brief Initialize a circular buffer.
 *
 * Resets the circular buffer pointers. Initializes its data to
 * 0. Circular buffer is made up of words. Number of words
 * required is (number of elements in the buffer times size of
 * each element in words). Indexing is used for reading and
 * writing. In and out indices point to words not elements as
 * elements can be more than one word long. 
 *
 * @param[in]  q        Pointer to circular buffer
 * @param[in]  base     Base address of the buffer 
 * @param[in]  length_n Number of elements in the buffer 
 * @param[in]  elm_size Size of each element in words
 *
 * @remark Total number of words must be less than or equal to 32767.
 *                     
 */
void cbNew (cbCircBuf_t *q, void *base, tint length_n, tint elm_size);
/**
 * @brief Reset the circbuf. 
 * 
 * @param[in]  q        Pointer to circular buffer
 *
 * @remark Memory allocated to the circular buffer 
 *         must be released after call to this function.
 *****************************************************************************/
void cbDelete ( cbCircBuf_t *q );

/**
 * @brief Copy a linear array to a circbuf.
 *
 * Pushes elements of a linear buffer into a circular buffer. 
 * The linear buffer elements are written to the circular buffer
 * in the order of increasing addresses.
 *
 * @param[in]  cb        Pointer to circular buffer
 * @param[in]  x        Pointer to linear buffer
 * @param[in]  length_x Number of elements in linear buffer 
 *
 * @remark Assumes that the length of the array is less than the length of the
 *  circular buffer. Linear and circular buffers must have elements of the same type.
 *
 */
void cbLinearToCircular (cbCircBuf_t *cb, const void *x, tint length_x);

/**
 * @brief Removes elements from a circular buffer and writes them to a
 *        linear buffer in order of increasing addresses.
 *
 * @param[in]  x         Pointer to linear buffer
 * @param[in]  cb         Pointer to circular buffer
 * @param[in]  length_cp Number of elements in circular buffer 
 *
 * @remark Linear and circular buffers must have elements of the same type.
 */
void cbCircularToLinear (void *x, cbCircBuf_t *cb, tint length_cp);

/* @} */ /* ingroup */
#endif  /* _CIRCBUF_H */
/* nothing past this point */
