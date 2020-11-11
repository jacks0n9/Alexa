/*
 * cslr_mcbsp.h
 *
 * This file contains the macros for Register Chip Support Library (CSL) which 
 * can be used for operations on the respective underlying hardware/peripheral
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/


/*********************************************************************
* file: cslr_mcbsp.h
*
* Brief: This file contains the Register Description for mcbsp
*
*********************************************************************/
#ifndef _CSLR_MCBSP_H_
#define _CSLR_MCBSP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/pspiom/cslr/cslr.h>
#include <ti/pspiom/cslr/tistdtypes.h>

/** SYSCLK1 19.2 MHS for AIC3204						**/
/** Using 96M_FCLK for testig							**/
#define CSL_96M_FCLK_FREQ                     (96000000u)
#define CSL_MCBSP_1_MODULE_FREQ               (CSL_96M_FCLK_FREQ)
#define CSL_MCBSP_2_MODULE_FREQ               (CSL_96M_FCLK_FREQ)
#define CSL_MCBSP_4_MODULE_FREQ               (CSL_96M_FCLK_FREQ)
#define CSL_CONTROL_DEVCONF0                  (0x48002274)

/** CONTROL_DEVCONF0 bit defintion 						**/
#define CONTROL_DEVCONF0_MCBSP2_CLKS_MASK     (0x40)
#define CONTROL_DEVCONF0_MCBSP1_FSR_MASK      (0x10)
#define CONTROL_DEVCONF0_MCBSP1_CLKR_MASK     (0x8)
#define CONTROL_DEVCONF0_MCBSP1_CLKS_MASK     (0x4)
#define CONTROL_DEVCONF0_SENSDMAREQ1_MASK     (0x2)
#define CONTROL_DEVCONF0_SENSDMAREQ0_MASK     (0x1)

/**************************************************************************\
* Register Overlay Structure
\**************************************************************************/
typedef struct  {
    volatile Uint32 DRR;
    volatile Uint32 SPARE1;
    volatile Uint32 DXR;
    volatile Uint32 SPARE2;
    volatile Uint32 SPCR2;
    volatile Uint32 SPCR1;
    volatile Uint32 RCR2;
    volatile Uint32 RCR1;
    volatile Uint32 XCR2;
    volatile Uint32 XCR1;
    volatile Uint32 SRGR2;
    volatile Uint32 SRGR1;
    volatile Uint32 MCR2;
    volatile Uint32 MCR1;
    volatile Uint32 RCERA;
    volatile Uint32 RCERB;
    volatile Uint32 XCERA;
    volatile Uint32 XCERB;
    volatile Uint32 PCR;
    volatile Uint32 RCERC;
    volatile Uint32 RCERD;
    volatile Uint32 XCERC;
    volatile Uint32 XCERD;
    volatile Uint32 RCERE;
    volatile Uint32 RCERF;
    volatile Uint32 XCERE;
    volatile Uint32 XCERF;
    volatile Uint32 RCERG;
    volatile Uint32 RCERH;
    volatile Uint32 XCERG;
    volatile Uint32 XCERH;
    volatile Uint32 REV;
    volatile Uint32 RINTCLR;
    volatile Uint32 XINTCLR;
    volatile Uint32 ROVFLCLR;
    volatile Uint32 SYSCONFIG;
    volatile Uint32 THRSH2;
    volatile Uint32 THRSH1;
    volatile Uint32 SPARE3;
    volatile Uint32 SPARE4;
    volatile Uint32 IRQSTATUS;
    volatile Uint32 IRQENABLE;
    volatile Uint32 WAKEUPEN;
    volatile Uint32 XCCR;
    volatile Uint32 RCCR;
    volatile Uint32 XBUFFSTAT;
    volatile Uint32 RBUFFSTAT;
    volatile Uint32 SSELCR;
    volatile Uint32 STATUS;
} CSL_McbspRegs;


/**************************************************************************\
* Register Overlay Structure
\**************************************************************************/
typedef struct  {
    volatile Uint32 REV;              //offset 0x0
    volatile Uint32 REV_dummy1;       //inserted to get proper offset for next reg
    volatile Uint32 REV_dummy2;       //inserted to get proper offset for next reg
    volatile Uint32 REV_dummy3;       //inserted to get proper offset for next reg
    volatile Uint32 SYSCONFIG;        //offset 0x10
    volatile Uint32 SYSCONFIG_dummy1; //inserted to get proper offset for next reg
    volatile Uint32 IRQSTATUS;        //offset 0x18
    volatile Uint32 IRQENABLE;        //offset 0x1C
    volatile Uint32 IRQENABLE_dummy1; //inserted to get proper offset for next reg
    volatile Uint32 SGAINCR;          //offset 0x24
    volatile Uint32 SFIRCR;           //offset 0x28
    volatile Uint32 SSELCR;           //offset 0x2C
} CSL_SidetoneRegs;


