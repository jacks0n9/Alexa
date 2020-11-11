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
 *  ======== ITMS320CDM644x.xdc ========
 */

package ti.bios.power;


/*! Interface for TMS320DM644x devices.
 *
 *  The common configuration parameters for all TMS320DM644x devices
 *  are contained within this module.
 */
metaonly interface ITMS320CDM644x {

    //! Runtime Operating System
    enum OpersysCfg {
        Opersys_PrOS_DaVinci,   //! PrOS on DaVinci platform
        Opersys_Linux,          //! Linux (HLOS)
        Opersys_DSPBIOS         //! DSP/BIOS
    };

    //! Define which operating system is running on the CPU (required)
    config OpersysCfg opersys;

    //! Transport Layer used between GPP and DSP
    enum TransportCfg {
        Transport_Link_PrOS_DaVinci,    //! DSP/BIOS Link for PrOS on DaVinci
        Transport_Link_Linux            //! DSP/BIOS Link for Linux
    };

    //! Define which transport layer is used (required)
    config TransportCfg transport;

    /*! Proxy device used for new devices
     *
     *  To support new devices which behave the same as existing devices,
     *  set this configuration parameter to a known device which can be
     *  used as a proxy for the new device.
     */
    config String proxyFor;
}
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:30; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

