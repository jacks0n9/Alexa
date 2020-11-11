
/*
 * linux/sound/soc/codecs/tlv320aic32xx.c
 *
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 *
 * Based on sound/soc/codecs/wm8753.c by Liam Girdwood
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * Includes
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>

#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include <mach/gpio.h>

#include <plat/board-omap3misto-id.h>

#include "tlv320aic32xx.h"

/*
 * enable debug prints in the driver
 */

// Microphone analog gain in half dB
#define MIC_PGA_GAIN        40      // 20dB Gain
#define MIC_PGA_GAIN_IDC    40      // 20db Gain

//#define CONFIG_SND_SOC_TLV320AIC32XX_DEBUG
#define AMP_ENABLE_GPIO 129
#define LINEOUT_SEL_GPIO 71

// HP gain as register values
#define P_HP_GAIN_LOCAL_SPEAKER   0x3A    // -6dB Gain
#define P_HP_GAIN_LINE_OUT        0x05    // +5dB Gain

// Uncomment to disable DRC
//#define SET_DRC_OFF

#ifdef CONFIG_SND_SOC_TLV320AIC32XX_DEBUG
#    define dprintk(x...)   printk(x)
#else
#    define dprintk(x...)
#endif


/*
*****************************************************************************
* Function Prototype
*****************************************************************************
*/
inline void aic32xx_write_reg_cache(struct snd_soc_codec *codec, u8 device,
                                    u8 page, u16 reg, u8 value);
int aic3i25x_reset_cache(struct snd_soc_codec *codec);
static int aic32xx_hw_params_capt(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *);
static int aic32xx_hw_params_play(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *);
static int aic32xx_mute_capt(struct snd_soc_dai *dai, int mute);
static int aic32xx_mute_play(struct snd_soc_dai *dai, int mute);

static int aic32xx_set_dai_sysclk(struct snd_soc_dai *codec_dai,
                                  int clk_id, unsigned int freq, int dir);
static int aic32xx_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt);

static int aic32xx_trigger(struct snd_pcm_substream *substream, int cmd,
                           struct snd_soc_dai *);
static int aic32xx_set_bias_level(struct snd_soc_codec *codec,
                                  enum snd_soc_bias_level level);
static unsigned int aic32xx_hw_read(struct snd_soc_codec *codec,
                                    unsigned int count);
static unsigned int aic32xx_read_reg_cache(struct snd_soc_codec *codec,
                                           unsigned int reg);
static int aic32xx_write(struct snd_soc_codec *codec,
                         unsigned int reg, unsigned int value);

static int __amp_enable_info(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_info *uinfo);
static int __amp_enable_get(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol);
static int __amp_enable_put(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol);

static int __IDC_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo);
static int __IDC_get( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
static int __IDC_put( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);

int aic32xx_change_page(struct snd_soc_codec *codec, u8 new_page);

// Headphone driver in GC mode
static int aic32xx_hp_event(struct snd_soc_dapm_widget *w,
                            struct snd_kcontrol *kcontrol, int event);

/*
*****************************************************************************
* Global Variable
*****************************************************************************
*/
#if 0
static u8 aic32xx_reg_ctl;
static u8 adc31xx_reg_ctl_a;
static u8 adc31xx_reg_ctl_b;
#endif
static struct i2c_client *control_data[];
static int amp_enable;
static int board_is_p = 0;

/*
*****************************************************************************
* Structure Declaration
*****************************************************************************
*/

static const char *mute[] = { "Unmuted", "Muted" };
static const char *dacvolume_extra[] =
    { "L&R Ind Vol", "LVol=RVol", "RVol=LVol" };
static const char *dacsoftstep_control[] =
    { "1 step/sample", "1 step/2 sample", "disabled" };
static const char *drc_enable[] = { "Disabled", "Enabled" };
static const char *beep_generator[] = { "Disabled", "Enabled" };
static const char *beepvolume_extra[] =
    { "L&R Ind Vol", "LVol=RVol", "RVol=LVol" };
static const char *micbias_voltage_aic[] =
    { "1.04/1.25V", "1.425/1.7V", "2.075/2.5V", "POWER SUPPY" };
static const char *micpga_selection[] = { "off", "10k", "20k", "40k" };
static const char *micpgagain_enable[] = { "Disabled", "Enabled" };
static const char *powerup[] = { "Power Down", "Power Up" };
static const char *vol_generation[] = { "AVDD", "LDOIN" };
static const char *path_control[] =
    { "Disabled", "LDAC Data", "RDAC Data", "L&RDAC Data" };
static const char *agc_enable[] = { "Disabled", "Enabled" };
static const char *headset_detection[] = { "Disabled", "Enabled" };
static const char *headset_detect_debounce_time_text[] = {
    "16 ms",
    "32 ms",
    "64 ms",
    "128 ms",
    "256 ms",
    "512 ms",
};

static const struct soc_enum headset_detect_debounce_time =
    SOC_ENUM_SINGLE(HEADSET_DETECT, 4, 6, headset_detect_debounce_time_text);
static const char *headset_button_press_debounce_time_text[] = {
    "disabled",
    "8 ms",
    "16 ms",
    "32 ms",
};

static const struct soc_enum headset_button_press_debounce_time =
    SOC_ENUM_SINGLE(HEADSET_DETECT, 1, 4,
                    headset_button_press_debounce_time_text);
static const DECLARE_TLV_DB_SCALE(drc_threshold, -2400, 300, 0);
static const DECLARE_TLV_DB_SCALE(drc_hysteresis, 0, 100, 0);
static const char *drc_atk_text[] = {
    "4dB per DAC Word Clock"
        "2dB per DAC Word Clock"
        "1dB per DAC Word Clock"
        "0.5dB per DAC Word Clock"
        "0.25dB per DAC Word Clock"
        "0.125dB per DAC Word Clock"
        "0.0625dB per DAC Word Clock"
        "3.125e-2dB per DAC Word Clock"
        "1.562e-2dB per DAC Word Clock"
        "7.812e-3dB per DAC Word Clock"
        "3.906e-3dB per DAC Word Clock"
        "1.953e-3dB per DAC Word Clock"
        "9.765e-4dB per DAC Word Clock"
        "4.882e-4dB per DAC Word Clock"
        "2.414e-4dB per DAC Word Clock" "1.220e-4dB per DAC Word Clock"
};

static const struct soc_enum drc_atk =
SOC_ENUM_SINGLE(DRC_CTRL_REG3, 7, 16, drc_atk_text);

static const char *drc_dcy_text[] = {
    "1.5625e-2dB per DAC Word Clock"
        "7.8125e-3dB per DAC Word Clock"
        "3.9062e-3dB per DAC Word Clock"
        "1.9531e-3dB per DAC Word Clock"
        "9.7656e-4dB per DAC Word Clock"
        "4.8823e-4dB per DAC Word Clock"
        "2.4141e-4dB per DAC Word Clock"
        "1.220e-4dB per DAC Word Clock"
        "6.1035e-5dB per DAC Word Clock"
        "3.051e-5dB per DAC Word Clock"
        "1.525e-5dB per DAC Word Clock"
        "7.629e-6dB per DAC Word Clock"
        "3.814e-6dB per DAC Word Clock"
        "1.907e-6dB per DAC Word Clock"
        "9.5367e-7dB per DAC Word Clock" "4.7683e-7dB per DAC Word Clock"
};

static const struct soc_enum drc_dcy =
SOC_ENUM_SINGLE(DRC_CTRL_REG3, 3, 16, drc_dcy_text);
#if 0
static const char *dc_offset_correction_text[] = {
    "disabled",
    "Reserved",
    "All enabled signal routings",
    "All possible signal routings",
};
static const char *hp_driver_power_conf_text[] = {
    "100%",
    "75%",
    "50%",
    "25%",
};
#endif

/* adc3101 */
static const char *micbias_voltage_adc[] = { "off", "2V", "2.5V", "AVDD" };
static const char *linein_attenuation[] = { "0db", "-6db" };
static const char *adc_softstepping[] = { "1 step", "2 step", "off" };

/*Creates an array of the Single Ended Widgets */
static const struct soc_enum aic32xx_enum[] = {
    SOC_ENUM_SINGLE(DAC_MUTE_CTRL_REG, 3, 2, mute),

    SOC_ENUM_SINGLE(DAC_CHN_REG, 0, 3, dacsoftstep_control),
    SOC_ENUM_SINGLE(DAC_MUTE_CTRL_REG, 0, 4, dacvolume_extra),
    SOC_ENUM_SINGLE(HEADSET_DETECT, 7, 2, headset_detection),
    SOC_ENUM_SINGLE(ADC_FGA, 7, 2, mute),
    SOC_ENUM_DOUBLE(DRC_CTRL_REG1, 6, 5, 2, drc_enable),
    SOC_ENUM_SINGLE(BEEP_CTRL_REG1, 7, 2, beep_generator),
    SOC_ENUM_SINGLE(BEEP_CTRL_REG2, 6, 3, beepvolume_extra),
    SOC_ENUM_SINGLE(MICBIAS_CTRL, 4, 4, micbias_voltage_aic),

    SOC_ENUM_SINGLE(LMICPGA_PIN_CFG, 6, 4, micpga_selection),
    SOC_ENUM_SINGLE(LMICPGA_PIN_CFG, 4, 4, micpga_selection),
    SOC_ENUM_SINGLE(LMICPGA_PIN_CFG, 2, 4, micpga_selection),
    SOC_ENUM_SINGLE(LMICPGA_PIN_CFG, 0, 4, micpga_selection),
    SOC_ENUM_SINGLE(LMICPGA_NIN_CFG, 6, 4, micpga_selection),
    SOC_ENUM_SINGLE(LMICPGA_NIN_CFG, 4, 4, micpga_selection),
    SOC_ENUM_SINGLE(LMICPGA_NIN_CFG, 2, 4, micpga_selection),
    SOC_ENUM_SINGLE(LMICPGA_NIN_CFG, 0, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_PIN_CFG, 6, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_PIN_CFG, 4, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_PIN_CFG, 2, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_PIN_CFG, 0, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_NIN_CFG, 6, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_NIN_CFG, 4, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_NIN_CFG, 2, 4, micpga_selection),
    SOC_ENUM_SINGLE(RMICPGA_NIN_CFG, 0, 4, micpga_selection),

    SOC_ENUM_SINGLE(LMICPGA_VOL_CTRL, 7, 2, micpgagain_enable),
    SOC_ENUM_SINGLE(RMICPGA_VOL_CTRL, 7, 2, micpgagain_enable),

    SOC_ENUM_SINGLE(MICBIAS_CTRL, 6, 2, powerup),
    SOC_ENUM_SINGLE(MICBIAS_CTRL, 3, 2, vol_generation),
    SOC_ENUM_SINGLE(ADC_CHN_REG, 7, 2, powerup),
    SOC_ENUM_SINGLE(ADC_CHN_REG, 6, 2, powerup),
    SOC_ENUM_SINGLE(DAC_CHN_REG, 7, 2, powerup),
    SOC_ENUM_SINGLE(DAC_CHN_REG, 6, 2, powerup),

    SOC_ENUM_SINGLE(DAC_MUTE_CTRL_REG, 2, 2, mute),
    SOC_ENUM_SINGLE(ADC_FGA, 3, 2, mute),
    SOC_ENUM_SINGLE(DAC_CHN_REG, 4, 4, path_control),
    SOC_ENUM_SINGLE(DAC_CHN_REG, 2, 4, path_control),
    SOC_ENUM_DOUBLE(OUT_PWR_CTRL, 5, 4, 2, powerup),
    SOC_ENUM_DOUBLE(OUT_PWR_CTRL, 3, 2, 2, powerup),
    SOC_ENUM_DOUBLE(OUT_PWR_CTRL, 1, 0, 2, powerup),
    SOC_ENUM_SINGLE(HPL_GAIN, 6, 2, mute),
    SOC_ENUM_SINGLE(HPR_GAIN, 6, 2, mute),
    SOC_ENUM_SINGLE(LEFT_AGC_REG1, 7, 2, agc_enable),
    SOC_ENUM_SINGLE(RIGHT_AGC_REG1, 7, 2, agc_enable),
};


/* Creates an array of the Single Ended Widgets for ADC3101_A*/
static const struct soc_enum adc31xx_enum_a[] = {
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(0)     , 5, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(0)     , 3, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(0)   , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(0)   , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(0)   , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(0)  , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(0)  , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(0)  , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_ADC_DIGITAL(0)      , 0, 3, adc_softstepping),
};

/* Creates an array of the Single Ended Widgets for ADC3101_B*/
static const struct soc_enum adc31xx_enum_b[] = {
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(1)     , 5, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(1)     , 3, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(1)   , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(1)   , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(1)   , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(1)  , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(1)  , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(1)  , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_ADC_DIGITAL(1)      , 0, 3, adc_softstepping),
};
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
// Adding two ADC3101

/* Creates an array of the Single Ended Widgets for ADC3101_C*/
static const struct soc_enum adc31xx_enum_c[] = {
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(2)     , 5, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(2)     , 3, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(2)   , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(2)   , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(2)   , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(2)  , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(2)  , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(2)  , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_ADC_DIGITAL(2)      , 0, 3, adc_softstepping),
};

/* Creates an array of the Single Ended Widgets for ADC3101_D*/
static const struct soc_enum adc31xx_enum_d[] = {
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(3)     , 5, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_MICBIAS_CTRL(3)     , 3, 4, micbias_voltage_adc),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(3)   , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(3)   , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_LEFT_PGA_SEL_1(3)   , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(3)  , 0, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(3)  , 2, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_RIGHT_PGA_SEL_1(3)  , 4, 2, linein_attenuation),
    SOC_ENUM_SINGLE(ADC_ADC_DIGITAL(3)      , 0, 3, adc_softstepping),
};

#endif


static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -6350, 50, 0);
static const DECLARE_TLV_DB_SCALE(hp_dac_vol_tlv, -600, 100, 2900);
static const DECLARE_TLV_DB_SCALE(adc_vol_tlv, -1200, 50, 0);
static const DECLARE_TLV_DB_SCALE(adc_3101_vol_tlv, -1200, 50, 0);
static const DECLARE_TLV_DB_SCALE(output_gain_tlv, -600, 100, 0);
static const DECLARE_TLV_DB_SCALE(micpga_gain_tlv, 0, 50, 0);
static const DECLARE_TLV_DB_SCALE(in1_hp_gain_tlv, -7230, 50, 1);


/*
 * Structure Initialization
 */
static const struct snd_kcontrol_new aic32xx_snd_controls[] = {


    /* Left/Right DAC Digital Volume Control */
    SOC_DOUBLE_R_SX_TLV("3203 DAC Digital Volume Control",
                        LDAC_VOL, RDAC_VOL, 8, 0xffffff81, 0x30, dac_vol_tlv),
    /*HP gain */
    SOC_DOUBLE_R_TLV("3203 HP Analog Volume Control",
                        HPL_GAIN, HPR_GAIN, 0, 0x3A, 0, hp_dac_vol_tlv),

    SOC_DOUBLE_R_SX_TLV("3101A ADC Digital Volume Control",
        ADC_LADC_VOL(0), ADC_RADC_VOL(0), 7, 0xffffff68, 0x28, adc_3101_vol_tlv),
    SOC_SINGLE("3101A ADC Left Fine Volume",    ADC_ADC_FGA(0), 4, 4, 1),
    SOC_SINGLE("3101A ADC Right Fine Volume",   ADC_ADC_FGA(0), 0, 4, 1),

    SOC_DOUBLE_R_SX_TLV("3101B ADC Digital Volume Control",
        ADC_LADC_VOL(1), ADC_RADC_VOL(1), 7, 0xffffff68, 0x28, adc_3101_vol_tlv),
    SOC_SINGLE("3101B ADC Left Fine Volume",    ADC_ADC_FGA(1), 4, 4, 1),
    SOC_SINGLE("3101B ADC Right Fine Volume",   ADC_ADC_FGA(1), 0, 4, 1),
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
    // Add two more 3101
    SOC_DOUBLE_R_SX_TLV("3101C ADC Digital Volume Control",
        ADC_LADC_VOL(2), ADC_RADC_VOL(2), 7, 0xffffff68, 0x28, adc_3101_vol_tlv),
    SOC_SINGLE("3101C ADC Left Fine Volume",    ADC_ADC_FGA(2), 4, 4, 1),
    SOC_SINGLE("3101C ADC Right Fine Volume",   ADC_ADC_FGA(2), 0, 4, 1),

    SOC_DOUBLE_R_SX_TLV("3101D ADC Digital Volume Control",
        ADC_LADC_VOL(3), ADC_RADC_VOL(3), 7, 0xffffff68, 0x28, adc_3101_vol_tlv),
    SOC_SINGLE("3101D ADC Left Fine Volume",    ADC_ADC_FGA(3), 4, 4, 1),
    SOC_SINGLE("3101D ADC Right Fine Volume",   ADC_ADC_FGA(3), 0, 4, 1),
#endif

    /* Left DAC Mute Control */
    SOC_ENUM("3203 Left DAC Mute Control", aic32xx_enum[LDMUTE_ENUM]),

    /* Right DAC Mute Control */
    SOC_ENUM("3203 Right DAC Mute Control", aic32xx_enum[RDMUTE_ENUM]),

    /* sound new kcontrol for Programming the registers from user space */
    {
        .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
        .name = "Amp Enable",
        .info = __amp_enable_info,
        .get = __amp_enable_get,
        .put = __amp_enable_put,
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
    },

   SOC_DOUBLE_R_TLV("3101 ADC_A MICPGA Volume Ctrl",
       ADC_LEFT_APGA_CTRL(0), ADC_RIGHT_APGA_CTRL(0), 0, 0x50, 0, micpga_gain_tlv),

   SOC_DOUBLE_R_TLV("3101 ADC_B MICPGA Volume Ctrl",
        ADC_LEFT_APGA_CTRL(1), ADC_RIGHT_APGA_CTRL(1), 0, 0x50, 0, micpga_gain_tlv),

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
   SOC_DOUBLE_R_TLV("3101 ADC_C MICPGA Volume Ctrl",
        ADC_LEFT_APGA_CTRL(2), ADC_RIGHT_APGA_CTRL(2), 0, 0x50, 0, micpga_gain_tlv),

   SOC_DOUBLE_R_TLV("3101 ADC_D MICPGA Volume Ctrl",
        ADC_LEFT_APGA_CTRL(3), ADC_RIGHT_APGA_CTRL(3), 0, 0x50, 0, micpga_gain_tlv),
#endif

    /* Control to enable Internal Data Collection from user space */
    {
        .iface  = SNDRV_CTL_ELEM_IFACE_MIXER,
        .name   = "IDC Enable",
        .info   = __IDC_info,
        .get    = __IDC_get,
        .put    = __IDC_put,
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
    },

    SOC_ENUM("DRC Control", aic32xx_enum[DRC_ENUM])
};

