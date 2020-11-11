/******************************************************************************
 * FILE PURPOSE: Defines libarary directory name using getLibs
 ******************************************************************************
 * FILE NAME: package.xs
 *
 * DESCRIPTION: This file defines the library directory name for proper build
 *              in case a different directory name for storing library files 
 *              other than "lib" is used. XDC by default assumes that the 
 *              library directory is "lib" is not sepcifically indicated by use
 *              the attributes in a file called package.xs  
 *
 * TABS: NONE
 *
 * Copyright (C) 2006, Texas Instruments, Inc.
 *****************************************************************************/

function getLibs(prog)
{
  var lib = "";

  /* "mangle" program build attrs into an appropriate directory name */
  if(prog.build.target.name == 'C64')
  var lib = "c64/veu_c.a64";

  if(prog.build.target.name == 'C64_big_endian')
  var lib = "c64e/veu_c.a64e";

  if(prog.build.target.name == 'C64P')
  var lib = "c64P/veu_c.a64P";

  if(prog.build.target.name == 'C64P_big_endian')
  var lib = "c64Pe/veu_c.a64Pe";

  return (lib);
}
/* nothing past this point */