/**************************************************************************\
* Overlay structure typedef definition
\**************************************************************************/
typedef volatile CSL_McbspRegs          *CSL_McbspRegsOvly;
typedef volatile CSL_SidetoneRegs       *CSL_SidetoneRegsOvly;


/*************************************************************************\
 * McBSP registers reset value
\*************************************************************************/
#define CSL_MCBSP_DRR_RESETVAL          (0x00000000u)
#define CSL_MCBSP_DXR_RESETVAL          (0x00000000u)
#define CSL_MCBSP_SPCR2_RESETVAL        (0x00000000u)
#define CSL_MCBSP_SPCR1_RESETVAL        (0x00000000u)
#define CSL_MCBSP_RCR2_RESETVAL         (0x00000000u)
#define CSL_MCBSP_RCR1_RESETVAL         (0x00000000u)
#define CSL_MCBSP_XCR2_RESETVAL         (0x00000000u)
#define CSL_MCBSP_XCR1_RESETVAL         (0x00000000u)
#define CSL_MCBSP_SRGR2_RESETVAL        CSL_FMK(MCBSP_SRGR2_CLKSM,CSL_MCBSP_SRGR2_CLKSM_RESETVAL)
#define CSL_MCBSP_SRGR1_RESETVAL        CSL_FMK(MCBSP_SRGR1_CLKGDV,CSL_MCBSP_SRGR1_CLKGDV_RESETVAL)
#define CSL_MCBSP_MCR2_RESETVAL         (0x00000000u)
#define CSL_MCBSP_MCR1_RESETVAL         (0x00000000u)
#define CSL_MCBSP_RCERA_RESETVAL        (0x00000000u)
#define CSL_MCBSP_RCERB_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERA_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERB_RESETVAL        (0x00000000u)
#define CSL_MCBSP_PCR_RESETVAL          (0x00000000u)
#define CSL_MCBSP_RCERC_RESETVAL        (0x00000000u)
#define CSL_MCBSP_RCERD_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERC_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERD_RESETVAL        (0x00000000u)
#define CSL_MCBSP_RCERE_RESETVAL        (0x00000000u)
#define CSL_MCBSP_RCERF_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERE_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERF_RESETVAL        (0x00000000u)
#define CSL_MCBSP_RCERG_RESETVAL        (0x00000000u)
#define CSL_MCBSP_RCERH_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERG_RESETVAL        (0x00000000u)
#define CSL_MCBSP_XCERH_RESETVAL        (0x00000000u)   
#define CSL_MCBSP_SYSCONFIG_RESETVAL    (0x00000000u)
#define CSL_MCBSP_THRSH2_RESETVAL       (0x00000000u)
#define CSL_MCBSP_THRSH1_RESETVAL       (0x00000000u)
#define CSL_MCBSP_IRQSTATUS_RESETVAL    (0x00001FFFu)
#define CSL_MCBSP_IRQENABLE_RESETVAL    (0x00000000u)
#define CSL_MCBSP_WAKEUPEN_RESETVAL     (0x00000000u)
#define CSL_MCBSP_XCCR_RESETVAL         CSL_FMK(MCBSP_XCCR_DXENDLY,CSL_MCBSP_XCCR_DXENDLY_RESETVAL) \
                                        | CSL_FMK(MCBSP_XCCR_XDMAEN,CSL_MCBSP_XCCR_XDMAEN_RESETVAL)
#define CSL_MCBSP_RCCR_RESETVAL         CSL_FMK(MCBSP_RCCR_RDMAEN,CSL_MCBSP_RCCR_RDMAEN_RESETVAL)
#define CSL_MCBSP_SSELCR_RESETVAL      (0x00000084u)

/**************************************************************************\
* Field Definition Macros
\**************************************************************************/

/* DRR */

#define CSL_MCBSP_DRR_DRR_MASK           (0xFFFFFFFFu)
#define CSL_MCBSP_DRR_DRR_SHIFT          (0x00000000u)
#define CSL_MCBSP_DRR_DRR_RESETVAL       (0x00000000u)

/* DXR */

#define CSL_MCBSP_DXR_DXR_MASK           (0xFFFFFFFFu)
#define CSL_MCBSP_DXR_DXR_SHIFT          (0x00000000u)
#define CSL_MCBSP_DXR_DXR_RESETVAL       (0x00000000u)

