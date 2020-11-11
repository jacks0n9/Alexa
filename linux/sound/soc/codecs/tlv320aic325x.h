/*
 * linux/sound/soc/codecs/tlv320aic325x.h
 *
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 *
 *
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * History:
 * Rev 1.1	Added More ENUM Macros		18-01-2011
 *
 */

#ifndef _TLV320AIC325x_H
#define _TLV320AIC325x_H

#define AUDIO_NAME "aic325x"
#define AIC325x_VERSION "1.1"

//#define AIC3253_CODEC_SUPPORT
//#define AIC3254_CODEC_SUPPORT
#define AIC3204_CODEC_SUPPORT
//#define AIC3256_CODEC_SUPPORT
//#define AIC3206_CODEC_SUPPORT

/* Enable slave / master mode for codec */
#define AIC325x_MCBSP_SLAVE
//#undef AIC325x_MCBSP_SLAVE

/* Macro enables or disables support for miniDSP in the driver */
#if defined(AIC3254_CODEC_SUPPORT) || defined( AIC3253_CODEC_SUPPORT) || defined ( AIC3256_CODEC_SUPPORT)
 
//#define CONFIG_MINI_DSP
#undef CONFIG_MINI_DSP

#endif

/* Enable headset detection */
//#define HEADSET_DETECTION
#undef HEADSET_DETECTION

/* Macro enables or disables  AIC3xxx TiLoad Support */
#define AIC325x_TiLoad
//#undef AIC3xxx_TiLoad
/* Enable register caching on write */
#define EN_REG_CACHE

/* AIC325x supported sample rate are 8k to 192k */
#define AIC325x_RATES	SNDRV_PCM_RATE_8000_192000

/* AIC325x supports the word formats 16bits, 20bits, 24bits and 32 bits */
#define AIC325x_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE \
			 | SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

#define AIC325x_FREQ_12000000 12000000
#define AIC325x_FREQ_24000000 24000000
#define AIC325x_FREQ_19200000 19200000
#define AIC325x_FREQ_26000000 26000000
#define AIC325x_FREQ_38400000 38400000

/* Audio data word length = 16-bits (default setting) */
#define AIC325x_WORD_LEN_16BITS		0x00
#define AIC325x_WORD_LEN_20BITS		0x01
#define AIC325x_WORD_LEN_24BITS		0x02
#define AIC325x_WORD_LEN_32BITS		0x03



/* sink: name of target widget */
#define AIC325x_WIDGET_NAME             0
/* control: mixer control name */
#define AIC325x_CONTROL_NAME		1
/* source: name of source name */
#define AIC325x_SOURCE_NAME		2

/* D15..D8 aic325x register offset */
#define AIC325x_REG_OFFSET_INDEX        0
/* D7...D0 register data */
#define AIC325x_REG_DATA_INDEX          1

/* Serial data bus uses I2S mode (Default mode) */
#define AIC325x_I2S_MODE		0x00
#define AIC325x_DSP_MODE		0x01
#define AIC325x_RIGHT_JUSTIFIED_MODE	0x02
#define AIC325x_LEFT_JUSTIFIED_MODE	0x03

/* 8 bit mask value */
#define AIC325x_8BITS_MASK              0xFF

/* shift value for CLK_REG_3 register */
#define CLK_REG_3_SHIFT			6
/* shift value for DAC_OSR_MSB register */
#define DAC_OSR_MSB_SHIFT		4

/* number of codec specific register for configuration */
#define NO_FEATURE_REGS     		2

/* AIC325x register space */
#define	AIC325x_CACHEREGNUM		256

