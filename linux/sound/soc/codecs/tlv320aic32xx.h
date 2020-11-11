/*
 * linux/sound/soc/codecs/tlv320aic32xx.h
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
 * Rev 1.1  Added More ENUM Macros      Mistral     18-01-2011
 *
 */

#ifndef _TLV320AIC32xx_AIC32xx_H
#define _TLV320AIC32xx_AIC31xx_H

#define AUDIO_NAME "aic32xx_adc31xx"
#define AIC32xx_ADC31xx_VERSION "1.1"
#define AIC32xx_VERSION "1.1"


#define AIC3204_CODEC_SUPPORT
#define ADC3101_CODEC_SUPPORT

/* Enable slave / master mode for codec */
#define AIC32xx_MCBSP_SLAVE
//#undef AIC32xx_MCBSP_SLAVE

/* 8 bit mask value */
#define ADC3xxx_8BITS_MASK           0xFF

/* Macro enables or disables support for miniDSP in the driver */
#if defined(AIC3254_CODEC_SUPPORT) || defined( AIC3253_CODEC_SUPPORT) || defined ( AIC3256_CODEC_SUPPORT)

//#define CONFIG_MINI_DSP
#undef CONFIG_MINI_DSP

#endif

/* Enable headset detection */
//#define HEADSET_DETECTION
#undef HEADSET_DETECTION

/* Macro enables or disables  AIC3xxx TiLoad Support */
//#define AIC32xx_TiLoad
#undef AIC3xxx_TiLoad
/* Enable register caching on write */
#define EN_REG_CACHE

/* AIC32xx supported sample rate are 8k to 192k */
#define AIC32xx_RATES   SNDRV_PCM_RATE_8000_192000

/* AIC32xx supports the word formats 16bits, 20bits, 24bits and 32 bits */
#define AIC32xx_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE \
             | SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

#define AIC32xx_FREQ_12000000 12000000
#define AIC32xx_FREQ_24000000 24000000
#define AIC32xx_FREQ_19200000 19200000
#define AIC32xx_FREQ_26000000 26000000
#define AIC32xx_FREQ_38400000 38400000

/* Audio data word length = 16-bits (default setting) */
#define AIC32xx_WORD_LEN_16BITS     0x00
#define AIC32xx_WORD_LEN_20BITS     0x01
#define AIC32xx_WORD_LEN_24BITS     0x02
#define AIC32xx_WORD_LEN_32BITS     0x03



/* sink: name of target widget */
#define AIC32xx_WIDGET_NAME             0
/* control: mixer control name */
#define AIC32xx_CONTROL_NAME        1
/* source: name of source name */
#define AIC32xx_SOURCE_NAME     2

/* D15..D8 aic32xx register offset */
#define AIC32xx_REG_OFFSET_INDEX        0
/* D7...D0 register data */
#define AIC32xx_REG_DATA_INDEX          1

/* Serial data bus uses I2S mode (Default mode) */
#define AIC32xx_I2S_MODE        0x00
#define AIC32xx_DSP_MODE        0x01
#define AIC32xx_RIGHT_JUSTIFIED_MODE    0x02
#define AIC32xx_LEFT_JUSTIFIED_MODE 0x03

/* 8 bit mask value */
#define AIC32xx_8BITS_MASK              0xFF

/* shift value for CLK_REG_3 register */
#define CLK_REG_3_SHIFT         6
/* shift value for DAC_OSR_MSB register */
#define DAC_OSR_MSB_SHIFT       4

/* number of codec specific register for configuration */
#define NO_FEATURE_REGS             2

/* AIC32xx and two ADC31xx register space */
#define AIC3204_CACHEREGNUM     (128 + 128)
#define ADC3101_CACHEREGNUM     (128 + 128)
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
// Use 4 ADC3101
#define NUM_ADC3101             4
#else
// Use 2 ADC 3101
#define NUM_ADC3101             2
#endif

#define AIC32xx_CACHEREGNUM     (AIC3204_CACHEREGNUM + ADC3101_CACHEREGNUM * NUM_ADC3101)

/*
 * Creates the register using 8 bits for reg, 3 bits for device
 * and 7 bits for page. The extra 2 high order bits for page are
 * necessary for large pages, which may be sent in with the 32
 * bit reg value to the following functions where dev, page, and
 * reg no are properly masked out:
 * - aic32xx_write
 * - aic32xx_read_reg_cache
 * For ALSA calls (where the register is limited to 16bits), the
 * 5 bits for page is sufficient, and no high order bits will be
 * truncated.
 */
#define MAKE_REG(device, page, reg) (u32) \
    (((page) & 0x7f) << 11 | ((device) & 0x7) << 8 | ((reg) & 0xff))