/* SPCR2 */

#define CSL_MCBSP_SPCR2_FREE_MASK         (0x00000200u)
#define CSL_MCBSP_SPCR2_FREE_SHIFT        (0x00000009u)
#define CSL_MCBSP_SPCR2_FREE_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SPCR2_SOFT_MASK         (0x00000100u)
#define CSL_MCBSP_SPCR2_SOFT_SHIFT        (0x00000008u)
#define CSL_MCBSP_SPCR2_SOFT_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SPCR2_FRST_MASK         (0x00000080u)
#define CSL_MCBSP_SPCR2_FRST_SHIFT        (0x00000007u)
#define CSL_MCBSP_SPCR2_FRST_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SPCR2_GRST_MASK         (0x00000040u)
#define CSL_MCBSP_SPCR2_GRST_SHIFT        (0x00000006u)
#define CSL_MCBSP_SPCR2_GRST_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SPCR2_XINTM_MASK        (0x00000030u)
#define CSL_MCBSP_SPCR2_XINTM_SHIFT       (0x00000004u)
#define CSL_MCBSP_SPCR2_XINTM_RESETVAL    (0x00000000u)

#define CSL_MCBSP_SPCR2_XSYNCERR_MASK     (0x00000008u)
#define CSL_MCBSP_SPCR2_XSYNCERR_SHIFT    (0x00000003u)
#define CSL_MCBSP_SPCR2_XSYNCERR_RESETVAL (0x00000000u)

#define CSL_MCBSP_SPCR2_XEMPTY_MASK       (0x00000004u)
#define CSL_MCBSP_SPCR2_XEMPTY_SHIFT      (0x00000002u)
#define CSL_MCBSP_SPCR2_XEMPTY_RESETVAL   (0x00000000u)

#define CSL_MCBSP_SPCR2_XRDY_MASK         (0x00000002u)
#define CSL_MCBSP_SPCR2_XRDY_SHIFT        (0x00000001u)
#define CSL_MCBSP_SPCR2_XRDY_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SPCR2_XRST_MASK         (0x00000001u)
#define CSL_MCBSP_SPCR2_XRST_SHIFT        (0x00000000u)
#define CSL_MCBSP_SPCR2_XRST_RESETVAL     (0x00000000u)


/* SPCR1 */

#define CSL_MCBSP_SPCR1_ALB_MASK          (0x00008000u)
#define CSL_MCBSP_SPCR1_ALB_SHIFT         (0x0000000Fu)
#define CSL_MCBSP_SPCR1_ALB_RESETVAL      (0x00000000u)

#define CSL_MCBSP_SPCR1_RJUST_MASK        (0x00006000u)
#define CSL_MCBSP_SPCR1_RJUST_SHIFT       (0x0000000Du)
#define CSL_MCBSP_SPCR1_RJUST_RESETVAL    (0x00000000u)

#define CSL_MCBSP_SPCR1_DXENA_MASK        (0x00000080u)
#define CSL_MCBSP_SPCR1_DXENA_SHIFT       (0x00000007u)
#define CSL_MCBSP_SPCR1_DXENA_RESETVAL    (0x00000000u)

#define CSL_MCBSP_SPCR1_RINTM_MASK        (0x00000030u)
#define CSL_MCBSP_SPCR1_RINTM_SHIFT       (0x00000004u)
#define CSL_MCBSP_SPCR1_RINTM_RESETVAL    (0x00000000u)

#define CSL_MCBSP_SPCR1_RSYNCERR_MASK     (0x00000008u)
#define CSL_MCBSP_SPCR1_RSYNCERR_SHIFT    (0x00000003u)
#define CSL_MCBSP_SPCR1_RSYNCERR_RESETVAL (0x00000000u)

#define CSL_MCBSP_SPCR1_RFULL_MASK        (0x00000004u)
#define CSL_MCBSP_SPCR1_RFULL_SHIFT       (0x00000002u)
#define CSL_MCBSP_SPCR1_RFULL_RESETVAL    (0x00000000u)

#define CSL_MCBSP_SPCR1_RRDY_MASK         (0x00000002u)
#define CSL_MCBSP_SPCR1_RRDY_SHIFT        (0x00000001u)
#define CSL_MCBSP_SPCR1_RRDY_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SPCR1_RRST_MASK         (0x00000001u)
#define CSL_MCBSP_SPCR1_RRST_SHIFT        (0x00000000u)
#define CSL_MCBSP_SPCR1_RRST_RESETVAL     (0x00000000u)




/* RCR2 */