/* ****************** Page 0 Registers **************************************/
/* Page select register */
#define	PAGE_SELECT			0
/* Software reset register */
#define	RESET				1
/*Clock setting register 1, Multiplexers*/
#define	CLK_REG_1			4
/*Clock setting register 2, PLL*/
#define	CLK_REG_2			5
/*Clock setting register 3, PLL*/
#define	CLK_REG_3			6
/*Clock setting register 4, PLL*/
#define	CLK_REG_4			7
/*Clock setting register 5, PLL*/
#define	CLK_REG_5			8
/*Clock setting register 6, PLL*/
#define	NDAC_CLK_REG_6		        11
/*Clock setting register 7, PLL*/
#define	MDAC_CLK_REG_7		        12
/*DAC OSR setting register1,MSB value*/
#define DAC_OSR_MSB			13
/*DAC OSR setting register 2,LSB value*/
#define DAC_OSR_LSB			14
/*Clock setting register 8, PLL*/
#define	NADC_CLK_REG_8	         	18
/*Clock setting register 9, PLL*/
#define	MADC_CLK_REG_9	        	19
/*ADC Oversampling (AOSR) Register*/
#define ADC_OSR_REG			20
/*Clock setting register 9, Multiplexers*/
#define CLK_MUX_REG_9		        25
/*Clock setting register 10, CLOCKOUT M divider value*/
#define CLK_REG_10			26
/*Audio Interface Setting Register 1*/
#define INTERFACE_SET_REG_1	        27
/*Audio Interface Setting Register 2*/
#define AIS_REG_2			28
/*Audio Interface Setting Register 3*/
#define AIS_REG_3			29
/*Clock setting register 11,BCLK N Divider*/
#define CLK_REG_11			30
/*Audio Interface Setting Register 4,Secondary Audio Interface*/
#define AIS_REG_4			31
/*Audio Interface Setting Register 5*/
#define AIS_REG_5			32
/*Audio Interface Setting Register 6*/
#define AIS_REG_6			33
 /**/

#define  INT1_CTRL			48
#define GPIO_CTRL			52
/*DOUT Function Control*/
#define DOUT_CTRL			53
/*DIN Function Control*/
#define DIN_CTL				54
/*DAC Signal Processing Block*/
#define DAC_PRB				60
/*ADC Signal Processing Block*/
#define ADC_PRB				61
/*DAC channel setup register*/
#define DAC_CHN_REG                     63
/*DAC Mute and volume control register*/
#define DAC_MUTE_CTRL_REG	        64
/*Left DAC channel digital volume control*/
#define LDAC_VOL			65
/*Right DAC channel digital volume control*/
#define RDAC_VOL			66

/*Headset Detection Configuration Register*/
#define HEADSET_DETECT                  67

/* DRC control Registers */
#define DRC_CTRL_REG1                   68
#define DRC_CTRL_REG2                   69
#define DRC_CTRL_REG3                   70

/* Beep Generator Control Registers */
#define BEEP_CTRL_REG1                   71
#define BEEP_CTRL_REG2                   72
#define BEEP_CTRL_REG3                   73
#define BEEP_CTRL_REG4                   74
#define BEEP_CTRL_REG5                   75
#define BEEP_CTRL_REG6                   76
#define BEEP_CTRL_REG7                   77
#define BEEP_CTRL_REG8                   78
#define BEEP_CTRL_REG9                   79

/*ADC Register 1*/
#define ADC_CHN_REG			81
/*ADC Fine Gain Adjust*/
#define	ADC_FGA				82
/*Left ADC Channel Volume Control*/
#define LADC_VOL			83
/*Right ADC Channel Volume Control*/
#define RADC_VOL			84

#ifndef AIC3253_CODEC_SUPPORT

/*Left Channel AGC Control Register 1*/
#define LEFT_AGC_REG1		        86
/*Left Channel AGC Control Register 2*/
#define LEFT_AGC_REG2		        87
/*Left Channel AGC Control Register 3 */
#define LEFT_AGC_REG3		        88
/*Left Channel AGC Control Register 4 */
#define LEFT_AGC_REG4		        89
/*Left Channel AGC Control Register 5 */
#define LEFT_AGC_REG5		        90
/*Left Channel AGC Control Register 6 */
#define LEFT_AGC_REG6		        91
/*Left Channel AGC Control Register 7 */
#define LEFT_AGC_REG7		        92
/*Right Channel AGC Control Register 1*/
#define RIGHT_AGC_REG1		        94
/*Right Channel AGC Control Register 2*/
#define RIGHT_AGC_REG2 	                95
/*Right Channel AGC Control Register 3 */
#define RIGHT_AGC_REG3		        96
/*Right Channel AGC Control Register 4 */
#define RIGHT_AGC_REG4		        97
/*Right Channel AGC Control Register 5 */
#define RIGHT_AGC_REG5		        98
/*Right Channel AGC Control Register 6 */
#define RIGHT_AGC_REG6		        99
/*Right Channel AGC Control Register 7 */
#define RIGHT_AGC_REG7		        100

#endif

