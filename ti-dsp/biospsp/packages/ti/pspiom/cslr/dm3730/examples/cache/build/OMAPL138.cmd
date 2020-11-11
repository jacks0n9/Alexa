/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2002, 2003, 2004, 2005
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *  ============================================================================
 */

/*  ============================================================================
 *  OMAPL137.cmd
 *
 *    Usage:  lnk6x <obj files...>    -o <out file> -m <map file> lnk.cmd
 *            cl6x  <src files...> -z -o <out file> -m <map file> lnk.cmd
 *
 *    Description: This file is a sample linker command file that can be
 *                 used for linking programs built with the C compiler and
 *                 running the resulting .out file.  You will want
 *                 to change the memory layout to match your specific C6xxx
 *                 target system.  You may want to change the allocation
 *                 scheme according to the size of your program.
 *
 *  ============================================================================
 */

-c
-heap  0x1000
-stack 0x1000

MEMORY
{
   L1D:     o = 00F00000h   l = 00008000h
   L1P:     o = 00E00000h   l = 00008000h
   L2:      o = 00800000h   l = 00040000h
}

SECTIONS
{
    .text       >       L2
    .stack      >       L2
    .bss        >       L2
    .cinit      >       L2
    .cio        >       L2
    .const      >       L2
    .sysmem     >       L2
    .far        >       L2
    .switch     >       L2
    .bios       >       L2
}
