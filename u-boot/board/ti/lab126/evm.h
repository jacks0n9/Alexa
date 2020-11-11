/*
 * (C) Copyright 2008
 * Nishanth Menon <menon.nishanth@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _EVM_H_
#define _EVM_H_

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"OMAP3 LAB126 board",
#if defined(CONFIG_ENV_IS_IN_ONENAND)
	"OneNAND",
#else
	"NAND",
#endif
};

/*
 * OMAP35x EVM revision
 * Run time detection of EVM revision is done by reading Ethernet
 * PHY ID -
 *      GEN_1   = 0x01150000
 *      GEN_2   = 0x92200000
 */
enum {
	OMAP3EVM_BOARD_GEN_1 = 0,	/* EVM Rev between  A - D */
	OMAP3EVM_BOARD_GEN_2,		/* EVM Rev >= Rev E */
};

u8 get_omap3_board_rev(void);

static void setup_net_chip(void);

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */

#define MUX_BOARD() \
 /*SDRC*/\
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0)) /*SDRC_D0*/\
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0)) /*SDRC_D1*/\
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0)) /*SDRC_D2*/\
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0)) /*SDRC_D3*/\
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0)) /*SDRC_D4*/\
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0)) /*SDRC_D5*/\
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0)) /*SDRC_D6*/\
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0)) /*SDRC_D7*/\
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0)) /*SDRC_D8*/\
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0)) /*SDRC_D9*/\
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0)) /*SDRC_D10*/\
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0)) /*SDRC_D11*/\
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0)) /*SDRC_D12*/\
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0)) /*SDRC_D13*/\
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0)) /*SDRC_D14*/\
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0)) /*SDRC_D15*/\
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0)) /*SDRC_D16*/\
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0)) /*SDRC_D17*/\
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0)) /*SDRC_D18*/\
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0)) /*SDRC_D19*/\
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0)) /*SDRC_D20*/\
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0)) /*SDRC_D21*/\
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0)) /*SDRC_D22*/\
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0)) /*SDRC_D23*/\
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0)) /*SDRC_D24*/\
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0)) /*SDRC_D25*/\
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0)) /*SDRC_D26*/\
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0)) /*SDRC_D27*/\
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0)) /*SDRC_D28*/\
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0)) /*SDRC_D29*/\
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0)) /*SDRC_D30*/\
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0)) /*SDRC_D31*/\
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0)) /*SDRC_CLK*/\
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0)) /*SDRC_DQS0*/\
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0)) /*SDRC_DQS1*/\
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0)) /*SDRC_DQS2*/\
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0)) /*SDRC_DQS3*/\
 /*GPMC*/\
	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0)) /*SYS_32K*/\
	MUX_VAL(CP(SYS_CLKREQ),		(IEN  | PTD | DIS | M0)) /*SYS_CLKREQ*/\
	MUX_VAL(CP(SYS_NIRQ),		(IEN  | PTU | EN  | M0)) /*SYS_nIRQ*/\
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTD | DIS | M4)) /*GPIO_2*/\
								 /* - PEN_IRQ */\
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTD | DIS | M4)) /*GPIO_3 */\
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTD | DIS | M4)) /*GPIO_4*/\
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTD | DIS | M4)) /*GPIO_5*/\
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTD | DIS | M4)) /*GPIO_6*/\
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTD | DIS | M4)) /*GPIO_7*/\
	MUX_VAL(CP(SYS_BOOT6),		(IDIS | PTD | DIS | M4)) /*GPIO_8*/\
								 /* - VIO_1V8*/\
	MUX_VAL(CP(SYS_OFF_MODE),	(IEN  | PTD | DIS | M0)) /*SYS_OFF_MODE*/\
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTD | DIS | M0)) /*SYS_CLKOUT1*/\
	MUX_VAL(CP(SYS_CLKOUT2),	(IEN  | PTU | EN  | M0)) /*SYS_CLKOUT2*/\
	MUX_VAL(CP(JTAG_nTRST),		(IEN  | PTD | DIS | M0)) /*JTAG_nTRST*/\
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0)) /*JTAG_TCK*/\
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTD | DIS | M0)) /*JTAG_TMS*/\
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTD | DIS | M0)) /*JTAG_TDI*/\
	MUX_VAL(CP(JTAG_EMU0),		(IEN  | PTD | DIS | M0)) /*JTAG_EMU0*/\
	MUX_VAL(CP(JTAG_EMU1),		(IEN  | PTD | DIS | M0)) /*JTAG_EMU1*/\
	MUX_VAL(CP(ETK_CLK_ES2),	(IDIS | PTU | EN  | M0)) /*ETK_CLK*/\
	MUX_VAL(CP(ETK_CTL_ES2),	(IDIS | PTD | DIS | M4)) /*GPIO 13 - Atheros reset pin*/\
	MUX_VAL(CP(ETK_D2_ES2 ),	(IEN  | PTU | EN  | M4)) /*ETK_D0-GPIO16 ETH_RESETN */\
	MUX_VAL(CP(ETK_D1_ES2 ),	(IEN  | PTU | DIS | M4)) /*ETK_D1-GPIO15 ETH_INTR */\
	MUX_VAL(CP(ETK_D0_ES2 ),	(IEN  | PTD | DIS | M4)) /*ETK_D2-GPIO14 ETH_PME*/\
	MUX_VAL(CP(ETK_D3_ES2 ),	(IEN  | PTD | DIS | M0)) /*ETK_D3*/\
	MUX_VAL(CP(ETK_D4_ES2 ),	(IEN  | PTD | DIS | M0)) /*ETK_D4*/\
	MUX_VAL(CP(ETK_D5_ES2 ),	(IEN  | PTD | DIS | M0)) /*ETK_D5*/\
	MUX_VAL(CP(ETK_D6_ES2 ),	(IEN  | PTD | DIS | M0)) /*ETK_D6*/\
	MUX_VAL(CP(ETK_D7_ES2 ),	(IEN  | PTU | EN | M4)) /*GPIO 21  Bluetooth Dis/enable -- HIGH*/\
	MUX_VAL(CP(ETK_D8_ES2 ),	(IEN  | PTU | EN | M4)) /*GPIO 22  Bluetooth reset*/\
	MUX_VAL(CP(ETK_D9_ES2 ),	(IEN  | PTD | DIS | M0)) /*ETK_D9*/\
	MUX_VAL(CP(ETK_D10_ES2),	(IEN  | PTD | DIS | M0)) /*ETK_D10*/\
	MUX_VAL(CP(ETK_D11_ES2),	(IEN  | PTD | DIS | M0)) /*ETK_D11*/\
	MUX_VAL(CP(ETK_D12_ES2),	(IEN  | PTD | DIS | M0)) /*ETK_D12*/\
	MUX_VAL(CP(ETK_D13_ES2),	(IEN  | PTD | DIS | M4)) /*ETK_D13 - BoardID*/\
	MUX_VAL(CP(ETK_D14_ES2),	(IEN  | PTD | DIS | M4)) /*ETK_D14 - BoardID*/\
	MUX_VAL(CP(ETK_D15_ES2),	(IEN  | PTD | DIS | M4)) /*ETK_D15 - BoardID*/\
 /*DSS*/\
	MUX_VAL(CP(DSS_HSYNC),		(IDIS | PTD | DIS | M0)) /*GPIO_67*/\
	MUX_VAL(CP(DSS_VSYNC),		(IDIS | PTD | DIS | M4)) /*GPIO_68  LED3 DSS_VSYNC*/\
	MUX_VAL(CP(DSS_DATA0),		(IEN | PTU | EN | M4)) /*GPIO 70 - MIC MUTE*/\
	MUX_VAL(CP(DSS_DATA1),		(IEN | PTU | EN | M4)) /*GPIO 71 - SEL*/\
	MUX_VAL(CP(DSS_DATA2),		(IEN | PTU | EN | M4)) /*GPIO 72*/\
	MUX_VAL(CP(DSS_DATA5),		(IDIS | PTD | DIS | M4)) /*LP5521 ADDR0 - GPIO 75*/\
	MUX_VAL(CP(DSS_DATA10),		(IDIS | PTD | DIS | M4)) /*LP5521 ADDR1 - GPIO 80*/\
	MUX_VAL(CP(DSS_DATA11),		(IDIS | PTU | DIS | M4)) /*LP5521 EN    - GPIO 81*/\
	MUX_VAL(CP(DSS_DATA15),		(IDIS | PTD | EN | M4)) /*GPIO 85  - AIC3101 reset*/\
	MUX_VAL(CP(DSS_DATA4),		(IDIS | PTU | EN | M4)) /*GPIO 74  - DAC3203 reset*/\