/* ADC A Left input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_a_left_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_A IN1_L switch", ADC_LEFT_PGA_SEL_1(0), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A IN2_L switch", ADC_LEFT_PGA_SEL_1(0), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A IN3_L switch", ADC_LEFT_PGA_SEL_1(0), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A DIF1_L switch", ADC_LEFT_PGA_SEL_1(0), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A DIF2_L switch", ADC_LEFT_PGA_SEL_2(0), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A DIF3_L switch", ADC_LEFT_PGA_SEL_2(0), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A IN1_R switch", ADC_LEFT_PGA_SEL_2(0), 1, 0x1, 1),
};

/* ADC A Right input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_a_right_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_A IN1_R switch", ADC_RIGHT_PGA_SEL_1(0), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A IN2_R switch", ADC_RIGHT_PGA_SEL_1(0), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A IN3_R switch", ADC_RIGHT_PGA_SEL_1(0), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A DIF1_R switch", ADC_RIGHT_PGA_SEL_1(0), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A DIF2_R switch", ADC_RIGHT_PGA_SEL_2(0), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A DIF3_R switch", ADC_RIGHT_PGA_SEL_2(0), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_A IN1_L switch", ADC_RIGHT_PGA_SEL_2(0), 1, 0x1, 1),
};

/* Left Digital Mic input for left ADC_A */
static const struct snd_kcontrol_new adc_a_left_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_A Left ADC switch", ADC_ADC_DIGITAL(0), 3, 0x1, 0),
};

/* Right Digital Mic input for Right ADC_A */
static const struct snd_kcontrol_new adc_a_right_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_A Right ADC switch", ADC_ADC_DIGITAL(0), 2, 0x1, 0),
};

/* ADC B Left input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_b_left_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_B IN1_L switch", ADC_LEFT_PGA_SEL_1(1), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B IN2_L switch", ADC_LEFT_PGA_SEL_1(1), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B IN3_L switch", ADC_LEFT_PGA_SEL_1(1), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B DIF1_L switch", ADC_LEFT_PGA_SEL_1(1), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B DIF2_L switch", ADC_LEFT_PGA_SEL_2(1), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B DIF3_L switch", ADC_LEFT_PGA_SEL_2(1), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B IN1_R switch", ADC_LEFT_PGA_SEL_2(1), 1, 0x1, 1),
};

/* ADC B Right input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_b_right_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_B IN1_R switch", ADC_RIGHT_PGA_SEL_1(1), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B IN2_R switch", ADC_RIGHT_PGA_SEL_1(1), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B IN3_R switch", ADC_RIGHT_PGA_SEL_1(1), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B DIF1_R switch", ADC_RIGHT_PGA_SEL_1(1), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B DIF2_R switch", ADC_RIGHT_PGA_SEL_2(1), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B DIF3_R switch", ADC_RIGHT_PGA_SEL_2(1), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_B IN1_L switch", ADC_RIGHT_PGA_SEL_2(1), 1, 0x1, 1),
};

/* Left Digital Mic input for left ADC B */
static const struct snd_kcontrol_new adc_b_left_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_B Left ADC switch", ADC_ADC_DIGITAL(1), 3, 0x1, 0),
};

/* Right Digital Mic input for Right ADC B */
static const struct snd_kcontrol_new adc_b_right_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_B Right ADC switch", ADC_ADC_DIGITAL(1), 2, 0x1, 0),
};

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
// Add two ADC 3101
//
/* ADC B Left input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_c_left_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_C IN1_L switch", ADC_LEFT_PGA_SEL_1(2), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C IN2_L switch", ADC_LEFT_PGA_SEL_1(2), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C IN3_L switch", ADC_LEFT_PGA_SEL_1(2), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C DIF1_L switch", ADC_LEFT_PGA_SEL_1(2), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C DIF2_L switch", ADC_LEFT_PGA_SEL_2(2), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C DIF3_L switch", ADC_LEFT_PGA_SEL_2(2), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C IN1_R switch", ADC_LEFT_PGA_SEL_2(2), 1, 0x1, 1),
};

/* ADC B Right input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_c_right_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_C IN1_R switch", ADC_RIGHT_PGA_SEL_1(2), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C IN2_R switch", ADC_RIGHT_PGA_SEL_1(2), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C IN3_R switch", ADC_RIGHT_PGA_SEL_1(2), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C DIF1_R switch", ADC_RIGHT_PGA_SEL_1(2), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C DIF2_R switch", ADC_RIGHT_PGA_SEL_2(2), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C DIF3_R switch", ADC_RIGHT_PGA_SEL_2(2), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_C IN1_L switch", ADC_RIGHT_PGA_SEL_2(2), 1, 0x1, 1),
};

/* Left Digital Mic input for left ADC B */
static const struct snd_kcontrol_new adc_c_left_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_C Left ADC switch", ADC_ADC_DIGITAL(2), 3, 0x1, 0),
};

/* Right Digital Mic input for Right ADC B */
static const struct snd_kcontrol_new adc_c_right_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_C Right ADC switch", ADC_ADC_DIGITAL(2), 2, 0x1, 0),
};


/* ADC B Left input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_d_left_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_D IN1_L switch", ADC_LEFT_PGA_SEL_1(3), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D IN2_L switch", ADC_LEFT_PGA_SEL_1(3), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D IN3_L switch", ADC_LEFT_PGA_SEL_1(3), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D DIF1_L switch", ADC_LEFT_PGA_SEL_1(3), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D DIF2_L switch", ADC_LEFT_PGA_SEL_2(3), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D DIF3_L switch", ADC_LEFT_PGA_SEL_2(3), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D IN1_R switch", ADC_LEFT_PGA_SEL_2(3), 1, 0x1, 1),
};

/* ADC B Right input selection, Single Ended inputs and Differential inputs */
static const struct snd_kcontrol_new adc_d_right_input_mixer_controls[] = {
    SOC_DAPM_SINGLE("ADC_D IN1_R switch", ADC_RIGHT_PGA_SEL_1(3), 1, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D IN2_R switch", ADC_RIGHT_PGA_SEL_1(3), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D IN3_R switch", ADC_RIGHT_PGA_SEL_1(3), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D DIF1_R switch", ADC_RIGHT_PGA_SEL_1(3), 7, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D DIF2_R switch", ADC_RIGHT_PGA_SEL_2(3), 5, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D DIF3_R switch", ADC_RIGHT_PGA_SEL_2(3), 3, 0x1, 1),
    SOC_DAPM_SINGLE("ADC_D IN1_L switch", ADC_RIGHT_PGA_SEL_2(3), 1, 0x1, 1),
};

/* Left Digital Mic input for left ADC B */
static const struct snd_kcontrol_new adc_d_left_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_D Left ADC switch", ADC_ADC_DIGITAL(3), 3, 0x1, 0),
};

/* Right Digital Mic input for Right ADC B */
static const struct snd_kcontrol_new adc_d_right_input_dmic_controls[] = {
    SOC_DAPM_SINGLE("ADC_D Right ADC switch", ADC_ADC_DIGITAL(3), 2, 0x1, 0),
};

#endif

/* the sturcture contains the different values for mclk */
static const struct aic32xx_rate_divs aic32xx_divs[] = {

/*
 * mclk, rate, p_val, pll_j, pll_d, dosr, ndac, mdac, aosr, nadc, madc, blck_N,
 * codec_speficic_initializations
 */

    /* 8k rate */
    {19200000, 8000, 5, 24, 0, 128, 15, 6, 128, 15, 6, 3, {{60, 1}, {61, 1}}},

    /* 16k rate */
    {19200000, 16000, 1, 5, 1200, 128, 8, 6, 128, 8, 6, 3, {{60, 1}, {61, 1}}},

    /* 44.1k rate */
    {19200000, 44100, 1, 4, 7040, 128, 4, 4, 128, 4, 4, 2, {{60, 1}, {61, 1}}},

    /* 48k rate */
    {19200000, 48000, 1, 5, 1200, 128, 2, 8, 128, 2, 8, 4, {{60, 1}, {61, 1}}}
};

/*
 *----------------------------------------------------------------------------
 * @struct  snd_soc_codec_dai_ops |
 *          It is SoC Codec DAI Operations structure.
 *          ..._ops_capt and ..._play used for async mcbsp mode only
 *----------------------------------------------------------------------------
 */
static struct snd_soc_dai_ops aic32xx_dai_ops_capt = {
    .hw_params = aic32xx_hw_params_capt,
    .digital_mute = aic32xx_mute_capt,
    .set_sysclk = aic32xx_set_dai_sysclk,
    .set_fmt = aic32xx_set_dai_fmt,
    .trigger = aic32xx_trigger,
};

static struct snd_soc_dai_ops aic32xx_dai_ops_play = {
    .hw_params = aic32xx_hw_params_play,
    .digital_mute = aic32xx_mute_play,
    .set_sysclk = aic32xx_set_dai_sysclk,
    .set_fmt = aic32xx_set_dai_fmt,
    .trigger = aic32xx_trigger,
};

struct snd_soc_dai_driver tlv320aic32xx_dai_driver[] = {
    {
     .name = "tlv320aic32xx-capture",
     .capture = {
                 .stream_name = "AIC32xx Capture",
                 .channels_min = 1,
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
                 .channels_max = 8,
#else
                 .channels_max = 4,
#endif
                 .rates = AIC32xx_RATES,
                 .formats = AIC32xx_FORMATS,
                 },
     .ops = &aic32xx_dai_ops_capt,
     },
    {
     .name = "tlv320aic32xx-playback",
     .playback = {
                  .stream_name = "AIC32xx Playback",
                  .channels_min = 1,
                  .channels_max = 2,
                  .rates = AIC32xx_RATES,
                  .formats = AIC32xx_FORMATS,
                  },
     .ops = &aic32xx_dai_ops_play,
    },
};

/*
*****************************************************************************
* Initializations
*****************************************************************************
*/

/*
 * AIC32xx register cache
 * We are caching the registers here. Only caching Page0 and Page1. Will not
 * cache biquad and miniDSP registers.
 * There is no point in caching the reset register.
 * NOTE: In AIC32xx, there are 127 registers supported in both page0 and page1
 *       The following table contains the page0 and page1 registers values.
 */