#define CSL_MCBSP_RCR2_RPHASE_MASK        (0x00008000u)
#define CSL_MCBSP_RCR2_RPHASE_SHIFT       (0x0000000Fu)
#define CSL_MCBSP_RCR2_RPHASE_RESETVAL    (0x00000000u)

#define CSL_MCBSP_RCR2_RFRLEN2_MASK       (0x00007F00u)
#define CSL_MCBSP_RCR2_RFRLEN2_SHIFT      (0x00000008u)
#define CSL_MCBSP_RCR2_RFRLEN2_RESETVAL   (0x00000000u)

#define CSL_MCBSP_RCR2_RWDLEN2_MASK       (0x000000E0u)
#define CSL_MCBSP_RCR2_RWDLEN2_SHIFT      (0x00000005u)
#define CSL_MCBSP_RCR2_RWDLEN2_RESETVAL   (0x00000000u)

#define CSL_MCBSP_RCR2_RREVERSE_MASK      (0x00000018u)  
#define CSL_MCBSP_RCR2_RREVERSE_SHIFT     (0x00000003u)
#define CSL_MCBSP_RCR2_RREVERSE_RESETVAL  (0x00000000u)

#define CSL_MCBSP_RCR2_RDATDLY_MASK       (0x00000003u)
#define CSL_MCBSP_RCR2_RDATDLY_SHIFT      (0x00000000u)
#define CSL_MCBSP_RCR2_RDATDLY_RESETVAL   (0x00000000u)


/* RCR1 */

#define CSL_MCBSP_RCR1_RFRLEN1_MASK       (0x00007F00u)
#define CSL_MCBSP_RCR1_RFRLEN1_SHIFT      (0x00000008u)
#define CSL_MCBSP_RCR1_RFRLEN1_RESETVAL   (0x00000000u)

#define CSL_MCBSP_RCR1_RWDLEN1_MASK       (0x000000E0u)
#define CSL_MCBSP_RCR1_RWDLEN1_SHIFT      (0x00000005u)
#define CSL_MCBSP_RCR1_RWDLEN1_RESETVAL   (0x00000000u)



/* XCR2 */

#define CSL_MCBSP_XCR2_XPHASE_MASK        (0x00008000u)
#define CSL_MCBSP_XCR2_XPHASE_SHIFT       (0x0000000Fu)
#define CSL_MCBSP_XCR2_XPHASE_RESETVAL    (0x00000000u)

#define CSL_MCBSP_XCR2_XFRLEN2_MASK       (0x00007F00u)
#define CSL_MCBSP_XCR2_XFRLEN2_SHIFT      (0x00000008u)
#define CSL_MCBSP_XCR2_XFRLEN2_RESETVAL   (0x00000000u)

#define CSL_MCBSP_XCR2_XWDLEN2_MASK       (0x000000E0u)
#define CSL_MCBSP_XCR2_XWDLEN2_SHIFT      (0x00000005u)
#define CSL_MCBSP_XCR2_XWDLEN2_RESETVAL   (0x00000000u)

#define CSL_MCBSP_XCR2_XREVERSE_MASK      (0x00000018u)   
#define CSL_MCBSP_XCR2_XREVERSE_SHIFT     (0x00000003u)
#define CSL_MCBSP_XCR2_XREVERSE_RESETVAL  (0x00000000u)

#define CSL_MCBSP_XCR2_XDATDLY_MASK       (0x00000003u)
#define CSL_MCBSP_XCR2_XDATDLY_SHIFT      (0x00000000u)
#define CSL_MCBSP_XCR2_XDATDLY_RESETVAL   (0x00000000u)



/* XCR1 */

#define CSL_MCBSP_XCR1_XFRLEN1_MASK       (0x00007F00u)
#define CSL_MCBSP_XCR1_XFRLEN1_SHIFT      (0x00000008u)
#define CSL_MCBSP_XCR1_XFRLEN1_RESETVAL   (0x00000000u)

#define CSL_MCBSP_XCR1_XWDLEN1_MASK       (0x000000E0u)
#define CSL_MCBSP_XCR1_XWDLEN1_SHIFT      (0x00000005u)
#define CSL_MCBSP_XCR1_XWDLEN1_RESETVAL   (0x00000000u)



/* SRGR2 */

#define CSL_MCBSP_SRGR2_GSYNC_MASK        (0x00008000u)
#define CSL_MCBSP_SRGR2_GSYNC_SHIFT       (0x0000000Fu)
#define CSL_MCBSP_SRGR2_GSYNC_RESETVAL    (0x00000000u)

