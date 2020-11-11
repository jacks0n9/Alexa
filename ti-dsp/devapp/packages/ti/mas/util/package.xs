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
  /* "mangle" program build attrs into an appropriate directory name */
  if(prog.build.target.name == 'C55')
  var lib = "c55/util_a.a55;c55/util_c.a55;c55/util_cm.a55";
  
  else if (prog.build.target.name == 'C55_large')
  var lib = "c55L/util_a.a55L;c55L/util_c.a55L;c55L/util_cm.a55L";
 
  /* no C model for 54x - utilnum.c won't compile due to "long long" in utlDot */
  else if (prog.build.target.name == 'C54_far')
  var lib = "c54f/util_a.a54f;c54f/util_c.a54f";

  else if (prog.build.target.name == 'C54')
  var lib = "c54/util_a.a54;c54/util_c.a54";

  else if (prog.build.target.name == 'C64')
  var lib = "c64/util_a.a64;c64/util_c.a64;c64/util_cm.a64";

  else if (prog.build.target.name == 'C64_big_endian')
  var lib = "c64e/util_a.a64e;c64e/util_c.a64e;c64e/util_cm.a64e";

  else if (prog.build.target.name == 'C674')
  var lib = "c674/util_a.a674;c674/util_c.a674;c674/util_cm.a674";

  else if (prog.build.target.name == 'C674_big_endian')
  var lib = "c674e/util_a.a674e;c674e/util_c.a674e;c674e/util_cm.a674e";

  else if (prog.build.target.name == 'C64P')
  var lib = "c64P/util_a.a64P;c64P/util_c.a64P;c64P/util_cm.a64P";

  else if (prog.build.target.name == 'C64P_big_endian')
  var lib = "c64Pe/util_a.a64Pe;c64Pe/util_c.a64Pe;c64Pe/util_cm.a64Pe";
  
  else if (prog.build.target.name == 'C66')
  var lib = "c66/util_a.ae66;c66/util_c.ae66P;c66/util_cm.ae66";

  else if (prog.build.target.name == 'C66_big_endian')
  var lib = "c66e/util_a.ae66e;c66/util_c.ae66e;c66e/util_cm.ae66e";
  
  else if (prog.build.target.name == 'MVArm9')
  var lib = "mvarm9/util_c.a470MV";
  
  else if (prog.build.target.name == 'GCArmv6')
  var lib = "gcarmv6/util_c.av6";
  
  else if (prog.build.target.name == 'GCArmv7A')
  var lib = "gcarmv7a/util_c.av7A";

  else if (prog.build.target.name == 'WinCE')
  var lib = "wince/util_c.av4TCE";
  
  else if (prog.build.target.name == 'VC98')
  var lib = "vc98/util_c.a86";
  
  else if (prog.build.target.name == 'Arm11')
  var lib = "tiarmv6le/util_c.a11";
  
  else if (prog.build.target.name == 'Arm11_big_endian')
  var lib = "tiarmv6be/util_c.a11e";
  
  /* Temporary patches for 64P elf support */
  if (prog.build.target.suffix == 'e64P')
  var lib = "c66/util_a.ae66;c66/util_c.ae66P;c66/util_cm.ae66";

  if (prog.build.target.suffix == 'e64Pe')
  var lib = "c66e/util_a.ae66e;c66e/util_c.ae66e;c66e/util_cm.ae66e";
  
  

  return (lib);
}
/* nothing past this point */