/******************** Page 1 Registers **************************************/
#define PAGE_1				128
/*Power Conguration*/
#define POW_CFG				(PAGE_1 + 1)
/*LDO Control*/
#define LDO_CTRL				(PAGE_1 + 2)

#if defined (AIC3256_CODEC_SUPPORT) || defined( AIC3206_CODEC_SUPPORT)
/*power control register 2 */
#define PWR_CTRL_REG                   (PAGE_1 + 2)
#endif


/*Output Driver Power Control*/
#define OUT_PWR_CTRL			(PAGE_1 + 9)

/* Full Chip Common Mode register */
#ifdef AIC3256_CODEC_SUPPORT
#define CM_CTRL_REG			(PAGE_1 + 10)
#endif
/*HPL Routing Selection*/
#define HPL_ROUTE_CTRL		        (PAGE_1 + 12)
/*HPR Routing Selection*/
#define HPR_ROUTE_CTRL		        (PAGE_1 + 13)

#ifndef AIC3253_CODEC_SUPPORT
/*LOL Routing Selection*/
#define LOL_ROUTE_CTRL		        (PAGE_1 + 14)
/*LOR Routing Selection*/
#define LOR_ROUTE_CTRL		        (PAGE_1 + 15)
#endif

/*HPL Driver Gain*/
#define	HPL_GAIN			(PAGE_1 + 16)
/*HPR Driver Gain*/
#define	HPR_GAIN			(PAGE_1 + 17)

#ifndef AIC3253_CODEC_SUPPORT
/*LOL Driver Gain*/
#define	LOL_GAIN			(PAGE_1 + 18)
/*LOR Driver Gain*/
#define	LOR_GAIN			(PAGE_1 + 19)
#endif

/*Headphone Driver Startup Control Register*/
#define HPHONE_STARTUP_CTRL	        (PAGE_1 + 20)
/*IN1L to HPL Volume Control Register */
#define IN1L_HPL_VOL_CTRL                   (PAGE_1 + 22)
/*IN1R to HPR Volume Control Register */
#define IN1R_HPR_VOL_CTRL                   (PAGE_1 + 23)

#ifndef AIC3253_CODEC_SUPPORT
/*MAL Volume Control Register*/
#define MAL_CTRL_REG                    (PAGE_1 + 24)
/*MAR Volume Control Register*/
#define MAR_CTRL_REG                    (PAGE_1 + 25)
#endif

/*MICBIAS Configuration Register*/
#define MICBIAS_CTRL		        (PAGE_1 + 51)

#ifndef AIC3253_CODEC_SUPPORT
/*Left MICPGA Positive Terminal Input Routing Configuration Register*/
#define LMICPGA_PIN_CFG		        (PAGE_1 + 52)
/*Left MICPGA Negative Terminal Input Routing Configuration Register*/
#define LMICPGA_NIN_CFG		        (PAGE_1 + 54)
/*Right MICPGA Positive Terminal Input Routing Configuration Register*/
#define RMICPGA_PIN_CFG		        (PAGE_1 + 55)
/*Right MICPGA Negative Terminal Input Routing Configuration Register*/
#define RMICPGA_NIN_CFG		        (PAGE_1 + 57)
#endif

/*Floating Input Configuration Register*/
#define INPUT_CFG_REG		        (PAGE_1 + 58)

#ifndef AIC3253_CODEC_SUPPORT
/*Left MICPGA Volume Control Register*/
#define LMICPGA_VOL_CTRL	        (PAGE_1 + 59)
/*Right MICPGA Volume Control Register*/
#define RMICPGA_VOL_CTRL	        (PAGE_1 + 60)
#endif

#define ANALOG_INPUT_CHARGING_CFG	(PAGE_1 + 71)

#define REF_PWR_UP_CONF_REG	(PAGE_1 + 123)

#if defined (AIC3256_CODEC_SUPPORT) || defined( AIC3206_CODEC_SUPPORT)
/*charge control register*/
#define CHRG_CTRL_REG                   (PAGE_1+124)
/*headphone driver configuration register*/
#define HP_DRIVER_CONF_REG                    (PAGE_1+125)
#endif