#define CSL_MCBSP_SRGR2_CLKSP_MASK        (0x00004000u)
#define CSL_MCBSP_SRGR2_CLKSP_SHIFT       (0x0000000Eu)
#define CSL_MCBSP_SRGR2_CLKSP_RESETVAL    (0x00000000u)

#define CSL_MCBSP_SRGR2_CLKSM_MASK        (0x00002000u)
#define CSL_MCBSP_SRGR2_CLKSM_SHIFT       (0x0000000Du)
#define CSL_MCBSP_SRGR2_CLKSM_RESETVAL    (0x00000001u)

#define CSL_MCBSP_SRGR2_FSGM_MASK         (0x00001000u)
#define CSL_MCBSP_SRGR2_FSGM_SHIFT        (0x0000000Cu)
#define CSL_MCBSP_SRGR2_FSGM_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SRGR2_FPER_MASK         (0x00000FFFu)
#define CSL_MCBSP_SRGR2_FPER_SHIFT        (0x00000000u)
#define CSL_MCBSP_SRGR2_FPER_RESETVAL     (0x00000000u)



/* SRGR1 */

#define CSL_MCBSP_SRGR1_FWID_MASK         (0x0000FF00u)
#define CSL_MCBSP_SRGR1_FWID_SHIFT        (0x00000008u)
#define CSL_MCBSP_SRGR1_FWID_RESETVAL     (0x00000000u)

#define CSL_MCBSP_SRGR1_CLKGDV_MASK       (0x000000FFu)
#define CSL_MCBSP_SRGR1_CLKGDV_SHIFT      (0x00000000u)
#define CSL_MCBSP_SRGR1_CLKGDV_RESETVAL   (0x00000001u)




/* MCR2 */

#define CSL_MCBSP_MCR2_XMCME_MASK         (0x00000200u)
#define CSL_MCBSP_MCR2_XMCME_SHIFT        (0x00000009u)
#define CSL_MCBSP_MCR2_XMCME_RESETVAL     (0x00000000u)

#define CSL_MCBSP_MCR2_XPBBLK_MASK        (0x00000180u)
#define CSL_MCBSP_MCR2_XPBBLK_SHIFT       (0x00000007u)
#define CSL_MCBSP_MCR2_XPBBLK_RESETVAL    (0x00000000u)

#define CSL_MCBSP_MCR2_XPABLK_MASK        (0x00000060u)
#define CSL_MCBSP_MCR2_XPABLK_SHIFT       (0x00000005u)
#define CSL_MCBSP_MCR2_XPABLK_RESETVAL    (0x00000000u)

#define CSL_MCBSP_MCR2_XMCM_MASK          (0x00000003u)
#define CSL_MCBSP_MCR2_XMCM_SHIFT         (0x00000000u)
#define CSL_MCBSP_MCR2_XMCM_RESETVAL      (0x00000000u)


/* MCR1 */

#define CSL_MCBSP_MCR1_RMCME_MASK         (0x00000200u)
#define CSL_MCBSP_MCR1_RMCME_SHIFT        (0x00000009u)
#define CSL_MCBSP_MCR1_RMCME_RESETVAL     (0x00000000u)

#define CSL_MCBSP_MCR1_RPBBLK_MASK        (0x00000180u)
#define CSL_MCBSP_MCR1_RPBBLK_SHIFT       (0x00000007u)
#define CSL_MCBSP_MCR1_RPBBLK_RESETVAL    (0x00000000u)

#define CSL_MCBSP_MCR1_RPABLK_MASK        (0x00000060u)
#define CSL_MCBSP_MCR1_RPABLK_SHIFT       (0x00000005u)
#define CSL_MCBSP_MCR1_RPABLK_RESETVAL    (0x00000000u)

#define CSL_MCBSP_MCR1_RMCM_MASK          (0x00000001u)
#define CSL_MCBSP_MCR1_RMCM_SHIFT         (0x00000000u)
#define CSL_MCBSP_MCR1_RMCM_RESETVAL      (0x00000000u)



/* PCR */

#define CSL_MCBSP_PCR_IDLEN_MASK         (0x00004000u)
#define CSL_MCBSP_PCR_IDLEN_SHIFT        (0x0000000Eu)
#define CSL_MCBSP_PCR_IDLEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_PCR_XIOEN_MASK         (0x00002000u)
#define CSL_MCBSP_PCR_XIOEN_SHIFT        (0x0000000Du)
#define CSL_MCBSP_PCR_XIOEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_PCR_RIOEN_MASK         (0x00001000u)
#define CSL_MCBSP_PCR_RIOEN_SHIFT        (0x0000000Cu)
#define CSL_MCBSP_PCR_RIOEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_PCR_FSXM_MASK          (0x00000800u)
#define CSL_MCBSP_PCR_FSXM_SHIFT         (0x0000000Bu)
#define CSL_MCBSP_PCR_FSXM_RESETVAL      (0x00000000u)