/* ****************** Page 0 Registers **************************************/
/* Page select register */
#define PAGE_SELECT         MAKE_REG(0,0,0)
/* Software reset register */
#define RESET               MAKE_REG(0,0,1)
/*Clock setting register 1, Multiplexers*/
#define CLK_REG_1           MAKE_REG(0,0,4)
/*Clock setting register 2, PLL*/
#define CLK_REG_2           MAKE_REG(0,0,5)
/*Clock setting register 3, PLL*/
#define CLK_REG_3           MAKE_REG(0,0,6)
/*Clock setting register 4, PLL*/
#define CLK_REG_4           MAKE_REG(0,0,7)
/*Clock setting register 5, PLL*/
#define CLK_REG_5           MAKE_REG(0,0,8)
/*Clock setting register 6, PLL*/
#define NDAC_CLK_REG_6      MAKE_REG(0,0,11)
/*Clock setting register 7, PLL*/
#define MDAC_CLK_REG_7      MAKE_REG(0,0,12)
/*DAC OSR setting register1,MSB value*/
#define DAC_OSR_MSB         MAKE_REG(0,0,13)
/*DAC OSR setting register 2,LSB value*/
#define DAC_OSR_LSB         MAKE_REG(0,0,14)
/*Clock setting register 8, PLL*/
#define NADC_CLK_REG_8      MAKE_REG(0,0,18)
/*Clock setting register 9, PLL*/
#define MADC_CLK_REG_9      MAKE_REG(0,0,19)
/*ADC Oversampling (AOSR) Register*/
#define ADC_OSR_REG         MAKE_REG(0,0,20)
/*Clock setting register 9, Multiplexers*/
#define CLK_MUX_REG_9       MAKE_REG(0,0,25)
/*Clock setting register 10, CLOCKOUT M divider value*/
#define CLK_REG_10          MAKE_REG(0,0,26)
/*Audio Interface Setting Register 1*/
#define INTERFACE_SET_REG_1 MAKE_REG(0,0,27)
/*Audio Interface Setting Register 2*/
#define AIS_REG_2           MAKE_REG(0,0,28)
/*Audio Interface Setting Register 3*/
#define AIS_REG_3           MAKE_REG(0,0,29)
/*Clock setting register 11,BCLK N Divider*/
#define CLK_REG_11          MAKE_REG(0,0,30)
/*Audio Interface Setting Register 4,Secondary Audio Interface*/
#define AIS_REG_4           MAKE_REG(0,0,31)
/*Audio Interface Setting Register 5*/
#define AIS_REG_5           MAKE_REG(0,0,32)
/*Audio Interface Setting Register 6*/
#define AIS_REG_6           MAKE_REG(0,0,33)
 /**/

#define  INT1_CTRL          MAKE_REG(0,0,48)
#define GPIO_CTRL           MAKE_REG(0,0,52)
/*DOUT Function Control*/
#define DOUT_CTRL           MAKE_REG(0,0,53)
/*DIN Function Control*/
#define DIN_CTL             MAKE_REG(0,0,54)
/*SCLK Control Register*/
#define SCLK_CTRL             MAKE_REG(0,0,56)
/*DAC Signal Processing Block*/
#define DAC_PRB             MAKE_REG(0,0,60)
/*ADC Signal Processing Block*/
#define ADC_PRB             MAKE_REG(0,0,61)
/*DAC channel setup register*/
#define DAC_CHN_REG         MAKE_REG(0,0,63)
/*DAC Mute and volume control register*/
#define DAC_MUTE_CTRL_REG   MAKE_REG(0,0,64)
/*Left DAC channel digital volume control*/
#define LDAC_VOL            MAKE_REG(0,0,65)
/*Right DAC channel digital volume control*/
#define RDAC_VOL            MAKE_REG(0,0,66)

/*Headset Detection Configuration Register*/
#define HEADSET_DETECT      MAKE_REG(0,0,67)

/* DRC control Registers */
#define DRC_CTRL_REG1       MAKE_REG(0,0,68)
#define DRC_CTRL_REG2       MAKE_REG(0,0,69)
#define DRC_CTRL_REG3       MAKE_REG(0,0,70)

/* Beep Generator Control Registers */
#define BEEP_CTRL_REG1      MAKE_REG(0,0,71)
#define BEEP_CTRL_REG2      MAKE_REG(0,0,72)
#define BEEP_CTRL_REG3      MAKE_REG(0,0,73)
#define BEEP_CTRL_REG4      MAKE_REG(0,0,74)
#define BEEP_CTRL_REG5      MAKE_REG(0,0,75)
#define BEEP_CTRL_REG6      MAKE_REG(0,0,76)
#define BEEP_CTRL_REG7      MAKE_REG(0,0,77)
#define BEEP_CTRL_REG8      MAKE_REG(0,0,78)
#define BEEP_CTRL_REG9      MAKE_REG(0,0,79)

/*ADC Register 1*/
#define ADC_CHN_REG         MAKE_REG(0,0,81)
/*ADC Fine Gain Adjust*/
#define ADC_FGA             MAKE_REG(0,0,82)
/*Left ADC Channel Volume Control*/
#define LADC_VOL            MAKE_REG(0,0,83)
/*Right ADC Channel Volume Control*/
#define RADC_VOL            MAKE_REG(0,0,84)

