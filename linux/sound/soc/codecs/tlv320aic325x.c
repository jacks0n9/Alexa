/*
 * linux/sound/soc/codecs/tlv320aic325x.c
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
 * History:
 *
 * Rev 0.1   ASoC driver support         31-04-2009
 * The AIC32 ASoC driver is ported for the codec AIC325x.
 *
 *
 * Rev 1.0   Mini DSP support             11-05-2009
 * Added mini DSP programming support
 *
 * Rev 1.1   Mixer controls           18-01-2011
 * Added all the possible mixer controls.
 *
 *
 *
 * Rev 1.2   Additional Codec driver support        2-02-2011
 * Support for AIC3253,AIC3206,AIC3256         
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

#include "tlv320aic325x.h"

/*
 * enable debug prints in the driver
 */

#define CONFIG_SND_SOC_TLV320AIC325x_DEBUG 


#ifdef CONFIG_SND_SOC_TLV320AIC325x_DEBUG
#define dprintk(x...)   printk(x)
#else
#define dprintk(x...)
#endif

/*
 * Macros
 */
#ifdef CONFIG_MINI_DSP
extern int aic325x_minidsp_program(struct snd_soc_codec *codec);
extern void aic325x_add_minidsp_controls(struct snd_soc_codec *codec);
#endif

#ifdef AIC325x_TiLoad
extern int aic325x_driver_init(struct snd_soc_codec *codec);
#endif


/* User defined Macros kcontrol builders */
#define SOC_SINGLE_AIC325x(xname)                                       \
        {                                                               \
                .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname,     \
                        .info = __new_control_info, .get = __new_control_get, \
                        .put = __new_control_put,                       \
                        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,      \
                        }

/*
*****************************************************************************
* Function Prototype
*****************************************************************************
*/
inline void aic325x_write_reg_cache(struct snd_soc_codec *codec,
                                    u16 reg, u8 value);
int aic3i25x_reset_cache (struct snd_soc_codec *codec);
static int aic325x_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *);
static int aic325x_mute(struct snd_soc_dai *dai, int mute);
static int aic325x_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir);
static int aic325x_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt);
static int aic325x_set_bias_level(struct snd_soc_codec *codec, enum snd_soc_bias_level level);
static u8 aic325x_read(struct snd_soc_codec *codec, u16 reg);
static int __new_control_info(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_info *uinfo);
static int __new_control_get(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol);
static int __new_control_put(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol);
static int snd_soc_info_volsw_2r_aic325x(struct snd_kcontrol *kcontrol,
                                         struct snd_ctl_elem_info *uinfo);
/*static int snd_soc_get_volsw_2r_aic325x(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol);
static int snd_soc_put_volsw_2r_aic325x(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol);*/
// Headphone driver in GC mode
static int aic325x_hp_event(struct snd_soc_dapm_widget *w,                                                                                                   
                               struct snd_kcontrol *kcontrol, int event);
#ifdef AIC3256_CODEC_SUPPORT
// Charge pump 
static int aic3256_cp_event(struct snd_soc_dapm_widget *w,                                                                                                   
                               struct snd_kcontrol *kcontrol, int event)  ; 

#endif
/*
*****************************************************************************
* Global Variable
*****************************************************************************
*/
static u8 aic325x_reg_ctl;

/* whenever aplay/arecord is run, aic325x_hw_params() function gets called.
 * This function reprograms the clock dividers etc. this flag can be used to
 * disable this when the clock dividers are programmed by pps config file
 */
static int soc_static_freq_config = 1;

/*
*****************************************************************************
* Structure Declaration
*****************************************************************************
*/


static const char *mute[] = { "Unmute", "Mute" };
static const char *dacvolume_extra[] =
{ "L&R Ind Vol", "LVol=RVol", "RVol=LVol" };
static const char *dacsoftstep_control[] =
{ "1 step/sample", "1 step/2 sample", "disabled" };
static const char *headset_detection[] = { "Disabled", "Enabled" };
static const char *drc_enable[] = { "Disabled", "Enabled" };
static const char *beep_generator[] = { "Disabled", "Enabled" };
static const char *beepvolume_extra[] =
{ "L&R Ind Vol", "LVol=RVol", "RVol=LVol" };
static const char *micbias_voltage[] =
{ "1.04/1.25V", "1.425/1.7V", "2.075/2.5V", "POWER SUPPY" };
static const char *micpga_selection[] = { "off", "10k", "20k", "40k" };
static const char  *micpgagain_enable[] = { "Disabled", "Enabled" };
static const char *powerup[] = { "Power Down", "Power Up" };
static const char *vol_generation[] = { "AVDD", "LDOIN" };
static const char *path_control[] =
{ "Disabled", "LDAC Data", "RDAC Data", "L&RDAC Data" };
static const char  *agc_enable[] = { "Disabled", "Enabled" };

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
	SOC_ENUM_SINGLE(HEADSET_DETECT, 1, 4, headset_button_press_debounce_time_text);

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
	"2.414e-4dB per DAC Word Clock"
	"1.220e-4dB per DAC Word Clock"
	
};
static const struct soc_enum drc_atk = 
	SOC_ENUM_SINGLE(DRC_CTRL_REG2, 7, 16, drc_atk_text);

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
	"9.5367e-7dB per DAC Word Clock"
	"4.7683e-7dB per DAC Word Clock"
};	

static const struct soc_enum drc_dcy = 
	SOC_ENUM_SINGLE(DRC_CTRL_REG2, 3, 16, drc_dcy_text);

static const char *dc_offset_correction_text[] = {
	"disabled",
	"Reserved",
	"All enabled signal routings",
	"All possible signal routings",
};

//static const struct soc_enum dc_offset_correction =
//	SOC_ENUM_SINGLE(HP_DRIVER_CONF_REG, 0, 4, dc_offset_correction_text);

static const char *hp_driver_power_conf_text[] = {
	"100%",
	"75%",
	"50%",
	"25%",
};

//static const struct soc_enum hp_driver_power_conf =
//	SOC_ENUM_SINGLE(HP_DRIVER_CONF_REG, 2, 4, hp_driver_power_conf_text);
/*Creates an array of the Single Ended Widgets */
static const struct soc_enum aic325x_enum[] = {
        SOC_ENUM_SINGLE (DAC_MUTE_CTRL_REG, 3, 2, mute),
        SOC_ENUM_SINGLE (DAC_CHN_REG, 0, 3, dacsoftstep_control),
        SOC_ENUM_SINGLE (DAC_MUTE_CTRL_REG, 0, 4, dacvolume_extra),
        SOC_ENUM_SINGLE (HEADSET_DETECT, 7, 2, headset_detection),
        SOC_ENUM_SINGLE (ADC_FGA, 7, 2, mute),
        SOC_ENUM_DOUBLE (DRC_CTRL_REG1, 6, 5, 2, drc_enable),
        SOC_ENUM_SINGLE (BEEP_CTRL_REG1, 7, 2, beep_generator),
        SOC_ENUM_SINGLE (BEEP_CTRL_REG2, 6, 3, beepvolume_extra),
        SOC_ENUM_SINGLE (MICBIAS_CTRL, 4, 4, micbias_voltage),

#ifndef AIC3253_CODEC_SUPPORT
        SOC_ENUM_SINGLE (LMICPGA_PIN_CFG, 6, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_PIN_CFG, 4, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_PIN_CFG, 2, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_PIN_CFG, 0, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_NIN_CFG, 6, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_NIN_CFG, 4, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_NIN_CFG, 2, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_NIN_CFG, 0, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_PIN_CFG, 6, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_PIN_CFG, 4, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_PIN_CFG, 2, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_PIN_CFG, 0, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_NIN_CFG, 6, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_NIN_CFG, 4, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_NIN_CFG, 2, 4, micpga_selection),
        SOC_ENUM_SINGLE (RMICPGA_NIN_CFG, 0, 4, micpga_selection),
        SOC_ENUM_SINGLE (LMICPGA_VOL_CTRL, 7, 2, micpgagain_enable),
        SOC_ENUM_SINGLE (RMICPGA_VOL_CTRL, 7, 2, micpgagain_enable),
#endif

	SOC_ENUM_SINGLE (MICBIAS_CTRL, 6, 2, powerup),
	SOC_ENUM_SINGLE (MICBIAS_CTRL, 3, 2, vol_generation),

#ifndef AIC3253_CODEC_SUPPORT
	SOC_ENUM_SINGLE (ADC_CHN_REG, 7, 2, powerup),
	SOC_ENUM_SINGLE (ADC_CHN_REG, 6, 2, powerup),
#endif

	SOC_ENUM_SINGLE (DAC_CHN_REG, 7, 2, powerup),
	SOC_ENUM_SINGLE (DAC_CHN_REG, 6, 2, powerup),
        SOC_ENUM_SINGLE (DAC_MUTE_CTRL_REG, 2, 2, mute),

#ifndef AIC3253_CODEC_SUPPORT
        SOC_ENUM_SINGLE (ADC_FGA, 3, 2, mute),
#endif

        SOC_ENUM_SINGLE (DAC_CHN_REG, 4, 4, path_control),
        SOC_ENUM_SINGLE (DAC_CHN_REG, 2, 4, path_control),
	SOC_ENUM_DOUBLE (OUT_PWR_CTRL, 5, 4, 2, powerup),
	SOC_ENUM_DOUBLE (OUT_PWR_CTRL, 3, 2, 2, powerup),
	SOC_ENUM_DOUBLE (OUT_PWR_CTRL, 1, 0, 2, powerup),
	SOC_ENUM_SINGLE (HPL_GAIN, 6, 2, mute),
	SOC_ENUM_SINGLE (HPR_GAIN, 6, 2, mute),

#ifndef AIC3253_CODEC_SUPPORT
	SOC_ENUM_SINGLE (LOL_GAIN, 6, 2, mute),
	SOC_ENUM_SINGLE (LOR_GAIN, 6, 2, mute),
	SOC_ENUM_SINGLE (LEFT_AGC_REG1, 7, 2, agc_enable),
	SOC_ENUM_SINGLE (RIGHT_AGC_REG1, 7, 2, agc_enable),
#endif
};


static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -6350, 50, 0);
static const DECLARE_TLV_DB_SCALE(adc_vol_tlv, -1200, 50, 0);
static const DECLARE_TLV_DB_SCALE(output_gain_tlv, -600, 100, 0);
static const DECLARE_TLV_DB_SCALE(micpga_gain_tlv, 0, 50, 0);
static const DECLARE_TLV_DB_SCALE(in1_hp_gain_tlv, -7230, 50, 1);
/*
 * Structure Initialization
 */
