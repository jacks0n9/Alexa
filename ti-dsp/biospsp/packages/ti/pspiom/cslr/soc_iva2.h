/*
 * iva2.h
 *
 * This file contains the peripheral registers base addresseses, peripheral
 * counts, peripheral inettupr details etc for the respective SoC
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

#ifndef _IVA2_H_
#define _IVA2_H_

/* Minimum definition of DSP for DM37390 */
#ifdef __cplusplus
extern "C" {
#endif

#include <ti/pspiom/cslr/cslr.h>

/** Definition for peripheral frequencies				**/

/** MCBSP IVA2 Virtual address (depend on the MMU settings) */
#define CSL_MCBSP_PER_CNT                       5
#define CSL_MCBSP_1_REGS			(0x48074000)
#define CSL_MCBSP_2_REGS			(0x49022000)
#define CSL_MCBSP_3_REGS			(0x49024000)
#define CSL_MCBSP_4_REGS			(0x49026000)
#define CSL_MCBSP_5_REGS			(0x48096000)

/** Sidetone IVA2 virtual address */
#define CSL_MCBSP_2_ST_REGS			(0x49028000)
#define CSL_MCBSP_3_ST_REGS			(0x4902A000)

/** Base address of Channel controller  memory mapped registers */
#define CSL_EDMACC_REGS				(0x01C00000)
/** Base address of Transfer controller  memory mapped registers */
#define CSL_EDMATC_0_REGS			(0x01C10000) 
#define CSL_EDMATC_1_REGS			(0x01C10400) 

/** Base address of GP Timer registers */
#define CSL_GPT5_REGS				(0x11800000)
#define CSL_GPT6_REGS				(0x11801000)
#define CSL_GPT7_REGS				(0x11802000)
#define CSL_GPT8_REGS				(0x11803000)

/*************************** EDMA RELATED DEFINES  ****************************/

/******************************************************************************\
*  Parameterizable Configuration:- These are fed directly from the RTL
*  parameters for the given SOC
\******************************************************************************/

#define CSL_EDMA3_NUM_DMACH                 64
#define CSL_EDMA3_NUM_QDMACH                4
#define CSL_EDMA3_NUM_PARAMSETS             128
#define CSL_EDMA3_NUM_EVQUE                 2
#define CSL_EDMA3_NUM_REGIONS               8

/******************************************************************************\
* Channel Instance count
\******************************************************************************/
#define CSL_EDMA3_CHA_CNT                   (CSL_EDMA3_NUM_DMACH + \
                                             CSL_EDMA3_NUM_QDMACH)

/* EDMA channel synchronization events                                        */

/* McBSP1 TRANSMIT Event                                                      */
#define CSL_EDMA3_CHA_MCBSP1_TX             0
/* McBSP1 RECEIVE Event                                                       */
#define CSL_EDMA3_CHA_MCBSP1_RX             1

/* McBSP2 TRANSMIT Event                                                      */
#define CSL_EDMA3_CHA_MCBSP2_TX             2
/* McBSP2 RECEIVE Event                                                       */
#define CSL_EDMA3_CHA_MCBSP2_RX             3

/* McBSP4 TRANSMIT Event                                                      */
#define CSL_EDMA3_CHA_MCBSP4_TX             6
/* McBSP4 RECEIVE Event                                                       */
#define CSL_EDMA3_CHA_MCBSP4_RX             7

/* I2C0 Receive Event                                                         */
#define CSL_EDMA3_CHA_I2C0_RX               24
/* I2C0 Transmit Event                                                        */
#define CSL_EDMA3_CHA_I2C0_TX               25
/* I2C1 Receive Event                                                         */
#define CSL_EDMA3_CHA_I2C1_RX               26
/* I2C1 Transmit Event                                                        */
#define CSL_EDMA3_CHA_I2C1_TX               27

/* QDMA channels                                                              */
#define    CSL_EDMA3_QCHA_BASE              CSL_EDMA3_NUM_DMACH         /* QDMA Channel Base */
#define    CSL_EDMA3_QCHA_0                 (CSL_EDMA3_QCHA_BASE + 0)   /* QDMA Channel 0 */
#define    CSL_EDMA3_QCHA_1                 (CSL_EDMA3_QCHA_BASE + 1)   /* QDMA Channel 1 */
#define    CSL_EDMA3_QCHA_2                 (CSL_EDMA3_QCHA_BASE + 2)   /* QDMA Channel 2 */
#define    CSL_EDMA3_QCHA_3                 (CSL_EDMA3_QCHA_BASE + 3)   /* QDMA Channel 3 */


