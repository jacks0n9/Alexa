/*
 *  Copyright 2007 by Texas Instruments Incorporated.
 *
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */

/*
 *  ======== package.xs ========
 *    Implementation of the xdc.IPackage interface.
 */

/*
 *  ======== getLibs ========
 *  Determine the name of the library to use, based on the program
 *  configuration (prog).
 */
function getLibs(prog)
{
    var lib = "";

    /*
     * Note that this package demonstrates several different ways to
     * build, including CCS projects, GNU makefiles and XDC Builds.
     * The location of the libraries, however are always in the same place.
     * As a result, this getLibs function is relatively simple.
     */

    if (this.profile == "debug") {
      /* "mangle" program build attrs into an appropriate directory name */
      if (prog.build.target.isa == '64P') {
        lib = "lib/l16pcmdec.a" + prog.build.target.suffix;
      }
    }
    else {
      if (prog.build.target.isa == '64P') {
        lib = "lib/l16pcmdec.a" + prog.build.target.suffix;
      }
    }

    /* and dump a helpful breadcrumb */
    print("    will link with " + this.$name + ":" + lib);

    return (lib);
}