static const u8 aic32xx_reg[AIC32xx_CACHEREGNUM] = {
    0x00, 0x00, 0x00, 0x00, /* 0 */
    0x00, 0x11, 0x04, 0x00, /* 4 */
    0x00, 0x00, 0x00, 0x01, /* 8 */
    0x01, 0x00, 0x80, 0x02, /* 12 */
    0x00, 0x08, 0x01, 0x01, /* 16 */
    0x80, 0x01, 0x00, 0x04, /* 20 */
    0x00, 0x00, 0x01, 0x00, /* 24 */
    0x00, 0x00, 0x01, 0x00, /* 28 */
    0x00, 0x00, 0x00, 0x00, /* 32 */
    0x00, 0x00, 0x00, 0x00, /* 36 */
    0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, /* 44 */
    0x00, 0x00, 0x00, 0x00, /* 48 */
    0x00, 0x12, 0x02, 0x02, /* 52 */
    0x02, 0x00, 0x00, 0x00, /* 56 */
    0x01, 0x01, 0x00, 0x14, /* 60 */
    0x0C, 0x00, 0x00, 0x00, /* 64 */
    0x67, 0x00, 0xC6, 0x00, /* 68 */
    0x00, 0x00, 0x00, 0xEE, /* 72 */
    0x10, 0xD8, 0x7E, 0xE3, /* 76 */
    0x00, 0x00, 0x88, 0x00, /* 80 */
    0x00, 0x00, 0x00, 0x00, /* 84 */
    0x7F, 0x00, 0x00, 0x00, /* 88 */
    0x00, 0x00, 0x00, 0x00, /* 92 */
    0x7F, 0x00, 0x00, 0x00, /* 96 */
    0x00, 0x00, 0x00, 0x00, /* 100 */
    0x00, 0x00, 0x00, 0x00, /* 104 */
    0x00, 0x00, 0x00, 0x00, /* 108 */
    0x00, 0x00, 0x00, 0x00, /* 112 */
    0x00, 0x00, 0x00, 0x00, /* 116 */
    0x00, 0x00, 0x00, 0x00, /* 120 */
    0x00, 0x00, 0x00, 0x00, /* 124 - PAGE0 Registers(127) ends here */
    0x00, 0x00, 0x08, 0x00, /* 128, PAGE1-0 */
    0x00, 0x00, 0x00, 0x00, /* 132, PAGE1-4 */
    0x00, 0x00, 0x00, 0x10, /* 136, PAGE1-8 */
    0x00, 0x00, 0x00, 0x00, /* 140, PAGE1-12 */
    0x40, 0x40, 0x40, 0x40, /* 144, PAGE1-16 */
    0x00, 0x00, 0x00, 0x00, /* 148, PAGE1-20 */
    0x00, 0x00, 0x00, 0x00, /* 152, PAGE1-24 */
    0x00, 0x00, 0x00, 0x00, /* 156, PAGE1-28 */
    0x00, 0x00, 0x00, 0x00, /* 160, PAGE1-32 */
    0x00, 0x00, 0x00, 0x00, /* 164, PAGE1-36 */
    0x00, 0x00, 0x00, 0x00, /* 168, PAGE1-40 */
    0x00, 0x00, 0x00, 0x00, /* 172, PAGE1-44 */
    0x00, 0x00, 0x00, 0x00, /* 176, PAGE1-48 */
    0x00, 0x00, 0x00, 0x00, /* 180, PAGE1-52 */
    0x00, 0x00, 0x00, 0x80, /* 184, PAGE1-56 */
    0x80, 0x00, 0x00, 0x00, /* 188, PAGE1-60 */
    0x00, 0x00, 0x00, 0x00, /* 192, PAGE1-64 */
    0x00, 0x00, 0x00, 0x00, /* 196, PAGE1-68 */
    0x00, 0x00, 0x00, 0x00, /* 200, PAGE1-72 */
    0x00, 0x00, 0x00, 0x00, /* 204, PAGE1-76 */
    0x00, 0x00, 0x00, 0x00, /* 208, PAGE1-80 */
    0x00, 0x00, 0x00, 0x00, /* 212, PAGE1-84 */
    0x00, 0x00, 0x00, 0x00, /* 216, PAGE1-88 */
    0x00, 0x00, 0x00, 0x00, /* 220, PAGE1-92 */
    0x00, 0x00, 0x00, 0x00, /* 224, PAGE1-96 */
    0x00, 0x00, 0x00, 0x00, /* 228, PAGE1-100 */
    0x00, 0x00, 0x00, 0x00, /* 232, PAGE1-104 */
    0x00, 0x00, 0x00, 0x00, /* 236, PAGE1-108 */
    0x00, 0x00, 0x00, 0x00, /* 240, PAGE1-112 */
    0x00, 0x00, 0x00, 0x00, /* 244, PAGE1-116 */
    0x00, 0x00, 0x00, 0x00, /* 248, PAGE1-120 */
    0x00, 0x00, 0x00, 0x00, /* 252, PAGE1-124 */
    0x00, 0x00, 0x00, 0x00, /* 0 */ /*ADC A registers start here */
    0x00, 0x11, 0x04, 0x00, /* 4 */
    0x00, 0x00, 0x00, 0x00, /* 8 */
    0x00, 0x00, 0x00, 0x00, /* 12 */
    0x00, 0x00, 0x01, 0x01, /* 16 */
    0x80, 0x80, 0x04, 0x00, /* 20 */
    0x00, 0x00, 0x01, 0x00, /* 24 */
    0x00, 0x02, 0x01, 0x00, /* 28 */
    0x00, 0x10, 0x00, 0x00, /* 32 */
    0x00, 0x00, 0x02, 0x00, /* 36 */
    0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, /* 44 */
    0x00, 0x00, 0x00, 0x00, /* 48 */
    0x00, 0x12, 0x00, 0x00, /* 52 */
    0x00, 0x00, 0x00, 0x44, /* 56 */
    0x00, 0x01, 0x00, 0x00, /* 60 */
    0x00, 0x00, 0x00, 0x00, /* 64 */
    0x00, 0x00, 0x00, 0x00, /* 68 */
    0x00, 0x00, 0x00, 0x00, /* 72 */
    0x00, 0x00, 0x00, 0x00, /* 76 */
    0x00, 0x00, 0x88, 0x00, /* 80 */
    0x00, 0x00, 0x00, 0x00, /* 84 */
    0x7F, 0x00, 0x00, 0x00, /* 88 */
    0x00, 0x00, 0x00, 0x00, /* 92 */
    0x7F, 0x00, 0x00, 0x00, /* 96 */
    0x00, 0x00, 0x00, 0x00, /* 100 */
    0x00, 0x00, 0x00, 0x00, /* 104 */
    0x00, 0x00, 0x00, 0x00, /* 108 */
    0x00, 0x00, 0x00, 0x00, /* 112 */
    0x00, 0x00, 0x00, 0x00, /* 116 */
    0x00, 0x00, 0x00, 0x00, /* 120 */
    0x00, 0x00, 0x00, 0x00, /* 124 - ADC A PAGE0 Registers(127) ends here */
    0x00, 0x00, 0x00, 0x00, /* 128, PAGE1-0 */
    0x00, 0x00, 0x00, 0x00, /* 132, PAGE1-4 */
    0x00, 0x00, 0x00, 0x00, /* 136, PAGE1-8 */
    0x00, 0x00, 0x00, 0x00, /* 140, PAGE1-12 */
    0x00, 0x00, 0x00, 0x00, /* 144, PAGE1-16 */
    0x00, 0x00, 0x00, 0x00, /* 148, PAGE1-20 */
    0x00, 0x00, 0x00, 0x00, /* 152, PAGE1-24 */
    0x00, 0x00, 0x00, 0x00, /* 156, PAGE1-28 */
    0x00, 0x00, 0x00, 0x00, /* 160, PAGE1-32 */
    0x00, 0x00, 0x00, 0x00, /* 164, PAGE1-36 */
    0x00, 0x00, 0x00, 0x00, /* 168, PAGE1-40 */
    0x00, 0x00, 0x00, 0x00, /* 172, PAGE1-44 */
    0x00, 0x00, 0x00, 0x00, /* 176, PAGE1-48 */
    0xFF, 0x00, 0x3F, 0xFF, /* 180, PAGE1-52 */
    0x00, 0x3F, 0x00, 0x80, /* 184, PAGE1-56 */
    0x80, 0x00, 0x00, 0x00, /* 188, PAGE1-60 */
    0x00, 0x00, 0x00, 0x00, /* 192, PAGE1-64 */
    0x00, 0x00, 0x00, 0x00, /* 196, PAGE1-68 */
    0x00, 0x00, 0x00, 0x00, /* 200, PAGE1-72 */
    0x00, 0x00, 0x00, 0x00, /* 204, PAGE1-76 */
    0x00, 0x00, 0x00, 0x00, /* 208, PAGE1-80 */
    0x00, 0x00, 0x00, 0x00, /* 212, PAGE1-84 */
    0x00, 0x00, 0x00, 0x00, /* 216, PAGE1-88 */
    0x00, 0x00, 0x00, 0x00, /* 220, PAGE1-92 */
    0x00, 0x00, 0x00, 0x00, /* 224, PAGE1-96 */
    0x00, 0x00, 0x00, 0x00, /* 228, PAGE1-100 */
    0x00, 0x00, 0x00, 0x00, /* 232, PAGE1-104 */
    0x00, 0x00, 0x00, 0x00, /* 236, PAGE1-108 */
    0x00, 0x00, 0x00, 0x00, /* 240, PAGE1-112 */
    0x00, 0x00, 0x00, 0x00, /* 244, PAGE1-116 */
    0x00, 0x00, 0x00, 0x00, /* 248, PAGE1-120 */
    0x00, 0x00, 0x00, 0x00, /* 252, PAGE1-124 */
    0x00, 0x00, 0x00, 0x00, /* 0 *//* ADC_B regsiters start here */
    0x00, 0x11, 0x04, 0x00, /* 4 */
    0x00, 0x00, 0x00, 0x00, /* 8 */
    0x00, 0x00, 0x00, 0x00, /* 12 */
    0x00, 0x00, 0x01, 0x01, /* 16 */
    0x80, 0x80, 0x04, 0x00, /* 20 */
    0x00, 0x00, 0x01, 0x00, /* 24 */
    0x00, 0x02, 0x01, 0x00, /* 28 */
    0x00, 0x10, 0x00, 0x00, /* 32 */
    0x00, 0x00, 0x02, 0x00, /* 36 */
    0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, /* 44 */
    0x00, 0x00, 0x00, 0x00, /* 48 */
    0x00, 0x12, 0x00, 0x00, /* 52 */
    0x00, 0x00, 0x00, 0x44, /* 56 */
    0x00, 0x01, 0x00, 0x00, /* 60 */
    0x00, 0x00, 0x00, 0x00, /* 64 */
    0x00, 0x00, 0x00, 0x00, /* 68 */
    0x00, 0x00, 0x00, 0x00, /* 72 */
    0x00, 0x00, 0x00, 0x00, /* 76 */
    0x00, 0x00, 0x88, 0x00, /* 80 */
    0x00, 0x00, 0x00, 0x00, /* 84 */
    0x7F, 0x00, 0x00, 0x00, /* 88 */
    0x00, 0x00, 0x00, 0x00, /* 92 */
    0x7F, 0x00, 0x00, 0x00, /* 96 */
    0x00, 0x00, 0x00, 0x00, /* 100 */
    0x00, 0x00, 0x00, 0x00, /* 104 */
    0x00, 0x00, 0x00, 0x00, /* 108 */
    0x00, 0x00, 0x00, 0x00, /* 112 */
    0x00, 0x00, 0x00, 0x00, /* 116 */
    0x00, 0x00, 0x00, 0x00, /* 120 */
    0x00, 0x00, 0x00, 0x00, /* 124 - ADC_B PAGE0 Registers(127) ends here */
    0x00, 0x00, 0x00, 0x00, /* 128, PAGE1-0 */
    0x00, 0x00, 0x00, 0x00, /* 132, PAGE1-4 */
    0x00, 0x00, 0x00, 0x00, /* 136, PAGE1-8 */
    0x00, 0x00, 0x00, 0x00, /* 140, PAGE1-12 */
    0x00, 0x00, 0x00, 0x00, /* 144, PAGE1-16 */
    0x00, 0x00, 0x00, 0x00, /* 148, PAGE1-20 */
    0x00, 0x00, 0x00, 0x00, /* 152, PAGE1-24 */
    0x00, 0x00, 0x00, 0x00, /* 156, PAGE1-28 */
    0x00, 0x00, 0x00, 0x00, /* 160, PAGE1-32 */
    0x00, 0x00, 0x00, 0x00, /* 164, PAGE1-36 */
    0x00, 0x00, 0x00, 0x00, /* 168, PAGE1-40 */
    0x00, 0x00, 0x00, 0x00, /* 172, PAGE1-44 */
    0x00, 0x00, 0x00, 0x00, /* 176, PAGE1-48 */
    0xFF, 0x00, 0x3F, 0xFF, /* 180, PAGE1-52 */
    0x00, 0x3F, 0x00, 0x80, /* 184, PAGE1-56 */
    0x80, 0x00, 0x00, 0x00, /* 188, PAGE1-60 */
    0x00, 0x00, 0x00, 0x00, /* 192, PAGE1-64 */
    0x00, 0x00, 0x00, 0x00, /* 196, PAGE1-68 */
    0x00, 0x00, 0x00, 0x00, /* 200, PAGE1-72 */
    0x00, 0x00, 0x00, 0x00, /* 204, PAGE1-76 */
    0x00, 0x00, 0x00, 0x00, /* 208, PAGE1-80 */
    0x00, 0x00, 0x00, 0x00, /* 212, PAGE1-84 */
    0x00, 0x00, 0x00, 0x00, /* 216, PAGE1-88 */
    0x00, 0x00, 0x00, 0x00, /* 220, PAGE1-92 */
    0x00, 0x00, 0x00, 0x00, /* 224, PAGE1-96 */
    0x00, 0x00, 0x00, 0x00, /* 228, PAGE1-100 */
    0x00, 0x00, 0x00, 0x00, /* 232, PAGE1-104 */
    0x00, 0x00, 0x00, 0x00, /* 236, PAGE1-108 */
    0x00, 0x00, 0x00, 0x00, /* 240, PAGE1-112 */
    0x00, 0x00, 0x00, 0x00, /* 244, PAGE1-116 */
    0x00, 0x00, 0x00, 0x00, /* 248, PAGE1-120 */
    0x00, 0x00, 0x00, 0x00, /* 252, PAGE1-124 */
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
    0x00, 0x00, 0x00, 0x00, /* 0 */ /*ADC C registers start here */
    0x00, 0x11, 0x04, 0x00, /* 4 */
    0x00, 0x00, 0x00, 0x00, /* 8 */
    0x00, 0x00, 0x00, 0x00, /* 12 */
    0x00, 0x00, 0x01, 0x01, /* 16 */
    0x80, 0x80, 0x04, 0x00, /* 20 */
    0x00, 0x00, 0x01, 0x00, /* 24 */
    0x00, 0x02, 0x01, 0x00, /* 28 */
    0x00, 0x10, 0x00, 0x00, /* 32 */
    0x00, 0x00, 0x02, 0x00, /* 36 */
    0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, /* 44 */
    0x00, 0x00, 0x00, 0x00, /* 48 */
    0x00, 0x12, 0x00, 0x00, /* 52 */
    0x00, 0x00, 0x00, 0x44, /* 56 */
    0x00, 0x01, 0x00, 0x00, /* 60 */
    0x00, 0x00, 0x00, 0x00, /* 64 */
    0x00, 0x00, 0x00, 0x00, /* 68 */
    0x00, 0x00, 0x00, 0x00, /* 72 */
    0x00, 0x00, 0x00, 0x00, /* 76 */
    0x00, 0x00, 0x88, 0x00, /* 80 */
    0x00, 0x00, 0x00, 0x00, /* 84 */
    0x7F, 0x00, 0x00, 0x00, /* 88 */
    0x00, 0x00, 0x00, 0x00, /* 92 */
    0x7F, 0x00, 0x00, 0x00, /* 96 */
    0x00, 0x00, 0x00, 0x00, /* 100 */
    0x00, 0x00, 0x00, 0x00, /* 104 */
    0x00, 0x00, 0x00, 0x00, /* 108 */
    0x00, 0x00, 0x00, 0x00, /* 112 */
    0x00, 0x00, 0x00, 0x00, /* 116 */
    0x00, 0x00, 0x00, 0x00, /* 120 */
    0x00, 0x00, 0x00, 0x00, /* 124 - ADC C PAGE0 Registers(127) ends here */
    0x00, 0x00, 0x00, 0x00, /* 128, PAGE1-0 */
    0x00, 0x00, 0x00, 0x00, /* 132, PAGE1-4 */
    0x00, 0x00, 0x00, 0x00, /* 136, PAGE1-8 */
    0x00, 0x00, 0x00, 0x00, /* 140, PAGE1-12 */
    0x00, 0x00, 0x00, 0x00, /* 144, PAGE1-16 */
    0x00, 0x00, 0x00, 0x00, /* 148, PAGE1-20 */
    0x00, 0x00, 0x00, 0x00, /* 152, PAGE1-24 */
    0x00, 0x00, 0x00, 0x00, /* 156, PAGE1-28 */
    0x00, 0x00, 0x00, 0x00, /* 160, PAGE1-32 */
    0x00, 0x00, 0x00, 0x00, /* 164, PAGE1-36 */
    0x00, 0x00, 0x00, 0x00, /* 168, PAGE1-40 */
    0x00, 0x00, 0x00, 0x00, /* 172, PAGE1-44 */
    0x00, 0x00, 0x00, 0x00, /* 176, PAGE1-48 */
    0xFF, 0x00, 0x3F, 0xFF, /* 180, PAGE1-52 */
    0x00, 0x3F, 0x00, 0x80, /* 184, PAGE1-56 */
    0x80, 0x00, 0x00, 0x00, /* 188, PAGE1-60 */
    0x00, 0x00, 0x00, 0x00, /* 192, PAGE1-64 */
    0x00, 0x00, 0x00, 0x00, /* 196, PAGE1-68 */
    0x00, 0x00, 0x00, 0x00, /* 200, PAGE1-72 */
    0x00, 0x00, 0x00, 0x00, /* 204, PAGE1-76 */
    0x00, 0x00, 0x00, 0x00, /* 208, PAGE1-80 */
    0x00, 0x00, 0x00, 0x00, /* 212, PAGE1-84 */
    0x00, 0x00, 0x00, 0x00, /* 216, PAGE1-88 */
    0x00, 0x00, 0x00, 0x00, /* 220, PAGE1-92 */
    0x00, 0x00, 0x00, 0x00, /* 224, PAGE1-96 */
    0x00, 0x00, 0x00, 0x00, /* 228, PAGE1-100 */
    0x00, 0x00, 0x00, 0x00, /* 232, PAGE1-104 */
    0x00, 0x00, 0x00, 0x00, /* 236, PAGE1-108 */
    0x00, 0x00, 0x00, 0x00, /* 240, PAGE1-112 */
    0x00, 0x00, 0x00, 0x00, /* 244, PAGE1-116 */
    0x00, 0x00, 0x00, 0x00, /* 248, PAGE1-120 */
    0x00, 0x00, 0x00, 0x00, /* 252, PAGE1-124 */
    0x00, 0x00, 0x00, 0x00, /* 0 *//* ADC_D regsiters start here */
    0x00, 0x11, 0x04, 0x00, /* 4 */
    0x00, 0x00, 0x00, 0x00, /* 8 */
    0x00, 0x00, 0x00, 0x00, /* 12 */
    0x00, 0x00, 0x01, 0x01, /* 16 */
    0x80, 0x80, 0x04, 0x00, /* 20 */
    0x00, 0x00, 0x01, 0x00, /* 24 */
    0x00, 0x02, 0x01, 0x00, /* 28 */
    0x00, 0x10, 0x00, 0x00, /* 32 */
    0x00, 0x00, 0x02, 0x00, /* 36 */
    0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, /* 44 */
    0x00, 0x00, 0x00, 0x00, /* 48 */
    0x00, 0x12, 0x00, 0x00, /* 52 */
    0x00, 0x00, 0x00, 0x44, /* 56 */
    0x00, 0x01, 0x00, 0x00, /* 60 */
    0x00, 0x00, 0x00, 0x00, /* 64 */
    0x00, 0x00, 0x00, 0x00, /* 68 */
    0x00, 0x00, 0x00, 0x00, /* 72 */
    0x00, 0x00, 0x00, 0x00, /* 76 */
    0x00, 0x00, 0x88, 0x00, /* 80 */
    0x00, 0x00, 0x00, 0x00, /* 84 */
    0x7F, 0x00, 0x00, 0x00, /* 88 */
    0x00, 0x00, 0x00, 0x00, /* 92 */
    0x7F, 0x00, 0x00, 0x00, /* 96 */
    0x00, 0x00, 0x00, 0x00, /* 100 */
    0x00, 0x00, 0x00, 0x00, /* 104 */
    0x00, 0x00, 0x00, 0x00, /* 108 */
    0x00, 0x00, 0x00, 0x00, /* 112 */
    0x00, 0x00, 0x00, 0x00, /* 116 */
    0x00, 0x00, 0x00, 0x00, /* 120 */
    0x00, 0x00, 0x00, 0x00, /* 124 - ADC_D PAGE0 Registers(127) ends here */
    0x00, 0x00, 0x00, 0x00, /* 128, PAGE1-0 */
    0x00, 0x00, 0x00, 0x00, /* 132, PAGE1-4 */
    0x00, 0x00, 0x00, 0x00, /* 136, PAGE1-8 */
    0x00, 0x00, 0x00, 0x00, /* 140, PAGE1-12 */
    0x00, 0x00, 0x00, 0x00, /* 144, PAGE1-16 */
    0x00, 0x00, 0x00, 0x00, /* 148, PAGE1-20 */
    0x00, 0x00, 0x00, 0x00, /* 152, PAGE1-24 */
    0x00, 0x00, 0x00, 0x00, /* 156, PAGE1-28 */
    0x00, 0x00, 0x00, 0x00, /* 160, PAGE1-32 */
    0x00, 0x00, 0x00, 0x00, /* 164, PAGE1-36 */
    0x00, 0x00, 0x00, 0x00, /* 168, PAGE1-40 */
    0x00, 0x00, 0x00, 0x00, /* 172, PAGE1-44 */
    0x00, 0x00, 0x00, 0x00, /* 176, PAGE1-48 */
    0xFF, 0x00, 0x3F, 0xFF, /* 180, PAGE1-52 */
    0x00, 0x3F, 0x00, 0x80, /* 184, PAGE1-56 */
    0x80, 0x00, 0x00, 0x00, /* 188, PAGE1-60 */
    0x00, 0x00, 0x00, 0x00, /* 192, PAGE1-64 */
    0x00, 0x00, 0x00, 0x00, /* 196, PAGE1-68 */
    0x00, 0x00, 0x00, 0x00, /* 200, PAGE1-72 */
    0x00, 0x00, 0x00, 0x00, /* 204, PAGE1-76 */
    0x00, 0x00, 0x00, 0x00, /* 208, PAGE1-80 */
    0x00, 0x00, 0x00, 0x00, /* 212, PAGE1-84 */
    0x00, 0x00, 0x00, 0x00, /* 216, PAGE1-88 */
    0x00, 0x00, 0x00, 0x00, /* 220, PAGE1-92 */
    0x00, 0x00, 0x00, 0x00, /* 224, PAGE1-96 */
    0x00, 0x00, 0x00, 0x00, /* 228, PAGE1-100 */
    0x00, 0x00, 0x00, 0x00, /* 232, PAGE1-104 */
    0x00, 0x00, 0x00, 0x00, /* 236, PAGE1-108 */
    0x00, 0x00, 0x00, 0x00, /* 240, PAGE1-112 */
    0x00, 0x00, 0x00, 0x00, /* 244, PAGE1-116 */
    0x00, 0x00, 0x00, 0x00, /* 248, PAGE1-120 */
    0x00, 0x00, 0x00, 0x00, /* 252, PAGE1-124 */
#endif
};


/*
 * aic32xx and adc31xx biquad settings
 * This structure should be loaded with biquad coefficients
 * for ADC's and AIC's as generated by the TI biquad coefficient tool
 * and as put into proper form with the biquad script
 */