/*Left Channel AGC Control Register 1*/
#define LEFT_AGC_REG1       MAKE_REG(0,0,86)
/*Left Channel AGC Control Register 2*/
#define LEFT_AGC_REG2       MAKE_REG(0,0,87)
/*Left Channel AGC Control Register 3 */
#define LEFT_AGC_REG3       MAKE_REG(0,0,88)
/*Left Channel AGC Control Register 4 */
#define LEFT_AGC_REG4       MAKE_REG(0,0,89)
/*Left Channel AGC Control Register 5 */
#define LEFT_AGC_REG5       MAKE_REG(0,0,90)
/*Left Channel AGC Control Register 6 */
#define LEFT_AGC_REG6       MAKE_REG(0,0,91)
/*Left Channel AGC Control Register 7 */
#define LEFT_AGC_REG7       MAKE_REG(0,0,92)
/*Right Channel AGC Control Register 1*/
#define RIGHT_AGC_REG1      MAKE_REG(0,0,94)
/*Right Channel AGC Control Register 2*/
#define RIGHT_AGC_REG2      MAKE_REG(0,0,95)
/*Right Channel AGC Control Register 3 */
#define RIGHT_AGC_REG3      MAKE_REG(0,0,96)
/*Right Channel AGC Control Register 4 */
#define RIGHT_AGC_REG4      MAKE_REG(0,0,97)
/*Right Channel AGC Control Register 5 */
#define RIGHT_AGC_REG5      MAKE_REG(0,0,98)
/*Right Channel AGC Control Register 6 */
#define RIGHT_AGC_REG6      MAKE_REG(0,0,99)
/*Right Channel AGC Control Register 7 */
#define RIGHT_AGC_REG7      MAKE_REG(0,0,100)

/******************** Page 1 Registers **************************************/
#define PAGE_1              128
/*Power Conguration*/
#define POW_CFG             MAKE_REG(0,1,1)
/*LDO Control*/
#define LDO_CTRL            MAKE_REG(0,1,2)
/*power control register 2 */
#define PWR_CTRL_REG        MAKE_REG(0,1,2)
/*Output Driver Power Control*/
#define OUT_PWR_CTRL        MAKE_REG(0,1,9)
/* Full Chip Common Mode register */
#define CM_CTRL_REG         MAKE_REG(0,1,10)
/*HPL Routing Selection*/
#define HPL_ROUTE_CTRL      MAKE_REG(0,1,12)
/*HPR Routing Selection*/
#define HPR_ROUTE_CTRL      MAKE_REG(0,1,13)
/*LOL Routing Selection*/
#define LOL_ROUTE_CTRL      MAKE_REG(0,1,14)
/*LOR Routing Selection*/
#define LOR_ROUTE_CTRL      MAKE_REG(0,1,15)
/*HPL Driver Gain*/
#define HPL_GAIN            MAKE_REG(0,1,16)
/*HPR Driver Gain*/
#define HPR_GAIN            MAKE_REG(0,1,17)
/*LOL Driver Gain*/
#define LOL_GAIN            MAKE_REG(0,1,18)
/*LOR Driver Gain*/
#define LOR_GAIN            MAKE_REG(0,1,19)
/*Headphone Driver Startup Control Register*/
#define HPHONE_STARTUP_CTRL MAKE_REG(0,1,20)
/*IN1L to HPL Volume Control Register */
#define IN1L_HPL_VOL_CTRL   MAKE_REG(0,1,22)
/*IN1R to HPR Volume Control Register */
#define IN1R_HPR_VOL_CTRL   MAKE_REG(0,1,23)
/*MAL Volume Control Register*/
#define MAL_CTRL_REG        MAKE_REG(0,1,24)
/*MAR Volume Control Register*/
#define MAR_CTRL_REG        MAKE_REG(0,1,25)
/*MICBIAS Configuration Register*/
#define MICBIAS_CTRL        MAKE_REG(0,1,51)
/*Left MICPGA Positive Terminal Input Routing Configuration Register*/
#define LMICPGA_PIN_CFG     MAKE_REG(0,1,52)
/*Left MICPGA Negative Terminal Input Routing Configuration Register*/
#define LMICPGA_NIN_CFG     MAKE_REG(0,1,54)
/*Right MICPGA Positive Terminal Input Routing Configuration Register*/
#define RMICPGA_PIN_CFG     MAKE_REG(0,1,55)
/*Right MICPGA Negative Terminal Input Routing Configuration Register*/
#define RMICPGA_NIN_CFG     MAKE_REG(0,1,57)
/*Floating Input Configuration Register*/
#define INPUT_CFG_REG       MAKE_REG(0,1,58)
/*Left MICPGA Volume Control Register*/
#define LMICPGA_VOL_CTRL    MAKE_REG(0,1,59)
/*Right MICPGA Volume Control Register*/
#define RMICPGA_VOL_CTRL    MAKE_REG(0,1,60)
#define ANALOG_INPUT_CHARGING_CFG MAKE_REG(0,1,71)
#define REF_PWR_UP_CONF_REG MAKE_REG(0,1,123)
/*charge control register*/
#define CHRG_CTRL_REG       MAKE_REG(0,1,124)
/*headphone driver configuration register*/
#define HP_DRIVER_CONF_REG  MAKE_REG(0,1,125)

