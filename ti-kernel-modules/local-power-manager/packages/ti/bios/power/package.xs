/* 
 * Copyright (c) 2011, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*
 *  ======== close ========
 */
function close()
{
    var device = Program.cpu.deviceName;
    var isa = Program.build.target.isa;


    // if running on dsp on DM644[16], bring in bios package
    if (device.match(/TMS320CDM644[16]/) && isa.match(/^64P$/)) {

        xdc.loadPackage('ti.bios');

        // must disable pwrm on DM6446 when using this package
        if ($$legacy.prog.module("PWRM") !== undefined) {
            $$legacy.prog.module("PWRM").ENABLE = 0;
        }

        // add our power handler to the bios idle loop
        var pwrmIdlObj = $$legacy.prog.module("IDL").create("pwrmIdlObj");
        pwrmIdlObj.fxn = $$legacy.prog.extern("PWRM_idleFxn");
        pwrmIdlObj.calibration = false;

        // must call prog.gen() to recreate the configuration model
        $$legacy.prog.gen();
    }

    // if running on dsp on OMAP3430/3530/2530, bring in bios package
    else if (device.match(/TMS320C3430|OMAP[23]530/) && isa.match(/^64P$/)) {

        xdc.loadPackage('ti.bios');

        // make sure pwrm is available
        if ($$legacy.prog.module("PWRM") == undefined) {
            throw new Error("Advanced LPM features are not available on " +
                "this device. Contact your TI representative for " +
                "additional details.");
        }

        // must enable pwrm on omap2 & omap3, supported by bios
        $$legacy.prog.module("PWRM").ENABLE = 1;

        // add our power handler to the bios idle loop
        var pwrmIdlObj = $$legacy.prog.module("IDL").create("pwrmIdlObj");
        pwrmIdlObj.fxn = $$legacy.prog.extern("LPM_TAL_idleFxn");
        pwrmIdlObj.calibration = false;

        // must call prog.gen() to recreate the configuration model
        $$legacy.prog.gen();
    }
}

/*
 *  ======== getSects ========
 */
function getSects()
{
    // convert the qualified package name to a pathname for the linker
    return this.$name.replace(/\./g, "/") + "/link.xdt";
}

 

/*
 *  ======== getLibs ========
 *  This function is called when a program's configuration files are
 *  being generated and it returns the name of a library appropriate
 *  for the program's configuration.
 *
 *  The 'this' object is the current package object being included by
 *  the program being configured.  Thus, it is possible to
 *  conditionally return library names based on package attributes
 *  including the program object itself (Program).
 *
 *  Parameters
 *      this        - the current package object
 *      prog        - the program being configured (after all of the
 *                    program's configuration scripts complete)
 *      prog.build  - program attributes specified in the build script
 *
 *  Returns
 *      null        - indicates that no library is to be linked
 *      non-null    - name of the library to link with (relative to the
 *                    package directory).
 */