static const struct aic32xx_configs biquad_settings[] = {
//    ========================
//    =    Capture Filters   =
//    ========================
//    20 ADC HPF Bessel
//    Cleaner HPF without bump in 16kHz mode
//    Coefficient write to 3101-A ADC Left
    //Page change to 4
    {0x210e, 0x7f}, // reg[4][14] = 127
    {0x210f, 0x22}, // reg[4][15] = 34
    {0x2110, 0x80}, // reg[4][16] = 128
    {0x2111, 0xde}, // reg[4][17] = 222
    {0x2112, 0x7f}, // reg[4][18] = 127
    {0x2113, 0x22}, // reg[4][19] = 34
    {0x2114, 0x7f}, // reg[4][20] = 127
    {0x2115, 0x21}, // reg[4][21] = 33
    {0x2116, 0x81}, // reg[4][22] = 129
    {0x2117, 0xba}, // reg[4][23] = 186
//    Coefficient write to 3101-A ADC Rght
    //Page change to 4
    {0x214e, 0x7f}, // reg[4][78] = 127
    {0x214f, 0x22}, // reg[4][79] = 34
    {0x2150, 0x80}, // reg[4][80] = 128
    {0x2151, 0xde}, // reg[4][81] = 222
    {0x2152, 0x7f}, // reg[4][82] = 127
    {0x2153, 0x22}, // reg[4][83] = 34
    {0x2154, 0x7f}, // reg[4][84] = 127
    {0x2155, 0x21}, // reg[4][85] = 33
    {0x2156, 0x81}, // reg[4][86] = 129
    {0x2157, 0xba}, // reg[4][87] = 186
//    Coefficient write to 3101-B ADC Left
    //Page change to 4
    {0x220e, 0x7f}, // reg[4][14] = 127
    {0x220f, 0x22}, // reg[4][15] = 34
    {0x2210, 0x80}, // reg[4][16] = 128
    {0x2211, 0xde}, // reg[4][17] = 222
    {0x2212, 0x7f}, // reg[4][18] = 127
    {0x2213, 0x22}, // reg[4][19] = 34
    {0x2214, 0x7f}, // reg[4][20] = 127
    {0x2215, 0x21}, // reg[4][21] = 33
    {0x2216, 0x81}, // reg[4][22] = 129
    {0x2217, 0xba}, // reg[4][23] = 186
//    Coefficient write to 3101-B ADC Rght
    //Page change to 4
    {0x224e, 0x7f}, // reg[4][78] = 127
    {0x224f, 0x22}, // reg[4][79] = 34
    {0x2250, 0x80}, // reg[4][80] = 128
    {0x2251, 0xde}, // reg[4][81] = 222
    {0x2252, 0x7f}, // reg[4][82] = 127
    {0x2253, 0x22}, // reg[4][83] = 34
    {0x2254, 0x7f}, // reg[4][84] = 127
    {0x2255, 0x21}, // reg[4][85] = 33
    {0x2256, 0x81}, // reg[4][86] = 129
    {0x2257, 0xba}, // reg[4][87] = 186
};

static const struct aic32xx_configs biquad_settings_Play[] = {
//    ========================
//    =    Playback Filters  =
//    ========================
//  Crossover filter (Fc @ 1Khz) and 2nd order HPF40Hz
      { MAKE_REG( 0, 44, 12), 0x00},
      { MAKE_REG( 0, 44, 13), 0x80},
      { MAKE_REG( 0, 44, 14), 0x52},
      { MAKE_REG( 0, 44, 16), 0x00},
      { MAKE_REG( 0, 44, 17), 0x80},
      { MAKE_REG( 0, 44, 18), 0x52},
      { MAKE_REG( 0, 44, 20), 0x00},
      { MAKE_REG( 0, 44, 21), 0x80},
      { MAKE_REG( 0, 44, 22), 0x52},
      { MAKE_REG( 0, 44, 24), 0x74},
      { MAKE_REG( 0, 44, 25), 0x2E},
      { MAKE_REG( 0, 44, 26), 0x2A},
      { MAKE_REG( 0, 44, 28), 0x95},
      { MAKE_REG( 0, 44, 29), 0xA2},
      { MAKE_REG( 0, 44, 30), 0x61},
      { MAKE_REG( 0, 44, 32), 0x00},
      { MAKE_REG( 0, 44, 33), 0x80},
      { MAKE_REG( 0, 44, 34), 0x52},
      { MAKE_REG( 0, 44, 36), 0x00},
      { MAKE_REG( 0, 44, 37), 0x80},
      { MAKE_REG( 0, 44, 38), 0x52},
      { MAKE_REG( 0, 44, 40), 0x00},
      { MAKE_REG( 0, 44, 41), 0x80},
      { MAKE_REG( 0, 44, 42), 0x52},
      { MAKE_REG( 0, 44, 44), 0x74},
      { MAKE_REG( 0, 44, 45), 0x2E},
      { MAKE_REG( 0, 44, 46), 0x2A},
      { MAKE_REG( 0, 44, 48), 0x95},
      { MAKE_REG( 0, 44, 49), 0xA2},
      { MAKE_REG( 0, 44, 50), 0x61},
      { MAKE_REG( 0, 44, 52), 0x7F},
      { MAKE_REG( 0, 44, 53), 0x55},
      { MAKE_REG( 0, 44, 54), 0x18},
      { MAKE_REG( 0, 44, 56), 0x80},
      { MAKE_REG( 0, 44, 57), 0xAA},
      { MAKE_REG( 0, 44, 58), 0xE8},
      { MAKE_REG( 0, 44, 60), 0x7F},
      { MAKE_REG( 0, 44, 61), 0x55},
      { MAKE_REG( 0, 44, 62), 0x18},
      { MAKE_REG( 0, 44, 64), 0x7F},
      { MAKE_REG( 0, 44, 65), 0x54},
      { MAKE_REG( 0, 44, 66), 0xDE},
      { MAKE_REG( 0, 44, 68), 0x81},
      { MAKE_REG( 0, 44, 69), 0x55},
      { MAKE_REG( 0, 44, 70), 0x5D},
      { MAKE_REG( 0, 45, 20), 0x74},
      { MAKE_REG( 0, 45, 21), 0xAE},
      { MAKE_REG( 0, 45, 22), 0x7C},
      { MAKE_REG( 0, 45, 24), 0x8B},
      { MAKE_REG( 0, 45, 25), 0x51},
      { MAKE_REG( 0, 45, 26), 0x84},
      { MAKE_REG( 0, 45, 28), 0x74},
      { MAKE_REG( 0, 45, 29), 0xAE},
      { MAKE_REG( 0, 45, 30), 0x7C},
      { MAKE_REG( 0, 45, 32), 0x74},
      { MAKE_REG( 0, 45, 33), 0x2E},
      { MAKE_REG( 0, 45, 34), 0x2A},
      { MAKE_REG( 0, 45, 36), 0x95},
      { MAKE_REG( 0, 45, 37), 0xA2},
      { MAKE_REG( 0, 45, 38), 0x61},
      { MAKE_REG( 0, 45, 40), 0x74},
      { MAKE_REG( 0, 45, 41), 0xAE},
      { MAKE_REG( 0, 45, 42), 0x7C},
      { MAKE_REG( 0, 45, 44), 0x8B},
      { MAKE_REG( 0, 45, 45), 0x51},
      { MAKE_REG( 0, 45, 46), 0x84},
      { MAKE_REG( 0, 45, 48), 0x74},
      { MAKE_REG( 0, 45, 49), 0xAE},
      { MAKE_REG( 0, 45, 50), 0x7C},
      { MAKE_REG( 0, 45, 52), 0x74},
      { MAKE_REG( 0, 45, 53), 0x2E},
      { MAKE_REG( 0, 45, 54), 0x2A},
      { MAKE_REG( 0, 45, 56), 0x95},
      { MAKE_REG( 0, 45, 57), 0xA2},
      { MAKE_REG( 0, 45, 58), 0x61},
      { MAKE_REG( 0, 45, 60), 0x7F},
      { MAKE_REG( 0, 45, 61), 0x55},
      { MAKE_REG( 0, 45, 62), 0x18},
      { MAKE_REG( 0, 45, 64), 0x80},
      { MAKE_REG( 0, 45, 65), 0xAA},
      { MAKE_REG( 0, 45, 66), 0xE8},
      { MAKE_REG( 0, 45, 68), 0x7F},
      { MAKE_REG( 0, 45, 69), 0x55},
      { MAKE_REG( 0, 45, 70), 0x18},
      { MAKE_REG( 0, 45, 72), 0x7F},
      { MAKE_REG( 0, 45, 73), 0x54},
      { MAKE_REG( 0, 45, 74), 0xDE},
      { MAKE_REG( 0, 45, 76), 0x81},
      { MAKE_REG( 0, 45, 77), 0x55},
      { MAKE_REG( 0, 45, 78), 0x5D},
      { MAKE_REG( 0, 46, 52), 0x7F},    //DRC HPF: Page 46: N0 52 - 55
      { MAKE_REG( 0, 46, 53), 0xFF},    //Bypassed: N0 = 0x7FFFFF;
      { MAKE_REG( 0, 46, 54), 0xFF},
      { MAKE_REG( 0, 46, 55), 0x00},    //N1 = 0
      { MAKE_REG( 0, 46, 56), 0x00},
      { MAKE_REG( 0, 46, 57), 0x00},
      { MAKE_REG( 0, 46, 58), 0x00},
      { MAKE_REG( 0, 46, 59), 0x00},
      { MAKE_REG( 0, 46, 60), 0x00},   //D1
      { MAKE_REG( 0, 46, 61), 0x00},
      { MAKE_REG( 0, 46, 62), 0x00},
      { MAKE_REG( 0, 46, 63), 0x00},
      { MAKE_REG( 0, 46, 64), 0x7F},    //DRC LPF: Page 46: N0 64 - 67
      { MAKE_REG( 0, 46, 65), 0xFF},    //Bypassed: N0 = 0x7FFFFF;
      { MAKE_REG( 0, 46, 66), 0xFF},
      { MAKE_REG( 0, 46, 67), 0x00},    //N1 = 0
      { MAKE_REG( 0, 46, 68), 0x00},
      { MAKE_REG( 0, 46, 69), 0x00},
      { MAKE_REG( 0, 46, 70), 0x00},
      { MAKE_REG( 0, 46, 71), 0x00},
      { MAKE_REG( 0, 46, 72), 0x00},   //D1
      { MAKE_REG( 0, 46, 73), 0x00},
      { MAKE_REG( 0, 46, 74), 0x00},
      { MAKE_REG( 0, 46, 75), 0x00},

#ifdef SET_DRC_OFF
      // Force DRC (Dynamic Range Compression) off
      {DRC_CTRL_REG1, 0x0F},
#else
      // Enable DRC
      {DRC_CTRL_REG1, 0x67},
      {DRC_CTRL_REG2, 0x00},
      {DRC_CTRL_REG3, 0xC6}
#endif

};

static const struct aic32xx_configs biquad_Play_P[] = {
//    ========================
//    =    Playback Filters  =
//    ========================
#if 1 // SET_DRC_OFF
      // Force DRC (Dynamic Range Compression) off
      {DRC_CTRL_REG1, 0x0F},
#else
      // Enable DRC
      {DRC_CTRL_REG1, 0x67},
      {DRC_CTRL_REG2, 0x00},
      {DRC_CTRL_REG3, 0xC6}
#endif

};



/* Left HPL Mixer */
static const struct snd_kcontrol_new hpl_output_mixer_controls[] = {
    SOC_DAPM_SINGLE("L_DAC switch", HPL_ROUTE_CTRL, 3, 1, 0),
    SOC_DAPM_SINGLE("IN1_L switch", HPL_ROUTE_CTRL, 2, 1, 0),
    SOC_DAPM_SINGLE("MAL switch", HPL_ROUTE_CTRL, 1, 1, 0),
    SOC_DAPM_SINGLE("MAR switch", HPL_ROUTE_CTRL, 0, 1, 0),
};

/* Right HPR Mixer */
static const struct snd_kcontrol_new hpr_output_mixer_controls[] = {
    SOC_DAPM_SINGLE("L_DAC switch", HPR_ROUTE_CTRL, 4, 1, 0),
    SOC_DAPM_SINGLE("R_DAC switch", HPR_ROUTE_CTRL, 3, 1, 0),
    SOC_DAPM_SINGLE("IN1_R switch", HPR_ROUTE_CTRL, 2, 1, 0),
    SOC_DAPM_SINGLE("MAR switch", HPR_ROUTE_CTRL, 1, 1, 0),
};

/* AIC32xx Widget Structure */
static const struct snd_soc_dapm_widget aic32xx_dapm_widgets[] = {
    /* Left DAC to Left Outputs */
    /* dapm widget (stream domain) for left DAC */
    SND_SOC_DAPM_DAC("Left DAC", "Left Playback", DAC_CHN_REG, 7, 0),

    /* dapm widget (path domain) for left DAC_L Mixer */
    SND_SOC_DAPM_MIXER("HPL Output Mixer", SND_SOC_NOPM, 0, 0,
                       &hpl_output_mixer_controls[0],
                       ARRAY_SIZE(hpl_output_mixer_controls)),

    /* dapm widget for Left Head phone Power */
    SND_SOC_DAPM_PGA_E("HPL PGA", OUT_PWR_CTRL, 5, 0, NULL, 0,
                       aic32xx_hp_event, SND_SOC_DAPM_PRE_PMU),

    /* Right DAC to Right Outputs */
    /* dapm widget (stream domain) for right DAC */
    SND_SOC_DAPM_DAC("Right DAC", "Right Playback", DAC_CHN_REG, 6, 0),

    /* dapm widget (path domain) for right DAC_R mixer */
    SND_SOC_DAPM_MIXER("HPR Output Mixer", SND_SOC_NOPM, 0, 0,
                       &hpr_output_mixer_controls[0],
                       ARRAY_SIZE(hpr_output_mixer_controls)),

    /* dapm widget for Right Head phone Power */
    SND_SOC_DAPM_PGA_E("HPR PGA", OUT_PWR_CTRL, 4, 0, NULL, 0,
                       aic32xx_hp_event, SND_SOC_DAPM_PRE_PMU),

    /* Left MicPGA to Mixer PGA Left */
    SND_SOC_DAPM_PGA("MAL PGA", OUT_PWR_CTRL, 1, 0, NULL, 0),

    /* Right Inputs to Mixer PGA Right */
    SND_SOC_DAPM_PGA("MAR PGA", OUT_PWR_CTRL, 0, 0, NULL, 0),

    /* dapm widget (platform domain) name for HPLOUT */
    SND_SOC_DAPM_OUTPUT("HPL"),
    /* dapm widget (platform domain) name for HPROUT */
    SND_SOC_DAPM_OUTPUT("HPR"),

    /* dapm widget (platform domain) name for LINE1L */
    SND_SOC_DAPM_INPUT("IN1_L"),

    /* dapm widget (platform domain) name for LINE1R */
    SND_SOC_DAPM_INPUT("IN1_R"),

    /* dapm widget (platform domain) name for LINE2L */
    SND_SOC_DAPM_INPUT("IN2_L"),

    /* dapm widget (platform domain) name for LINE2R */
    SND_SOC_DAPM_INPUT("IN2_R"),

    /* dapm widget (platform domain) name for LINE3L */
    SND_SOC_DAPM_INPUT("IN3_L"),

    /* dapm widget (platform domain) name for LINE3R */
    SND_SOC_DAPM_INPUT("IN3_R"),

    SND_SOC_DAPM_MICBIAS("Mic Bias", MICBIAS_CTRL, 6, 0),
};


/* adc31xx_a Widget structure */
static const struct snd_soc_dapm_widget adc31xx_dapm_widgets_a[] = {

    /* Left Input Selection */
    SND_SOC_DAPM_MIXER("ADC_A Left Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_a_left_input_mixer_controls[0],
                       ARRAY_SIZE(adc_a_left_input_mixer_controls)),
    /* Right Input Select */
    SND_SOC_DAPM_MIXER("ADC_A Right Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_a_right_input_mixer_controls[0],
                       ARRAY_SIZE(adc_a_right_input_mixer_controls)),
    /*PGA selection */
    SND_SOC_DAPM_PGA("ADC_A Left PGA", ADC_LEFT_APGA_CTRL(0), 7, 1, NULL, 0),
    SND_SOC_DAPM_PGA("ADC_A Right PGA", ADC_RIGHT_APGA_CTRL(0), 7, 1, NULL, 0),

    /*Digital Microphone Input Control for Left/Right ADC */
    SND_SOC_DAPM_MIXER("ADC_A Left DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_a_left_input_dmic_controls[0],
                       ARRAY_SIZE(adc_a_left_input_dmic_controls)),
    SND_SOC_DAPM_MIXER("ADC_A Right DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_a_right_input_dmic_controls[0],
                       ARRAY_SIZE(adc_a_right_input_dmic_controls)),

    /* Left/Right ADC */
    SND_SOC_DAPM_ADC("ADC_A Left ADC", "ADC_A Left Capture", ADC_ADC_DIGITAL(0), 7, 0),
    SND_SOC_DAPM_ADC("ADC_A Right ADC", "ADC_A Right Capture", ADC_ADC_DIGITAL(0), 6, 0),
    /* Inputs */
    SND_SOC_DAPM_INPUT("ADC_A_IN1_L"),
    SND_SOC_DAPM_INPUT("ADC_A_IN1_R"),
    SND_SOC_DAPM_INPUT("ADC_A_IN2_L"),
    SND_SOC_DAPM_INPUT("ADC_A_IN2_R"),
    SND_SOC_DAPM_INPUT("ADC_A_IN3_L"),
    SND_SOC_DAPM_INPUT("ADC_A_IN3_R"),
    SND_SOC_DAPM_INPUT("ADC_A_DIF1_L"),
    SND_SOC_DAPM_INPUT("ADC_A_DIF2_L"),
    SND_SOC_DAPM_INPUT("ADC_A_DIF3_L"),
    SND_SOC_DAPM_INPUT("ADC_A_DIF1_R"),
    SND_SOC_DAPM_INPUT("ADC_A_DIF2_R"),
    SND_SOC_DAPM_INPUT("ADC_A_DIF3_R"),
    SND_SOC_DAPM_INPUT("ADC_A_DMic_L"),
    SND_SOC_DAPM_INPUT("ADC_A_DMic_R"),
};