/****************************************************************************/
#define BIT7                (0x01 << 7)
#define CODEC_CLKIN_MASK        0x03
#define MCLK_2_CODEC_CLKIN      0x00
#define PLLCLK_2_CODEC_CLKIN            0x03
/*Bclk_in selection*/
#define BDIV_CLKIN_MASK         0x03
#define DAC_MOD_CLK_2_BDIV_CLKIN        0x01
#define ADC_CLK_2_BDIV_CLKIN            0x02
#define SOFT_RESET          0x01
#define PAGE0               0x00
#define PAGE1               0x01
#define DOUT_HIGH_IMP                   0x01
#define BIT_CLK_MASTER          0x08
#define WORD_CLK_MASTER         0x04
#define HIGH_PLL            (0x01 << 6)
#define ENABLE_CLK_MASK         BIT7
#define ENABLE_PLL          BIT7
#define ENABLE_NDAC         BIT7
#define ENABLE_MDAC         BIT7
#define ENABLE_NADC         BIT7
#define ENABLE_MADC         BIT7
#define ENABLE_BCLK         BIT7
#define ENABLE_DAC              (0x03 << 6)
#define LDAC_2_LCHN         (0x01 << 4 )
#define RDAC_2_RCHN         (0x01 << 2 )
#define LDAC_CHNL_2_HPL         (0x01 << 3)
#define RDAC_CHNL_2_HPR         (0x01 << 3)
#define SOFT_STEP_2WCLK         (0x01)
#define MUTE_ON             0x0C
#define DEFAULT_VOL         0x0
#define DISABLE_ANALOG          (0x01 << 3)
#define LDAC_2_HPL_ROUTEON      0x08
#define RDAC_2_HPR_ROUTEON      0x08

/****************************************************************************
 * Mixer control  related #defines
 ***************************************************************************
 */
/*AIC3204*/
#define LDMUTE_ENUM     0
#define SOFTSTEP_ENUM       1
#define DACEXTRA_ENUM       2
#define HSET_ENUM       3
#define LAMUTE_ENUM     4
#define DRC_ENUM        5
#define BEEP_ENUM       6
#define BEEPEXTRA_ENUM      7
#define MICBIAS_ENUM        8

#define IN1L_LMICPGA_ENUM   9
#define IN2L_LMICPGA_ENUM   10
#define IN3L_LMICPGA_ENUM   11
#define IN1R_LMICPGA_ENUM   12

#define CM1L_LMICPGA_ENUM   13
#define IN2R_LMICPGA_ENUM   14
#define IN3R_LMICPGA_ENUM   15
#define CM2L_LMICPGA_ENUM   16

#define IN1R_RMICPGA_ENUM   17
#define IN2R_RMICPGA_ENUM   18
#define IN3R_RMICPGA_ENUM   19
#define IN2L_RMICPGA_ENUM   20

#define CM1R_RMICPGA_ENUM   21
#define IN1L_RMICPGA_ENUM   22
#define IN3L_RMICPGA_ENUM   23
#define CM2R_RMICPGA_ENUM   24

#define LMICPGAGAIN_ENUM    25
#define RMICPGAGAIN_ENUM    26

#define MICBIASPOWER_ENUM       27
#define MICBIASVOLTAGE_ENUM     28
#define LDACPOWER_ENUM      29
#define RDACPOWER_ENUM      30
#define LADCPOWER_ENUM      31
#define RADCPOWER_ENUM      32

#define RDMUTE_ENUM     33
#define RAMUTE_ENUM     34
#define LDACPATH_ENUM       35
#define RDACPATH_ENUM       36
#define HPPOWER_ENUM        37
#define LOPOWER_ENUM        38
#define MAPOWER_ENUM        39
#define LHPMUTE_ENUM        40
#define RHPMUTE_ENUM        41
#define LLOMUTE_ENUM        42
#define RLOMUTE_ENUM        43
#define LAGC_ENUM       44
#define RAGC_ENUM       45

/*ADC3101_A*/

#define ADC_A_MICBIAS1_ENUM     0
#define ADC_A_MICBIAS2_ENUM     1
#define ADC_A_ATTLINEL1_ENUM        2
#define ADC_A_ATTLINEL2_ENUM        3
#define ADC_A_ATTLINEL3_ENUM        4
#define ADC_A_ATTLINER1_ENUM        5
#define ADC_A_ATTLINER2_ENUM        6
#define ADC_A_ATTLINER3_ENUM        7
#define ADC_A_ADCSOFTSTEP_ENUM      8

/*ADC3101_B*/

#define ADC_B_MICBIAS1_ENUM     0
#define ADC_B_MICBIAS2_ENUM     1
#define ADC_B_ATTLINEL1_ENUM        2
#define ADC_B_ATTLINEL2_ENUM        3
#define ADC_B_ATTLINEL3_ENUM        4
#define ADC_B_ATTLINER1_ENUM        5
#define ADC_B_ATTLINER2_ENUM        6
#define ADC_B_ATTLINER3_ENUM        7
#define ADC_B_ADCSOFTSTEP_ENUM      8

#define HP_DRIVER_BUSY_MASK     0x04
/* Headphone driver Configuration Register Page 1, Register 125 */
#define GCHP_ENABLE     0x10
#define DC_OC_FOR_ALL_COMB      0x03
#define DC_OC_FOR_PROG_COMB     0x02

/* Reference Power-Up configuration register */
#define REF_PWR_UP_MASK     0x4
#define AUTO_REF_PWR_UP     0x0
#define FORCED_REF_PWR_UP   0x4

