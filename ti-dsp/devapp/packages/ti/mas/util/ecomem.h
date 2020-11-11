#ifndef _ECOMEM_H
#define _ECOMEM_H

/* ========================================================================== */
/**
 *  @file   ecomem.h
 *
 *  @brief  ECO Memory definition
 *  
 *  (C) Copyright 2007, Texas Instruments, Inc.
 */

/** @defgroup  ECOMEM
 *  This module contains definition of memory descriptor structure
 *  and memory classes for ECO (Embedded Communication Objects).
 *
 */

/** @ingroup ECOMEM */
/* @{ */

#include<ti/mas/types/types.h>

 /* Memory classes (max. 32767) */
enum {
  ecomem_CLASS_EXTERNAL = 0,   /**< generic (bulk) external memory */
  ecomem_CLASS_INTERNAL = 1,   /**< generic (bulk) internal memory */
  ecomem_CLASS_GMP      = 1    /**< Global Memory Pool (this may have to change
                                    once GMP starts to be supported again.) */
};
  
/*! @brief Memory Buffer Structure.
 *
 * This structure is used to request and/or supply the 
 * dynamic memory to the components.
 */
typedef struct {

  tint  mclass;    /**< Memory class. It is used to describe kind of 
                       memory that is requested or returned. For 
                       example: external, internal, or similar. One 
                       should use MEM_CLASS_xxx constants. On RETURN, 
                       class may be changed if original memory class 
                       is exhausted. */
  tuint log2align; /**< Alignment information (>= 0). If zero, no 
                       alignment is required nor provided. Otherwise, 
                       memory buffer is aligned on appropriate power 
                       of 2 (e.g. if log2align=3, buffer is aligned 
                       modulo-8). */
  tsize size;      /**< Number of twords that are requested or supplied. 
                       Word length depends on the platform, and 
                       corresponds to the shortest element that can be 
                       addressed by the CPU. Word storage length in bits 
                       is defined in types.h as TYP_TWORD_SIZE. The 
                       maximum size depends on the platform. */
  tbool  volat;    /**< TRUE: Memory buffer must be restored before and 
                       saved after it is used.
                       FALSE: Save/restore are not necessary.
                       On RETURN, volatile flag may be changed if 
                       original request could not be satisfied. */
  void  *base;     /**< Base address of the requested buffer. */
} ecomemBuffer_t;

/* @} */ /* ingroup */

#endif

/* nothing past this point */
