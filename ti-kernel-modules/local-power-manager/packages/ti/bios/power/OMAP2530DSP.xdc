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
 *  ======== OMAP2530DSP.xdc ========
 */

package ti.bios.power;


/*! DSP configuration module for the OMAP2530 device.
 *
 *  Configuration parameters for the Local Power Manager running
 *  on the DSP core of an OMAP2530 device.
 */
@Template("./config.xdt")

metaonly module OMAP2530DSP inherits ITMS320CDM644x {

    //! dsp runtime context buffer
    struct RuntimeContext {
        UInt32  len;            //! length of context buffer
        Bool    useUserFxns;    //! override save/restore functions
        String  saveFxn;        //! context save function name
        String  restoreFxn;     //! context restore function name
    };

    //! maximum allowable size for context buffer
    readonly config unsigned CONTEXT_BUFFER_MAX_SIZE = 0x40000;

    //! default value for dsp runtime context buffer
    config RuntimeContext dspRC = {
        len:            0x40000,
        useUserFxns:    false,
        saveFxn:        "",
        restoreFxn:     "",
    };

    //! default edma channel to use for save/restore transfers
    config UInt32 edmaChannel = 45;

    /*! name given to target memory range for storing context data
     *
     *  This must be set to a valid target memory range defined in the
     *  linker command file's memory section.
     */
    config string targetMemoryRangeName = "";

    //! cpu name, useful for adding validation checks
    readonly config string CPU_NAME = "DSP";

    //! stop BIOS clock when hibernating
    config Bool stopBiosClockOnHibernate;

    //! stop CPU load monitoring when hibernating
    config Bool stopCpuLoadMonitorOnHibernate = true;

    //! WUGEN wake event mask
    config UInt32 wugenEventMask[3];
}
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:30; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

