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
 * Copyright (C) 2008, Texas Instruments, Inc.
 *****************************************************************************/


function getLibs(prog)
{
    var lib="";

    /* "mangle" program build attrs into an appropriate directory name */
    if (prog.build.target.name == 'C64')
    lib = "c64/genCtl_c.a64";
   
    else if (prog.build.target.name == 'C64P')
    lib = "c64P/genCtl_c.a64P";

    else if (prog.build.target.name == 'C64P_big_endian')
    lib = "c64Pe/genCtl_c.a64Pe";

    else if (prog.build.target.name == 'C64P_little_endian')
    lib = "c64Pe/genCtl_c.a64Ple";

    return (lib);
}