/* Button GPIO */\
	MUX_VAL(CP(DSS_ACBIAS),		(IEN | PTU | DIS | M4)) /* GPIO 69  - VOLUME UP */\
	MUX_VAL(CP(DSS_DATA6),		(IEN | PTU | DIS | M4)) /* GPIO 76  - VOLUME DOWN */\
	MUX_VAL(CP(DSS_DATA7),		(IEN | PTU | EN | M4)) /* GPIO 77 - Reset*/\
	MUX_VAL(CP(DSS_DATA16),		(IDIS | PTD | EN | M4)) /* GPIO 86 */\
 /*CAMERA*/\
	MUX_VAL(CP(CAM_HS),		(IEN  | PTU | EN  | M0)) /*CAM_HS */\
	MUX_VAL(CP(CAM_VS),		(IDIS | PTD | DIS | M0)) /*GPIO_95*/\
	MUX_VAL(CP(CAM_XCLKA),		(IDIS | PTD | DIS | M0)) /*CAM_XCLKA*/\
	MUX_VAL(CP(CAM_PCLK),		(IEN  | PTU | EN  | M0)) /*CAM_PCLK*/\
	MUX_VAL(CP(CAM_FLD),		(IDIS | PTD | DIS | M4)) /*GPIO_98 LED8 */\
								 /* - CAM_RESET*/\
	MUX_VAL(CP(CAM_D0),		(IEN  | PTD | DIS | M0)) /*CAM_D0*/\
	MUX_VAL(CP(CAM_D1),		(IEN  | PTD | DIS | M0)) /*CAM_D1*/\
	MUX_VAL(CP(CAM_D2),		(IEN  | PTD | DIS | M0)) /*CAM_D2*/\
	MUX_VAL(CP(CAM_D3),		(IEN  | PTD | DIS | M4)) /*GPIO_102 LED5 CAM_D3*/\
	MUX_VAL(CP(CAM_D4),		(IEN  | PTD | DIS | M4)) /*GPIO_103 LED4 CAM_D4*/\
	MUX_VAL(CP(CAM_D5),		(IEN  | PTD | DIS | M0)) /*CAM_D5*/\
	MUX_VAL(CP(CAM_D6),		(IEN  | PTD | DIS | M0)) /*CAM_D6*/\
	MUX_VAL(CP(CAM_D7),		(IEN  | PTD | DIS | M0)) /*CAM_D7*/\
	MUX_VAL(CP(CAM_D8),		(IEN  | PTD | DIS | M0)) /*CAM_D8*/\
	MUX_VAL(CP(CAM_D9),		(IEN  | PTD | DIS | M0)) /*CAM_D9*/\
	MUX_VAL(CP(CAM_D10),		(IEN  | PTD | DIS | M4)) /*GPIO_110 LED1 CAM_D10*/\
	MUX_VAL(CP(CAM_D11),		(IEN  | PTD | DIS | M4)) /*GPIO_109 LED2 CAM_D11*/\
	MUX_VAL(CP(CAM_XCLKB),		(IDIS | PTD | DIS | M0)) /*CAM_XCLKB*/\
	MUX_VAL(CP(CAM_WEN),		(IEN | PTU | DIS | M4)) /*GPIO_167*/\
	MUX_VAL(CP(CAM_STROBE),		(IDIS | PTD | DIS | M7)) /*CAM_STROBE Safe mode*/\
	MUX_VAL(CP(GPIO_126),		(IDIS | PTU | DIS | M4)) /*GPIO_126*/\
	MUX_VAL(CP(CSI2_DX0),		(IEN  | PTD | DIS | M0)) /*CSI2_DX0*/\
	MUX_VAL(CP(CSI2_DY0),		(IEN  | PTD | DIS | M0)) /*CSI2_DY0*/\
	MUX_VAL(CP(CSI2_DX1),		(IEN  | PTD | DIS | M0)) /*CSI2_DX1*/\
	MUX_VAL(CP(CSI2_DY1),		(IEN  | PTD | DIS | M0)) /*CSI2_DY1*/\
 /*Audio Interface */\
 /*Expansion card  */\
	MUX_VAL(CP(MMC1_CLK),		(IDIS | PTU | EN  | M0)) /*MMC1_CLK*/\
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | EN  | M0)) /*MMC1_CMD*/\
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT0*/\
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT1*/\
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT2*/\
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT3*/\
	MUX_VAL(CP(MMC1_DAT4),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT4*/\
	MUX_VAL(CP(MMC1_DAT5),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT5*/\
	MUX_VAL(CP(MMC1_DAT6),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT6*/\
	MUX_VAL(CP(MMC1_DAT7),		(IEN  | PTU | EN  | M0)) /*MMC1_DAT7*/\
 /*Wireless LAN */\
	MUX_VAL(CP(MMC2_CLK),		(IEN  | PTU | EN  | M0)) /*MMC2_CLK*/\
	MUX_VAL(CP(MMC2_CMD),		(IEN  | PTU | EN  | M0)) /*MMC2_CMD*/\
	MUX_VAL(CP(MMC2_DAT0),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT0*/\
	MUX_VAL(CP(MMC2_DAT1),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT1*/\
	MUX_VAL(CP(MMC2_DAT2),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT2*/\
	MUX_VAL(CP(MMC2_DAT3),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT3*/\
	MUX_VAL(CP(MMC2_DAT4),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT4*/\
	MUX_VAL(CP(MMC2_DAT5),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT5*/\
	MUX_VAL(CP(MMC2_DAT6),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT6 */\
	MUX_VAL(CP(MMC2_DAT7),		(IEN  | PTU | EN  | M0)) /*MMC2_DAT7*/\
 /*Bluetooth*/\
	MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | EN  | M0)) /*UART2_CTS*/\
	MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M0)) /*UART2_RTS*/\
	MUX_VAL(CP(UART2_TX),		(IDIS | PTD | DIS | M0)) /*UART2_TX*/\
	MUX_VAL(CP(UART2_RX),		(IEN  | PTD | DIS | M0)) /*UART2_RX*/\
 /*Modem Interface */\
	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0)) /*UART1_TX*/\
	MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | DIS | M0)) /*UART1_RTS*/\
	MUX_VAL(CP(UART1_CTS),		(IEN  | PTU | DIS | M0)) /*UART1_CTS*/\
	MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0)) /*UART1_RX*/\
	MUX_VAL(CP(MCBSP4_CLKX),	(IDIS | PTD | DIS | M4)) /*GPIO_152*/\
								 /* - LCD_INI*/\
	MUX_VAL(CP(MCBSP4_DR),		(IDIS | PTD | DIS | M4)) /*GPIO_153*/\
								 /* - LCD_ENVDD */\
	MUX_VAL(CP(MCBSP4_DX),		(IDIS | PTD | DIS | M4)) /*GPIO_154*/\
								 /* - LCD_QVGA/nVGA */\
	MUX_VAL(CP(MCBSP4_FSX),		(IDIS | PTD | DIS | M4)) /*GPIO_155*/\
								 /* - LCD_RESB */\
	MUX_VAL(CP(MCBSP1_CLKR),	(IEN  | PTD | DIS | M0)) /*MCBSP1_CLKR  */\
	MUX_VAL(CP(MCBSP1_FSR),		(IDIS | PTU | EN  | M0)) /*MCBSP1_FSR*/\
	MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTD | DIS | M0)) /*MCBSP1_DX*/\
	MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTD | DIS | M0)) /*MCBSP1_DR*/\
	MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTU | DIS | M0)) /*MCBSP_CLKS  */\
	MUX_VAL(CP(MCBSP1_FSX),		(IEN  | PTD | DIS | M0)) /*MCBSP1_FSX*/\
	MUX_VAL(CP(MCBSP1_CLKX),	(IEN  | PTD | DIS | M0)) /*MCBSP1_CLKX  */\
    MUX_VAL(CP(MCBSP2_FSX),         (IEN  | PTD | DIS | M0)) /*MCBSP2_FSX*/\
    MUX_VAL(CP(MCBSP2_CLKX),        (IEN  | PTD | DIS | M0)) /*MCBSP2_CLKX*/\
    MUX_VAL(CP(MCBSP2_DR),          (IEN  | PTD | DIS | M0)) /*MCBSP2_DR*/\
    MUX_VAL(CP(MCBSP2_DX),          (IDIS | PTD | DIS | M0)) /*MCBSP2_DX*/\
 /*Serial Interface*/\
	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTD | EN  | M0)) /*UART3_CTS_*/\
								 /* RCTX*/\
	MUX_VAL(CP(UART3_RTS_SD),	(IDIS | PTD | DIS | M0)) /*UART3_RTS_SD */\
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)) /*UART3_RX_IRRX*/\
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)) /*UART3_TX_IRTX*/\
	MUX_VAL(CP(HSUSB0_CLK),		(IEN  | PTD | DIS | M0)) /*HSUSB0_CLK*/\
	MUX_VAL(CP(HSUSB0_STP),		(IDIS | PTU | EN  | M0)) /*HSUSB0_STP*/\
	MUX_VAL(CP(HSUSB0_DIR),		(IEN  | PTU | DIS | M4)) /*HSUSB0_DIR GPIO_122 */\
	MUX_VAL(CP(HSUSB0_NXT),		(IEN  | PTD | DIS | M0)) /*HSUSB0_NXT*/\
	MUX_VAL(CP(HSUSB0_DATA0),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA0*/\
	MUX_VAL(CP(HSUSB0_DATA1),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA1*/\
	MUX_VAL(CP(HSUSB0_DATA2),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA2*/\
	MUX_VAL(CP(HSUSB0_DATA3),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA3*/\
	MUX_VAL(CP(HSUSB0_DATA4),	(IDIS  | PTU | EN | M4)) /*HSUSB0_DATA4 GPIO_188 */\
	MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA5*/\
	MUX_VAL(CP(HSUSB0_DATA6),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA6*/\
	MUX_VAL(CP(HSUSB0_DATA7),	(IEN  | PTD | DIS | M0)) /*HSUSB0_DATA7*/\
	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | EN  | M0)) /*I2C1_SCL*/\
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | EN  | M0)) /*I2C1_SDA*/\
	MUX_VAL(CP(I2C2_SCL),		(IEN  | PTU | EN  | M0)) /*I2C2_SCL*/\
	MUX_VAL(CP(I2C2_SDA),		(IEN  | PTU | EN  | M0)) /*I2C2_SDA*/\
	MUX_VAL(CP(I2C3_SCL),		(IEN  | PTU | EN  | M0)) /*I2C3_SCL*/\
	MUX_VAL(CP(I2C3_SDA),		(IEN  | PTU | EN  | M0)) /*I2C3_SDA*/\
	MUX_VAL(CP(I2C4_SCL),		(IEN  | PTU | EN  | M0)) /*I2C4_SCL*/\
	MUX_VAL(CP(I2C4_SDA),		(IEN  | PTU | EN  | M0)) /*I2C4_SDA*/\
	MUX_VAL(CP(HDQ_SIO),		(IEN  | PTU | EN  | M0)) /*HDQ_SIO*/\
	MUX_VAL(CP(MCSPI1_CLK),		(IDIS | PTU | EN | M0)) /*McSPI1_CLK ks8851*/\
	MUX_VAL(CP(MCSPI1_SIMO),	(IDIS | PTU | EN | M0)) /*McSPI1_SIMO  */\
	MUX_VAL(CP(MCSPI1_SOMI),	(IEN  | PTU | DIS  | M0)) /*McSPI1_SOMI  */\
	MUX_VAL(CP(MCSPI1_CS0),		(IDIS  | PTU | EN  | M0)) /*McSPI1_CS0*/\
	MUX_VAL(CP(MCSPI1_CS1),		(IEN  | PTD | DIS  | M0)) /*GPIO_175*/\
								 /* TS_PEN_IRQ */\
	MUX_VAL(CP(MCSPI1_CS3),		(IEN  | PTD | EN  | M0)) /*McSPI1_CS3*/\
	MUX_VAL(CP(MCSPI2_CLK),		(IEN  | PTD | DIS | M0)) /*McSPI2_CLK*/\
	MUX_VAL(CP(MCSPI2_SIMO),	(IEN  | PTD | DIS | M0)) /*McSPI2_SIMO*/\
	MUX_VAL(CP(MCSPI2_SOMI),	(IEN  | PTD | DIS | M0)) /*McSPI2_SOMI*/\
	MUX_VAL(CP(MCSPI2_CS0),		(IEN  | PTD | EN  | M0)) /*McSPI2_CS0*/\
	MUX_VAL(CP(MCSPI2_CS1),		(IEN  | PTD | EN  | M0)) /*McSPI2_CS1*/\
 /*Control and debug */\
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTU | EN  | M0)) /*GPMC_A1*/\
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTU | EN  | M0)) /*GPMC_A2*/\
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTU | EN  | M0)) /*GPMC_A3*/\
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTU | EN  | M0)) /*GPMC_A4*/\
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTU | EN  | M0)) /*GPMC_A5*/\
	MUX_VAL(CP(GPMC_A6),		(IEN  | PTD | DIS | M4)) /*GPMC_A6 - Board ID*/\
	MUX_VAL(CP(GPMC_A7),		(IEN  | PTD | DIS | M4)) /*GPMC_A7 - Board ID*/\
	MUX_VAL(CP(GPMC_A8),		(IEN  | PTD | DIS | M4)) /*GPMC_A8 - Board ID*/\
	MUX_VAL(CP(GPMC_A9),		(IEN  | PTD | DIS | M4)) /*GPMC_A9 - Board ID*/\
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTU | EN  | M0)) /*GPMC_A10*/\
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0)) /*GPMC_D0*/\
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0)) /*GPMC_D1*/\
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0)) /*GPMC_D2*/\
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0)) /*GPMC_D3*/\
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0)) /*GPMC_D4*/\
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0)) /*GPMC_D5*/\
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0)) /*GPMC_D6*/\
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0)) /*GPMC_D7*/\
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTU | EN  | M0)) /*GPMC_D8*/\
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTU | EN  | M0)) /*GPMC_D9*/\
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTU | EN  | M0)) /*GPMC_D10*/\
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTU | EN  | M0)) /*GPMC_D11*/\
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTU | EN  | M0)) /*GPMC_D12*/\
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTU | EN  | M0)) /*GPMC_D13*/\
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTU | EN  | M0)) /*GPMC_D14*/\
	MUX_VAL(CP(GPMC_D15),		(IEN  | PTU | EN  | M0)) /*GPMC_D15*/\
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0)) /*GPMC_nCS0*/\
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTU | EN  | M0)) /*GPMC_nCS1*/\
	MUX_VAL(CP(GPMC_NCS2),		(IDIS | PTU | EN  | M0)) /*GPMC_nCS2*/\
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTU | EN  | M0)) /*GPMC_nCS3*/\
	MUX_VAL(CP(GPMC_NCS4),		(IEN  | PTD | DIS  | M2)) /*MCBSP4_clkx*/\
	MUX_VAL(CP(GPMC_NCS5),		(IEN  | PTD | DIS  | M2)) /*MCBSP4_dr*/\
	MUX_VAL(CP(GPMC_NCS6),		(IDIS | PTD | DIS  | M4)) /*GPIO_57*/\
	MUX_VAL(CP(GPMC_NCS7),		(IDIS | PTU | DIS  | M4)) /*GPIO_58*/\
	MUX_VAL(CP(GPMC_CLK),		(IDIS | PTU | EN  | M0)) /*GPMC_CLK*/\
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0)) /*GPMC_nADV_ALE*/\
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0)) /*GPMC_nOE*/\
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0)) /*GPMC_nWE*/\
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0)) /*GPMC_nBE0_CLE*/\
	MUX_VAL(CP(GPMC_NBE1),		(IEN  | PTU | EN  | M0)) /*GPMC_nBE1*/\
	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | DIS | M0)) /*GPMC_nWP*/\
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | EN  | M0)) /*GPMC_WAIT0*/\
	MUX_VAL(CP(GPMC_WAIT1),		(IEN  | PTU | EN  | M0)) /*GPMC_WAIT1*/\
	MUX_VAL(CP(GPMC_WAIT2),		(IEN  | PTU | EN  | M4)) /*GPIO_64*/\
								 /* - ETH_nRESET*/\
	MUX_VAL(CP(GPMC_WAIT3),		(IEN  | PTU | EN  | M0)) /*GPMC_WAIT3*/\
	MUX_VAL(CP(GPIO_129),		(IDIS | PTU | DIS | M4)) /*GPIO_129*/\

#endif