#define CSL_MCBSP_PCR_FSRM_MASK          (0x00000400u)
#define CSL_MCBSP_PCR_FSRM_SHIFT         (0x0000000Au)
#define CSL_MCBSP_PCR_FSRM_RESETVAL      (0x00000000u)

#define CSL_MCBSP_PCR_CLKXM_MASK         (0x00000200u)
#define CSL_MCBSP_PCR_CLKXM_SHIFT        (0x00000009u)
#define CSL_MCBSP_PCR_CLKXM_RESETVAL     (0x00000000u)

#define CSL_MCBSP_PCR_CLKRM_MASK         (0x00000100u)
#define CSL_MCBSP_PCR_CLKRM_SHIFT        (0x00000008u)
#define CSL_MCBSP_PCR_CLKRM_RESETVAL     (0x00000000u)

#define CSL_MCBSP_PCR_SCLKME_MASK        (0x00000080u)
#define CSL_MCBSP_PCR_SCLKME_SHIFT       (0x00000007u)
#define CSL_MCBSP_PCR_SCLKME_RESETVAL    (0x00000000u)

#define CSL_MCBSP_PCR_CLKSSTAT_MASK      (0x00000040u)
#define CSL_MCBSP_PCR_CLKSSTAT_SHIFT     (0x00000006u)
#define CSL_MCBSP_PCR_CLKSSTAT_RESETVAL  (0x00000000u)

#define CSL_MCBSP_PCR_DXSTAT_MASK        (0x00000020u)
#define CSL_MCBSP_PCR_DXSTAT_SHIFT       (0x00000005u)
#define CSL_MCBSP_PCR_DXSTAT_RESETVAL    (0x00000000u)

#define CSL_MCBSP_PCR_DRSTAT_MASK        (0x00000010u)
#define CSL_MCBSP_PCR_DRSTAT_SHIFT       (0x00000004u)
#define CSL_MCBSP_PCR_DRSTAT_RESETVAL    (0x00000000u)

#define CSL_MCBSP_PCR_FSXP_MASK          (0x00000008u)
#define CSL_MCBSP_PCR_FSXP_SHIFT         (0x00000003u)
#define CSL_MCBSP_PCR_FSXP_RESETVAL      (0x00000000u)

#define CSL_MCBSP_PCR_FSRP_MASK          (0x00000004u)
#define CSL_MCBSP_PCR_FSRP_SHIFT         (0x00000002u)
#define CSL_MCBSP_PCR_FSRP_RESETVAL      (0x00000000u)

#define CSL_MCBSP_PCR_CLKXP_MASK         (0x00000002u)
#define CSL_MCBSP_PCR_CLKXP_SHIFT        (0x00000001u)
#define CSL_MCBSP_PCR_CLKXP_RESETVAL     (0x00000000u)

#define CSL_MCBSP_PCR_CLKRP_MASK         (0x00000001u)
#define CSL_MCBSP_PCR_CLKRP_SHIFT        (0x00000000u)
#define CSL_MCBSP_PCR_CLKRP_RESETVAL     (0x00000000u)


/*SYSCONFIG*/
#define CSL_MCBSP_SYSCONFIG_CLKACT_MASK          (0x00000300u)
#define CSL_MCBSP_SYSCONFIG_CLKACT_SHIFT         (0x00000008u)
#define CSL_MCBSP_SYSCONFIG_CLKACT_RESETVAL      (0x00000000u)

#define CSL_MCBSP_SYSCONFIG_SIDLEMODE_MASK       (0x00000018u)
#define CSL_MCBSP_SYSCONFIG_SIDLEMODE_SHIFT      (0x00000003u)
#define CSL_MCBSP_SYSCONFIG_SIDLEMODE_RESETVAL   (0x00000000u)

#define CSL_MCBSP_SYSCONFIG_ENAWAKEUP_MASK       (0x00000004u)
#define CSL_MCBSP_SYSCONFIG_ENAWAKEUP_SHIFT      (0x00000002u)
#define CSL_MCBSP_SYSCONFIG_ENAWAKEUP_RESETVAL   (0x00000000u)

#define CSL_MCBSP_SYSCONFIG_SOFTRESET_MASK       (0x00000002u)
#define CSL_MCBSP_SYSCONFIG_SOFTRESET_SHIFT      (0x00000001u)
#define CSL_MCBSP_SYSCONFIG_SOFTRESET_RESETVAL   (0x00000000u)