/* Various Controls For AIC325x */
static const struct snd_kcontrol_new aic325x_snd_controls1[] = {

	/* Left DAC Mute Control */
        SOC_ENUM ("Left DAC Mute Control", aic325x_enum[LDMUTE_ENUM]),

        /* Right DAC Mute Control */
        SOC_ENUM ("Right DAC Mute Control", aic325x_enum[RDMUTE_ENUM]),

	/* DAC Volume Soft Stepping Control */
        SOC_ENUM ("DAC Volume Soft Stepping", aic325x_enum[SOFTSTEP_ENUM]),

        /* DAC Extra Volume Control */
        SOC_ENUM ("DAC Extra Volume Control", aic325x_enum[DACEXTRA_ENUM]),

        /* Headset detection Enable/Disable control */
        SOC_ENUM ("Headset detection Enable/Disable", aic325x_enum[HSET_ENUM]),
	
	SOC_ENUM("Headset Detection Debounce Time", headset_detect_debounce_time),
	SOC_ENUM("Headset Button Press Debounce Time", headset_button_press_debounce_time),

#ifndef AIC3253_CODEC_SUPPORT
        /* Left ADC Mute Control */
        SOC_ENUM ("Left ADC Mute Control", aic325x_enum[LAMUTE_ENUM]),

        /* Right ADC Mute Control */
        SOC_ENUM ("Right ADC Mute Control", aic325x_enum[RAMUTE_ENUM]),
#endif


	/* Mic Bias Voltage */
        SOC_ENUM ("Mic Bias Voltage", aic325x_enum[MICBIAS_ENUM]),

};

/* Various Controls For AIC325x */
static const struct snd_kcontrol_new aic325x_snd_controls2[] = {

	/* IN1L/R to HPL/HPR Volume Control */
        SOC_DOUBLE_R_TLV ("IN1L->HPL & IN1R->HPR Volume Control",
                      IN1L_HPL_VOL_CTRL, IN1R_HPR_VOL_CTRL, 0, 0x72, 1, in1_hp_gain_tlv),

	/* Mic Bias Generation */
        SOC_ENUM ("Mic Bias Voltage Generation",aic325x_enum[MICBIASVOLTAGE_ENUM]),

        /* Mic Bias Power on */
        SOC_ENUM ("Mic Bias Power on",aic325x_enum[MICBIASPOWER_ENUM]),

	/* DRC Threshold value  Control */
	SOC_SINGLE_TLV("DRC Threshold Volume", DRC_CTRL_REG1, 4, 8, 1, drc_threshold), 
//        SOC_SINGLE ("DRC Threshold value (0=-3db, 7=-24db)", DRC_CTRL_REG1,
//                    2, 0x07, 0),

        /* DRC Hysteresis value control */
	SOC_SINGLE_TLV("DRC Hysteresis Volume", DRC_CTRL_REG1, 1, 4, 0, drc_hysteresis),
//        SOC_SINGLE ("DRC Hysteresis value (0=0db, 3=3db)", DRC_CTRL_REG1,
//                    0, 0x03, 0),

        /* DRC Hold time control */
        SOC_SINGLE ("DRC hold time", DRC_CTRL_REG2, 3, 0x0F, 0),

        /* DRC Attack rate control */
//	SOC_ENUM("drc Attack Rate", drc_atk),
        SOC_SINGLE ("DRC attack rate", DRC_CTRL_REG3, 4, 0x0F, 0),

      /* DRC Decay rate control */
//	SOC_ENUM("drc Decay Rate", drc_dcy),
        SOC_SINGLE ("DRC decay rate", DRC_CTRL_REG3, 0, 0x0F, 0),

        /* Beep Length MSB control */
        SOC_SINGLE ("Beep Length MSB", BEEP_CTRL_REG3, 0, 255, 0),

        /* Beep Length MID control */
        SOC_SINGLE ("Beep Length MID", BEEP_CTRL_REG4, 0, 255, 0),

        /* Beep Length LSB control */
        SOC_SINGLE ("Beep Length LSB", BEEP_CTRL_REG5, 0, 255, 0),

        /* Beep Sin(x) MSB control */
        SOC_SINGLE ("Beep Sin(x) MSB", BEEP_CTRL_REG6, 0, 255, 0),

        /* Beep Sin(x) LSB control */
        SOC_SINGLE ("Beep Sin(x) LSB", BEEP_CTRL_REG7, 0, 255, 0),

        /* Beep Cos(x) MSB control */
        SOC_SINGLE ("Beep Cos(x) MSB", BEEP_CTRL_REG8, 0, 255, 0),

        /* Beep Cos(x) LSB control */
        SOC_SINGLE ("Beep Cos(x) LSB", BEEP_CTRL_REG9, 0, 255, 0),
};


/*static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -6350, 50, 0);
static const DECLARE_TLV_DB_SCALE(adc_vol_tlv, -1200, 50, 0);
static const DECLARE_TLV_DB_SCALE(output_gain_tlv, -600, 100, 0);
static const DECLARE_TLV_DB_SCALE(micpga_gain_tlv, 0, 50, 0);*/
/* Various Controls For AIC325x */
static const struct snd_kcontrol_new aic325x_snd_controls3[] = {


        /* Left DAC Data Path Control */
        SOC_ENUM ("Left DAC Data Path Control", aic325x_enum[LDACPATH_ENUM]),

	/* Right DAC Data Path Control */
        SOC_ENUM ("Right DAC Data Path Control", aic325x_enum[RDACPATH_ENUM]),

        /* Left/Right DAC Digital Volume Control */
        SOC_DOUBLE_R_SX_TLV ("DAC Digital Volume Control",
                              LDAC_VOL, RDAC_VOL, 8,0xffffff81, 0x30, dac_vol_tlv),


#ifndef AIC3253_CODEC_SUPPORT

        /* Left/Right ADC Fine Gain Adjust */
	SOC_DOUBLE ("Left/Right ADC Fine Gain Adjust", ADC_FGA, 4, 0, 0x04, 0),

        /* Left/Right ADC Volume Control */
        SOC_DOUBLE_R_SX_TLV ("ADC Digital Volume Control",
                              LADC_VOL, RADC_VOL, 7, 0xffffff68, 0x28 , adc_vol_tlv),
#endif



        /* Left HP Driver Mute Control */
        SOC_ENUM ("Left HP driver mute", aic325x_enum[LHPMUTE_ENUM]),

        /* Right HP Driver Mute Control */
        SOC_ENUM ("Right HP driver mute", aic325x_enum[RHPMUTE_ENUM]),

#ifndef AIC3253_CODEC_SUPPORT
        /* Left LO Driver Mute control */
        SOC_ENUM ("Left LO driver mute", aic325x_enum[LLOMUTE_ENUM]),

	/* Right LO Driver Mute control */
        SOC_ENUM ("Right LO driver mute", aic325x_enum[RLOMUTE_ENUM]),
#endif

	/*HP Driver Gain Control*/
        SOC_DOUBLE_R_SX_TLV("HeadPhone Driver Gain", HPL_GAIN, HPR_GAIN, 6, 0xfffffffa, 0xe, output_gain_tlv),

#ifndef AIC3253_CODEC_SUPPORT
        /*LO Driver Gain Control*/
        SOC_DOUBLE_R_SX_TLV("Line Driver Gain", LOL_GAIN, LOR_GAIN, 6, 0xfffffffa, 0x1d , output_gain_tlv),


	/* Mixer Amplifier Volume Control */
        SOC_DOUBLE_R ("Mixer Amplier Volume Control",
                      MAL_CTRL_REG, MAR_CTRL_REG, 0, 0x50, 0),


        /*Left/Right MICPGA Volume Control */
        SOC_DOUBLE_R_TLV ("MICPGA Volume Control",
                      LMICPGA_VOL_CTRL, RMICPGA_VOL_CTRL, 0, 0x5F, 0, micpga_gain_tlv),

#endif

        /*Beep Generator Volume Control */
        SOC_DOUBLE_R_TLV ("Beep Generator Volume Control",
                      BEEP_CTRL_REG1, BEEP_CTRL_REG2, 0, 0x3f, 1, micpga_gain_tlv),

        /* Left Audio gain control (AGC) Enable/Disable Control */
        SOC_ENUM ("Left AGC Enable/Disable", aic325x_enum[LAGC_ENUM]),

#ifndef AIC3253_CODEC_SUPPORT
        /* Left/Right Audio gain control (AGC) Enable/Disable Control */
        SOC_ENUM ("Right AGC Enable/Disable", aic325x_enum[RAGC_ENUM]),

        /* Left/Right AGC Target level control */
        SOC_DOUBLE_R ("AGC Target Level Control",
                      LEFT_AGC_REG1, RIGHT_AGC_REG1, 4, 0x07, 1),

        /* Left/Right AGC Hysteresis Control */
        SOC_DOUBLE_R ("AGC Hysteresis Control",
                      LEFT_AGC_REG1, RIGHT_AGC_REG1, 0, 0x03, 0),

        /*Left/Right AGC Maximum PGA applicable */
        SOC_DOUBLE_R ("AGC Maximum PGA Control",
                      LEFT_AGC_REG3,RIGHT_AGC_REG3,  0, 0x7F, 0),

        /* Left/Right AGC Noise Threshold */
        SOC_DOUBLE_R ("AGC Noise Threshold",
                      LEFT_AGC_REG2, RIGHT_AGC_REG2, 1, 0x1F, 1),

        /* Left/Right AGC Attack Time control */
        SOC_DOUBLE_R ("AGC Attack Time control",
                      LEFT_AGC_REG4, RIGHT_AGC_REG4, 3, 0x1F, 0),

        /* Left/Right AGC Decay Time control */
        SOC_DOUBLE_R ("AGC Decay Time control",
                      LEFT_AGC_REG5, RIGHT_AGC_REG5, 3, 0x1F, 0),

        /* Left/Right AGC Noise Debounce control */
        SOC_DOUBLE_R ("AGC Noice bounce control",
                      LEFT_AGC_REG6, RIGHT_AGC_REG6, 0, 0x1F, 0),

        /* Left/Right AGC Signal Debounce control */
        SOC_DOUBLE_R ("Left/Right AGC Signal bounce control",
                      LEFT_AGC_REG7, RIGHT_AGC_REG7, 0, 0x0F, 0),
#endif

	/* DAC Signal Processing Block Control*/
	SOC_SINGLE ("DAC PRB Selection(1 to 25)", DAC_PRB,  0, 0x19, 0),
	
	/* ADC Signal Processing Block Control */
	SOC_SINGLE ("ADC PRB Selection(1 to 18)", ADC_PRB,  0, 0x12, 0),

        /* sound new kcontrol for Programming the registers from user space */
        SOC_SINGLE_AIC325x("Program Registers"),
};

