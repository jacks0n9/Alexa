#ifndef _CGA_C54_H
#define _CGA_C54_H
/*****************************************************************************
 * FILE PURPOSE: Code generation (compiler) assist macros
 *****************************************************************************
 * FILE NAME:   cga.h
 *
 * DESCRIPTION: Contains code generation assist macros that help to express
 *              assumptions to compiler to improve optimization. These 
 *              macros allow common C code to be used across processors. See
 *              coding guidelines for more information
 *
 *****************************************************************************
 * (C) Copyright 2007 Texas Instruments Inc.
******************************************************************************/

/* restrict qualifier, see compiler user manual for more information */
#define cga_RESTRICT 

/* loop information, helps express truth values about the loop counter */
#define cga_LOOP_COUNT_INFO(min, max, multiple, minOften, maxOften)

/* can be used to express alignment assumptions */
#define cga_PTR_IS_ALIGNED(ptr, pow2) 

#endif
/* Nothing past this point */
