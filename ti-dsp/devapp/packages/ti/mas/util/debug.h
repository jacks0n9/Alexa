#ifndef _DEBUG_H
#define _DEBUG_H

/*!
 *  @file   debug.h
 *
 *  @brief  Debug definition.
 *
 *  (C) Copyright 2007, Texas Instruments, Inc.
 */

/** @defgroup  DEBUG 
 * This module contains function and macros for component debugging. 
 * It is used by components for reporting debug info, warnings or critical exceptions to the host.
 */

/** @ingroup DEBUG */
/* @{ */

#include<ti/mas/types/types.h> 

  #define dbg_TRACE_INFO      0  /**< General Debug Info*/
  #define dbg_WARNING         1  /**< Non-Critical Debug Info*/
  #define dbg_EXCEPTION_FATAL 2  /**< Critical Debug Info*/

/*! Defines debug structure to be used by the modules to communicate 
    debug information to the system*/
typedef void (*dbgInfo_t)(void *, tuint, tuint, tuint, tuint*); 

#endif
/* @} */ /* ingroup */

/* nothing past this point */