/* Various Controls For AIC325x */
static const struct snd_kcontrol_new aic325x_snd_controls4[] = {

#ifndef AIC3253_CODEC_SUPPORT

        /* Left MICPGA Gain Enabled/Disable */
        SOC_ENUM ("Left MICPGA Gain Enable", aic325x_enum[LMICPGAGAIN_ENUM]),

        /* Right MICPGA Gain Enabled/Disable */
        SOC_ENUM ("Right MICPGA Gain Enable", aic325x_enum[RMICPGAGAIN_ENUM]),
#endif

        /* DRC Enable/Disable Control */
        SOC_ENUM ("DRC Enable/Disable", aic325x_enum[DRC_ENUM]),

        /* Beep generator Enable/Disable control */
        SOC_ENUM ("Beep generator Enable/Disable", aic325x_enum[BEEP_ENUM]),

        /* Beep Master Volume Control */
        SOC_ENUM ("Beep Master Volume Control", aic325x_enum[BEEPEXTRA_ENUM]),


#if defined(AIC3206_CODEC_SUPPORT) || defined( AIC3256_CODEC_SUPPORT )

        /*charge pump configuration for n/8 peak load current*/
        SOC_SINGLE("Charge pump configuration for n/8 peak load current", CHRG_CTRL_REG, 4, 8, 0),
       
        /*charge pump clock divide control*/
        SOC_SINGLE("charge pump clock divide control", CHRG_CTRL_REG, 0, 16, 0),  
       

        /*HPL, HPR master gain control in ground centerd mode */
        SOC_SINGLE("HPL & HPR master gain control in ground centered mode", HP_DRIVER_CONF_REG, 5, 4, 0),

        /*headphone amplifier compensation adjustment */
        SOC_SINGLE("headphone amplifier compensation adjustment", HP_DRIVER_CONF_REG, 7, 1, 0),

        /*Disable/Enable ground centered mode*/
        SOC_SINGLE("Disable/Enable ground centered mode", HP_DRIVER_CONF_REG, 4, 1, 0),

        /*headphone driver power configuration*/
        SOC_ENUM("Headphone driver power configration", hp_driver_power_conf),
//        SOC_SINGLE("Headphone driver power configration", HP_DRIVER_CONF_REG, 2, 4, 0),

        /*DC offset correction*/
        SOC_ENUM("DC offset correction", dc_offset_correction),
//        SOC_SINGLE("DC offset correction", HP_DRIVER_CONF_REG, 0, 4, 0),

#endif 


};


/* the sturcture contains the different values for mclk */
static const struct aic325x_rate_divs aic325x_divs[] = {
/*
 * mclk, rate, p_val, pll_j, pll_d, dosr, ndac, mdac, aosr, nadc, madc, blck_N,
 * codec_speficic_initializations
 */
	/* 8k rate */
#ifdef  CONFIG_MINI_DSP
	{19200000, 8000, 1, 5, 1200, 768, 2, 8, 128, 2, 48, 4,
	 {}},
	{38400000, 8000, 1, 4, 1200, 256, 24, 4, 128, 12, 8, 8,
	 {}},
#else
	{12000000, 8000, 1, 8, 1920, 128, 12, 8, 128, 8, 6, 4,
	 {{60, 1}, {61, 1}}},
	{24000000, 8000, 1, 4, 96, 128, 12, 8, 128, 12, 8, 4,
	 {{60, 1}, {61, 1}}},
	{19200000, 8000, 1, 5, 1200, 768, 2, 8, 128, 2, 48, 4,
	 {{60, 1}, {61, 1}}},
//	{26000000, 4800, 2, 7, 5619, 128, 2, 8, 128, 2, 8, 4,
//	 {{60, 1}, {61, 1}}},
	{38400000, 8000, 1, 4, 1200, 256, 24, 4, 128, 12, 8, 8,
	 {{60, 1}, {61, 1}}},
#endif
	/* 11.025k rate */
	{12000000, 11025, 1, 1, 8816, 1024, 8, 2, 128, 8, 2, 4,
	 {{60, 1}, {61, 1}}},
	{24000000, 11025, 1, 3, 7632, 128, 8, 8, 128, 8, 8, 4,
	 {{60, 1}, {61, 1}}},
	/* 16k rate */
	{12000000, 16000, 1, 8, 1920, 128, 8, 6, 128, 8, 6, 4,
	 {{60, 1}, {61, 1}}},
	{24000000, 16000, 1, 4, 96, 128, 8, 6, 128, 8, 6, 4,
	 {{60, 1}, {61, 1}}},
	{38400000, 16000, 1, 5, 1200, 256, 12, 4, 128, 8, 6, 8,
	 {{60, 1}, {61, 1}}},

	/* 22.05k rate */
	{12000000, 22050, 1, 3, 7632, 128, 8, 2, 128, 8, 2, 4,
	 {{60, 1}, {61, 1}}},
	{24000000, 22050, 1, 3, 7632, 128, 8, 3, 128, 8, 3, 4,
	 {{60, 1}, {61, 1}}},
	/* 32k rate */
	{12000000, 32000, 1, 5, 4613, 128, 8, 2, 128, 8, 2, 4,
	 {{60, 1}, {61, 1}}},
	{24000000, 32000, 1, 4, 96, 128, 6, 4, 128, 6, 4, 4,
	 {{60, 1}, {61, 1}}},
	{38400000, 32000, 1, 5, 1200, 256, 6, 4, 128, 6, 4, 8,
	 {{60, 1}, {61, 1}}},

	/* 44.1k rate */
#ifdef  CONFIG_MINI_DSP
	{12000000, 44100, 1, 7, 5264, 128, 2, 8, 128, 2, 8, 4,
	 {}},
	{19200000, 44100, 1, 4, 7040, 128, 2, 8, 128, 2, 8, 4,
	 {}},
	{24000000, 44100, 2, 3, 7632, 128, 4, 4, 128, 4, 4, 4,
	 {}},
	{38400000, 44100, 2, 4, 7040, 128, 2, 8, 128, 2, 8, 4,
	 {}},
#else
	{12000000, 44100, 1, 7, 5264, 128, 8, 2, 128, 8, 2, 4,
	 {{60, 1}, {61, 1}}},

	{19200000, 44100, 1, 4, 7040, 128, 8, 2, 128, 8, 2, 4,
	 {{60, 1}, {61, 1}}},

	{24000000, 44100, 1, 3, 7632, 128, 4, 4, 64, 4, 4, 4,
	 {{60, 1}, {61, 1}}},
	{38400000, 44100, 2, 4, 7040, 128, 2, 8, 128, 2, 8, 4,
	 {{60, 1}, {61, 1}}},

#endif
/* 48k rate */
#ifdef  CONFIG_MINI_DSP
	{12000000, 48000, 1, 8, 000, 128, 2, 8, 128, 2, 8, 4,
	 {{}, {}}},
	{19200000, 48000, 1, 5, 1200, 128, 2, 8, 128, 2, 8, 4,
	 {{}, {}}},
	{24000000, 48000, 1, 4, 960, 128, 4, 4, 128, 4, 4, 4,
	 {{}, {}}},
	{38400000, 48000, 2, 5, 1200, 128, 2, 8, 128, 2, 8, 4,
	 {{}, {}}},
#else
	/* 48k rate */
	{12000000, 48000, 1, 8, 1920, 128, 2, 8, 128, 2, 8, 4,
	 {{60, 1}, {61, 1}}},
	{19200000, 48000, 1, 5, 1200, 128, 8, 2, 128, 8, 2, 4,
	 {{60, 1}, {61, 1}}},
	{24000000, 48000, 1, 4, 960, 128, 4, 4, 128, 4, 4, 4,
	 {{60, 1}, {61, 1}}},
	{26000000, 48000, 2, 7, 5619, 128, 2, 8, 128, 2, 8, 4,
	 {{60, 1}, {61, 1}}},
	{38400000, 48000, 2, 5, 1200, 128, 2, 8, 128, 2, 8, 4,
	 {{60, 1}, {61, 1}}},
#endif
	/*96k rate */
	{12000000, 96000, 1, 16, 3840, 128, 8, 2, 128, 8, 2 , 4,
	 {{60, 7}, {61, 7}}},
	{24000000, 96000, 1, 4, 960, 128, 4, 2, 128, 4, 2, 2,
	 {{60, 7}, {61, 7}}},
	/*192k */
	{12000000, 192000, 1, 32, 7680, 128, 8, 2, 128, 8, 2, 4,
	 {{60, 17}, {61, 13}}},
	{24000000, 192000, 1, 4, 960, 128, 2, 2, 128, 2, 2, 4,
	 {{60, 17}, {61, 13}}},
};


/*
 *----------------------------------------------------------------------------
 * @struct  snd_soc_codec_dai_ops |
 *          It is SoC Codec DAI Operations structure
 *----------------------------------------------------------------------------
 */
static struct snd_soc_dai_ops aic325x_dai_ops = {
        .hw_params = aic325x_hw_params,
        .digital_mute = aic325x_mute,
        .set_sysclk = aic325x_set_dai_sysclk,
        .set_fmt = aic325x_set_dai_fmt,
};

struct snd_soc_dai_driver tlv320aic325x_dai_driver[] = {                                                                                                     
        {                                                                                                                                                    
        .name = "tlv320aic325x-MM_EXT",                                                                                                                            
        .playback = {                                                                                                                                        
                     .stream_name = "Playback",                                                                                                              
                     .channels_min = 1,                                                                                                                      
                     .channels_max = 2,                                                                                                                      
                     .rates = AIC325x_RATES,                                                                                                                 
                     .formats = AIC325x_FORMATS,                                                                                                             
                     },                                                                                                                                      
        .capture = {                                                                                                                                         
                    .stream_name = "Capture",                                                                                                                
                    .channels_min = 1,                                                                                                                       
                    .channels_max = 2,                                                                                                                       
                    .rates = AIC325x_RATES,                                                                                                                  
                    .formats = AIC325x_FORMATS,                                                                                                              
                    },                                                                                                                                       
        .ops = &aic325x_dai_ops,                                                                                                                             
        },                                                                                                                                                   
                                                                                                                                                             
};       

/*
*****************************************************************************
* Initializations
*****************************************************************************
*/

/*
 * AIC325x register cache
 * We are caching the registers here.
 * There is no point in caching the reset register.
 * NOTE: In AIC325x, there are 127 registers supported in both page0 and page1
 *       The following table contains the page0 and page1 registers values.
 */
