/* ========================================================================== */
/**
 *  @file   ti/mas/aeu/AEU.xs
 *
 *  @brief  AIU codec configuration information.
 *
 *
 *  ============================================================================
 */
/* --COPYRIGHT--,BSD
 * Copyright (c) 2008, Texas Instruments Incorporated
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
 * --/COPYRIGHT--*/

/*!
 *  ======== AEU.xs ========
 *  This file implements the functions specified in the ti.mas.eco.IECO
 *  interface.  These functions enable the configuration tool to validate
 *  user supplied configuration parameters (e.g., stack size for the
 *  thread running this codec).
 */

var verbose = false;   /* set to true to enable verbose output during config */

/*
 *  ======== getStackSize ========
 */
function getStackSize(prog)
{
    if (verbose) {
        print("getting stack size for " + this.$name
            + " built for the target " + prog.build.target.$name
            + ", running on platform " +  prog.platformName);
    }

    return (1024);
}

/*
 *  ======== getDaramScratchSize ========
 */
function getDaramScratchSize(prog)
{
    if (verbose) {
        print("getting DARAM scratch size for " + this.$name
            + " built for the target " + prog.build.target.$name
            + ", running on platform " +  prog.platformName);
    }

    return (0);
}

/*
 *  ======== getSaramScratchSize ========
 */
function getSaramScratchSize(prog)
{
    if (verbose) {
        print("getting SARAM scratch size for " + this.$name
            + " built for the target " + prog.build.target.$name
            + ", running on platform " +  prog.platformName);
    }

    return (0);
}

