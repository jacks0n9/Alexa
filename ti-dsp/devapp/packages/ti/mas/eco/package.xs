/*
 *  Copyright 2009 by Texas Instruments Incorporated.
 *
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */

/*
 *  ======== package.xs ========
 */
function getLibs(prog)
{
    var lib="";

    /* "mangle" program build attrs into an appropriate directory name */
    if (prog.build.target.name == 'C64P')
      lib = "c64P/eco_c" + ".a" + prog.build.target.suffix;

    else if (prog.build.target.name == 'MVArm9')
      lib = "a470MV/eco_a" + ".a" + prog.build.target.suffix;

    else if (prog.build.target.name == 'GCArmv6')
      lib = "av6/eco_a" + ".a" + prog.build.target.suffix;

    else if (prog.build.target.name == 'GCArmv7A')
      lib = "av7A/eco_a" + ".a" + prog.build.target.suffix;

    else if (prog.build.target.name == 'WinCE')
      lib = "av4TCE/eco_a" + ".a" + prog.build.target.suffix;

    /* return the library name: name.a<arch> */
    print("    will link with " + this.$name + ":" + lib);

    return (lib);
}