/* Power Configuration register 1 */
#define WEAK_AVDD_TO_DVDD_DIS   0x8

/* Power Configuration register 1 */
#define ANALOG_BLOCK_POWER_CONTROL_MASK     0x08
#define ENABLE_ANALOG_BLOCK 0x0
#define DISABLE_ANALOG_BLOCK    0x8

/* Floating input Configuration register P1_R58 */
#define WEAK_BIAS_INPUTS_MASK   0xFC

/* Common Mode Control Register */
#define GCHP_HPL_STATUS     0x4

/* Audio Interface Register 3 P0_R29 */
#define BCLK_WCLK_BUFFER_POWER_CONTROL_MASK 0x4
#define BCLK_WCLK_BUFFER_ON         0x4

/* Power Configuration Register */
#define AVDD_CONNECTED_TO_DVDD_MASK 0x8
#define DISABLE_AVDD_TO_DVDD        0x8
#define ENABLE_AVDD_TO_DVDD     0x0

/****************************************************************************/
/*          ADC31xx_a Page 0 Registers                              */
/****************************************************************************/
typedef union reg {
    struct adc31xx_reg{
    u8 reserved;
    u8 device;
    u8 page;
    u8 offset;
    }adc31xx_register;
    unsigned int adc31xx_register_int;
}adc31xx_reg_union;



/* Page select register */
#define ADC_PAGE_SELECT(cnum)         MAKE_REG((cnum)+1,0,0)
/* Software reset register */
#define ADC_RESET(cnum)               MAKE_REG((cnum)+1,0,1)

/* 2-3 Reserved */

/* PLL programming register B */
#define ADC_CLKGEN_MUX(cnum)          MAKE_REG((cnum)+1,0,4)
/* PLL P and R-Val */
#define ADC_PLL_PROG_PR(cnum)         MAKE_REG((cnum)+1,0,5)
/* PLL J-Val */
#define ADC_PLL_PROG_J(cnum)          MAKE_REG((cnum)+1,0,6)
/* PLL D-Val MSB */
#define ADC_PLL_PROG_D_MSB(cnum)      MAKE_REG((cnum)+1,0,7)
/* PLL D-Val LSB */
#define ADC_PLL_PROG_D_LSB(cnum)      MAKE_REG((cnum)+1,0,8)

/* 9-17 Reserved */

/* ADC NADC */
#define ADC_ADC_NADC(cnum)            MAKE_REG((cnum)+1,0,18)
/* ADC MADC */
#define ADC_ADC_MADC(cnum)            MAKE_REG((cnum)+1,0,19)
/* ADC AOSR */
#define ADC_ADC_AOSR(cnum)            MAKE_REG((cnum)+1,0,20)
/* ADC IADC */
#define ADC_ADC_IADC(cnum)            MAKE_REG((cnum)+1,0,21)
/* ADC miniDSP engine decimation */
#define ADC_MINIDSP_DECIMATION(cnum)   MAKE_REG((cnum)+1,0,22)

/* 23-24 Reserved */

/* CLKOUT MUX */
#define ADC_CLKOUT_MUX(cnum)      MAKE_REG((cnum)+1,0,25)
/* CLOCKOUT M divider value */
#define ADC_CLKOUT_M_DIV(cnum)      MAKE_REG((cnum)+1,0,26)
/*Audio Interface Setting Register 1*/
#define ADC_INTERFACE_CTRL_1(cnum)     MAKE_REG((cnum)+1,0,27)
/* Data Slot Offset (Ch_Offset_1) */
#define ADC_CH_OFFSET_1(cnum)      MAKE_REG((cnum)+1,0,28)
/* ADC interface control 2 */
#define ADC_INTERFACE_CTRL_2(cnum)     MAKE_REG((cnum)+1,0,29)
/* BCLK N Divider */
#define ADC_BCLK_N_DIV(cnum)      MAKE_REG((cnum)+1,0,30)
/* Secondary audio interface control 1 */
#define ADC_INTERFACE_CTRL_3(cnum)     MAKE_REG((cnum)+1,0,31)
/* Secondary audio interface control 2 */
#define ADC_INTERFACE_CTRL_4(cnum)     MAKE_REG((cnum)+1,0,32)
/* Secondary audio interface control 3 */
#define ADC_INTERFACE_CTRL_5(cnum)     MAKE_REG((cnum)+1,0,33)
/* I2S sync */
#define ADC_I2S_SYNC(cnum)      MAKE_REG((cnum)+1,0,34)

/* 35 Reserved */

/* ADC flag register */
#define ADC_ADC_FLAG(cnum)      MAKE_REG((cnum)+1,0,36)
/* Data slot offset 2 (Ch_Offset_2) */
#define ADC_CH_OFFSET_2(cnum)      MAKE_REG((cnum)+1,0,37)
/* I2S TDM control register */
#define ADC_I2S_TDM_CTRL(cnum)      MAKE_REG((cnum)+1,0,38)

/* 39-41 Reserved */

/* Interrupt flags (overflow) */
#define ADC_INTR_FLAG_1(cnum)      MAKE_REG((cnum)+1,0,42)
/* Interrupt flags (overflow) */
#define ADC_INTR_FLAG_2(cnum)      MAKE_REG((cnum)+1,0,43)