/****************************************************************************/
#define BIT7				(0x01 << 7)
#define CODEC_CLKIN_MASK		0x03
#define MCLK_2_CODEC_CLKIN		0x00
#define PLLCLK_2_CODEC_CLKIN	        0x03
/*Bclk_in selection*/
#define BDIV_CLKIN_MASK			0x03
#define	DAC_MOD_CLK_2_BDIV_CLKIN        0x01
#define SOFT_RESET			0x01
#define PAGE0				0x00
#define PAGE1				0x01
#define BIT_CLK_MASTER			0x08
#define WORD_CLK_MASTER			0x04
#define	HIGH_PLL 			(0x01 << 6)
#define ENABLE_CLK_MASK			BIT7
#define ENABLE_PLL			BIT7
#define ENABLE_NDAC			BIT7
#define ENABLE_MDAC			BIT7
#define ENABLE_NADC			BIT7
#define ENABLE_MADC			BIT7
#define ENABLE_BCLK			BIT7
#define ENABLE_DAC		        (0x03 << 6)
#define LDAC_2_LCHN			(0x01 << 4 )
#define RDAC_2_RCHN			(0x01 << 2 )
#define LDAC_CHNL_2_HPL			(0x01 << 3)
#define RDAC_CHNL_2_HPR			(0x01 << 3)
#define SOFT_STEP_2WCLK			(0x01)
#define MUTE_ON				0x0C
#define DEFAULT_VOL			0x0
#define DISABLE_ANALOG			(0x01 << 3)
#define LDAC_2_HPL_ROUTEON		0x08
#define RDAC_2_HPR_ROUTEON		0x08

/****************************************************************************
 * Mixer control  related #defines
 ***************************************************************************
 */

#define LDMUTE_ENUM		0
#define SOFTSTEP_ENUM		1
#define DACEXTRA_ENUM		2
#define HSET_ENUM		3
#define LAMUTE_ENUM		4
#define DRC_ENUM		5
#define BEEP_ENUM		6
#define BEEPEXTRA_ENUM		7
#define MICBIAS_ENUM		8

#define IN1L_LMICPGA_ENUM	9
#define IN2L_LMICPGA_ENUM	10 	
#define IN3L_LMICPGA_ENUM	11
#define IN1R_LMICPGA_ENUM 	12

#define CM1L_LMICPGA_ENUM	13
#define IN2R_LMICPGA_ENUM	14
#define IN3R_LMICPGA_ENUM 	15
#define CM2L_LMICPGA_ENUM 	16

#define IN1R_RMICPGA_ENUM 	17
#define IN2R_RMICPGA_ENUM 	18
#define IN3R_RMICPGA_ENUM	19
#define IN2L_RMICPGA_ENUM 	20

#define CM1R_RMICPGA_ENUM	21 
#define IN1L_RMICPGA_ENUM	22
#define IN3L_RMICPGA_ENUM 	23
#define CM2R_RMICPGA_ENUM 	24

#define LMICPGAGAIN_ENUM	25
#define RMICPGAGAIN_ENUM	26

#define MICBIASPOWER_ENUM       27
#define MICBIASVOLTAGE_ENUM 	28
#define LDACPOWER_ENUM		29
#define RDACPOWER_ENUM		30
#define LADCPOWER_ENUM		31
#define RADCPOWER_ENUM		32
#define RDMUTE_ENUM		33
#define RAMUTE_ENUM		34
#define LDACPATH_ENUM		35
#define RDACPATH_ENUM		36
#define HPPOWER_ENUM		37
#define LOPOWER_ENUM		38
#define MAPOWER_ENUM		39
#define LHPMUTE_ENUM		40
#define RHPMUTE_ENUM		41
#define LLOMUTE_ENUM		42
#define RLOMUTE_ENUM		43
#define LAGC_ENUM		44
#define RAGC_ENUM		45


#define HP_DRIVER_BUSY_MASK		0x04	
/* Headphone driver Configuration Register Page 1, Register 125 */
#define GCHP_ENABLE		0x10	
#define DC_OC_FOR_ALL_COMB		0x03
#define DC_OC_FOR_PROG_COMB		0x02

/* Reference Power-Up configuration register */
#define REF_PWR_UP_MASK		0x4
#define AUTO_REF_PWR_UP		0x0
#define FORCED_REF_PWR_UP	0x4

/* Power Configuration register 1 */
#define WEAK_AVDD_TO_DVDD_DIS	0x8

/* Power Configuration register 1 */
#define ANALOG_BLOCK_POWER_CONTROL_MASK 	0x08
#define ENABLE_ANALOG_BLOCK	0x0
#define DISABLE_ANALOG_BLOCK	0x8