/* adc31xx_b Widget structure */
static const struct snd_soc_dapm_widget adc31xx_dapm_widgets_b[] = {

    /* Left Input Selection */
    SND_SOC_DAPM_MIXER("ADC_B Left Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_b_left_input_mixer_controls[0],
                       ARRAY_SIZE(adc_b_left_input_mixer_controls)),
    /* Right Input Selection */
    SND_SOC_DAPM_MIXER("ADC_B Right Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_b_right_input_mixer_controls[0],
                       ARRAY_SIZE(adc_b_right_input_mixer_controls)),
    /*PGA selection */
    SND_SOC_DAPM_PGA("ADC_B Left PGA", ADC_LEFT_APGA_CTRL(1), 7, 1, NULL, 0),
    SND_SOC_DAPM_PGA("ADC_B Right PGA", ADC_RIGHT_APGA_CTRL(1), 7, 1, NULL, 0),

    /*Digital Microphone Input Control for Left/Right ADC */
    SND_SOC_DAPM_MIXER("ADC_B Left DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_b_left_input_dmic_controls[0],
                       ARRAY_SIZE(adc_b_left_input_dmic_controls)),
    SND_SOC_DAPM_MIXER("ADC_B Right DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_b_right_input_dmic_controls[0],
                       ARRAY_SIZE(adc_b_right_input_dmic_controls)),

    SND_SOC_DAPM_ADC("ADC_B Left ADC", "ADC_B Left Capture", ADC_ADC_DIGITAL(1),
                     7, 0),
    SND_SOC_DAPM_ADC("ADC_B Right ADC", "ADC_B Right Capture",
                     ADC_ADC_DIGITAL(1), 6, 0),

    /* Inputs */
    SND_SOC_DAPM_INPUT("ADC_B_IN1_L"),
    SND_SOC_DAPM_INPUT("ADC_B_IN1_R"),
    SND_SOC_DAPM_INPUT("ADC_B_IN2_L"),
    SND_SOC_DAPM_INPUT("ADC_B_IN2_R"),
    SND_SOC_DAPM_INPUT("ADC_B_IN3_L"),
    SND_SOC_DAPM_INPUT("ADC_B_IN3_R"),
    SND_SOC_DAPM_INPUT("ADC_B_DIF1_L"),
    SND_SOC_DAPM_INPUT("ADC_B_DIF2_L"),
    SND_SOC_DAPM_INPUT("ADC_B_DIF3_L"),
    SND_SOC_DAPM_INPUT("ADC_B_DIF1_R"),
    SND_SOC_DAPM_INPUT("ADC_B_DIF2_R"),
    SND_SOC_DAPM_INPUT("ADC_B_DIF3_R"),
    SND_SOC_DAPM_INPUT("ADC_B_DMic_L"),
    SND_SOC_DAPM_INPUT("ADC_B_DMic_R"),
};

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
/* adc31xx_b Widget structure */
static const struct snd_soc_dapm_widget adc31xx_dapm_widgets_c[] = {

    /* Left Input Selection */
    SND_SOC_DAPM_MIXER("ADC_C Left Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_c_left_input_mixer_controls[0],
                       ARRAY_SIZE(adc_c_left_input_mixer_controls)),
    /* Right Input Selection */
    SND_SOC_DAPM_MIXER("ADC_C Right Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_c_right_input_mixer_controls[0],
                       ARRAY_SIZE(adc_c_right_input_mixer_controls)),
    /*PGA selection */
    SND_SOC_DAPM_PGA("ADC_C Left PGA", ADC_LEFT_APGA_CTRL(2), 7, 1, NULL, 0),
    SND_SOC_DAPM_PGA("ADC_C Right PGA", ADC_RIGHT_APGA_CTRL(2), 7, 1, NULL, 0),

    /*Digital Microphone Input Control for Left/Right ADC */
    SND_SOC_DAPM_MIXER("ADC_C Left DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_c_left_input_dmic_controls[0],
                       ARRAY_SIZE(adc_c_left_input_dmic_controls)),
    SND_SOC_DAPM_MIXER("ADC_C Right DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_c_right_input_dmic_controls[0],
                       ARRAY_SIZE(adc_c_right_input_dmic_controls)),

    SND_SOC_DAPM_ADC("ADC_C Left ADC", "ADC_C Left Capture", ADC_ADC_DIGITAL(2), 7, 0),
    SND_SOC_DAPM_ADC("ADC_C Right ADC", "ADC_C Right Capture", ADC_ADC_DIGITAL(2), 6, 0),

    /* Inputs */
    SND_SOC_DAPM_INPUT("ADC_C_IN1_L"),
    SND_SOC_DAPM_INPUT("ADC_C_IN1_R"),
    SND_SOC_DAPM_INPUT("ADC_C_IN2_L"),
    SND_SOC_DAPM_INPUT("ADC_C_IN2_R"),
    SND_SOC_DAPM_INPUT("ADC_C_IN3_L"),
    SND_SOC_DAPM_INPUT("ADC_C_IN3_R"),
    SND_SOC_DAPM_INPUT("ADC_C_DIF1_L"),
    SND_SOC_DAPM_INPUT("ADC_C_DIF2_L"),
    SND_SOC_DAPM_INPUT("ADC_C_DIF3_L"),
    SND_SOC_DAPM_INPUT("ADC_C_DIF1_R"),
    SND_SOC_DAPM_INPUT("ADC_C_DIF2_R"),
    SND_SOC_DAPM_INPUT("ADC_C_DIF3_R"),
    SND_SOC_DAPM_INPUT("ADC_C_DMic_L"),
    SND_SOC_DAPM_INPUT("ADC_C_DMic_R"),
};

/* adc31xx_b Widget structure */
static const struct snd_soc_dapm_widget adc31xx_dapm_widgets_d[] = {

    /* Left Input Selection */
    SND_SOC_DAPM_MIXER("ADC_D Left Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_d_left_input_mixer_controls[0],
                       ARRAY_SIZE(adc_d_left_input_mixer_controls)),
    /* Right Input Selection */
    SND_SOC_DAPM_MIXER("ADC_D Right Ip Select", SND_SOC_NOPM, 0, 0,
                       &adc_d_right_input_mixer_controls[0],
                       ARRAY_SIZE(adc_d_right_input_mixer_controls)),
    /*PGA selection */
    SND_SOC_DAPM_PGA("ADC_D Left PGA", ADC_LEFT_APGA_CTRL(3), 7, 1, NULL, 0),
    SND_SOC_DAPM_PGA("ADC_D Right PGA", ADC_RIGHT_APGA_CTRL(3), 7, 1, NULL, 0),

    /*Digital Microphone Input Control for Left/Right ADC */
    SND_SOC_DAPM_MIXER("ADC_D Left DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_d_left_input_dmic_controls[0],
                       ARRAY_SIZE(adc_d_left_input_dmic_controls)),
    SND_SOC_DAPM_MIXER("ADC_D Right DMic Ip", SND_SOC_NOPM, 0, 0,
                       &adc_d_right_input_dmic_controls[0],
                       ARRAY_SIZE(adc_d_right_input_dmic_controls)),

    SND_SOC_DAPM_ADC("ADC_D Left ADC", "ADC_D Left Capture", ADC_ADC_DIGITAL(3), 7, 0),
    SND_SOC_DAPM_ADC("ADC_D Right ADC", "ADC_D Right Capture", ADC_ADC_DIGITAL(3), 6, 0),

    /* Inputs */
    SND_SOC_DAPM_INPUT("ADC_D_IN1_L"),
    SND_SOC_DAPM_INPUT("ADC_D_IN1_R"),
    SND_SOC_DAPM_INPUT("ADC_D_IN2_L"),
    SND_SOC_DAPM_INPUT("ADC_D_IN2_R"),
    SND_SOC_DAPM_INPUT("ADC_D_IN3_L"),
    SND_SOC_DAPM_INPUT("ADC_D_IN3_R"),
    SND_SOC_DAPM_INPUT("ADC_D_DIF1_L"),
    SND_SOC_DAPM_INPUT("ADC_D_DIF2_L"),
    SND_SOC_DAPM_INPUT("ADC_D_DIF3_L"),
    SND_SOC_DAPM_INPUT("ADC_D_DIF1_R"),
    SND_SOC_DAPM_INPUT("ADC_D_DIF2_R"),
    SND_SOC_DAPM_INPUT("ADC_D_DIF3_R"),
    SND_SOC_DAPM_INPUT("ADC_D_DMic_L"),
    SND_SOC_DAPM_INPUT("ADC_D_DMic_R"),
};


#endif

static const struct snd_soc_dapm_route aic32xx_dapm_routes[] = {
    /* Left  Headphone Output */
    {"HPL Output Mixer", "L_DAC switch", "Left DAC"},
    {"HPL Output Mixer", "IN1_L switch", "IN1_L"},
    {"HPL Output Mixer", "MAL switch", "MAL PGA"},      //MAL
    {"HPL Output Mixer", "MAR switch", "MAR PGA"},      //MAR

    /* Right Headphone Output */
    {"HPR Output Mixer", "R_DAC switch", "Right DAC"},
    {"HPR Output Mixer", "IN1_R switch", "IN1_R"},
    {"HPR Output Mixer", "MAR switch", "MAR PGA"},      //MAR
    {"HPR Output Mixer", "L_DAC switch", "Left DAC"},

    {"HPL PGA", NULL, "HPL Output Mixer"},
    {"HPR PGA", NULL, "HPR Output Mixer"},

    {"HPL", NULL, "HPL PGA"},
    {"HPR", NULL, "HPR PGA"},

};

/* DAPM Routing related array declaration for ADC_A */
static const struct snd_soc_dapm_route intercon_a[] = {

/* Left input selection from switchs */
    {"ADC_A Left Ip Select", "ADC_A IN1_L switch", "ADC_A_IN1_L"},
    {"ADC_A Left Ip Select", "ADC_A IN2_L switch", "ADC_A_IN2_L"},
    {"ADC_A Left Ip Select", "ADC_A IN3_L switch", "ADC_A_IN3_L"},
    {"ADC_A Left Ip Select", "ADC_A DIF1_L switch", "ADC_A_DIF1_L"},
    {"ADC_A Left Ip Select", "ADC_A DIF2_L switch", "ADC_A_DIF2_L"},
    {"ADC_A Left Ip Select", "ADC_A DIF3_L switch", "ADC_A_DIF3_L"},
    {"ADC_A Left Ip Select", "ADC_A IN1_R switch", "ADC_A_IN1_R"},

/* Left input selection to left PGA */
    {"ADC_A Left PGA", NULL, "ADC_A Left Ip Select"},

/* Left PGA to left ADC */
    {"ADC_A Left ADC", NULL, "ADC_A Left PGA"},

/* Right input selection from switchs */
    {"ADC_A Right Ip Select", "ADC_A IN1_R switch", "ADC_A_IN1_R"},
    {"ADC_A Right Ip Select", "ADC_A IN2_R switch", "ADC_A_IN2_R"},
    {"ADC_A Right Ip Select", "ADC_A IN3_R switch", "ADC_A_IN3_R"},
    {"ADC_A Right Ip Select", "ADC_A DIF1_R switch", "ADC_A_DIF1_R"},
    {"ADC_A Right Ip Select", "ADC_A DIF2_R switch", "ADC_A_DIF2_R"},
    {"ADC_A Right Ip Select", "ADC_A DIF3_R switch", "ADC_A_DIF3_R"},
    {"ADC_A Right Ip Select", "ADC_A IN1_L switch", "ADC_A_IN1_L"},

/* Right input selection to right PGA */
    {"ADC_A Right PGA", NULL, "ADC_A Right Ip Select"},

/* Right PGA to right ADC */
    {"ADC_A Right ADC", NULL, "ADC_A Right PGA"},

/* Left DMic Input selection from switch */
    {"ADC_A Left DMic Ip", "ADC_A Left ADC switch", "ADC_A_DMic_L"},

/* Left DMic to left ADC */
    {"ADC_A Left ADC", NULL, "ADC_A Left DMic Ip"},

/* Right DMic Input selection from switch */
    {"ADC_A Right DMic Ip", "ADC_A Right ADC switch", "ADC_A_DMic_R"},

/* Right DMic to right ADC */
    {"ADC_A Right ADC", NULL, "ADC_A Right DMic Ip"},
};


/* DAPM Routing related array declaration for ADC_B */
static const struct snd_soc_dapm_route intercon_b[] = {

/* Left input selection from switchs */
    {"ADC_B Left Ip Select", "ADC_B IN1_L switch", "ADC_B_IN1_L"},
    {"ADC_B Left Ip Select", "ADC_B IN2_L switch", "ADC_B_IN2_L"},
    {"ADC_B Left Ip Select", "ADC_B IN3_L switch", "ADC_B_IN3_L"},
    {"ADC_B Left Ip Select", "ADC_B DIF1_L switch", "ADC_B_DIF1_L"},
    {"ADC_B Left Ip Select", "ADC_B DIF2_L switch", "ADC_B_DIF2_L"},
    {"ADC_B Left Ip Select", "ADC_B DIF3_L switch", "ADC_B_DIF3_L"},
    {"ADC_B Left Ip Select", "ADC_B IN1_R switch", "ADC_B_IN1_R"},

/* Left input selection to left PGA */
    {"ADC_B Left PGA", NULL, "ADC_B Left Ip Select"},

/* Left PGA to left ADC */
    {"ADC_B Left ADC", NULL, "ADC_B Left PGA"},

/* Right input selection from switchs */
    {"ADC_B Right Ip Select", "ADC_B IN1_R switch", "ADC_B_IN1_R"},
    {"ADC_B Right Ip Select", "ADC_B IN2_R switch", "ADC_B_IN2_R"},
    {"ADC_B Right Ip Select", "ADC_B IN3_R switch", "ADC_B_IN3_R"},
    {"ADC_B Right Ip Select", "ADC_B DIF1_R switch", "ADC_B_DIF1_R"},
    {"ADC_B Right Ip Select", "ADC_B DIF2_R switch", "ADC_B_DIF2_R"},
    {"ADC_B Right Ip Select", "ADC_B DIF3_R switch", "ADC_B_DIF3_R"},
    {"ADC_B Right Ip Select", "ADC_B IN1_L switch", "ADC_B_IN1_L"},

/* Right input selection to right PGA */
    {"ADC_B Right PGA", NULL, "ADC_B Right Ip Select"},

/* Right PGA to right ADC */
    {"ADC_B Right ADC", NULL, "ADC_B Right PGA"},

/* Left DMic Input selection from switch */
    {"ADC_B Left DMic Ip", "ADC_B Left ADC switch", "ADC_B_DMic_L"},

/* Left DMic to left ADC */
    {"ADC_B Left ADC", NULL, "ADC_B Left DMic Ip"},

/* Right DMic Input selection from switch */
    {"ADC_B Right DMic Ip", "ADC_B Right ADC switch", "ADC_B_DMic_R"},

/* Right DMic to right ADC */
    {"ADC_B Right ADC", NULL, "ADC_B Right DMic Ip"},
};

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
/* DAPM Routing related array declaration for ADC_C */
static const struct snd_soc_dapm_route intercon_c[] = {

/* Left input selection from switchs */
    {"ADC_C Left Ip Select", "ADC_C IN1_L switch", "ADC_C_IN1_L"},
    {"ADC_C Left Ip Select", "ADC_C IN2_L switch", "ADC_C_IN2_L"},
    {"ADC_C Left Ip Select", "ADC_C IN3_L switch", "ADC_C_IN3_L"},
    {"ADC_C Left Ip Select", "ADC_C DIF1_L switch", "ADC_C_DIF1_L"},
    {"ADC_C Left Ip Select", "ADC_C DIF2_L switch", "ADC_C_DIF2_L"},
    {"ADC_C Left Ip Select", "ADC_C DIF3_L switch", "ADC_C_DIF3_L"},
    {"ADC_C Left Ip Select", "ADC_C IN1_R switch", "ADC_C_IN1_R"},

/* Left input selection to left PGA */
    {"ADC_C Left PGA", NULL, "ADC_C Left Ip Select"},

/* Left PGA to left ADC */
    {"ADC_C Left ADC", NULL, "ADC_C Left PGA"},

/* Right input selection from switchs */
    {"ADC_C Right Ip Select", "ADC_C IN1_R switch", "ADC_C_IN1_R"},
    {"ADC_C Right Ip Select", "ADC_C IN2_R switch", "ADC_C_IN2_R"},
    {"ADC_C Right Ip Select", "ADC_C IN3_R switch", "ADC_C_IN3_R"},
    {"ADC_C Right Ip Select", "ADC_C DIF1_R switch", "ADC_C_DIF1_R"},
    {"ADC_C Right Ip Select", "ADC_C DIF2_R switch", "ADC_C_DIF2_R"},
    {"ADC_C Right Ip Select", "ADC_C DIF3_R switch", "ADC_C_DIF3_R"},
    {"ADC_C Right Ip Select", "ADC_C IN1_L switch", "ADC_C_IN1_L"},

/* Right input selection to right PGA */
    {"ADC_C Right PGA", NULL, "ADC_C Right Ip Select"},

/* Right PGA to right ADC */
    {"ADC_C Right ADC", NULL, "ADC_C Right PGA"},

/* Left DMic Input selection from switch */
    {"ADC_C Left DMic Ip", "ADC_C Left ADC switch", "ADC_C_DMic_L"},

/* Left DMic to left ADC */
    {"ADC_C Left ADC", NULL, "ADC_C Left DMic Ip"},

/* Right DMic Input selection from switch */
    {"ADC_C Right DMic Ip", "ADC_C Right ADC switch", "ADC_C_DMic_R"},

/* Right DMic to right ADC */
    {"ADC_C Right ADC", NULL, "ADC_C Right DMic Ip"},
};


/* DAPM Routing related array declaration for ADC_D */
static const struct snd_soc_dapm_route intercon_d[] = {

/* Left input selection from switchs */
    {"ADC_D Left Ip Select", "ADC_D IN1_L switch", "ADC_D_IN1_L"},
    {"ADC_D Left Ip Select", "ADC_D IN2_L switch", "ADC_D_IN2_L"},
    {"ADC_D Left Ip Select", "ADC_D IN3_L switch", "ADC_D_IN3_L"},
    {"ADC_D Left Ip Select", "ADC_D DIF1_L switch", "ADC_D_DIF1_L"},
    {"ADC_D Left Ip Select", "ADC_D DIF2_L switch", "ADC_D_DIF2_L"},
    {"ADC_D Left Ip Select", "ADC_D DIF3_L switch", "ADC_D_DIF3_L"},
    {"ADC_D Left Ip Select", "ADC_D IN1_R switch", "ADC_D_IN1_R"},

/* Left input selection to left PGA */
    {"ADC_D Left PGA", NULL, "ADC_D Left Ip Select"},

/* Left PGA to left ADC */
    {"ADC_D Left ADC", NULL, "ADC_D Left PGA"},

/* Right input selection from switchs */
    {"ADC_D Right Ip Select", "ADC_D IN1_R switch", "ADC_D_IN1_R"},
    {"ADC_D Right Ip Select", "ADC_D IN2_R switch", "ADC_D_IN2_R"},
    {"ADC_D Right Ip Select", "ADC_D IN3_R switch", "ADC_D_IN3_R"},
    {"ADC_D Right Ip Select", "ADC_D DIF1_R switch", "ADC_D_DIF1_R"},
    {"ADC_D Right Ip Select", "ADC_D DIF2_R switch", "ADC_D_DIF2_R"},
    {"ADC_D Right Ip Select", "ADC_D DIF3_R switch", "ADC_D_DIF3_R"},
    {"ADC_D Right Ip Select", "ADC_D IN1_L switch", "ADC_D_IN1_L"},

/* Right input selection to right PGA */
    {"ADC_D Right PGA", NULL, "ADC_D Right Ip Select"},

/* Right PGA to right ADC */
    {"ADC_D Right ADC", NULL, "ADC_D Right PGA"},

/* Left DMic Input selection from switch */
    {"ADC_D Left DMic Ip", "ADC_D Left ADC switch", "ADC_D_DMic_L"},

/* Left DMic to left ADC */
    {"ADC_D Left ADC", NULL, "ADC_D Left DMic Ip"},

/* Right DMic Input selection from switch */
    {"ADC_D Right DMic Ip", "ADC_D Right ADC switch", "ADC_D_DMic_R"},

/* Right DMic to right ADC */
    {"ADC_D Right ADC", NULL, "ADC_D Right DMic Ip"},
};