/*THRSH2*/
#define CSL_MCBSP_THRSH2_XTHRES_MASK       (0x0000007Fu)
#define CSL_MCBSP_THRSH2_XTHRES_SHIFT      (0x00000000u)
#define CSL_MCBSP_THRSH2_XTHRES_RESETVAL   (0x00000000u)


/*THRSH1*/
#define CSL_MCBSP_THRSH1_RTHRES_MASK       (0x0000007Fu)
#define CSL_MCBSP_THRSH1_RTHRES_SHIFT      (0x00000000u)
#define CSL_MCBSP_THRSH1_RTHRES_RESETVAL   (0x00000000u)


/*IRQSTATUS*/
//TODO if needed

/*IRQENABLE*/
//TODO if needed

/*WAKEUPEN*/
#define CSL_MCBSP_WAKEUPEN_XRDYEN_MASK         (0x00000400u)
#define CSL_MCBSP_WAKEUPEN_XRDYEN_SHIFT        (0x0000000au)
#define CSL_MCBSP_WAKEUPEN_XRDYEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_WAKEUPEN_XEOFEN_MASK         (0x00000200u)
#define CSL_MCBSP_WAKEUPEN_XEOFEN_SHIFT        (0x00000009u)
#define CSL_MCBSP_WAKEUPEN_XEOFEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_WAKEUPEN_XFSXEN_MASK         (0x00000100u)
#define CSL_MCBSP_WAKEUPEN_XFSXEN_SHIFT        (0x00000008u)
#define CSL_MCBSP_WAKEUPEN_XFSXEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_WAKEUPEN_XSYNCERREN_MASK     (0x00000080u)
#define CSL_MCBSP_WAKEUPEN_XSYNCERREN_SHIFT    (0x00000007u)
#define CSL_MCBSP_WAKEUPEN_XSYNCERREN_RESETVAL (0x00000000u)

#define CSL_MCBSP_WAKEUPEN_RRDYEN_MASK         (0x00000008u)
#define CSL_MCBSP_WAKEUPEN_RRDYEN_SHIFT        (0x00000003u)
#define CSL_MCBSP_WAKEUPEN_RRDYEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_WAKEUPEN_REOFEN_MASK         (0x00000004u)
#define CSL_MCBSP_WAKEUPEN_REOFEN_SHIFT        (0x00000002u)
#define CSL_MCBSP_WAKEUPEN_REOFEN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_WAKEUPEN_RFSREN_MASK         (0x00000002u)
#define CSL_MCBSP_WAKEUPEN_RFSREN_SHIFT        (0x00000001u)
#define CSL_MCBSP_WAKEUPEN_RFSREN_RESETVAL     (0x00000000u)

#define CSL_MCBSP_WAKEUPEN_RSYNCERREN_MASK     (0x00000001u)
#define CSL_MCBSP_WAKEUPEN_RSYNCERREN_SHIFT    (0x00000000u)
#define CSL_MCBSP_WAKEUPEN_RSYNCERREN_RESETVAL (0x00000000u)

/*XCCR*/
#define CSL_MCBSP_XCCR_PPCONNECT_MASK       (0x00004000u)
#define CSL_MCBSP_XCCR_PPCONNECT_SHIFT      (0x0000000Eu)
#define CSL_MCBSP_XCCR_PPCONNECT_RESETVAL   (0x00000000u)

#define CSL_MCBSP_XCCR_DXENDLY_MASK         (0x00003000u)
#define CSL_MCBSP_XCCR_DXENDLY_SHIFT        (0x0000000Cu)
#define CSL_MCBSP_XCCR_DXENDLY_RESETVAL     (0x00000001u)

#define CSL_MCBSP_XCCR_DLB_MASK             (0x00000020u)
#define CSL_MCBSP_XCCR_DLB_SHIFT            (0x00000005u)
#define CSL_MCBSP_XCCR_DLB_RESETVAL         (0x00000000u)

#define CSL_MCBSP_XCCR_XDMAEN_MASK          (0x00000008u)
#define CSL_MCBSP_XCCR_XDMAEN_SHIFT         (0x00000003u)
#define CSL_MCBSP_XCCR_XDMAEN_RESETVAL      (0x00000001u)

#define CSL_MCBSP_XCCR_XDISABLE_MASK        (0x00000001u)
#define CSL_MCBSP_XCCR_XDISABLE_SHIFT       (0x00000000u)
#define CSL_MCBSP_XCCR_XDISABLE_RESETVAL    (0x00000000u)