/* 44 Reserved */

/* Interrupt flags ADC */
#define ADC_INTR_FLAG_ADC1(cnum)      MAKE_REG((cnum)+1,0,45)

/* 46 Reserved */

/* Interrupt flags ADC */
#define ADC_INTR_FLAG_ADC2(cnum)      MAKE_REG((cnum)+1,0,47)
/* INT1 interrupt control */
#define ADC_INT1_CTRL(cnum)      MAKE_REG((cnum)+1,0,48)
/* INT2 interrupt control */
#define ADC_INT2_CTRL(cnum)      MAKE_REG((cnum)+1,0,49)

/* 50 Reserved */

/* DMCLK/GPIO2 control */
#define ADC_GPIO2_CTRL(cnum)      MAKE_REG((cnum)+1,0,51)
/* DMDIN/GPIO1 control */
#define ADC_GPIO1_CTRL(cnum)      MAKE_REG((cnum)+1,0,52)
/* DOUT Control */
#define ADC_DOUT_CTRL(cnum)      MAKE_REG((cnum)+1,0,53)

/* 54-56 Reserved */

/* ADC sync control 1 */
#define ADC_SYNC_CTRL_1(cnum)      MAKE_REG((cnum)+1,0,57)
/* ADC sync control 2 */
#define ADC_SYNC_CTRL_2(cnum)      MAKE_REG((cnum)+1,0,58)
/* ADC CIC filter gain control */
#define ADC_CIC_GAIN_CTRL(cnum)      MAKE_REG((cnum)+1,0,59)

/* 60 Reserved */

/* ADC processing block selection  */
#define ADC_PRB_SELECT(cnum)      MAKE_REG((cnum)+1,0,61)
/* Programmable instruction mode control bits */
#define ADC_INST_MODE_CTRL(cnum)      MAKE_REG((cnum)+1,0,62)

/* 63-79 Reserved */

/* Digital microphone polarity control */
#define ADC_MIC_POLARITY_CTRL(cnum)    MAKE_REG((cnum)+1,0,80)
/* ADC Digital */
#define ADC_ADC_DIGITAL(cnum)          MAKE_REG((cnum)+1,0,81)
/* ADC Fine Gain Adjust */
#define ADC_ADC_FGA(cnum)              MAKE_REG((cnum)+1,0,82)
/* Left ADC Channel Volume Control */
#define ADC_LADC_VOL(cnum)             MAKE_REG((cnum)+1,0,83)
/* Right ADC Channel Volume Control */
#define ADC_RADC_VOL(cnum)             MAKE_REG((cnum)+1,0,84)
/* ADC phase compensation */
#define ADC_ADC_PHASE_COMP(cnum)       MAKE_REG((cnum)+1,0,85)
/* Left Channel AGC Control Register 1 */
#define ADC_LEFT_CHN_AGC_1(cnum)       MAKE_REG((cnum)+1,0,86)
/* Left Channel AGC Control Register 2 */
#define ADC_LEFT_CHN_AGC_2(cnum)       MAKE_REG((cnum)+1,0,87)
/* Left Channel AGC Control Register 3 */
#define ADC_LEFT_CHN_AGC_3(cnum)       MAKE_REG((cnum)+1,0,88)
/* Left Channel AGC Control Register 4 */
#define ADC_LEFT_CHN_AGC_4(cnum)       MAKE_REG((cnum)+1,0,89)
/* Left Channel AGC Control Register 5 */
#define ADC_LEFT_CHN_AGC_5(cnum)       MAKE_REG((cnum)+1,0,90)
/* Left Channel AGC Control Register 6 */
#define ADC_LEFT_CHN_AGC_6(cnum)       MAKE_REG((cnum)+1,0,91)
/* Left Channel AGC Control Register 7 */
#define ADC_LEFT_CHN_AGC_7(cnum)       MAKE_REG((cnum)+1,0,92)
/* Left AGC gain */
#define ADC_LEFT_AGC_GAIN(cnum)        MAKE_REG((cnum)+1,0,93)
/* Right Channel AGC Control Register 1 */
#define ADC_RIGHT_CHN_AGC_1(cnum)      MAKE_REG((cnum)+1,0,94)
/* Right Channel AGC Control Register 2 */
#define ADC_RIGHT_CHN_AGC_2(cnum)      MAKE_REG((cnum)+1,0,95)
/* Right Channel AGC Control Register 3 */
#define ADC_RIGHT_CHN_AGC_3(cnum)      MAKE_REG((cnum)+1,0,96)
/* Right Channel AGC Control Register 4 */
#define ADC_RIGHT_CHN_AGC_4(cnum)      MAKE_REG((cnum)+1,0,97)
/* Right Channel AGC Control Register 5 */
#define ADC_RIGHT_CHN_AGC_5(cnum)      MAKE_REG((cnum)+1,0,98)
/* Right Channel AGC Control Register 6 */
#define ADC_RIGHT_CHN_AGC_6(cnum)      MAKE_REG((cnum)+1,0,99)
/* Right Channel AGC Control Register 7 */
#define ADC_RIGHT_CHN_AGC_7(cnum)      MAKE_REG((cnum)+1,0,100)
/* Right AGC gain */
#define ADC_RIGHT_AGC_GAIN(cnum)       MAKE_REG((cnum)+1,0,101)