#endif

/*
*****************************************************************************
* Function Definitions
*****************************************************************************
*/
static int aic32xx_hp_event(struct snd_soc_dapm_widget *w,
                            struct snd_kcontrol *kcontrol, int event)
{
    return 0;
}


/*
 *----------------------------------------------------------------------------
 * Function : __amp_enable_info
 * Purpose  : This function is to initialize data to enable the power amplifier.
 *----------------------------------------------------------------------------
 */
static int __amp_enable_info(struct snd_kcontrol *kcontrol,
                              struct snd_ctl_elem_info *uinfo)
{
        uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
        uinfo->count = 1;
        uinfo->value.integer.min = 0;
        uinfo->value.integer.max = 1;

        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : __amp_enable_get
 * Purpose  : This function is used to get the state of the power amplifier.
 *----------------------------------------------------------------------------
 */
static int __amp_enable_get(struct snd_kcontrol *kcontrol,
                             struct snd_ctl_elem_value *ucontrol)
{
        //struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

        ucontrol->value.integer.value[0] = amp_enable;

        return 0;
}



/*
 *----------------------------------------------------------------------------
 * Function : __amp_enable_put
 * Purpose  : __amp_enable_put is called to enable the power amplifier
 *----------------------------------------------------------------------------
 */
static int __amp_enable_put(struct snd_kcontrol *kcontrol,
                             struct snd_ctl_elem_value *ucontrol)
{
	int enable = ucontrol->value.integer.value[0];

	if(enable)
		gpio_set_value(AMP_ENABLE_GPIO, 1);
	else
		gpio_set_value(AMP_ENABLE_GPIO, 0);

	amp_enable = enable;
        return 0;
}



/**********************************************************************************
 * Internal Data Collection
 * Fix the values for DRC - PGA - Crossover
 *********************************************************************************/
static const struct aic32xx_configs biquad_settings_Capt_IDC[] = {
//    ========================
//    =    Capture Filters   =
//    ========================
//    20 ADC HPF Bessel
//    Cleaner HPF without bump in 16kHz mode
//    Coefficient write to 3101-A ADC Left
    //Page change to 4
    {0x210e, 0x7f}, // reg[4][14] = 127
    {0x210f, 0x22}, // reg[4][15] = 34
    {0x2110, 0x80}, // reg[4][16] = 128
    {0x2111, 0xde}, // reg[4][17] = 222
    {0x2112, 0x7f}, // reg[4][18] = 127
    {0x2113, 0x22}, // reg[4][19] = 34
    {0x2114, 0x7f}, // reg[4][20] = 127
    {0x2115, 0x21}, // reg[4][21] = 33
    {0x2116, 0x81}, // reg[4][22] = 129
    {0x2117, 0xba}, // reg[4][23] = 186
//    Coefficient write to 3101-A ADC Rght
    //Page change to 4
    {0x214e, 0x7f}, // reg[4][78] = 127
    {0x214f, 0x22}, // reg[4][79] = 34
    {0x2150, 0x80}, // reg[4][80] = 128
    {0x2151, 0xde}, // reg[4][81] = 222
    {0x2152, 0x7f}, // reg[4][82] = 127
    {0x2153, 0x22}, // reg[4][83] = 34
    {0x2154, 0x7f}, // reg[4][84] = 127
    {0x2155, 0x21}, // reg[4][85] = 33
    {0x2156, 0x81}, // reg[4][86] = 129
    {0x2157, 0xba}, // reg[4][87] = 186
//    Coefficient write to 3101-B ADC Left
    //Page change to 4
    {0x220e, 0x7f}, // reg[4][14] = 127
    {0x220f, 0x22}, // reg[4][15] = 34
    {0x2210, 0x80}, // reg[4][16] = 128
    {0x2211, 0xde}, // reg[4][17] = 222
    {0x2212, 0x7f}, // reg[4][18] = 127
    {0x2213, 0x22}, // reg[4][19] = 34
    {0x2214, 0x7f}, // reg[4][20] = 127
    {0x2215, 0x21}, // reg[4][21] = 33
    {0x2216, 0x81}, // reg[4][22] = 129
    {0x2217, 0xba}, // reg[4][23] = 186
//    Coefficient write to 3101-B ADC Rght
    //Page change to 4
    {0x224e, 0x7f}, // reg[4][78] = 127
    {0x224f, 0x22}, // reg[4][79] = 34
    {0x2250, 0x80}, // reg[4][80] = 128
    {0x2251, 0xde}, // reg[4][81] = 222
    {0x2252, 0x7f}, // reg[4][82] = 127
    {0x2253, 0x22}, // reg[4][83] = 34
    {0x2254, 0x7f}, // reg[4][84] = 127
    {0x2255, 0x21}, // reg[4][85] = 33
    {0x2256, 0x81}, // reg[4][86] = 129
    {0x2257, 0xba}, // reg[4][87] = 186

    /* config record path default gains */
    {ADC_LEFT_APGA_CTRL(0),  MIC_PGA_GAIN_IDC},     // PGA 20dB Gain
    {ADC_RIGHT_APGA_CTRL(0), MIC_PGA_GAIN_IDC},     // PGA 20dB Gain
    {ADC_LEFT_APGA_CTRL(1),  MIC_PGA_GAIN_IDC},     // PGA 20dB Gain
    {ADC_RIGHT_APGA_CTRL(1), MIC_PGA_GAIN_IDC},     // PGA 20dB Gain
};

static const struct aic32xx_configs biquad_settings_Play_IDC[] = {
//    ========================
//    =    Playback Filters  =
//    ========================
//  Crossover filter (Fc @ 1Khz) and 2nd order HPF40Hz
      { MAKE_REG( 0, 44, 12), 0x00},
      { MAKE_REG( 0, 44, 13), 0x80},
      { MAKE_REG( 0, 44, 14), 0x52},
      { MAKE_REG( 0, 44, 16), 0x00},
      { MAKE_REG( 0, 44, 17), 0x80},
      { MAKE_REG( 0, 44, 18), 0x52},
      { MAKE_REG( 0, 44, 20), 0x00},
      { MAKE_REG( 0, 44, 21), 0x80},
      { MAKE_REG( 0, 44, 22), 0x52},
      { MAKE_REG( 0, 44, 24), 0x74},
      { MAKE_REG( 0, 44, 25), 0x2E},
      { MAKE_REG( 0, 44, 26), 0x2A},
      { MAKE_REG( 0, 44, 28), 0x95},
      { MAKE_REG( 0, 44, 29), 0xA2},
      { MAKE_REG( 0, 44, 30), 0x61},
      { MAKE_REG( 0, 44, 32), 0x00},
      { MAKE_REG( 0, 44, 33), 0x80},
      { MAKE_REG( 0, 44, 34), 0x52},
      { MAKE_REG( 0, 44, 36), 0x00},
      { MAKE_REG( 0, 44, 37), 0x80},
      { MAKE_REG( 0, 44, 38), 0x52},
      { MAKE_REG( 0, 44, 40), 0x00},
      { MAKE_REG( 0, 44, 41), 0x80},
      { MAKE_REG( 0, 44, 42), 0x52},
      { MAKE_REG( 0, 44, 44), 0x74},
      { MAKE_REG( 0, 44, 45), 0x2E},
      { MAKE_REG( 0, 44, 46), 0x2A},
      { MAKE_REG( 0, 44, 48), 0x95},
      { MAKE_REG( 0, 44, 49), 0xA2},
      { MAKE_REG( 0, 44, 50), 0x61},
      { MAKE_REG( 0, 44, 52), 0x7F},
      { MAKE_REG( 0, 44, 53), 0x55},
      { MAKE_REG( 0, 44, 54), 0x18},
      { MAKE_REG( 0, 44, 56), 0x80},
      { MAKE_REG( 0, 44, 57), 0xAA},
      { MAKE_REG( 0, 44, 58), 0xE8},
      { MAKE_REG( 0, 44, 60), 0x7F},
      { MAKE_REG( 0, 44, 61), 0x55},
      { MAKE_REG( 0, 44, 62), 0x18},
      { MAKE_REG( 0, 44, 64), 0x7F},
      { MAKE_REG( 0, 44, 65), 0x54},
      { MAKE_REG( 0, 44, 66), 0xDE},
      { MAKE_REG( 0, 44, 68), 0x81},
      { MAKE_REG( 0, 44, 69), 0x55},
      { MAKE_REG( 0, 44, 70), 0x5D},
      { MAKE_REG( 0, 45, 20), 0x74},
      { MAKE_REG( 0, 45, 21), 0xAE},
      { MAKE_REG( 0, 45, 22), 0x7C},
      { MAKE_REG( 0, 45, 24), 0x8B},
      { MAKE_REG( 0, 45, 25), 0x51},
      { MAKE_REG( 0, 45, 26), 0x84},
      { MAKE_REG( 0, 45, 28), 0x74},
      { MAKE_REG( 0, 45, 29), 0xAE},
      { MAKE_REG( 0, 45, 30), 0x7C},
      { MAKE_REG( 0, 45, 32), 0x74},
      { MAKE_REG( 0, 45, 33), 0x2E},
      { MAKE_REG( 0, 45, 34), 0x2A},
      { MAKE_REG( 0, 45, 36), 0x95},
      { MAKE_REG( 0, 45, 37), 0xA2},
      { MAKE_REG( 0, 45, 38), 0x61},
      { MAKE_REG( 0, 45, 40), 0x74},
      { MAKE_REG( 0, 45, 41), 0xAE},
      { MAKE_REG( 0, 45, 42), 0x7C},
      { MAKE_REG( 0, 45, 44), 0x8B},
      { MAKE_REG( 0, 45, 45), 0x51},
      { MAKE_REG( 0, 45, 46), 0x84},
      { MAKE_REG( 0, 45, 48), 0x74},
      { MAKE_REG( 0, 45, 49), 0xAE},
      { MAKE_REG( 0, 45, 50), 0x7C},
      { MAKE_REG( 0, 45, 52), 0x74},
      { MAKE_REG( 0, 45, 53), 0x2E},
      { MAKE_REG( 0, 45, 54), 0x2A},
      { MAKE_REG( 0, 45, 56), 0x95},
      { MAKE_REG( 0, 45, 57), 0xA2},
      { MAKE_REG( 0, 45, 58), 0x61},
      { MAKE_REG( 0, 45, 60), 0x7F},
      { MAKE_REG( 0, 45, 61), 0x55},
      { MAKE_REG( 0, 45, 62), 0x18},
      { MAKE_REG( 0, 45, 64), 0x80},
      { MAKE_REG( 0, 45, 65), 0xAA},
      { MAKE_REG( 0, 45, 66), 0xE8},
      { MAKE_REG( 0, 45, 68), 0x7F},
      { MAKE_REG( 0, 45, 69), 0x55},
      { MAKE_REG( 0, 45, 70), 0x18},
      { MAKE_REG( 0, 45, 72), 0x7F},
      { MAKE_REG( 0, 45, 73), 0x54},
      { MAKE_REG( 0, 45, 74), 0xDE},
      { MAKE_REG( 0, 45, 76), 0x81},
      { MAKE_REG( 0, 45, 77), 0x55},
      { MAKE_REG( 0, 45, 78), 0x5D},
      { MAKE_REG( 0, 46, 52), 0x7F},    //DRC HPF: Page 46: N0 52 - 55
      { MAKE_REG( 0, 46, 53), 0xFF},    //Bypassed: N0 = 0x7FFFFF;
      { MAKE_REG( 0, 46, 54), 0xFF},
      { MAKE_REG( 0, 46, 55), 0x00},    //N1 = 0
      { MAKE_REG( 0, 46, 56), 0x00},
      { MAKE_REG( 0, 46, 57), 0x00},
      { MAKE_REG( 0, 46, 58), 0x00},
      { MAKE_REG( 0, 46, 59), 0x00},
      { MAKE_REG( 0, 46, 60), 0x00},   //D1
      { MAKE_REG( 0, 46, 61), 0x00},
      { MAKE_REG( 0, 46, 62), 0x00},
      { MAKE_REG( 0, 46, 63), 0x00},
      { MAKE_REG( 0, 46, 64), 0x7F},    //DRC LPF: Page 46: N0 64 - 67
      { MAKE_REG( 0, 46, 65), 0xFF},    //Bypassed: N0 = 0x7FFFFF;
      { MAKE_REG( 0, 46, 66), 0xFF},
      { MAKE_REG( 0, 46, 67), 0x00},    //N1 = 0
      { MAKE_REG( 0, 46, 68), 0x00},
      { MAKE_REG( 0, 46, 69), 0x00},
      { MAKE_REG( 0, 46, 70), 0x00},
      { MAKE_REG( 0, 46, 71), 0x00},
      { MAKE_REG( 0, 46, 72), 0x00},   //D1
      { MAKE_REG( 0, 46, 73), 0x00},
      { MAKE_REG( 0, 46, 74), 0x00},
      { MAKE_REG( 0, 46, 75), 0x00},
#ifdef SET_DRC_OFF
      // Force DRC (Dynamic Range Compression) off
      {DRC_CTRL_REG1, 0x0F},
#else
      // Enable DRC
      {DRC_CTRL_REG1, 0x6F},
      {DRC_CTRL_REG2, 0x00},
      {DRC_CTRL_REG3, 0xC6},
#endif
};


/**----------------------------------------------------------------------------
 * Function : __IDC_info
 * Purpose  : This function is to initialize data to enable IDC mode.
 *----------------------------------------------------------------------------*/
static int __IDC_info(struct snd_kcontrol *kcontrol,
                      struct snd_ctl_elem_info *uinfo)
{
        uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
        uinfo->count = 1;
        uinfo->value.integer.min = 0;
        uinfo->value.integer.max = 1;

        return 0;
}

static int IDC_enabled = 0;
/**----------------------------------------------------------------------------
 * Function : __IDC_get
 * Purpose  : This function is used to get the state of IDC mode.
 *----------------------------------------------------------------------------*/
static int __IDC_get(struct snd_kcontrol *kcontrol,
                     struct snd_ctl_elem_value *ucontrol)
{
        ucontrol->value.integer.value[0] = IDC_enabled;

        return 0;
}



/**----------------------------------------------------------------------------
 * Function : __IDC_put
 * Purpose  : Called to enable the IDC mode.
 *----------------------------------------------------------------------------*/
static int __IDC_put(struct snd_kcontrol *kcontrol,
                     struct snd_ctl_elem_value *ucontrol)
{
	int enable = ucontrol->value.integer.value[0];
	if(enable) {
        printk("IDC Enabled\n");
    }
    else if( IDC_enabled) {
        printk("IDC Disabled\n");
    }
    IDC_enabled = enable;
    return 0;
}

/*********************************************************************************
 * End of Internal Data Collection
 ********************************************************************************/


/*
 *----------------------------------------------------------------------------

 * Function : aic32xx_reset_cache
 * Purpose  : This function is to reset the cache.
 *----------------------------------------------------------------------------
 */
int aic32xx_reset_cache(struct snd_soc_codec *codec)
{
    if (codec->reg_cache != NULL) {
        memcpy(codec->reg_cache, aic32xx_reg, sizeof(aic32xx_reg));
        return 0;
    }

    codec->reg_cache = kmemdup(aic32xx_reg, sizeof(aic32xx_reg), GFP_KERNEL);
    if (codec->reg_cache == NULL) {
        printk(KERN_ERR "aic32xx: kmemdup failed\n");
        return -ENOMEM;
    }

    return 0;
}


/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_change_page
 * Purpose  : This function is to switch between page 0 and page 1.
 *----------------------------------------------------------------------------
 */
int aic32xx_change_page(struct snd_soc_codec *codec, u8 new_page)
{
    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);

    u8      data[2];
    int     status;

    data[0] = 0;
    data[1] = new_page;
    aic32xx->page_no = new_page;
    status = i2c_master_send(codec->control_data, data, 2);

