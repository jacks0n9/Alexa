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
    var lib = "c64/vpe_c.a64;c64/vpe_cm.a64";

    else if (prog.build.target.name == 'C64_big_endian')
    var lib = "c64e/vpe_c.a64e;c64e/vpe_cm.a64e";
    
    else if (prog.build.target.name == 'C64P')
    var lib = "c64P/vpe_c.a64P;c64P/vpe_cm.a64P";

    else if (prog.build.target.name == 'C64P_big_endian')
    var lib = "c64Pe/vpe_c.a64Pe;c64Pe/vpe_cm.a64Pe";

    else if (prog.build.target.name == 'C55')
    var lib = "c55/vpe_c.a55;c55/vpe_a.a55;c55/vpe_cm.a55";

    else if (prog.build.target.name == 'C55_large')
    var lib = "c55L/vpe_c.a55L;c55L/vpe_a.a55L;c55L/vpe_cm.a55L";

    else if (prog.build.target.name == 'C54')
    var lib = "c54n/vpe_cm.a54n";

    else if (prog.build.target.name == 'C54_far')
    var lib = "c54f/vpe_cm.a54f";

    return (lib);
}