/* Enumerations for EDMA Controlleres */
typedef enum {
  CSL_EDMACC_ANY               =           -1,/* Any instance of EDMACC module*/
  CSL_EDMACC_0                 =            0 /* EDMACC Instance 0            */
} CSL_EdmaccNum;


/* Enumerations for EDMA Event Queues */
typedef enum {
    CSL_EDMA3_QUE_0            =            0, /* Queue 0                     */
    CSL_EDMA3_QUE_1            =            1  /* Queue 1                     */
} CSL_Edma3Que;

/* Enumerations for EDMA Transfer Controllers
 *
 * There are 2 Transfer Controllers. Typically a one to one mapping exists
 * between Event Queues and Transfer Controllers.
 *
 */
typedef enum {
  CSL_EDMATC_ANY               = -1, /* Any instance of EDMATC                */
  CSL_EDMATC_0                 =  0, /* EDMATC Instance 0                     */
  CSL_EDMATC_1                 =  1  /* EDMATC Instance 1                     */
} CSL_EdmatcNum;


#define CSL_EDMA3_REGION_GLOBAL            (-1)
#define CSL_EDMA3_REGION_0                  0
#define CSL_EDMA3_REGION_1                  1
#define CSL_EDMA3_REGION_2                  2
#define CSL_EDMA3_REGION_3                  3
#define CSL_EDMA3_REGION_4                  4
#define CSL_EDMA3_REGION_5                  5
#define CSL_EDMA3_REGION_6                  6
#define CSL_EDMA3_REGION_7                  7

/******************************************************************************
 * IVA2 IRQ[0-47]
 ******************************************************************************/
#define CSL_MCBSP1_IRQ_TX_IVA2_IRQ		(16)
#define CSL_MCBSP1_IRQ_RX_IVA2_IRQ		(17)

#define CSL_MCBSP2_IRQ_TX_IVA2_IRQ		(19)
#define CSL_MCBSP2_IRQ_RX_IVA2_IRQ		(20)

#define CSL_MCBSP4_IRQ_TX_IVA2_IRQ		(23)
#define CSL_MCBSP4_IRQ_RX_IVA2_IRQ		(24)

#define CSL_MCBSP1_IRQ_IVA2_IRQ			(33)

#define CSL_MCBSP2_IRQ_IVA2_IRQ			(34)

#define CSL_MCBSP4_IRQ_IVA2_IRQ			(36)

/******************************************************************************\
* Interrupt Event IDs
\******************************************************************************/

/*
 * @brief   Interrupt Event IDs
 */
/* Output of event combiner 0, for events 1 to 31                             */
#define    CSL_INTC_EVENTID_EVT0                (0)
/* Output of event combiner 0, for events 32 to 63                            */
#define    CSL_INTC_EVENTID_EVT1                (1)
/* Output of event combiner 0, for events 64 to 95                            */
#define    CSL_INTC_EVENTID_EVT2                (2)
/* Output of event combiner 0, for events 96 to 127                           */
#define    CSL_INTC_EVENTID_EVT3                (3)

/* EMU interrupt                                                              */
#define    CSL_INTC_EVENTID_EMU_DTDMA           (9)

/* EMU real time data exchange receive complete                               */
#define    CSL_INTC_EVENTID_EMU_RTDXRX          (11)
/* EMU RTDX transmit complete                                                 */
#define    CSL_INTC_EVENTID_EMU_RTDXTX          (12)

/* IDMA Channel 0 Interrupt                                                   */
#define    CSL_INTC_EVENTID_IDMA0               (13)
/* IDMA Channel 1 Interrupt                                                   */
#define    CSL_INTC_EVENTID_IDMA1               (14)

#define     CSL_INTC_EVENTID_CCMPINT          	(28)
#define     CSL_INTC_EVENTID_CCINTG				(29)
#define     CSL_INTC_EVENTID_CCINT3				(30)
#define     CSL_INTC_EVENTID_CCINT4				(31)
#define     CSL_INTC_EVENTID_CCINT5				(32)
#define     CSL_INTC_EVENTID_CCINT6				(33)
#define     CSL_INTC_EVENTID_CCINT7				(34)
#define     CSL_INTC_EVENTID_CCINT8				(35)
#define     CSL_INTC_EVENTID_CCINT1				(36)
#define     CSL_INTC_EVENTID_CCINT2				(37)
#define     CSL_INTC_EVENTID_EDMA3_0_CC0_ERRINT	(38)
#define     CSL_INTC_EVENTID_EDMA3_0_TC0_ERRINT	(39)
#define     CSL_INTC_EVENTID_EDMA3_0_TC1_ERRINT	(40)