    if (status != 2) {
        printk("%s: Error in changing page to 1. Error = %d\n", __FUNCTION__,
             status);
        return -1;
    }

    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : adc31xx_change_page
 * Purpose  : This function is to switch between page 0 and page 1.
 *
 *----------------------------------------------------------------------------
 */
int adc31xx_change_page(unsigned int device, struct snd_soc_codec *codec, u8 new_page)
{
    struct aic32xx_priv *adc31xx = snd_soc_codec_get_drvdata(codec);

    u8      data[2];
    int     status;

    data[0] = 0x0;
    data[1] = new_page;

    if(device == 0){
        printk("adc31xx_change_page %d Error\n", device);
        return -1;
    }
    device -= 1;

    status = i2c_master_send(adc31xx->adc_control_data[device], data, 2);
    if (status != 2) {
        printk("%s device %d: I2C Wrte Error = %d\n", __FUNCTION__,
             device, status);
        return -1;
    }
    adc31xx->adc_page_no[device] = new_page;
    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_write_reg_cache
 * Purpose  : This function is to write aic32xx register cache
 *----------------------------------------------------------------------------
 */
inline void aic32xx_write_reg_cache(struct snd_soc_codec *codec, u8 device,
                                    u8 page, u16 reg, u8 value)
{
    u8     *cache = codec->reg_cache;
    int    offset;

    if(device >= 1) {
        offset = AIC3204_CACHEREGNUM + (device -1) * ADC3101_CACHEREGNUM;
    }
    else {
            offset = 0;
    }

    if (page == 1)
        offset += 128;

    if (offset + reg >= AIC32xx_CACHEREGNUM)
        return;

//    printk("(%d) wrote %#x to %#x (%d) cache offset %d\n", device,
//           value, reg, reg, offset);

    cache[offset + reg] = value;
}


/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_read_reg_cache
 * Purpose  : This function is to read the register cache
 *
 *----------------------------------------------------------------------------
 */
static unsigned int aic32xx_read_reg_cache(struct snd_soc_codec *codec,
                                           unsigned int reg)
{
    u8     *cache = codec->reg_cache;
    u16     device, page, reg_no;
    int    offset;

    page = (reg & 0x3f800) >> 11;
    device = (reg & 0x700) >> 8;
    reg_no = reg & 0xff;

    if(device >= 1) {
        offset = AIC3204_CACHEREGNUM + (device -1) * ADC3101_CACHEREGNUM;
    }
    else {
            offset = 0;
    }

    if (page == 1)
        offset += 128;

    if (offset + reg_no >= AIC32xx_CACHEREGNUM)
        return -1;
//    printk("(%d) read %#x from %#x (%d) cache offset %d\n", device,
//           cache[offset + reg_no], reg_no, reg_no, offset);
    return cache[offset + reg_no];
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_write
 * Purpose  : This function is to write to the aic32xx register space.
 *----------------------------------------------------------------------------
 */
static int aic32xx_write(struct snd_soc_codec *codec, unsigned int reg,
                         unsigned int value)
{
    int ret = 0;
    u8 data[2];
    u16 page;
    u16 device;
    unsigned int reg_no;
    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);
    struct i2c_client *i2cdev;

    page = (reg & 0x3f800) >> 11;
    device = (reg & 0x700) >> 8;
    reg_no = reg & 0xff;

    mutex_lock(&aic32xx->codecMutex);

    if (device >= 1) {
        i2cdev = aic32xx->adc_control_data[device-1];
        if (aic32xx->adc_page_no[device-1] != page)
            adc31xx_change_page(device, codec, page);
    }
    else {
        i2cdev = codec->control_data;
        if (aic32xx->page_no != page)
            aic32xx_change_page(codec, page);
    }

    dprintk("AIC Write dev %#x page %d reg %#x val %#x\n", device, page, reg_no, value);

    /*
     * data is
     *   D15..D8 register offset
     *   D7...D0 register data
     */
    data[0] = reg_no;
    data[1] = value;

    if (i2c_master_send(i2cdev, data, 2) != 2) {
        printk("Error in i2c write in i2c dev: %x \n", device);

        ret = -EIO;
    }
    else if (page <2) {
        // Shadow the first two pages only
        aic32xx_write_reg_cache(codec, device, page, reg_no, value);
    }

    mutex_unlock(&aic32xx->codecMutex);

    return ret;
}

int aic32xx_update_bits(struct snd_soc_codec *codec, unsigned short reg,
                        unsigned int mask, unsigned int value)
{
    int change;
    unsigned int old, new;

    old = aic32xx_read_reg_cache(codec, reg);
    new = (old & ~mask) | value;
    change = old != new;
    if (change)
        aic32xx_write(codec, reg, new);

    return change;
}

/*
 *----------------------------------------------------------------------------
 * Function : _get_divs
 * Purpose  : This function is to get required divisor from the "aic32xx_divs"
 *            table.
 *----------------------------------------------------------------------------
 */
static inline int _get_divs(int mclk, int rate)
{
    int     i;

    dprintk("mclk: %d, rate: %d\n", mclk, rate);
    for (i = 0; i < ARRAY_SIZE(aic32xx_divs); i++) {
        if ((aic32xx_divs[i].rate == rate)
            && (aic32xx_divs[i].mclk == mclk)) {
            return i;
        }
    }
    printk
        ("Master clock and sample rate is not supported, mclk = %d rate = %d\n",
         mclk, rate);
    return -EINVAL;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_add_widgets
 * Purpose  : This function is to add the dapm widgets. This routine will be
 *        invoked during the Audio Driver Initialization.
 *            The following are the main widgets supported :
 *                # Left DAC to Left Outputs
 *                # Right DAC to Right Outputs
 *        # Left Inputs to Left ADC
 *        # Right Inputs to Right ADC
 *----------------------------------------------------------------------------
 */
static int aic32xx_add_widgets(struct snd_soc_codec *codec)
{

    dprintk("Function : %s \n", __FUNCTION__);

        /***AIC3204 ****/
    snd_soc_dapm_new_controls(codec, aic32xx_dapm_widgets,
                              ARRAY_SIZE(aic32xx_dapm_widgets));
    dprintk("#Completed adding new dapm widget controls size=%d\n",
            ARRAY_SIZE(aic32xx_dapm_widgets));

    /* set up audi path interconnects */
    snd_soc_dapm_add_routes(codec, aic32xx_dapm_routes,
                            ARRAY_SIZE(aic32xx_dapm_routes));
    dprintk("#Completed adding DAPM routes = %d\n",
            ARRAY_SIZE(aic32xx_dapm_routes));

    //snd_soc_dapm_new_widgets(codec);

        /***ADC3101_A ****/

    snd_soc_dapm_new_controls(codec, adc31xx_dapm_widgets_a,
                              ARRAY_SIZE(adc31xx_dapm_widgets_a));

    /* set up audio path interconnects */
    snd_soc_dapm_add_routes(codec, intercon_a, ARRAY_SIZE(intercon_a));

    //snd_soc_dapm_new_widgets(codec);

        /***ADC3101_B ****/
    snd_soc_dapm_new_controls(codec, adc31xx_dapm_widgets_b,
                              ARRAY_SIZE(adc31xx_dapm_widgets_b));

    /* set up audio path interconnects */
    snd_soc_dapm_add_routes(codec, intercon_b, ARRAY_SIZE(intercon_b));
    snd_soc_dapm_new_widgets(codec);

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
        /***ADC3101_C ****/
    snd_soc_dapm_new_controls(codec, adc31xx_dapm_widgets_c,
                              ARRAY_SIZE(adc31xx_dapm_widgets_c));

    /* set up audio path interconnects */
    snd_soc_dapm_add_routes(codec, intercon_c, ARRAY_SIZE(intercon_c));
    snd_soc_dapm_new_widgets(codec);


        /***ADC3101_D ****/
    snd_soc_dapm_new_controls(codec, adc31xx_dapm_widgets_d,
                              ARRAY_SIZE(adc31xx_dapm_widgets_d));

    /* set up audio path interconnects */
    snd_soc_dapm_add_routes(codec, intercon_d, ARRAY_SIZE(intercon_d));
    snd_soc_dapm_new_widgets(codec);
#endif
    return 0;
}

/******************************************************************************
* Set sample rate (_play and _capt) are used for async mcbsp operation
*******************************************************************************/
static int __set_sample_rate_play(struct snd_soc_codec *codec, int div_idx)
{

    /* turn off PLL and M/N dividers on aic3204 */
    aic32xx_write(codec, CLK_REG_2, 0x00);
    aic32xx_write(codec, NDAC_CLK_REG_6, 0x00);
    aic32xx_write(codec, MDAC_CLK_REG_7, 0x00);
    aic32xx_write(codec, NADC_CLK_REG_8, 0x00);
    aic32xx_write(codec, MADC_CLK_REG_9, 0x00);
    aic32xx_write(codec, CLK_REG_11, 0x00);

    /* set OSR???s, M/N dividers and enable dividers; ensure NADC/NDAC not enabled */
    aic32xx_write(codec, DAC_OSR_MSB, (aic32xx_divs[div_idx].dosr >> 8) & 0x3);
    aic32xx_write(codec, DAC_OSR_LSB, aic32xx_divs[div_idx].dosr & 0xff);
    aic32xx_write(codec, ADC_OSR_REG, aic32xx_divs[div_idx].aosr & 0xff);


    aic32xx_write(codec, NDAC_CLK_REG_6,(aic32xx_divs[div_idx].ndac & 0x7f));
    aic32xx_write(codec, MDAC_CLK_REG_7,
        0x80 | (aic32xx_divs[div_idx].mdac & 0x7f));
    aic32xx_write(codec, NADC_CLK_REG_8,(aic32xx_divs[div_idx].nadc & 0x7f));

    aic32xx_write(codec, NDAC_CLK_REG_6,
        0x80 | (aic32xx_divs[div_idx].ndac & 0x7f));
    aic32xx_write(codec, NADC_CLK_REG_8,
        0x80 | (aic32xx_divs[div_idx].nadc & 0x7f));

    aic32xx_write(codec, NDAC_CLK_REG_6,(aic32xx_divs[div_idx].ndac & 0x7f));
    aic32xx_write(codec, NADC_CLK_REG_8,(aic32xx_divs[div_idx].nadc & 0x7f));

    aic32xx_write(codec, MDAC_CLK_REG_7,
        0x80 | (aic32xx_divs[div_idx].mdac & 0x7f));
    aic32xx_write(codec, MADC_CLK_REG_9,
        0x80 | (aic32xx_divs[div_idx].madc & 0x7f));

    aic32xx_write(codec, CLK_REG_11,
        0x80 | (aic32xx_divs[div_idx].blck_N & 0x7f));


    /* set PLL clock values aic3204 */
    aic32xx_write(codec, CLK_REG_3, aic32xx_divs[div_idx].pll_j & 0x3f);
    aic32xx_write(codec, CLK_REG_4, (aic32xx_divs[div_idx].pll_d >> 8) & 0x3f);
    aic32xx_write(codec, CLK_REG_5, aic32xx_divs[div_idx].pll_d & 0xff);
    aic32xx_write(codec, CLK_REG_2,
        0x80 | (aic32xx_divs[div_idx].p_val & 0x7) << 4 | 0x1);

    return 0;
}

static int __set_sample_rate_capt(struct snd_soc_codec *codec, int div_idx)
{

    int i = 0;
    /*turn off pll and m/n dividers on adc3101*/
    aic32xx_write(codec, ADC_PLL_PROG_PR(0), 0x00);
    aic32xx_write(codec, ADC_ADC_NADC(0), 0x00);
    aic32xx_write(codec, ADC_ADC_MADC(0), 0x00);
    aic32xx_write(codec, ADC_BCLK_N_DIV(0), 0X00);

    /* set M/N dividers for non master ADC3101's. Hardcoded */

    for(i=1; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec,ADC_ADC_NADC(i),0x81);
       aic32xx_write(codec,ADC_ADC_MADC(i),0x82);
       aic32xx_write(codec, ADC_PLL_PROG_PR(i), (aic32xx_divs[div_idx].p_val & 0x7) << 4 | 0x1);
       aic32xx_write(codec, ADC_ADC_AOSR(i), aic32xx_divs[div_idx].aosr & 0xff);
    }

    /* set OSR???s, M/N dividers and enable dividers; ensure NADC/NDAC not enabled */
    aic32xx_write(codec, ADC_ADC_AOSR(0), aic32xx_divs[div_idx].aosr & 0xff);

    aic32xx_write(codec, ADC_ADC_NADC(0),(aic32xx_divs[div_idx].nadc & 0x7f));

    aic32xx_write(codec, ADC_ADC_NADC(0),
        0x80 | (aic32xx_divs[div_idx].nadc & 0x7f));

    aic32xx_write(codec,ADC_ADC_NADC(0),(aic32xx_divs[div_idx].nadc & 0x7f));

    aic32xx_write(codec, ADC_ADC_MADC(0),
        0x80 | (aic32xx_divs[div_idx].madc & 0x7f));

    aic32xx_write(codec, ADC_BCLK_N_DIV(0),
        0x80 | (aic32xx_divs[div_idx].blck_N & 0x7f));

    /* set PLL clock values adc3101 */
    aic32xx_write(codec, ADC_PLL_PROG_J(0), aic32xx_divs[div_idx].pll_j & 0x3f);
    aic32xx_write(codec, ADC_PLL_PROG_D_MSB(0), (aic32xx_divs[div_idx].pll_d >> 8) & 0x3f);
    aic32xx_write(codec, ADC_PLL_PROG_D_LSB(0), aic32xx_divs[div_idx].pll_d & 0xff);
    aic32xx_write(codec, ADC_PLL_PROG_PR(0),
        0x80 | (aic32xx_divs[div_idx].p_val & 0x7) << 4 | 0x1);

    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_hw_params_play
 * Purpose  : This function is to set the hardware parameters for AIC32xx.
 *            The functions set the sample rate and audio serial data word
 *            length, this version used for async mcbsp operation only
 *----------------------------------------------------------------------------
 */
static int aic32xx_hw_params_play(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_codec *codec = rtd->codec;
    int     div_idx;
    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);
    int i, size;
    const struct aic32xx_configs * pConfigRegs;

    dprintk("Function : %s \n", __FUNCTION__);

    div_idx = _get_divs(aic32xx->sysclk, params_rate(params));
    if (div_idx < 0) {
        printk("sampling rate not supported\n");
        return div_idx;
    }

    /* # Software Reset */
    aic32xx_write(codec,RESET,0x1);
    mdelay(20);

    /* # DAI MM, DSP, 16 bit */
    aic32xx_write(codec,INTERFACE_SET_REG_1,0x4d);

    /* # BCLK from ADC Clk */
    aic32xx_write(codec, AIS_REG_3, 0x08);

      /* # WCLK from DAC CLK */
    aic32xx_write(codec, AIS_REG_6, 0x00);

    /* set clock MUX settings */
    aic32xx_write(codec, CLK_REG_1,0x03);

    /* Set PLL and M/N clock tree values; NADC/NDAC not yet enabled */
    __set_sample_rate_play(codec, div_idx);
    aic32xx->pll_divs_index = div_idx;

    /* # Turn off Dout */
    aic32xx_write(codec, DOUT_CTRL, 0x10);

    /* Set the signal processing block (PRB) modes */
    aic32xx_write(codec, DAC_PRB, 0x2);

    /* # Disable weak AVDD in presence of external */
    aic32xx_write(codec, POW_CFG, 0x08);

    /* # Enable Master Analog Power Control */
    aic32xx_write(codec, LDO_CTRL, 0x01);

	/* mic bias powered up */
    aic32xx_write(codec, MICBIAS_CTRL, 0x68);

    /*headset detect settings */
    mdelay(10);
    aic32xx_write(codec, INT1_CTRL,0x41); // No ISR Generated
    aic32xx_write(codec, GPIO_CTRL,0x14);
    aic32xx_write(codec, SCLK_CTRL, 0x00);
    aic32xx_write(codec, HEADSET_DETECT,0x17); // Disable Headset detection

    /* Unmute LADC/RADC and set default gain to -0.1dB */
    aic32xx_write(codec, ADC_FGA, 0x11);


/* ####################################################
   # Program the biquads and DRC
   #################################################### */
	if (board_is_p) {
        pConfigRegs = biquad_Play_P;
        size = ARRAY_SIZE(biquad_Play_P);
    }
    else if(IDC_enabled) {
        pConfigRegs = biquad_settings_Play_IDC;
        size = ARRAY_SIZE(biquad_settings_Play_IDC);
    }
    else {
		pConfigRegs = biquad_settings_Play;
		size = ARRAY_SIZE(biquad_settings_Play);
    }

    for (i = 0; i < size; i++) {
        // Get the register offset and value
        u32 regOffset = pConfigRegs[i].reg_offset;
        u8 regVal = pConfigRegs[i].reg_val;
        aic32xx_write(codec, regOffset, regVal);
    }


  /* ###############################################
   # Playback Path Settings - write the ALSA settings
   ############################################### */
    aic32xx_write(codec, OUT_PWR_CTRL,          0x00);
    aic32xx_write(codec, HPHONE_STARTUP_CTRL,   0x29);
    aic32xx_write(codec, CM_CTRL_REG,           0x33);
    aic32xx_write(codec, HPL_ROUTE_CTRL,        0x08);
    aic32xx_write(codec, HPR_ROUTE_CTRL,        0x08);
    aic32xx_write(codec, LDAC_VOL,              0x00);
    aic32xx_write(codec, RDAC_VOL,              0x00);
	if (!board_is_p) {
        aic32xx_write(codec, HPL_GAIN,              0x00);
        aic32xx_write(codec, HPR_GAIN,              0x00);
    }
    else {
        // Set the hp gain based on the current line out status
        if (aic32xx->line_out_status) {
            printk(KERN_INFO "aic32x: setting hp gain to line out\n");
            aic32xx_write(codec, HPL_GAIN, P_HP_GAIN_LINE_OUT);
            aic32xx_write(codec, HPR_GAIN, P_HP_GAIN_LINE_OUT);

            // Enabling line out at DAC power-up can eliminate two poppings
            printk(KERN_INFO "aic32x: enabling line out\n");
            gpio_set_value(LINEOUT_SEL_GPIO, 0);
        }
        else {
            printk(KERN_INFO "aic32x: setting hp gain to internal speaker\n");
            aic32xx_write(codec, HPL_GAIN, P_HP_GAIN_LOCAL_SPEAKER);
            aic32xx_write(codec, HPR_GAIN, P_HP_GAIN_LOCAL_SPEAKER);
        }
    }
    aic32xx_write(codec, OUT_PWR_CTRL,          0x30);


/* #################################################
   # Delayed Clock start in NADC
   ################################################# */

    /* # NADC = 01 */
    aic32xx_write(codec, NADC_CLK_REG_8,
        0x80 | (aic32xx_divs[div_idx].nadc & 0x7f));
    aic32xx_write(codec, NDAC_CLK_REG_6,
        0x80 | (aic32xx_divs[div_idx].ndac & 0x7f));

    aic32xx_write(codec, DAC_CHN_REG, 0xD5); // Enable soft stepping on volume change (ramp)

    //mandatory delay to allow ADC clocks to synchronize
    mdelay(100);

/* ###############################################
   # TDM Channel Settings
   ############################################### */

    /* # Channel 1&2 (AIC3204) offset (1 bit) */
	aic32xx_write(codec, AIS_REG_2, 0x01);

    /* # Turn Douts back on */
    aic32xx_write(codec, DOUT_CTRL, 0x12);

    if (params_format(params) != SNDRV_PCM_FORMAT_S16_LE) {
        printk("Only S16_LE supported in audio codec (%#x)\n",
               params_format(params));
        return -1;
    }

    return 0;
}


/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_hw_params_capt
 * Purpose  : This function is to set the hardware parameters for AIC32xx.
 *            The functions set the sample rate and audio serial data word
 *            length. This version for async mcbsp operation only
 *----------------------------------------------------------------------------
 */
static int aic32xx_hw_params_capt(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_codec *codec = rtd->codec;
    int     div_idx;
    int    i, size;
    u32 regOffset;
    u8 regVal;
    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);
    const struct aic32xx_configs * pConfigRegs;

    dprintk("Function : %s \n", __FUNCTION__);

    div_idx = _get_divs(aic32xx->sysclk, params_rate(params));
    if (div_idx < 0) {
        printk("sampling rate not supported\n");
        return div_idx;
    }

    /* # Software Reset */
    for(i=0; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec,ADC_RESET(i),0x1);
    }