/* 102-127 Reserved */

/****************************************************************************/
/*                           Page 1 Registers                               */
/****************************************************************************/
#define ADC_PAGE_1                    128

/* 1-25 Reserved */

/* Dither control */
#define ADC_DITHER_CTRL(cnum)          MAKE_REG((cnum)+1,1,26)

/* 27-50 Reserved */

/* MICBIAS Configuration Register */
#define ADC_MICBIAS_CTRL(cnum)         MAKE_REG((cnum)+1,1,51)
/* Left ADC input selection for Left PGA */
#define ADC_LEFT_PGA_SEL_1(cnum)       MAKE_REG((cnum)+1,1,52)

/* 53 Reserved */

/* Right ADC input selection for Left PGA */
#define ADC_LEFT_PGA_SEL_2(cnum)       MAKE_REG((cnum)+1,1,54)
/* Right ADC input selection for right PGA */
#define ADC_RIGHT_PGA_SEL_1(cnum)      MAKE_REG((cnum)+1,1,55)

/* 56 Reserved */

/* Right ADC input selection for right PGA */
#define ADC_RIGHT_PGA_SEL_2(cnum)      MAKE_REG((cnum)+1,1,57)

/* 58 Reserved */

/* Left analog PGA settings */
#define ADC_LEFT_APGA_CTRL(cnum)       MAKE_REG((cnum)+1,1,59)
/* Right analog PGA settings */
#define ADC_RIGHT_APGA_CTRL(cnum)      MAKE_REG((cnum)+1,1,60)
/* ADC Low current Modes */
#define ADC_LOW_CURRENT_MODES(cnum)    MAKE_REG((cnum)+1,1,61)
/* ADC analog PGA flags */
#define ADC_ANALOG_PGA_FLAGS(cnum)     MAKE_REG((cnum)+1,1,62)

/* 63-127 Reserved */

/****************************************************************************/
/*              Macros and definitions                              */
/****************************************************************************/

/* ADC31xx register space */
#define ADC31xx_CACHEREGNUM     192
#define ADC31xx_PAGE_SIZE       128

#define ADC31xx_RATES   SNDRV_PCM_RATE_8000_96000
#define ADC31xx_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
             SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

/* bits defined for easy usage */
#define D7                    (0x01 << 7)
#define D6                    (0x01 << 6)
#define D5                    (0x01 << 5)
#define D4                    (0x01 << 4)
#define D3                    (0x01 << 3)
#define D2                    (0x01 << 2)
#define D1                    (0x01 << 1)
#define D0                    (0x01 << 0)

/****************************************************************************/
/*              ADC31xx Register bits                               */
/****************************************************************************/
/* PLL Enable bits */
#define ADC_ENABLE_PLL              D7
#define ADC_ENABLE_NADC             D7
#define ADC_ENABLE_MADC             D7
#define ADC_ENABLE_BCLK             D7

/* Power bits */
#define ADC_LADC_PWR_ON             D7
#define ADC_RADC_PWR_ON             D6

#define ADC_SOFT_RESET              D0
#define ADC_BCLK_MASTER             D3
#define ADC_WCLK_MASTER             D2

/* Interface register masks */
#define ADC_FMT_MASK                (D7|D6|D3|D2)
#define ADC_WLENGTH_MASK            (D5|D4)

/* PLL P/R bit offsets */
#define ADC_PLLP_SHIFT          4
#define ADC_PLLR_SHIFT          0
#define ADC_PLL_PR_MASK         0x7F
#define ADC_PLLJ_MASK           0x3F
#define ADC_PLLD_MSB_MASK       0x3F
#define ADC_PLLD_LSB_MASK       0xFF
#define ADC_NADC_MASK           0x7F
#define ADC_MADC_MASK           0x7F
#define ADC_AOSR_MASK           0xFF
#define ADC_IADC_MASK           0xFF
#define ADC_BDIV_MASK           0x7F

/* PLL_CLKIN bits */
#define ADC_PLL_CLKIN_SHIFT         2
#define ADC_PLL_CLKIN_MCLK          0x0
#define ADC_PLL_CLKIN_BCLK          0x1
#define ADC_PLL_CLKIN_ZERO          0x3

/* CODEC_CLKIN bits */
#define ADC_CODEC_CLKIN_SHIFT       0
#define ADC_CODEC_CLKIN_MCLK        0x0
#define ADC_CODEC_CLKIN_BCLK        0x1
#define ADC_CODEC_CLKIN_PLL_CLK     0x3

#define ADC_USE_PLL                 (ADC_PLL_CLKIN_MCLK << ADC_PLL_CLKIN_SHIFT) |   \
                                (ADC_CODEC_CLKIN_PLL_CLK << ADC_CODEC_CLKIN_SHIFT)

/*  Analog PGA control bits */
#define ADC_LPGA_MUTE               D7
#define ADC_RPGA_MUTE               D7

#define ADC_LPGA_GAIN_MASK          0x7F
#define ADC_RPGA_GAIN_MASK          0x7F