static const u8 aic325x_reg[AIC325x_CACHEREGNUM] = {
	0x00, 0x00, 0x00, 0x00,	/* 0 */
	0x00, 0x11, 0x04, 0x00,	/* 4 */
	0x00, 0x00, 0x00, 0x01,	/* 8 */
	0x01, 0x00, 0x80, 0x02,	/* 12 */
	0x00, 0x08, 0x01, 0x01,	/* 16 */
	0x80, 0x01, 0x00, 0x04,	/* 20 */
	0x00, 0x00, 0x01, 0x00,	/* 24 */
	0x00, 0x00, 0x01, 0x00,	/* 28 */
	0x00, 0x00, 0x00, 0x00,	/* 32 */
	0x00, 0x00, 0x00, 0x00,	/* 36 */
	0x00, 0x00, 0x00, 0x00,	/* 40 */
	0x00, 0x00, 0x00, 0x00,	/* 44 */
	0x00, 0x00, 0x00, 0x00,	/* 48 */
	0x00, 0x42, 0x02, 0x02,	/* 52 */
	0x42, 0x02, 0x02, 0x02,	/* 56 */
	0x00, 0x00, 0x00, 0x01,	/* 60 */
	0x01, 0x00, 0x14, 0x00,	/* 64 */
	0x0C, 0x00, 0x00, 0x00,	/* 68 */
	0x00, 0x00, 0x00, 0xEE,	/* 72 */
	0x10, 0xD8, 0x10, 0xD8,	/* 76 */
	0x00, 0x00, 0x88, 0x00,	/* 80 */
	0x00, 0x00, 0x00, 0x00,	/* 84 */
	0x7F, 0x00, 0x00, 0x00,	/* 88 */
	0x00, 0x00, 0x00, 0x00,	/* 92 */
	0x7F, 0x00, 0x00, 0x00,	/* 96 */
	0x00, 0x00, 0x00, 0x00,	/* 100 */
	0x00, 0x00, 0x00, 0x00,	/* 104 */
	0x00, 0x00, 0x00, 0x00,	/* 108 */
	0x00, 0x00, 0x00, 0x00,	/* 112 */
	0x00, 0x00, 0x00, 0x00,	/* 116 */
	0x00, 0x00, 0x00, 0x00,	/* 120 */
	0x00, 0x00, 0x00, 0x00,	/* 124 - PAGE0 Registers(127) ends here */
	0x01, 0x00, 0x08, 0x00,	/* 128, PAGE1-0 */
	0x00, 0x00, 0x00, 0x00,	/* 132, PAGE1-4 */
	0x00, 0x00, 0x00, 0x10,	/* 136, PAGE1-8 */
	0x00, 0x00, 0x00, 0x00,	/* 140, PAGE1-12 */
	0x40, 0x40, 0x40, 0x40,	/* 144, PAGE1-16 */
	0x00, 0x00, 0x00, 0x00,	/* 148, PAGE1-20 */
	0x00, 0x00, 0x00, 0x00,	/* 152, PAGE1-24 */
	0x00, 0x00, 0x00, 0x00,	/* 156, PAGE1-28 */
	0x00, 0x00, 0x00, 0x00,	/* 160, PAGE1-32 */
	0x00, 0x00, 0x00, 0x00,	/* 164, PAGE1-36 */
	0x00, 0x00, 0x00, 0x00,	/* 168, PAGE1-40 */
	0x00, 0x00, 0x00, 0x00,	/* 172, PAGE1-44 */
	0x00, 0x00, 0x00, 0x00,	/* 176, PAGE1-48 */
	0x00, 0x00, 0x00, 0x00,	/* 180, PAGE1-52 */
	0x00, 0x00, 0x00, 0x80,	/* 184, PAGE1-56 */
	0x80, 0x00, 0x00, 0x00,	/* 188, PAGE1-60 */
	0x00, 0x00, 0x00, 0x00,	/* 192, PAGE1-64 */
	0x00, 0x00, 0x00, 0x00,	/* 196, PAGE1-68 */
	0x00, 0x00, 0x00, 0x00,	/* 200, PAGE1-72 */
	0x00, 0x00, 0x00, 0x00,	/* 204, PAGE1-76 */
	0x00, 0x00, 0x00, 0x00,	/* 208, PAGE1-80 */
	0x00, 0x00, 0x00, 0x00,	/* 212, PAGE1-84 */
	0x00, 0x00, 0x00, 0x00,	/* 216, PAGE1-88 */
	0x00, 0x00, 0x00, 0x00,	/* 220, PAGE1-92 */
	0x00, 0x00, 0x00, 0x00,	/* 224, PAGE1-96 */
	0x00, 0x00, 0x00, 0x00,	/* 228, PAGE1-100 */
	0x00, 0x00, 0x00, 0x00,	/* 232, PAGE1-104 */
	0x00, 0x00, 0x00, 0x00,	/* 236, PAGE1-108 */
	0x00, 0x00, 0x00, 0x00,	/* 240, PAGE1-112 */
	0x00, 0x00, 0x00, 0x00,	/* 244, PAGE1-116 */
	0x00, 0x00, 0x00, 0x00,	/* 248, PAGE1-120 */
	0x00, 0x00, 0x00, 0x00	/* 252, PAGE1-124 */
};

/*
 * aic325x initialization data
 * This structure initialization contains the initialization required for
 * AIC325x.
 * These registers values (reg_val) are written into the respective AIC325x
 * register offset (reg_offset) to  initialize AIC325x.
 * These values are used in aic325x_init() function only.
 */