    mdelay(20);

    //adc3101 master
    aic32xx_write(codec, ADC_CLKGEN_MUX(0), 0x3);
    //adc3101 slaves....
    for(i=1; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec, ADC_CLKGEN_MUX(i), 0x1);
    }

    /* Set PLL and M/N clock tree values; NADC/NDAC not yet enabled */
        __set_sample_rate_capt(codec, div_idx);
        aic32xx->pll_divs_index = div_idx;

    /* ADC3101 settings: DSP, 16-bits, Slave Mode 3-stating of DOUT enabled */
    //adc3101 master i2s format settings;
       aic32xx_write(codec,ADC_INTERFACE_CTRL_1(0), 0x4d);
       aic32xx_write(codec,ADC_INTERFACE_CTRL_2(0), 0x0e);

    //adc3101 slave i2s format settings
    for(i=1; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec,ADC_INTERFACE_CTRL_1(i), 0x41);
       aic32xx_write(codec,ADC_INTERFACE_CTRL_2(i), 0x0e);
    }

    /* ####################################################
       # Program the biquads
       #################################################### */
    pConfigRegs = ( IDC_enabled) ? biquad_settings_Capt_IDC : biquad_settings;
    size = ( IDC_enabled) ? ARRAY_SIZE(biquad_settings_Capt_IDC) : ARRAY_SIZE(biquad_settings);
    for (i = 0; i < size; i++){
        // Get the register offset and value
        regOffset = pConfigRegs[i].reg_offset;
        regVal    = pConfigRegs[i].reg_val;
        aic32xx_write(codec, regOffset, regVal);

        // If command for second ADC3101
        if( (regOffset & MAKE_REG(0xF, 0,0)) == (MAKE_REG(2,0,0))){
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
            // Add two ADC 3101
            regOffset += MAKE_REG(1,0,0);
            aic32xx_write(codec, regOffset, regVal);
            regOffset += MAKE_REG(1,0,0);
            aic32xx_write(codec, regOffset, regVal);
#endif
        }

     }

    /* # Turn off Dout */
    for(i=0; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec, ADC_DOUT_CTRL(i), 0x10);
    }

    /* Set the signal processing block (PRB) modes */
    for(i=0; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec, ADC_PRB_SELECT(i), 0x2);
    }


/* ###############################################
   # Recording Setup for Slaves (ADC3101s)
   ############################################### */

    for(i=0; i < NUM_ADC3101; i++)
    {
    /* set ADC digital vol */
       aic32xx_write(codec, ADC_LADC_VOL(i), 0x00);
       aic32xx_write(codec, ADC_RADC_VOL(i), 0x00);

    /* # Mic Bias Settings */
       aic32xx_write(codec, ADC_MICBIAS_CTRL(i), 0x50);

    /*  Left ADC Input selection */
       aic32xx_write(codec, ADC_LEFT_PGA_SEL_1(i), 0xF3);
       aic32xx_write(codec, ADC_RIGHT_PGA_SEL_1(i), 0xF3);

    /* # Left and Right Analog PGA Set */
       aic32xx_write(codec, ADC_LEFT_APGA_CTRL(i), MIC_PGA_GAIN);
       aic32xx_write(codec, ADC_RIGHT_APGA_CTRL(i), MIC_PGA_GAIN);

    /* # Power-up Left ADC and Right ADC */
       aic32xx_write(codec, ADC_ADC_DIGITAL(i), 0xC2);

    /* # Unmute ADC channels and set fine gain */
       aic32xx_write(codec, ADC_ADC_FGA(i), 0x00);
     }

    //for ADC3101 master
    aic32xx_write(codec, ADC_ADC_NADC(0),
        0x80 | (aic32xx_divs[div_idx].nadc & 0x7f));

    //mandatory delay to allow ADC clocks to synchronize
    mdelay(100);

/* ###############################################
   # TDM Channel Settings
   ############################################### */

    /* # Enable Time Slot Mode and Early 3-state */
    for(i=0; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec, ADC_I2S_TDM_CTRL(i), 0x03);
    }


    /* # Channel 3 offset (1 bits) */
    aic32xx_write(codec, ADC_CH_OFFSET_1(0), 0x01);
    /* # Channel 4 offset (default) */

    /* # Channel 5 offset (33 bits) */
    aic32xx_write(codec, ADC_CH_OFFSET_1(1), 0x21);
    /* # Channel 6 offset (default) */

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
    /* # Channel 7 offset (65 bits) */
    aic32xx_write(codec, ADC_CH_OFFSET_1(2), 0x41);
    /* # Channel 8 offset (default) */

    /* # Channel 9 offset (98 bits) */
    aic32xx_write(codec, ADC_CH_OFFSET_1(3), 0x61);
    /* # Channel 10 offset (default) */
#endif

    /* # Turn Douts back on */
    for(i=0; i < NUM_ADC3101; i++)
    {
       aic32xx_write(codec, ADC_DOUT_CTRL(i), 0x12);
    }

    if (params_format(params) != SNDRV_PCM_FORMAT_S16_LE) {
        printk("Only S16_LE supported in audio codec (%#x)\n",
               params_format(params));
        return -1;
    }

    return 0;
}

/*
 *-----------------------------------------------------------------------
 * Function : aic32xx_mute_capt
 * Purpose  : This function is to mute or unmute ADC's
 *-----------------------------------------------------------------------
 */
static int aic32xx_mute_capt(struct snd_soc_dai *dai, int mute)
{
    printk("Function : %s mute %d\n", __FUNCTION__, mute);
    return 0;
}

/*
 *-----------------------------------------------------------------------
 * Function : aic32xx_mute_play
 * Purpose  : This function is to mute or unmute the left and right DAC
 *-----------------------------------------------------------------------
 */
static int aic32xx_mute_play(struct snd_soc_dai *dai, int mute)
{
    struct snd_soc_codec *codec = dai->codec;
    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);
    u16    reg;
    u8     ramp_flag;
    u8 val = aic32xx_read_reg_cache(codec, DAC_MUTE_CTRL_REG) & ~MUTE_ON;
    u16 i = 0;

    dprintk("Function : %s+ mute %d\n", __FUNCTION__, mute);
    if (mute)
    {
        aic32xx_write(codec, DAC_MUTE_CTRL_REG, val | MUTE_ON);
    }
    else
    {
      /*################################################
        # DAC channel ramp. wait until complete for
        # best pop/click performance on outputs
        ################################################ */
        reg = 63;
        ramp_flag=0;
        /* Read from register. Do not use cache */
        mutex_lock(&aic32xx->codecMutex);
        aic32xx_change_page(codec, 1);
        i2c_master_send(codec->control_data, (char *)&reg, 1);
        i2c_master_recv(codec->control_data, &ramp_flag, 1);
        mutex_unlock(&aic32xx->codecMutex);
        /* The ramp loop (below) can take >= 2 seconds to complete.
         * Limit the wait by 2.2sec to avoid infinite loop in case ramp_flag
         * is not read correctly
         * 44 * 50ms = 2.2sec
         */
        while(ramp_flag < 0xc0 && i < 44)
        {
            reg = 63;
            mutex_lock(&aic32xx->codecMutex);
            aic32xx_change_page(codec, 1);
            i2c_master_send(codec->control_data, (char *)&reg, 1);
            i2c_master_recv(codec->control_data, &ramp_flag, 1);
            mutex_unlock(&aic32xx->codecMutex);
            /* Wait for 50ms between each read */
            mdelay(50);
            i++;
        }

        /* Unmute DAC */
        aic32xx_write(codec, DAC_MUTE_CTRL_REG, val);
    }
    printk("Function : %s mute=%d, ramp=0x%X i=%d\n", __FUNCTION__, mute,
         ramp_flag, i);

    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_set_dai_sysclk
 * Purpose  : This function is to set the DAI system clock
 *----------------------------------------------------------------------------
 */
static int aic32xx_set_dai_sysclk(struct snd_soc_dai *codec_dai,
                                  int clk_id, unsigned int freq, int dir)
{
    struct snd_soc_codec *codec = codec_dai->codec;
    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);

    dprintk("Function : %s, %d \n", __FUNCTION__, freq);

    switch (freq) {
        case AIC32xx_FREQ_12000000:
        case AIC32xx_FREQ_24000000:
        case AIC32xx_FREQ_19200000:
        case AIC32xx_FREQ_26000000:
        case AIC32xx_FREQ_38400000:
            aic32xx->sysclk = freq;
            return 0;
    }
    printk("Invalid frequency to set DAI system clock\n");
    return -EINVAL;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_set_dai_fmt
 * Purpose  : This function is to set the DAI format
 *----------------------------------------------------------------------------
 */
static int aic32xx_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
    dprintk("Function : %s\n", __FUNCTION__);

    return 0;
}



/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_trigger
 * Purpose  : This function is to
 *----------------------------------------------------------------------------
 */

static int aic32xx_trigger(struct snd_pcm_substream *substream, int cmd,
                           struct snd_soc_dai *dai)
{
    printk("Function : %s\n", __FUNCTION__);

    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_set_bias_level
 * Purpose  : This function is to get triggered when dapm events occurs.
 *----------------------------------------------------------------------------
 */
static int aic32xx_set_bias_level(struct snd_soc_codec *codec,
                                  enum snd_soc_bias_level level)
{
    printk("Function : %s\n", __FUNCTION__);

    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_suspend
 * Purpose  : This function is to suspend the AIC32xx driver.
 *----------------------------------------------------------------------------
 */
static int aic32xx_suspend(struct snd_soc_codec *codec, pm_message_t state)
{

    dprintk(KERN_INFO "Function : %s \n", __FUNCTION__);
    aic32xx_set_bias_level(codec, SND_SOC_BIAS_OFF);

    return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_resume
 * Purpose  : This function is to resume the AIC32xx driver
 *----------------------------------------------------------------------------
 */
static int aic32xx_resume(struct snd_soc_codec *codec)
{
    dprintk(KERN_INFO "Function : %s \n", __FUNCTION__);

    return 0;
}


/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_set_line_out
 * Purpose  : This function adjusts the codec settings when line out is enabled
 *            or disabled.
 *----------------------------------------------------------------------------
 */
void aic32xx_set_line_out(struct snd_soc_codec *codec, int enabled)
{
    struct aic32xx_priv *aic32xx;
    if (!codec) {
        printk (KERN_ERR "aic32x: %s parameter NULL.\n", __FUNCTION__);
        return;
    }
    aic32xx = snd_soc_codec_get_drvdata(codec);

    aic32xx->line_out_status = enabled;
}

EXPORT_SYMBOL_GPL(aic32xx_set_line_out);


/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_init
 * Purpose  : This function is to initialise the AIC32xx driver
 *            register the mixer and dsp interfaces with the kernel.
 *
 *----------------------------------------------------------------------------
 */
static int tlv320aic32xx_init(struct snd_soc_codec *codec)
{
    int     ret = 0;
    int     i;
    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);

    aic32xx->page_no = 0;
    for(i=0; i < NUM_ADC3101; i++){
        aic32xx->adc_page_no[i] = 0;
    }
    aic32xx->pll_divs_index = -1;
    aic32xx->line_out_status = 0;

    /* # Software Reset */
    aic32xx_write(codec,RESET,0x1);
    for(i=0; i < NUM_ADC3101; i++) {
       aic32xx_write(codec,ADC_RESET(i),0x1);
    }
    mdelay(20);

    snd_soc_add_controls(codec, aic32xx_snd_controls,
                         ARRAY_SIZE(aic32xx_snd_controls));
    aic32xx_add_widgets(codec);

    return ret;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_hw_read
 * Purpose  : i2c read function
 *----------------------------------------------------------------------------
 */
unsigned int aic32xx_hw_read(struct snd_soc_codec *codec, unsigned int count)
{
    struct i2c_client *client = codec->control_data;

    unsigned int buf;

    if (count > sizeof(unsigned int)) {
        return 0;
    }
    i2c_master_recv(client, (char *) &buf, count);
    return buf;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_probe
 * Purpose  : This is first driver function called by the SoC core driver.
 *
 *----------------------------------------------------------------------------
 */
static int aic32xx_probe(struct snd_soc_codec *codec)
{
    int     ret = 0;

    struct aic32xx_priv *aic32xx = snd_soc_codec_get_drvdata(codec);

/*  ret = snd_soc_codec_set_cache_io(codec, 8, 8, SND_SOC_I2C);
    if(ret != 0)
    {
        dev_err(codec->dev, "Failed to set cache I/O\n");
    }*/

    printk("in aic32xx_probe....\n");

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
    codec->hw_write = (hw_write_t) aic32xx_write;
    codec->hw_read = aic32xx_hw_read;
    codec->control_data = aic32xx->control_data;
#else
    /* Add other interfaces here */
#endif
    tlv320aic32xx_init(codec);

#ifdef CONFIG_MINI_DSP
    /* Program MINI DSP for ADC and DAC */
    aic32xx_minidsp_program(codec);
    aic32xx_change_page(codec, 0x0);
#endif
    return ret;

}

/*
 *----------------------------------------------------------------------------
 * Function : aic32xx_remove
 * Purpose  : to remove aic32xx soc device
 *
 *----------------------------------------------------------------------------
 */
static int aic32xx_remove(struct snd_soc_codec *codec)
{

    dprintk(KERN_INFO "Function : %s \n", __FUNCTION__);

    /* power down chip */
    if (codec->control_data)
        aic32xx_set_bias_level(codec, SND_SOC_BIAS_OFF);


    return 0;
}

static struct snd_soc_codec_driver soc_codec_driver_aic32xx = {
    .probe = aic32xx_probe,
    .remove = aic32xx_remove,
    .suspend = aic32xx_suspend,
    .resume = aic32xx_resume,
    .read = aic32xx_read_reg_cache,
    .write = aic32xx_write,
    .set_bias_level = aic32xx_set_bias_level,
    .reg_cache_size = ARRAY_SIZE(aic32xx_reg),
    .reg_word_size = sizeof(u8),
    .reg_cache_default = aic32xx_reg,
};

struct aic32xx_priv *aic32xx_global;

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
static int aic32xx_i2c_probe(struct i2c_client *pdev,
                             const struct i2c_device_id *id)
{
    int     ret = 0;
    struct aic32xx_priv *aic32xx;
    static int i;

    printk("Function : %s\n", __FUNCTION__);
    printk(KERN_INFO "AIC32xx Audio Codec %s\n", AIC32xx_VERSION);
    i++;

    if (i == 1) {
        aic32xx = kzalloc(sizeof(struct aic32xx_priv), GFP_KERNEL);
        if (aic32xx == NULL) {
            return -ENOMEM;
        }
        i2c_set_clientdata(pdev, aic32xx);
        aic32xx_global = aic32xx;
        aic32xx->control_data = pdev;

        mutex_init(&aic32xx->codecMutex);

	if (get_omap3_misto_rev() & MISTO_BOARD_IS_P) {
		board_is_p = 1;
		printk(KERN_INFO "codec: %s : p setup\n", __FUNCTION__);
	}
	else
		board_is_p = 0;

        ret = snd_soc_register_codec(&pdev->dev,
                                     &soc_codec_driver_aic32xx,
                                     tlv320aic32xx_dai_driver,
                                     ARRAY_SIZE(tlv320aic32xx_dai_driver));
        if (ret)
            printk(KERN_ERR "codec: %s : snd_soc_register_codec failed\n",
                   __FUNCTION__);
        else
            printk(KERN_ALERT "codec: %s : snd_soc_register_codec success\n",
                   __FUNCTION__);
    }
    if (i >= 2 && i < (2+NUM_ADC3101)) {
        aic32xx_global->adc_control_data[i-2] = pdev;
    }

    return ret;
}

static int aic32xx_i2c_remove(struct i2c_client *pdev)
{
    snd_soc_unregister_codec(&pdev->dev);
    return 0;
}

static const struct i2c_device_id aic3262_i2c_id[] = {
    {"tlv320aic32xx", 0},
    {"tlv320adc31xx_A", 1},
    {"tlv320adc31xx_B", 2},
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
    {"tlv320adc31xx_C", 3},
    {"tlv320adc31xx_D", 4},
#endif
    {}
};

MODULE_DEVICE_TABLE(i2c, aic32xx_i2c_id);

static struct i2c_driver aic32xx_i2c_driver = {
    .driver = {
               .name = "tlv320aic32xx",
               .owner = THIS_MODULE,
               },
    .probe = aic32xx_i2c_probe,
    .remove = aic32xx_i2c_remove,
    .id_table = aic3262_i2c_id,
};
#endif

/*
 *----------------------------------------------------------------------------
 * Function : tlv320aic3262_modinit
 * Purpose  : module init function. First function to run.
 *
 *----------------------------------------------------------------------------
 */
static int __init tlv320aic32xx_modinit(void)
{
    int     ret = 0;

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
    ret = i2c_add_driver(&aic32xx_i2c_driver);

    if (ret != 0) {
        printk(KERN_ERR "Failed to register TLV320AIC3x I2C driver: %d\n", ret);
    }
#endif
#ifdef CONFIG_MINI_DSP
    asdasda
#endif
        return ret;

}

module_init(tlv320aic32xx_modinit);

/*
 *----------------------------------------------------------------------------
 * Function : tlv320aic3262_exit
 * Purpose  : module init function. First function to run.
 *
 *----------------------------------------------------------------------------
 */
static void __exit tlv320aic32xx_exit(void)
{
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
    i2c_del_driver(&aic32xx_i2c_driver);
#endif
}

module_exit(tlv320aic32xx_exit);

MODULE_DESCRIPTION("ASoC TLV320AIC32xx codec driver");
MODULE_AUTHOR("Mukund Navada <navada@ti.com> ");
MODULE_LICENSE("GPL");