/*RCCR*/
#define CSL_MCBSP_RCCR_RDMAEN_MASK          (0x00000008u)
#define CSL_MCBSP_RCCR_RDMAEN_SHIFT         (0x00000003u)
#define CSL_MCBSP_RCCR_RDMAEN_RESETVAL      (0x00000001u)

#define CSL_MCBSP_RCCR_RDISABLE_MASK        (0x00000001u)
#define CSL_MCBSP_RCCR_RDISABLE_SHIFT       (0x00000000u)
#define CSL_MCBSP_RCCR_RDISABLE_RESETVAL    (0x00000000u)

/* SSELCR */
#define CSL_MCBSP_SSELCR_SIDETONEEN_MASK     (0x00000400u)
#define CSL_MCBSP_SSELCR_SIDETONEEN_SHIFT    (0x0000000Au)
#define CSL_MCBSP_SSELCR_SIDETONEEN_RESETVAL (0x00000000u)

#define CSL_MCBSP_SSELCR_ICH0ASSIGN_MASK     (0x00000003u)
#define CSL_MCBSP_SSELCR_ICH0ASSIGN_SHIFT    (0x00000000u)
#define CSL_MCBSP_SSELCR_ICH0ASSIGN_RESETVAL (0x00000000u)

#define CSL_MCBSP_SSELCR_ICH1ASSIGN_MASK     (0x0000000Cu)
#define CSL_MCBSP_SSELCR_ICH1ASSIGN_SHIFT    (0x00000002u)
#define CSL_MCBSP_SSELCR_ICH1ASSIGN_RESETVAL (0x00000001u)

#define CSL_MCBSP_SSELCR_OCH0ASSIGN_MASK     (0x00000070u)
#define CSL_MCBSP_SSELCR_OCH0ASSIGN_SHIFT    (0x00000004u)
#define CSL_MCBSP_SSELCR_OCH0ASSIGN_RESETVAL (0x00000000u)

#define CSL_MCBSP_SSELCR_OCH1ASSIGN_MASK     (0x00000380u)
#define CSL_MCBSP_SSELCR_OCH1ASSIGN_SHIFT    (0x00000007u)
#define CSL_MCBSP_SSELCR_OCH1ASSIGN_RESETVAL (0x00000001u)

/*XBUFFSTAT*/
//Read-only Register

/*RBUFFSTAT*/
//Read-only Register

/* SIDETONE */
#define CSL_ST_SYSCONFIG_AUTOIDLE_MASK       (0x00000001u)
#define CSL_ST_SYSCONFIG_AUTOIDLE_SHIFT      (0x00000000u)
#define CSL_ST_SYSCONFIG_AUTOIDLE_RESETVAL   (0x00000001u)

#define CSL_ST_SGAINCR_CH0GAIN_MASK          (0x0000FFFFu)
#define CSL_ST_SGAINCR_CH0GAIN_SHIFT         (0x00000000u)
#define CSL_ST_SGAINCR_CH0GAIN_RESETVAL      (0x00000000u)

#define CSL_ST_SGAINCR_CH1GAIN_MASK          (0xFFFF0000u)
#define CSL_ST_SGAINCR_CH1GAIN_SHIFT         (0x00000010u)
#define CSL_ST_SGAINCR_CH1GAIN_RESETVAL      (0x00000000u)

#define CSL_ST_SFIRCR_FIRCOEFF_MASK          (0x0000FFFFu)
#define CSL_ST_SFIRCR_FIRCOEFF_SHIFT         (0x00000000u)
#define CSL_ST_SFIRCR_FIRCOEFF_RESETVAL      (0x00000000u)

#define CSL_ST_SSELCR_SIDETONEEN_MASK        (0x00000001u)
#define CSL_ST_SSELCR_SIDETONEEN_SHIFT       (0x00000000u)
#define CSL_ST_SSELCR_SIDETONEEN_RESETVAL    (0x00000000u)

#define CSL_ST_SSELCR_COEFFWREN_MASK         (0x00000002u)
#define CSL_ST_SSELCR_COEFFWREN_SHIFT        (0x00000001u)
#define CSL_ST_SSELCR_COEFFWREN_RESETVAL     (0x00000000u)

#define CSL_ST_SSELCR_COEFFWRDONE_MASK       (0x00000004u)
#define CSL_ST_SSELCR_COEFFWRDONE_SHIFT      (0x00000002u)
#define CSL_ST_SSELCR_COEFFWRDONE_RESETVAL   (0x00000000u)


#ifdef __cplusplus
}
#endif

#endif