function getLibs(prog)
{
    var libs = new Array();
    var libPrefix = "lib/" + this.profile + "/";
    var module = this[this.$private.deviceModule];
    var device = prog.cpu.deviceName;
    var isa = prog.build.target.isa;
    var suffix;
    
    // XDCtools 3.10 has new findSuffix() function
    if ("findSuffix" in prog.build.target) {
        suffix = prog.build.target.findSuffix(this);
        if (suffix == null) {
            return "";  // nothing to contribute
        }
    }
    else {
        suffix = prog.build.target.suffix;
    }

    // check linux (gpp) first, same for all devices
    if (("os" in prog.build.target) && (prog.build.target.os != undefined)) {

        // lpm api user mode library
        libs.push(libPrefix + "lpm.a" + suffix);
    }

    // DM644x device, building for dsp side
    else if (device.match(/^TMS320CDM644[136]$/)
        && isa.match(/^64P$/)
        && (module.opersys == module.Opersys_DSPBIOS)
        && (module.transport == module.Transport_Link_Linux)) {

        // power management library
        libs.push(libPrefix + "pwrm_DaVinci.a" + suffix);
        libs.push(libPrefix + "tal_dsplink_bios_dm6446.a" + suffix);
    }

    // OMAP3430 device, building for dsp side
    else if (device.match(/^TMS320C3430$|^OMAP3530$/)
        && isa.match(/^64P$/)
        && (module.opersys == module.Opersys_DSPBIOS)
        && (module.transport == module.Transport_Link_Linux)) {

        libs.push(libPrefix + "tal_dsplink_bios_3530.a" + suffix);
    }

    // OMAP2530 device, building for dsp side
    else if (device.match(/^OMAP2530$/)
        && isa.match(/^64P$/)
        && (module.opersys == module.Opersys_DSPBIOS)
        && (module.transport == module.Transport_Link_Linux)) {

        libs.push(libPrefix + "tal_dsplink_bios_2530.a" + suffix);
    }

    else {
        // currently not supported
        throw new Error("unsupported device: " + device);
    }

    return libs.join(";");
}


/*
 *  ======== validate ========
 */
function validate()
{
    var device = this[this.$private.deviceModule];
    var moduleName = device.$name.match(/[^.]+$/)[0];

    // if proxy is given, then it must match the given device
    if ((device.proxyFor != undefined)
        && (device.proxyFor != Program.cpu.deviceName)) {

        throw "Error: device proxy (" + device.proxyFor + ") does not"
            + " match given device (" + Program.cpu.deviceName + ")";
    }

    // if proxy is undefined, then module must match given device
    else if ((device.proxyFor == undefined)
        && (!moduleName.match(Program.cpu.deviceName))) {

        throw "Error: power module (" + moduleName + ") does not support"
            + " given device (" + Program.cpu.deviceName + ")";
    }

    else if ((device.CPU_NAME == "DSP")
        && (!Program.build.target.name.match(/C64P/))) {

        throw "Error: power module cpu (" + device.CPU_NAME + ") does not"
            + " match given cpu (GPP)";
    }

    else if ((device.CPU_NAME == "GPP")
        && (Program.build.target.name.match(/C64P/))) {

        throw "Error: power module cpu (" + device.CPU_NAME + ") does not"
            + " match given cpu (DSP)";
    }

    // check for supported devices
    if (moduleName.match(/TMS320CDM644[16]DSP/)) {

        // setup some helpful aliases for readability
        var MAX = device.CONTEXT_BUFFER_MAX_SIZE;
        var dspRC = device.dspRC;
        var imcopRC = device.imcopRC;

        // target memory range name must be specified
        if (device.targetMemoryRangeName == "") {
            throw "Error: targetMemoryRangeName must be specified";
        }

        // validate DSP context buffer
        if (dspRC.len > MAX) {
            throw "Error: dspRC buffer size too large"
                  + " (max = 0x" + MAX.toString(16) + ")";
        }

        if (dspRC.useUserFxns) {
            if (dspRC.saveFxn == "") {
                throw "Error: must define dspRC.saveFxn when"
                      + " dspRC.useUserFxn = true";
            }
            if (dspRC.restoreFxn == "") {
                throw "Error: must define dspRC.restoreFxn when"
                      + " dspRC.useUserFxn = true";
            }
        }

        // validate IMCOP context buffer
        if (imcopRC.len > MAX) {
            throw "Error: imcopRC buffer size too large"
                  + " (max = 0x" + MAX.toString(16) + ")";
        }

        if (imcopRC.useUserFxns) {
            if (imcopRC.saveFxn == "") {
                throw "Error: must define imcopRC.saveFxn when"
                      + " imcopRC.useUserFxn = true";
            }
            if (imcopRC.restoreFxn == "") {
                throw "Error: must define imcopRC.restoreFxn when"
                      + " imcopRC.useUserFxn = true";
            }
        }
    }
}
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:32; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