/* ADC current modes */
#define ADC_LOW_CURR_MODE       D0

/* Left ADC Input selection bits */
#define ADC_LCH_SEL1_SHIFT          0
#define ADC_LCH_SEL2_SHIFT          2
#define ADC_LCH_SEL3_SHIFT          4
#define ADC_LCH_SEL4_SHIFT          6

#define ADC_LCH_SEL1X_SHIFT         0
#define ADC_LCH_SEL2X_SHIFT         2
#define ADC_LCH_SEL3X_SHIFT         4
#define ADC_LCH_COMMON_MODE         D6
#define ADC_BYPASS_LPGA             D7

/* Right ADC Input selection bits */
#define ADC_RCH_SEL1_SHIFT          0
#define ADC_RCH_SEL2_SHIFT          2
#define ADC_RCH_SEL3_SHIFT          4
#define ADC_RCH_SEL4_SHIFT          6

#define ADC_RCH_SEL1X_SHIFT         0
#define ADC_RCH_SEL2X_SHIFT         2
#define ADC_RCH_SEL3X_SHIFT         4
#define ADC_RCH_COMMON_MODE         D6
#define ADC_BYPASS_RPGA             D7

/* MICBIAS control bits */
#define ADC_MICBIAS1_SHIFT          5
#define ADC_MICBIAS2_SHIFT          3

#define ADC_MAX_VOLUME          64
#define ADC_POS_VOL         24

/*
 *****************************************************************************
 * Structures Definitions
 *****************************************************************************
 */
/*
 *----------------------------------------------------------------------------
 * @struct  aic32xx_setup_data |
 *          i2c specific data setup for AIC32xx.
 * @field   unsigned short |i2c_address |
 *          Unsigned short for i2c address.
 *----------------------------------------------------------------------------
 */
    struct aic32xx_setup_data {
    unsigned short i2c_address;
};

/*
 *----------------------------------------------------------------------------
 * @struct  aic32xx_priv |
 *          AIC32xx priviate data structure to set the system clock, mode and
 *          page number.
 * @field   u32 | sysclk |
 *          system clock
 * @field   s32 | master |
 *          master/slave mode setting for AIC32xx
 * @field   u8 | page_no |
 *          page number. Here, page 0 and page 1 are used.
 *----------------------------------------------------------------------------
 */
struct aic32xx_priv {
    u32 sysclk;
    s32 master;
    u8 page_no;
    u8 adc_page_no[NUM_ADC3101];
        u8 device_no;
    int pll_divs_index;
    struct i2c_client *control_data;
    struct i2c_client *adc_control_data[NUM_ADC3101];
    struct mutex codecMutex;
    int line_out_status;
};

/*
 *----------------------------------------------------------------------------
 * @struct  aic32xx_configs |
 *          AIC32xx initialization data which has register offset and register
 *          value.
 * @field   u16 | reg_offset |
 *          AIC32xx Register offsets required for initialization..
 * @field   u8 | reg_val |
 *          value to set the AIC32xx register to initialize the AIC32xx.
 *----------------------------------------------------------------------------
 */
struct aic32xx_configs {
    u32 reg_offset;
    u8 reg_val;
};

/*
 * adc3xxx initialization data
 * This structure initialization contains the initialization required for
 * ADC3xxx.
 * These registers values (reg_val) are written into the respective ADC3xxx
 * register offset (reg_offset) to  initialize ADC3xxx.
 * These values are used in adc3xxx_init() function only.
 */
struct adc31xx_configs {
    u32 reg_offset;
    u8 reg_val;
};

/*
 *----------------------------------------------------------------------------
 * @struct  aic32xx_rate_divs |
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
 * @field   u32 | aic32xx_configs |
 *          configurations for aic32xx register value
 *----------------------------------------------------------------------------
 */
struct aic32xx_rate_divs {
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
    struct aic32xx_configs codec_specific_regs[NO_FEATURE_REGS];
};

/****************** RATES TABLE FOR ADC31xx ************************/
struct adc31xx_rate_divs {
    u32 mclk;
    u32 rate;
    u8 pll_p;
    u8 pll_r;
    u8 pll_j;
    u16 pll_d;
    u8 nadc;
    u8 madc;
    u8 aosr;
    u8 bdiv_n;
    u8 iadc;
};

/*
 *----------------------------------------------------------------------------
 * @struct  snd_soc_codec_dai |
 *          It is SoC Codec DAI structure which has DAI capabilities viz.,
 *          playback and capture, DAI runtime information viz. state of DAI
 *          and pop wait state, and DAI private data.
 *----------------------------------------------------------------------------
 */
extern struct snd_soc_dai tlv320aic32xx_dai;

/*
 *----------------------------------------------------------------------------
 * @struct  snd_soc_codec_device |
 *          This structure is soc audio codec device sturecute which pointer
 *          to basic functions aic32xx_probe(), aic32xx_remove(),
 *          aic32xx_suspend() and aic32xx_resume()
 *
 */
extern struct snd_soc_codec_device soc_codec_dev_aic32xx;

void aic32xx_set_sel(struct snd_soc_codec *codec, int enabled);

#endif              /* _TLV320AIC32xx_H */