/* Floating input Configuration register P1_R58 */
#define WEAK_BIAS_INPUTS_MASK	0xFC	 

/* Common Mode Control Register */
#define GCHP_HPL_STATUS   	0x4

/* Audio Interface Register 3 P0_R29 */
#define BCLK_WCLK_BUFFER_POWER_CONTROL_MASK	0x4
#define BCLK_WCLK_BUFFER_ON			0x4

/* Power Configuration Register */
#define AVDD_CONNECTED_TO_DVDD_MASK	0x8
#define DISABLE_AVDD_TO_DVDD		0x8
#define ENABLE_AVDD_TO_DVDD		0x0

/*
 *****************************************************************************
 * Structures Definitions
 *****************************************************************************
 */
/*
 *----------------------------------------------------------------------------
 * @struct  aic325x_setup_data |
 *          i2c specific data setup for AIC325x.
 * @field   unsigned short |i2c_address |
 *          Unsigned short for i2c address.
 *----------------------------------------------------------------------------
 */
    struct aic325x_setup_data {
	unsigned short i2c_address;
};

/*
 *----------------------------------------------------------------------------
 * @struct  aic325x_priv |
 *          AIC325x priviate data structure to set the system clock, mode and
 *          page number.
 * @field   u32 | sysclk |
 *          system clock
 * @field   s32 | master |
 *          master/slave mode setting for AIC325x
 * @field   u8 | page_no |
 *          page number. Here, page 0 and page 1 are used.
 *----------------------------------------------------------------------------
 */
struct aic325x_priv {
	u32 sysclk;
	s32 master;
	u8 page_no;
	int pll_divs_index;	
	struct i2c_client *control_data;
};

/*
 *----------------------------------------------------------------------------
 * @struct  aic325x_configs |
 *          AIC325x initialization data which has register offset and register
 *          value.
 * @field   u16 | reg_offset |
 *          AIC325x Register offsets required for initialization..
 * @field   u8 | reg_val |
 *          value to set the AIC325x register to initialize the AIC325x.
 *----------------------------------------------------------------------------
 */
struct aic325x_configs {
	u8 reg_offset;
	u8 reg_val;
};

/*
 *----------------------------------------------------------------------------
 * @struct  aic325x_rate_divs |
 *          Setting up the values to get different freqencies
 *
 * @field   u32 | mclk |
 *          Master clock
 * @field   u32 | rate |
 *          sample rate
 * @field   u8 | p_val |
 *          value of p in PLL
 * @field   u32 | pll_j |
 *          value for pll_j
 * @field   u32 | pll_d |
 *          value for pll_d
 * @field   u32 | dosr |
 *          value to store dosr
 * @field   u32 | ndac |
 *          value for ndac
 * @field   u32 | mdac |
 *          value for mdac
 * @field   u32 | aosr |
 *          value for aosr
 * @field   u32 | nadc |
 *          value for nadc
 * @field   u32 | madc |
 *          value for madc
 * @field   u32 | blck_N |
 *          value for block N
 * @field   u32 | aic325x_configs |
 *          configurations for aic325x register value
 *----------------------------------------------------------------------------
 */
struct aic325x_rate_divs {
	u32 mclk;
	u32 rate;
	u8 p_val;
	u8 pll_j;
	u16 pll_d;
	u16 dosr;
	u8 ndac;
	u8 mdac;
	u8 aosr;
	u8 nadc;
	u8 madc;
	u8 blck_N;
	struct aic325x_configs codec_specific_regs[NO_FEATURE_REGS];
};

/*
 *----------------------------------------------------------------------------
 * @struct  snd_soc_codec_dai |
 *          It is SoC Codec DAI structure which has DAI capabilities viz.,
 *          playback and capture, DAI runtime information viz. state of DAI
 *			and pop wait state, and DAI private data.
 *----------------------------------------------------------------------------
 */
extern struct snd_soc_dai tlv320aic325x_dai;

/*
 *----------------------------------------------------------------------------
 * @struct  snd_soc_codec_device |
 *          This structure is soc audio codec device sturecute which pointer
 *          to basic functions aic325x_probe(), aic325x_remove(),
 *			aic325x_suspend() and aic325x_resume()
 *
 */
extern struct snd_soc_codec_device soc_codec_dev_aic325x;

#endif				/* _TLV320AIC325x_H */