static const struct aic325x_configs aic325x_reg_init[] = {
	{PAGE_SELECT, 0x0},
	/* Carry out the software reset */
	{RESET, 0x01},
	/* Disable crude LDO */
	{POW_CFG, 0x08},


        #if defined(AIC3256_CODEC_SUPPORT) || defined( AIC3206_CODEC_SUPPORT)
        {PWR_CTRL_REG, 0x00},
        #else
        {LDO_CTRL, 0x00},
	#endif
       	{REF_PWR_UP_CONF_REG, 0x1}, 
        #if defined(AIC3256_CODEC_SUPPORT) || defined( AIC3206_CODEC_SUPPORT)
        {CHRG_CTRL_REG, 0x06},
        #endif
        

        

	/* Connect IN1_L and IN1_R to CM */
	{INPUT_CFG_REG, 0xc0},
	/* PLL is CODEC_CLKIN */
	{CLK_REG_1, PLLCLK_2_CODEC_CLKIN},
	/* DAC_MOD_CLK is BCLK source */
	{AIS_REG_3, DAC_MOD_CLK_2_BDIV_CLKIN},
	/* Setting up DAC Channel */
	{DAC_CHN_REG,
	 LDAC_2_LCHN | RDAC_2_RCHN | SOFT_STEP_2WCLK},
	/* Headphone powerup */
//	{HPHONE_STARTUP_CTRL, 0x00},

	/* HPL unmute and gain 0db */
	{HPL_GAIN, 0x0},
	/* HPR unmute and gain 0db */
	{HPR_GAIN, 0x0},

#ifndef AIC3253_CODEC_SUPPORT
	/* LOL unmute and gain 0db */
	{LOL_GAIN, 0x0},
	/* LOR unmute and gain 0db */
	{LOR_GAIN, 0x0},
#endif

#ifndef AIC3253_CODEC_SUPPORT
	/* Set the input powerup time to 3.1ms (for ADC) */
	{ANALOG_INPUT_CHARGING_CFG, 0x31},
	/* Left mic PGA unmuted */
	{LMICPGA_VOL_CTRL, 0x00},
	/* Right mic PGA unmuted */
	{RMICPGA_VOL_CTRL, 0x00},
	/* ADC volume control change by 2 gain step per ADC Word Clock */
	{ADC_CHN_REG, 0x02},
	/* Unmute ADC left and right channels */
	{ADC_FGA, 0x00},
#endif

#ifdef AIC3256_CODEC_SUPPORT
	/* Enabled ground centered HP driver and enable DC offset correction always */
	{HP_DRIVER_CONF_REG, 0x12},
#endif
	/* GPIO output is INT1 */
	{GPIO_CTRL, 0x14 },
	/*  Headset Insertion event will generate a INT1 interrupt */
	{INT1_CTRL, 0x80},
	/*Enable headset detection and button press with a debounce time 64ms */
//	{HEADSET_DETECT, 0x89},
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

#ifndef AIC3253_CODEC_SUPPORT
/* Left Line out mixer */
static const struct snd_kcontrol_new lol_output_mixer_controls[] = {
	SOC_DAPM_SINGLE("R_DAC switch", LOL_ROUTE_CTRL, 4, 1, 0),
	SOC_DAPM_SINGLE("L_DAC switch", LOL_ROUTE_CTRL, 3, 1, 0),
	SOC_DAPM_SINGLE("MAL switch", LOL_ROUTE_CTRL, 1, 1, 0),
	SOC_DAPM_SINGLE("LOR switch", LOL_ROUTE_CTRL, 0, 1, 0),
};
/* Right Line out Mixer */
static const struct snd_kcontrol_new lor_output_mixer_controls[] = {
	SOC_DAPM_SINGLE("R_DAC switch", LOR_ROUTE_CTRL, 3, 1, 0),
	SOC_DAPM_SINGLE("MAR switch", LOR_ROUTE_CTRL, 1, 1, 0),
};

/* Left Input Mixer */
static const struct snd_kcontrol_new left_input_mixer_controls[] = {
	SOC_DAPM_SINGLE("IN1_L switch", LMICPGA_PIN_CFG, 6, 1, 0),
	SOC_DAPM_SINGLE("IN2_L switch", LMICPGA_PIN_CFG, 4, 1, 0),
	SOC_DAPM_SINGLE("IN3_L switch", LMICPGA_PIN_CFG, 2, 1, 0),
	SOC_DAPM_SINGLE("IN1_R switch", LMICPGA_PIN_CFG, 0, 1, 0),
	SOC_DAPM_SINGLE("CM1L switch", LMICPGA_NIN_CFG, 6, 1, 0),
	SOC_DAPM_SINGLE("IN2_R switch", LMICPGA_NIN_CFG, 4, 1, 0),
	SOC_DAPM_SINGLE("IN3_R switch", LMICPGA_NIN_CFG, 2, 1, 0),
	SOC_DAPM_SINGLE("CM2L switch", LMICPGA_NIN_CFG, 0, 1, 0),
};

/* Right Input Mixer */
static const struct snd_kcontrol_new right_input_mixer_controls[] = {
	SOC_DAPM_SINGLE("IN1_R switch", RMICPGA_PIN_CFG, 6, 1, 0),
	SOC_DAPM_SINGLE("IN2_R switch", RMICPGA_PIN_CFG, 4, 1, 0),
	SOC_DAPM_SINGLE("IN3_R switch", RMICPGA_PIN_CFG, 2, 1, 0),
	SOC_DAPM_SINGLE("IN2_L switch", RMICPGA_PIN_CFG, 0, 1, 0),
	SOC_DAPM_SINGLE("CM1R switch", RMICPGA_NIN_CFG, 6, 1, 0),
	SOC_DAPM_SINGLE("IN1_L switch", RMICPGA_NIN_CFG, 4, 1, 0),
	SOC_DAPM_SINGLE("IN3_L switch", RMICPGA_NIN_CFG, 2, 1, 0),
	SOC_DAPM_SINGLE("CM2R switch", RMICPGA_NIN_CFG, 0, 1, 0),
};


#endif

/* AIC325x Widget Structure */
static const struct snd_soc_dapm_widget aic325x_dapm_widgets[] = {
	/* Left DAC to Left Outputs */
	/* dapm widget (stream domain) for left DAC */
	SND_SOC_DAPM_DAC("Left DAC", "Left Playback", DAC_CHN_REG, 7, 0),

	/* dapm widget (path domain) for left DAC_L Mixer */
	SND_SOC_DAPM_MIXER("HPL Output Mixer", SND_SOC_NOPM, 0, 0,
			   &hpl_output_mixer_controls[0],
			   ARRAY_SIZE(hpl_output_mixer_controls)),

 	/* dapm widget for Left Head phone Power */
	SND_SOC_DAPM_PGA_E("HPL PGA", OUT_PWR_CTRL, 5, 0, NULL, 0,
				aic325x_hp_event, SND_SOC_DAPM_PRE_PMU),


#ifndef AIC3253_CODEC_SUPPORT
	/* dapm widget (path domain) for Left Line-out Output Mixer */
	SND_SOC_DAPM_MIXER("LOL Output Mixer", SND_SOC_NOPM, 0, 0,
			   &lol_output_mixer_controls[0],
			   ARRAY_SIZE(lol_output_mixer_controls)),

	/* dapm widget for Left Line-out Power */
	SND_SOC_DAPM_PGA("LOL PGA", OUT_PWR_CTRL, 3, 0, NULL, 0),
#endif

	/* Right DAC to Right Outputs */
	/* dapm widget (stream domain) for right DAC */
	SND_SOC_DAPM_DAC("Right DAC", "Right Playback", DAC_CHN_REG, 6, 0),

	/* dapm widget (path domain) for right DAC_R mixer */
	SND_SOC_DAPM_MIXER("HPR Output Mixer", SND_SOC_NOPM, 0, 0,
			   &hpr_output_mixer_controls[0],
			   ARRAY_SIZE(hpr_output_mixer_controls)),
  
 	/* dapm widget for Right Head phone Power */
	SND_SOC_DAPM_PGA_E("HPR PGA", OUT_PWR_CTRL, 4, 0, NULL, 0,
				aic325x_hp_event, SND_SOC_DAPM_PRE_PMU),

#ifndef AIC3253_CODEC_SUPPORT
	/* dapm widget for (path domain) Right Line-out Output Mixer */
	SND_SOC_DAPM_MIXER("LOR Output Mixer", SND_SOC_NOPM, 0, 0,
			   &lor_output_mixer_controls[0],
			   ARRAY_SIZE(lor_output_mixer_controls)),

 	/* dapm widget for Right Line-out Power */
	SND_SOC_DAPM_PGA("LOR PGA", OUT_PWR_CTRL, 2, 0, NULL, 0),
#endif

#ifdef AIC3256_CODEC_SUPPORT
	SND_SOC_DAPM_SUPPLY("Charge Pump", POW_CFG, 1, 0,                                                                                                                   aic3256_cp_event, SND_SOC_DAPM_POST_PMU),
#endif

	SND_SOC_DAPM_PGA("CM", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("CM1L", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("CM2L", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("CM1R", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_PGA("CM2R", SND_SOC_NOPM, 0, 0, NULL, 0),


#ifndef AIC3253_CODEC_SUPPORT
	/* Left MicPGA to Left ADC */
	SND_SOC_DAPM_ADC("Left ADC", "Left Capture", ADC_CHN_REG, 7, 0),

	/* Right MicPGA to Right ADC */
	SND_SOC_DAPM_ADC("Right ADC", "Right Capture", ADC_CHN_REG, 6, 0),

	/* Left Inputs to Left MicPGA */
	SND_SOC_DAPM_PGA("Left MicPGA", LMICPGA_VOL_CTRL , 7, 1, NULL, 0),

	/* Right Inputs to Right MicPGA */
	SND_SOC_DAPM_PGA("Right MicPGA", RMICPGA_VOL_CTRL, 7, 1, NULL, 0),

	/* Left MicPGA to Mixer PGA Left */
	SND_SOC_DAPM_PGA("MAL PGA", OUT_PWR_CTRL , 1, 0, NULL, 0),

	/* Right Inputs to Mixer PGA Right */
	SND_SOC_DAPM_PGA("MAR PGA", OUT_PWR_CTRL, 0, 0, NULL, 0),

#endif

	/* dapm widget for Left Input Mixer*/
	SND_SOC_DAPM_MIXER("Left Input Mixer", SND_SOC_NOPM, 0, 0,
			   &left_input_mixer_controls[0],
			   ARRAY_SIZE(left_input_mixer_controls)),

	/* dapm widget for Right Input Mixer*/
	SND_SOC_DAPM_MIXER("Right Input Mixer", SND_SOC_NOPM, 0, 0,
			   &right_input_mixer_controls[0],
			   ARRAY_SIZE(right_input_mixer_controls)),
	/* dapm widget (platform domain) name for HPLOUT */
	SND_SOC_DAPM_OUTPUT("HPL"),

	/* dapm widget (platform domain) name for HPROUT */
	SND_SOC_DAPM_OUTPUT("HPR"),

#ifndef AIC3253_CODEC_SUPPORT
	/* dapm widget (platform domain) name for LOLOUT */
	SND_SOC_DAPM_OUTPUT("LOL"),

	/* dapm widget (platform domain) name for LOROUT */
	SND_SOC_DAPM_OUTPUT("LOR"),

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
#endif


};

static const struct snd_soc_dapm_route aic325x_dapm_routes[] = {

	/* Left  Headphone Output */
	{"HPL Output Mixer", "L_DAC switch", "Left DAC"},
	{"HPL Output Mixer", "IN1_L switch", "IN1_L"},
	{"HPL Output Mixer", "MAL switch", "MAL PGA"},//MAL
	{"HPL Output Mixer", "MAR switch", "MAR PGA"},//MAR

	/* Right Headphone Output */
	{"HPR Output Mixer", "R_DAC switch", "Right DAC"},
	{"HPR Output Mixer", "IN1_R switch", "IN1_R"},
	{"HPR Output Mixer", "MAR switch", "MAR PGA"}, //MAR
	{"HPR Output Mixer", "L_DAC switch", "Left DAC"},


	{"HPL PGA", NULL, "HPL Output Mixer"},
	{"HPR PGA", NULL, "HPR Output Mixer"},

	{"HPL", NULL, "HPL PGA"},
	{"HPR", NULL, "HPR PGA"},

#ifdef AIC3256_CODEC_SUPPORT
	{"HPL PGA", NULL, "Charge Pump"},
	{"HPR PGA", NULL, "Charge Pump"},

#endif	

#ifndef AIC3253_CODEC_SUPPORT
	/* Left Line-out Output */
	{"LOL Output Mixer", "L_DAC switch", "Left DAC"},
	{"LOL Output Mixer", "MAL switch", "MAL PGA"},
	{"LOL Output Mixer", "R_DAC switch", "Right DAC"},
	{"LOL Output Mixer", "LOR switch","LOR PGA"},


	/* Right Line-out Output */
	{"LOR Output Mixer", "R_DAC switch", "Right DAC"},
	{"LOR Output Mixer", "MAR switch", "MAR PGA"},//MAR


	{"LOL PGA", NULL, "LOL Output Mixer"},
	{"LOR PGA", NULL, "LOR Output Mixer"},

	{"LOL", NULL, "LOL PGA"},
	{"LOR", NULL, "LOR PGA"},
#endif



#ifndef AIC3253_CODEC_SUPPORT

	/* ADC portions */ 
	/* Left Positive PGA input */
	{"Left Input Mixer", "IN1_L switch", "IN1_L"},
	{"Left Input Mixer", "IN2_L switch", "IN2_L"},
	{"Left Input Mixer", "IN3_L switch", "IN3_L"},
	{"Left Input Mixer", "IN1_R switch", "IN1_R"},
	/* Left Negative PGA input */
	{"Left Input Mixer", "IN2_R switch", "IN2_R"},
	{"Left Input Mixer", "IN3_R switch", "IN3_R"},
	{"Left Input Mixer", "CM1L switch", "CM1L"},
	{"Left Input Mixer", "CM2L switch", "CM2L"},

	/* Right Positive PGA Input */
	{"Right Input Mixer", "IN1_R switch", "IN1_R"},
	{"Right Input Mixer", "IN2_R switch", "IN2_R"},
	{"Right Input Mixer", "IN3_R switch", "IN3_R"},
	{"Right Input Mixer", "IN2_L switch", "IN2_L"},
	/* Right Negative PGA Input */
	{"Right Input Mixer", "IN1_L switch", "IN1_L"},
	{"Right Input Mixer", "IN3_L switch", "IN3_L"},
	{"Right Input Mixer", "CM1R switch", "CM1R"},
	{"Right Input Mixer", "CM2R switch", "CM2R"},

	
	{"CM1L", NULL, "CM"},
	{"CM2L", NULL, "CM"},
	{"CM1R", NULL, "CM"},
	{"CM1R", NULL, "CM"},


	/* Left MicPGA */
	{"Left MicPGA", NULL, "Left Input Mixer"},

	/* Right MicPGA */	
	{"Right MicPGA", NULL, "Right Input Mixer"},

	{"Left ADC", NULL, "Left MicPGA"},
	{"Right ADC", NULL, "Right MicPGA"},


	{"MAL PGA", NULL, "Left MicPGA"},
	{"MAR PGA", NULL, "Right MicPGA"},
#endif
};

#define AIC325x_DAPM_ROUTE_NUM (sizeof(aic325x_dapm_routes)/sizeof(struct snd_soc_dapm_route))

/*
*****************************************************************************
* Function Definitions
*****************************************************************************
*/
#ifdef AIC3256_CODEC_SUPPORT
/*                                                                                                                                                           
 * Event for headphone amplifier power changes.  Special                                                                                        
 * power up/down sequences are required in order to maximise pop/click                                                                                       
 * performance.                                                                                                                                              
 */                                                                                                                                                          
static int aic325x_hp_event(struct snd_soc_dapm_widget *w,                                                                                                
                               struct snd_kcontrol *kcontrol, int event)                                                                                     
{
	volatile u8 value;
	struct snd_soc_codec *codec = w->codec;     
	struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);
	if(w->shift == 5)// HPL
	{
//		snd_soc_update_bits(codec, CM_CTRL_REG, GCHP_HPL_STATUS, GCHP_HPL_STATUS);
//		value = snd_soc_read(codec, CM_CTRL_REG);
//		value |= GCHP_HPL_STATUS;
//		snd_soc_write(codec, CM_CTRL_REG, value);
		
	}
	if(w->shift == 4)// HPR
	{
//		snd_soc_update_bits(codec, CM_CTRL_REG, GCHP_HPL_STATUS, 0x0);
//		value = snd_soc_read(codec, CM_CTRL_REG);
//		value &= ~GCHP_HPL_STATUS;// HPR status
//		snd_soc_write(codec, CM_CTRL_REG, value);
	}
	// Wait for HP power on 
	if(event & SND_SOC_DAPM_POST_PMU) 
	{
//		 do {                                                                                                                                                 
//			msleep(5);
//	               value = snd_soc_read(codec, PWR_CTRL_REG);                                                                   
//	       } while ((value & HP_DRIVER_BUSY_MASK) == 0);   
		
			
	}
	return 0;
}
static int aic3256_cp_event(struct snd_soc_dapm_widget *w,                                                                                                
                               struct snd_kcontrol *kcontrol, int event)                                                                                     
{
	volatile u8 value;
	struct snd_soc_codec *codec = w->codec;     

	if(event & SND_SOC_DAPM_POST_PMU)
	{

		mdelay(6); // Wait 16 cycles of charge pump clock 
	}
	return 0;
}
#else

static int aic325x_hp_event(struct snd_soc_dapm_widget *w,                                                                                                
                               struct snd_kcontrol *kcontrol, int event)                                                                                     
{
	return 0;
}
#endif
/*
 *----------------------------------------------------------------------------
 * Function : __new_control_info
 * Purpose  : This function is to initialize data for new control required to
 *            program the AIC325x registers.
 *----------------------------------------------------------------------------
 */
static int __new_control_info(struct snd_kcontrol *kcontrol,
                              struct snd_ctl_elem_info *uinfo)
{
        uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
        uinfo->count = 1;
        uinfo->value.integer.min = 0;
        uinfo->value.integer.max = 65535;

        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_get
 * Purpose  : This function is to read data of new control for
 *            program the AIC325x registers.
 *----------------------------------------------------------------------------
 */
static int __new_control_get(struct snd_kcontrol *kcontrol,
                             struct snd_ctl_elem_value *ucontrol)
{
        struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
        u32 val;

        val = aic325x_read(codec, aic325x_reg_ctl);
        ucontrol->value.integer.value[0] = val;

        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_put
 * Purpose  : new_control_put is called to pass data from user/application to
 *            the driver.
 *----------------------------------------------------------------------------
 */
static int __new_control_put(struct snd_kcontrol *kcontrol,
                             struct snd_ctl_elem_value *ucontrol)
{
        struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   
	int i;

        u32 data_from_user = ucontrol->value.integer.value[0];
        u8 data[2];

        aic325x_reg_ctl = data[0] = (u8) ((data_from_user & 0xFF00) >> 8);
        data[1] = (u8) ((data_from_user & 0x00FF));

        if (!data[0]) {
                aic325x->page_no = data[1];
        }

        printk("reg = %d val = %x\n", data[0], data[1]);

	// HACK to re-init the device because the rootfs is writing some strange writes to the device */
	/* codec register initalization*/
        for (i = 0;
             i < sizeof(aic325x_reg_init) / sizeof(struct aic325x_configs);
             i++) {
                snd_soc_write(codec, aic325x_reg_init[i].reg_offset,
                              aic325x_reg_init[i].reg_val);
        }
/*        if (codec->hw_write(codec->control_data, data, 2) != 2) {
                printk("Error in i2c write\n");
                return -EIO;
        }*/
//	snd_soc_write(codec, aic325x_reg_ctl, data[1]);
        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_reset_cache
 * Purpose  : This function is to reset the cache.
 *----------------------------------------------------------------------------
 */
int aic325x_reset_cache (struct snd_soc_codec *codec)
{
#if defined(EN_REG_CACHE)
        if (codec->reg_cache != NULL)
        {
                memcpy(codec->reg_cache, aic325x_reg, sizeof (aic325x_reg));
                return 0;
        }

        codec->reg_cache = kmemdup (aic325x_reg, sizeof (aic325x_reg), GFP_KERNEL);
        if (codec->reg_cache == NULL)
        {
                printk (KERN_ERR "aic325x: kmemdup failed\n");
                return -ENOMEM;
        }
#endif
        return 0;
}


/*
 *----------------------------------------------------------------------------
 * Function : aic325x_change_page
 * Purpose  : This function is to switch between page 0 and page 1.
 *----------------------------------------------------------------------------
 */
int aic325x_change_page(struct snd_soc_codec *codec, u8 new_page)
{
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   
        u8 data[2];

        data[0] = 0;
        data[1] = new_page;
        aic325x->page_no = new_page;
	
	dprintk("w 30 00 %x\n", new_page);

        if (codec->hw_write(codec->control_data, data, 2) != 2) {
                printk("Error in changing page to 1\n");
                return -1;
        }
        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_write_reg_cache
 * Purpose  : This function is to write aic325x register cache
 *----------------------------------------------------------------------------
 */
inline void aic325x_write_reg_cache(struct snd_soc_codec *codec,
                                    u16 reg, u8 value)
{
        u8 *cache = codec->reg_cache;

        if (reg >= AIC325x_CACHEREGNUM) {
                return;
        }
        cache[reg] = value;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_write
 * Purpose  : This function is to write to the aic325x register space.
 *----------------------------------------------------------------------------
 */
int aic325x_write(struct snd_soc_codec *codec, u16 reg, u8 value)
{
        
        u8 data[2];
        u8 page;
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   
        
	
        page = reg / 128;
        data[AIC325x_REG_OFFSET_INDEX] = reg % 128;

        if (aic325x->page_no != page) {
                aic325x_change_page(codec, page);
        }


        /* data is
         *   D15..D8 aic325x register offset
         *   D7...D0 register data
         */
        data[AIC325x_REG_DATA_INDEX] = value & AIC325x_8BITS_MASK;

#if defined(EN_REG_CACHE)
        if ((page == 0) || (page == 1)) {
                aic325x_write_reg_cache(codec, reg, value);
        }
#endif
        if (!data[AIC325x_REG_OFFSET_INDEX]) {
                /* if the write is to reg0 update aic325x->page_no */
                aic325x->page_no = value;
        }

	dprintk("w 30 %x %x\n", (reg %128), value);

        if (codec->hw_write(codec->control_data, data, 2) != 2) {
                printk("Error in i2c write\n");
                return -EIO;
        }


        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_read
 * Purpose  : This function is to read the aic325x register space.
 *----------------------------------------------------------------------------
 */
static u8 aic325x_read(struct snd_soc_codec *codec, u16 reg)
{
        u8 value;
        u8 page = reg / 128;
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   

        reg = reg % 128;

        if (aic325x->page_no != page) {
                aic325x_change_page(codec, page);
        }

        i2c_master_send(codec->control_data, (char *)&reg, 1);
        i2c_master_recv(codec->control_data, &value, 1);
        return value;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_get_divs
 * Purpose  : This function is to get required divisor from the "aic325x_divs"
 *            table.
 *----------------------------------------------------------------------------
 */
static inline int aic325x_get_divs(int mclk, int rate)
{
        int i;
        
        for (i = 0; i < ARRAY_SIZE(aic325x_divs); i++) {
                if ((aic325x_divs[i].rate == rate)
                    && (aic325x_divs[i].mclk == mclk)) {
                        return i;
                }
        }
        printk("Master clock and sample rate is not supported, mclk = %d rate = %d\n", mclk, rate);
        return -EINVAL;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_add_widgets
 * Purpose  : This function is to add the dapm widgets. This routine will be
 *	      invoked during the Audio Driver Initialization.
 *            The following are the main widgets supported :
 *                # Left DAC to Left Outputs
 *                # Right DAC to Right Outputs
 *		  # Left Inputs to Left ADC
 *		  # Right Inputs to Right ADC
 *----------------------------------------------------------------------------
 */
static int aic325x_add_widgets(struct snd_soc_codec *codec)
{
        int i;

	dprintk( KERN_INFO "Function : %s \n", __FUNCTION__);
	snd_soc_dapm_new_controls(codec, aic325x_dapm_widgets, ARRAY_SIZE(aic325x_dapm_widgets));
 dprintk("#Completed adding new dapm widget controls size=%d\n",ARRAY_SIZE(aic325x_dapm_widgets));         

        /* set up audi path interconnects */
        snd_soc_dapm_add_routes(codec, aic325x_dapm_routes,
                                ARRAY_SIZE(aic325x_dapm_routes));
dprintk("#Completed adding DAPM routes = %d\n",ARRAY_SIZE(aic325x_dapm_routes));  

        snd_soc_dapm_new_widgets(codec);
        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_hw_params
 * Purpose  : This function is to set the hardware parameters for AIC325x.
 *            The functions set the sample rate and audio serial data word
 *            length.
 *----------------------------------------------------------------------------
 */
static int aic325x_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params,
                             struct snd_soc_dai *dai)
{
        struct snd_soc_pcm_runtime *rtd = substream->private_data;
        struct snd_soc_codec *codec = rtd->codec;
        int i, j;
        u8 data;
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   

	dprintk( KERN_INFO "Function : %s, %d \n", __FUNCTION__, aic325x->sysclk);

        i = aic325x_get_divs(aic325x->sysclk, params_rate(params));

        if (i < 0) {
                printk("sampling rate not supported\n");
                return i;
        }

	if(aic325x->pll_divs_index != i)
	{
        	if (soc_static_freq_config) {
	                /* We will fix R value to 1 and will make P & J=K.D as varialble */
	
	                /* Setting P & R values */
	                snd_soc_write(codec, CLK_REG_2, 
	                              ((aic325x_divs[i].p_val << 4) | 0x01));
	
	                /* J value */
	                snd_soc_write(codec, CLK_REG_3, aic325x_divs[i].pll_j);
	
 	               /* MSB & LSB for D value */
 	               snd_soc_write(codec, CLK_REG_4, (aic325x_divs[i].pll_d >> 8));
 	               snd_soc_write(codec, CLK_REG_5,
                              (aic325x_divs[i].pll_d & AIC325x_8BITS_MASK));

                	/* NDAC divider value */
                	snd_soc_write(codec, NDAC_CLK_REG_6, aic325x_divs[i].ndac);

                	/* MDAC divider value */
                	snd_soc_write(codec, MDAC_CLK_REG_7, aic325x_divs[i].mdac);

                	/* DOSR MSB & LSB values */
                	snd_soc_write(codec, DAC_OSR_MSB, aic325x_divs[i].dosr >> 8);
                	snd_soc_write(codec, DAC_OSR_LSB,
                              aic325x_divs[i].dosr & AIC325x_8BITS_MASK);

#ifndef AIC3253_CODEC_SUPPORT
                	/* NADC divider value */
                	snd_soc_write(codec, NADC_CLK_REG_8, aic325x_divs[i].nadc);

                	/* MADC divider value */
                	snd_soc_write(codec, MADC_CLK_REG_9, aic325x_divs[i].madc);
#endif

                	/* AOSR value */
                	snd_soc_write(codec, ADC_OSR_REG, aic325x_divs[i].aosr);
        	}
        	/* BCLK N divider */
        	snd_soc_write(codec, CLK_REG_11, aic325x_divs[i].blck_N);
	}

//        aic325x_set_bias_level(codec, SNDRV_CTL_POWER_D0);

        data = snd_soc_read(codec, INTERFACE_SET_REG_1);

        data = data & ~(3 << 4);

        switch (params_format(params)) {
        case SNDRV_PCM_FORMAT_S16_LE:
                break;
        case SNDRV_PCM_FORMAT_S20_3LE:
                data |= (AIC325x_WORD_LEN_20BITS << DAC_OSR_MSB_SHIFT);
                break;
        case SNDRV_PCM_FORMAT_S24_LE:
                data |= (AIC325x_WORD_LEN_24BITS << DAC_OSR_MSB_SHIFT);
                break;
        case SNDRV_PCM_FORMAT_S32_LE:
                data |= (AIC325x_WORD_LEN_32BITS << DAC_OSR_MSB_SHIFT);
                break;
        }

        snd_soc_update_bits(codec, INTERFACE_SET_REG_1,AIC325x_8BITS_MASK, data);

        for (j = 0; j < NO_FEATURE_REGS; j++) {
                snd_soc_update_bits(codec,
                              aic325x_divs[i].codec_specific_regs[j].reg_offset,AIC325x_8BITS_MASK,
                              aic325x_divs[i].codec_specific_regs[j].reg_val);
        }
	
	aic325x->pll_divs_index = i;
        return 0;
}



/*
 *-----------------------------------------------------------------------
 * Function : aic325x_mute
 * Purpose  : This function is to mute or unmute the left and right DAC
 *-----------------------------------------------------------------------
 */
static int aic325x_mute(struct snd_soc_dai *dai, int mute)
{
        struct snd_soc_codec *codec = dai->codec;
        u8 dac_reg;

	dprintk( KERN_INFO "Function : %s \n", __FUNCTION__);
        dac_reg = snd_soc_read(codec, DAC_MUTE_CTRL_REG) & ~MUTE_ON;
        if (mute)
                snd_soc_write(codec, DAC_MUTE_CTRL_REG, dac_reg | MUTE_ON);
        else
                snd_soc_write(codec, DAC_MUTE_CTRL_REG, dac_reg);

        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_set_dai_sysclk
 * Purpose  : This function is to set the DAI system clock
 *----------------------------------------------------------------------------
 */
static int aic325x_set_dai_sysclk(struct snd_soc_dai *codec_dai,
                                  int clk_id, unsigned int freq, int dir)
{
        struct snd_soc_codec *codec = codec_dai->codec;

        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   

	dprintk( KERN_INFO "Function : %s, %d \n", __FUNCTION__, freq);

        switch (freq) {
        case AIC325x_FREQ_12000000:
        case AIC325x_FREQ_24000000:
	case AIC325x_FREQ_19200000:
	case AIC325x_FREQ_26000000:
	case AIC325x_FREQ_38400000:
                aic325x->sysclk = freq;
                return 0;
        }
        printk("Invalid frequency to set DAI system clock\n");
        return -EINVAL;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_set_dai_fmt
 * Purpose  : This function is to set the DAI format
 *----------------------------------------------------------------------------
 */
static int aic325x_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
        struct snd_soc_codec *codec = codec_dai->codec;

        u8 iface_reg;
	u8 iface_reg1;
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   
	
        iface_reg = snd_soc_read(codec, INTERFACE_SET_REG_1);
//        iface_reg = iface_reg & ~(3 << 6 | 3 << 2);

        /* set master/slave audio interface */
        switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
        case SND_SOC_DAIFMT_CBM_CFM:
                aic325x->master = 1;
                iface_reg |= BIT_CLK_MASTER | WORD_CLK_MASTER;
                break;
        case SND_SOC_DAIFMT_CBS_CFS:
                aic325x->master = 0;
		iface_reg1 &= ~(BIT_CLK_MASTER | WORD_CLK_MASTER);
                break;
	case SND_SOC_DAIFMT_CBS_CFM:
		aic325x->master = 0;
		iface_reg1 |= BIT_CLK_MASTER;
		iface_reg1 &= ~(WORD_CLK_MASTER);
		break;
        default:
                printk("Invalid DAI master/slave interface\n");
                return -EINVAL;
        }

        /* interface format */
        switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
        case SND_SOC_DAIFMT_I2S:
                break;
        case SND_SOC_DAIFMT_DSP_A:
                iface_reg |= (AIC325x_DSP_MODE << CLK_REG_3_SHIFT);
                break;
        case SND_SOC_DAIFMT_RIGHT_J:
                iface_reg |= (AIC325x_RIGHT_JUSTIFIED_MODE << CLK_REG_3_SHIFT);
                break;
        case SND_SOC_DAIFMT_LEFT_J:
                iface_reg |= (AIC325x_LEFT_JUSTIFIED_MODE << CLK_REG_3_SHIFT);
                break;
        default:
                printk("Invalid DAI interface format\n");
                return -EINVAL;
        }

	dprintk( KERN_INFO "Function : %s, fmt = %d iface_reg = %x\n", __FUNCTION__, fmt, iface_reg);
        snd_soc_update_bits(codec, INTERFACE_SET_REG_1, 0xff, iface_reg);
        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_set_bias_level
 * Purpose  : This function is to get triggered when dapm events occurs.
 *----------------------------------------------------------------------------
 */
static int aic325x_set_bias_level(struct snd_soc_codec *codec,
                                  enum snd_soc_bias_level level)
{
        u8 value ;
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   

	dprintk( KERN_INFO "Function : %s \n", __FUNCTION__);
        switch (level) {
                /* full On */
        case SND_SOC_BIAS_ON:

                dprintk( KERN_INFO "Function : %s: BIAS ON \n", __FUNCTION__);
                /* all power is driven by DAPM system */
                break;

                /* partial On */
        case SND_SOC_BIAS_PREPARE:
                dprintk( KERN_INFO "Function : %s: BIAS PREPARE \n", __FUNCTION__);
		if(codec->bias_level == SND_SOC_BIAS_STANDBY)
		{
	                if (aic325x->master) {
                        /* switch on PLL */
//                        value = snd_soc_read(codec, CLK_REG_2);
 //                       snd_soc_write(codec, CLK_REG_2, (value | ENABLE_PLL));
			snd_soc_update_bits(codec, CLK_REG_2, ENABLE_CLK_MASK, ENABLE_PLL);
			mdelay(10); 

                        /* switch on NDAC Divider */
//                        value = snd_soc_read(codec, NDAC_CLK_REG_6);
 //                       snd_soc_write(codec, NDAC_CLK_REG_6,
  //                                    value | ENABLE_NDAC);
			snd_soc_update_bits(codec, NDAC_CLK_REG_6, ENABLE_CLK_MASK, ENABLE_NDAC);

                        /* switch on MDAC Divider */
//                        value = snd_soc_read(codec, MDAC_CLK_REG_7);
  //                      snd_soc_write(codec, MDAC_CLK_REG_7,
    //                                  value | ENABLE_MDAC);
			snd_soc_update_bits(codec, MDAC_CLK_REG_7, ENABLE_CLK_MASK, ENABLE_MDAC);
#ifndef AIC3253_CODEC_SUPPORT

                        /* Dont switch on NADC Divider */
//                        value = snd_soc_read(codec, NADC_CLK_REG_8);
  //                      snd_soc_write(codec, NADC_CLK_REG_8,
    //                                  value | ENABLE_MDAC);
			snd_soc_update_bits(codec, NADC_CLK_REG_8, ENABLE_CLK_MASK, ENABLE_NADC);

                        /* switch on MADC Divider */
//                        value = snd_soc_read(codec, MADC_CLK_REG_9);
  //                      snd_soc_write(codec, MADC_CLK_REG_9,
    //                                  value | ENABLE_MDAC);
			snd_soc_update_bits(codec, MADC_CLK_REG_9, ENABLE_CLK_MASK, ENABLE_MADC);

#endif
                        /* switch on BCLK_N Divider */
//                        value = snd_soc_read(codec, CLK_REG_11);
  //                      snd_soc_write(codec, CLK_REG_11, value | ENABLE_BCLK);
			snd_soc_update_bits(codec, CLK_REG_11, ENABLE_CLK_MASK, ENABLE_BCLK);
                	}
			
//			snd_soc_update_bits(codec, INPUT_CFG_REG, WEAK_BIAS_INPUTS_MASK, 0xc0); 
			snd_soc_update_bits(codec, AIS_REG_3, BCLK_WCLK_BUFFER_POWER_CONTROL_MASK, BCLK_WCLK_BUFFER_ON);
			
		}
                break;

                /* Off, with power */
        case SND_SOC_BIAS_STANDBY:
                dprintk( KERN_INFO "Function : %s: BIAS STANDBY\n", __FUNCTION__);
                /*
                 * all power is driven by DAPM system,
                 * so output power is safe if bypass was set
                 */
		if(codec->bias_level == SND_SOC_BIAS_OFF)// Coming out of sleep/suspend
		{
//			value = snd_soc_read(codec, POW_CFG);
//			value |= WEAK_AVDD_TO_DVDD_DIS;
//			snd_soc_write(codec, POW_CFG, value);
			snd_soc_update_bits(codec, POW_CFG, AVDD_CONNECTED_TO_DVDD_MASK, DISABLE_AVDD_TO_DVDD);
	
#ifdef AIC3256_SUPPORT_CODEC
			snd_soc_update_bits(codec, PWR_CTRL_REG, ANALOG_BLOCK_POWER_CONTROL_MASK, ENABLE_ANALOG_BLOCK);
#else
			snd_soc_update_bits(codec, LDO_CTRL, ANALOG_BLOCK_POWER_CONTROL_MASK, ENABLE_ANALOG_BLOCK);
#endif
		}
		else // coming after bias on
		{
			snd_soc_update_bits(codec, REF_PWR_UP_CONF_REG, REF_PWR_UP_MASK, FORCED_REF_PWR_UP);
//			snd_soc_update_bits(codec, INPUT_CFG_REG, WEAK_BIAS_INPUTS_MASK, 0); 
			snd_soc_update_bits(codec, AIS_REG_3, BCLK_WCLK_BUFFER_POWER_CONTROL_MASK, 0);
			
		}
                if (aic325x->master) {
                      
                        /* switch off NDAC Divider */
//                        value = snd_soc_read(codec, NDAC_CLK_REG_6);
 //                       snd_soc_write(codec, NDAC_CLK_REG_6,
  //                                    value & ~ENABLE_NDAC);
			snd_soc_update_bits(codec, NDAC_CLK_REG_6, ENABLE_CLK_MASK, 0x0);

                        /* switch off MDAC Divider */
//                        value = snd_soc_read(codec, MDAC_CLK_REG_7);
  //                    snd_soc_write(codec, MDAC_CLK_REG_7,
    //                                  value & ~ENABLE_MDAC);
			snd_soc_update_bits(codec, MDAC_CLK_REG_7, ENABLE_CLK_MASK, 0x0);
#ifndef AIC3253_CODEC_SUPPORT

                        /* Dont switch off NADC Divider */
//                      value = snd_soc_read(codec, NADC_CLK_REG_8);
  //                      snd_soc_write(codec, NADC_CLK_REG_8,
    //                                  value & ~ENABLE_NDAC);
			snd_soc_update_bits(codec, NADC_CLK_REG_8, ENABLE_CLK_MASK, 0x0);

                        /* switch off MADC Divider */
//                        value = snd_soc_read(codec, MADC_CLK_REG_9);
  //                      snd_soc_write(codec, MADC_CLK_REG_9,
    //                                  value & ~ENABLE_MDAC);
			snd_soc_update_bits(codec, MADC_CLK_REG_9, ENABLE_CLK_MASK, 0x0);

#endif

                        /* switch off BCLK_N Divider */
//                        value = snd_soc_read(codec, CLK_REG_11);
  //                      snd_soc_write(codec, CLK_REG_11, value & ~ENABLE_BCLK);
			snd_soc_update_bits(codec, CLK_REG_11, ENABLE_CLK_MASK, 0x0);

			  /* switch off PLL */
//                        value = snd_soc_read(codec, CLK_REG_2);
  //                      snd_soc_write(codec, CLK_REG_2, (value & ~ENABLE_PLL));
			snd_soc_update_bits(codec, CLK_REG_2, ENABLE_CLK_MASK, 0x0);

                }
                break;

                /* Off, without power */
        case SND_SOC_BIAS_OFF:
                dprintk( KERN_INFO "Function : %s: BIAS OFF \n", __FUNCTION__);
		snd_soc_update_bits(codec, REF_PWR_UP_CONF_REG, REF_PWR_UP_MASK, AUTO_REF_PWR_UP);
#ifdef AIC3256_SUPPORT_CODEC
		snd_soc_update_bits(codec, PWR_CTRL_REG, ANALOG_BLOCK_POWER_CONTROL_MASK, DISABLE_ANALOG_BLOCK);
#else
		snd_soc_update_bits(codec, LDO_CTRL, ANALOG_BLOCK_POWER_CONTROL_MASK, DISABLE_ANALOG_BLOCK);
#endif
		snd_soc_update_bits(codec, POW_CFG, AVDD_CONNECTED_TO_DVDD_MASK, ENABLE_AVDD_TO_DVDD);
//		value = snd_soc_read(codec, POW_CFG);
//		value &= ~WEAK_AVDD_TO_DVDD_DIS;
//		snd_soc_write(codec, POW_CFG, value);
		
                /* force all power off */
                break;
        }
        codec->bias_level = level;
        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_suspend
 * Purpose  : This function is to suspend the AIC325x driver.
 *----------------------------------------------------------------------------
 */
static int aic325x_suspend(struct snd_soc_codec *codec, pm_message_t state)
{

	dprintk( KERN_INFO "Function : %s \n", __FUNCTION__);
        aic325x_set_bias_level(codec, SND_SOC_BIAS_OFF);

        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_resume
 * Purpose  : This function is to resume the AIC325x driver
 *----------------------------------------------------------------------------
 */
static int aic325x_resume(struct snd_soc_codec *codec)
{
        int i;
        u8 data[2];
        u8 *cache = codec->reg_cache;

	dprintk( KERN_INFO "Function : %s \n", __FUNCTION__);
        aic325x_change_page(codec, 0);
        /* Sync reg_cache with the hardware */
        for (i = 0; i < ARRAY_SIZE(aic325x_reg); i++) {
                data[0] = i % 128;
                data[1] = cache[i];
                codec->hw_write(codec->control_data, data, 2);
        }
        aic325x_change_page(codec, 0);
        aic325x_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

        return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_init
 * Purpose  : This function is to initialise the AIC325x driver
 *            register the mixer and dsp interfaces with the kernel.
 *
 *----------------------------------------------------------------------------
 */
static int tlv320aic325x_init(struct snd_soc_codec *codec)
{
        int ret = 0;
        int i = 0;
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   
        aic325x->page_no = 0;

	dprintk( KERN_INFO "Function : %s \n", __FUNCTION__);
	/* codec register initalization*/
        for (i = 0;
             i < sizeof(aic325x_reg_init) / sizeof(struct aic325x_configs);
             i++) {
                snd_soc_write(codec, aic325x_reg_init[i].reg_offset,
                              aic325x_reg_init[i].reg_val);
        }

        /* off, with power on */
        aic325x_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	snd_soc_add_controls(codec, aic325x_snd_controls1,
                             ARRAY_SIZE(aic325x_snd_controls1));
	snd_soc_add_controls(codec, aic325x_snd_controls2,
                             ARRAY_SIZE(aic325x_snd_controls2));
	snd_soc_add_controls(codec, aic325x_snd_controls3,
                             ARRAY_SIZE(aic325x_snd_controls3));
	snd_soc_add_controls(codec, aic325x_snd_controls4,
                             ARRAY_SIZE(aic325x_snd_controls4));
        aic325x_add_widgets(codec);

#ifdef CONFIG_MINI_DSP
        aic325x_add_minidsp_controls(codec);
#endif

#ifdef AIC325x_TiLoad
        aic325x_driver_init(codec);
#endif

        return ret;

}


/*
 *----------------------------------------------------------------------------
 * Function : aic325x_hw_read
 * Purpose  : i2c read function
 *----------------------------------------------------------------------------
 */
unsigned int
aic325x_hw_read (struct snd_soc_codec *codec, unsigned int count)
{
        struct i2c_client *client = codec->control_data;
        unsigned int buf;

        if (count > sizeof (unsigned int)) {
                return 0;
        }
        i2c_master_recv(client, (char *)&buf, count);
        return buf;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_probe
 * Purpose  : This is first driver function called by the SoC core driver.
 *
 *----------------------------------------------------------------------------
 */
static int
aic325x_probe(struct snd_soc_codec *codec)
{
        int ret = 0;
	struct i2c_adapter *adapter;    
        struct aic325x_priv *aic325x = snd_soc_codec_get_drvdata(codec);   

/*	ret = snd_soc_codec_set_cache_io(codec, 8, 8, SND_SOC_I2C);
	if(ret != 0)
	{
		dev_err(codec->dev, "Failed to set cache I/O\n");
	}*/
        
        printk("in aic325x_probe....\n"); 

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)           
        codec->hw_write = (hw_write_t) i2c_master_send;
       codec->hw_read = aic325x_hw_read;
	codec->control_data = aic325x->control_data; 
	
	

#else
        /* Add other interfaces here */
#endif
	tlv320aic325x_init(codec);

#ifdef CONFIG_MINI_DSP
        /* Program MINI DSP for ADC and DAC */
        aic325x_minidsp_program(codec);
        aic325x_change_page(codec, 0x0);
#endif
	return ret;
	
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_remove
 * Purpose  : to remove aic325x soc device
 *
 *----------------------------------------------------------------------------
 */
static int
aic325x_remove(struct snd_soc_codec *codec)
{

	dprintk( KERN_INFO "Function : %s \n", __FUNCTION__);

        /* power down chip */
        if (codec->control_data)
                aic325x_set_bias_level(codec, SND_SOC_BIAS_OFF);


        return 0;
}

static struct snd_soc_codec_driver soc_codec_driver_aic325x = {                                                                                              
        .probe = aic325x_probe,                                                                                                                              
        .remove = aic325x_remove,                                                                                                                            
        .suspend = aic325x_suspend,                                                                                                                          
        .resume = aic325x_resume,                                                                                                                            
        .read = aic325x_read,                                                                                                                                
        .write = aic325x_write,                                                                                                                              
        .set_bias_level = aic325x_set_bias_level,                                                                                                            
        .reg_cache_size = ARRAY_SIZE(aic325x_reg),                                                                                                           
        .reg_word_size = sizeof(u8),                                                                                                                         
        .reg_cache_default = aic325x_reg,                                                                                                                    
};  

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)           
static int aic325x_i2c_probe(struct i2c_client *pdev, const struct i2c_device_id *id)
{
	int ret;

        struct aic325x_priv *aic325x;

	printk("aic325x_i2c_probe\n");

        printk(KERN_INFO "AIC325x Audio Codec %s\n", AIC325x_VERSION);

	aic325x = kzalloc(sizeof(struct aic325x_priv), GFP_KERNEL);
        if (aic325x == NULL) {
                return -ENOMEM;
        }

	i2c_set_clientdata(pdev, aic325x);
	aic325x->control_data = pdev;
	ret = snd_soc_register_codec(&pdev->dev,
			&soc_codec_driver_aic325x, tlv320aic325x_dai_driver,
			ARRAY_SIZE(tlv320aic325x_dai_driver));

	if(ret)
		printk(KERN_ERR "codec: %s : snd_soc_register_codec failed\n", __FUNCTION__);
	else
		printk(KERN_ALERT "codec: %s : snd_soc_register_codec success\n", __FUNCTION__);


	return ret;
}

static int aic325x_i2c_remove(struct i2c_client *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}
static const struct i2c_device_id aic3262_i2c_id[] = {
	{ "tlv320aic325x", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, aic325x_i2c_id);

static struct i2c_driver aic325x_i2c_driver = {
	.driver = {
		.name = "tlv320aic325x",
		.owner = THIS_MODULE,
	},
	.probe = aic325x_i2c_probe,
	.remove = aic325x_i2c_remove,
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
static int __init tlv320aic325x_modinit(void)
{
	int ret = 0;
	
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)           
	 ret = i2c_add_driver(&aic325x_i2c_driver);

	if (ret != 0) {
		printk(KERN_ERR "Failed to register TLV320AIC3x I2C driver: %d\n",
		       ret);
	}
#endif
#ifdef CONFIG_MINI_DSP
asdasda
#endif

	return ret;

}

module_init(tlv320aic325x_modinit);

/*
 *----------------------------------------------------------------------------
 * Function : tlv320aic3262_exit
 * Purpose  : module init function. First function to run.
 *
 *----------------------------------------------------------------------------
 */
static void __exit tlv320aic325x_exit(void)
{
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)           
		i2c_del_driver(&aic325x_i2c_driver);
#endif
}

module_exit(tlv320aic325x_exit);





MODULE_DESCRIPTION("ASoC TLV320AIC325x codec driver");
MODULE_AUTHOR("Mukund Navada <navada@ti.com> ");
MODULE_LICENSE("GPL");