#define     CSL_INTC_EVENTID_GPT5_IRQ           (51)
#define     CSL_INTC_EVENTID_GPT6_IRQ           (52)
#define     CSL_INTC_EVENTID_GPT7_IRQ           (53)
#define     CSL_INTC_EVENTID_GPT8_IRQ           (54)

#define     CSL_INTC_EVENTID_MAIL_U1_IVA2_IRQ   (55)

#define    CSL_INTC_EVENTID_CAM_IRQ1			(56)

#define    CSL_INTC_EVENTID_PRCM_IVA_IRQ		(57)
#define    CSL_INTC_EVENTID_DSS_IRQ				(58)

#define    CSL_INTC_EVENTID_UART3_IRQ			(60)

#define    CSL_INTC_EVENTID_MCBSP1_IRQ_TX		(61)
#define    CSL_INTC_EVENTID_MCBSP1_IRQ_RX		(62)

#define    CSL_INTC_EVENTID_MCBSP2_IRQ_TX		(64)
#define    CSL_INTC_EVENTID_MCBSP2_IRQ_RX		(65)
#define    CSL_INTC_EVENTID_MCBSP3_IRQ_TX		(66)
#define    CSL_INTC_EVENTID_MCBSP3_IRQ_RX		(67)
#define    CSL_INTC_EVENTID_MCBSP4_IRQ_TX		(68)
#define    CSL_INTC_EVENTID_MCBSP4_IRQ_RX		(69)
#define    CSL_INTC_EVENTID_MCBSP5_IRQ_TX		(70)
#define    CSL_INTC_EVENTID_MCBSP5_IRQ_RX		(71)

#define    CSL_INTC_EVENTID_GPIO1_IVA2_IRQ		(73)
#define    CSL_INTC_EVENTID_GPIO2_IVA2_IRQ		(74)
#define    CSL_INTC_EVENTID_GPIO3_IVA2_IRQ		(75)
#define    CSL_INTC_EVENTID_GPIO4_IVA2_IRQ		(76)
#define    CSL_INTC_EVENTID_GPIO5_IVA2_IRQ		(77)

#define    CSL_INTC_EVENTID_MCBSP1_IRQ			(78)
#define    CSL_INTC_EVENTID_MCBSP2_IRQ			(79)
#define    CSL_INTC_EVENTID_MCBSP3_IRQ			(80)
#define    CSL_INTC_EVENTID_MCBSP4_IRQ			(81)
#define    CSL_INTC_EVENTID_MCBSP5_IRQ			(82)

/* SYS CMPA CPU memory protection fault                                       */
#define    CSL_INTC_EVENTID_SYS_CMPA            (119)
/* PMC CPU memory protection fault                                            */
#define    CSL_INTC_EVENTID_PMC_CMPA            (120)
/* PMC DMA memory protection fault                                            */
#define    CSL_INTC_EVENTID_PMC_DMPA            (121)
/* DMC CPU memory protection fault                                            */
#define    CSL_INTC_EVENTID_DMC_CMPA            (122)
/* DMC DMA memory protection fault                                            */
#define    CSL_INTC_EVENTID_DMC_DMPA            (123)
/* UMC CPU memory protection fault                                            */
#define    CSL_INTC_EVENTID_UMC_CMPA            (124)
/* UMC DMA memory protection fault                                            */
#define    CSL_INTC_EVENTID_UMC_DMPA            (125)
/* IDMA CPU memory protection fault                                           */
#define    CSL_INTC_EVENTID_EMC_CMPA            (126)
/* IDMA Bus error interrupt                                                   */
#define    CSL_INTC_EVENTID_EMC_BUSERR          (127)

#define IVA_WUGEN_MEVTCLR0						(0x01C21070)
#define IVA_WUGEN_MEVTCLR1						(0x01C21074)
#define IVA_WUGEN_MEVTCLR2						(0x01C21078)

#ifdef __cplusplus
}
#endif

#endif  /* _SOC_IVA2_H_ */
