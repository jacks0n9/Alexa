/*
 * omap3evm-aic32xx_adc31xx.c - SoC audio for OMAP35xx EVM.
 *
 * Copyright (C) 2011
 *
 * Author: Sugumaran	<sugumaran.l@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
/* #include <sound/pcm_params.h> */
#include <sound/jack.h>
#include <plat/board.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/mcbsp.h>
#include <plat/board-omap3misto-id.h>

#include <mach/io.h>
#include <linux/io.h>

#include "../codecs/tlv320aic32xx.h"
/* #include "../codecs/newdriver.h" */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include "omap-pcm.h"
#include "omap-mcbsp.h"

#define MCLK 19200000

/* enable debug prints in the driver */
/* #define DEBUG */
#undef DEBUG

#ifdef DEBUG
#define dprintk(x...)	printk(x)
#else
#define dprintk(x...)
#endif

/* Pin numbers for jack interrupt and reset pins */
#define JACK_DETECT_GPIO	122
#define RESET_GPIO_3204		74
#define RESET_GPIO_3101_A	85
#define RESET_GPIO_3101_B	85
#define RESET_GPIO_3101_C	85
#define RESET_GPIO_3101_D	85
#define SEL_GPIO		71

/* Configure debounce */
#define JACK_DETECT_DEBOUNCE_MS 0x80

/* clock structures */
/* static struct clk *sys_clkout2; */
/* static struct clk *clkout2_src_ck; */
/* static struct clk *cm_96m_ck; */
static struct clk *sys_clkout1;

/* global variables for i/o operations */
static int omap3evm_jack_func = 1;
static int omap3evm_power_amp_func;
static int omap3evm_micin_a_p8_func = 1;
static int omap3evm_micin_b_p9_func = 1;
static int omap3evm_micin_c_p10_func = 1;
static int omap3evm_micin_d_p11_func = 1;
static int omap3evm_micin_e_p15_func = 1;
static int omap3evm_micin_f_p19_func = 1;
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
static int omap3evm_micin_g_all_func = 1;
#endif

static int sel_detect_active_high = 0;

static u8 work_completion_flag;

struct aic32xx_adc31xx_work {
	struct snd_soc_codec *codec;
	struct snd_soc_card *card;
	struct work_struct work;
};

static struct aic32xx_adc31xx_work private_data;
static struct platform_device *omap3evm_snd_device;

static void set_sel(int enable)
{
	dprintk(KERN_INFO "aic32x:%s Sel Enable =%d\n", __func__, enable);

	gpio_set_value(SEL_GPIO, !enable);
}

static int sel_detect(void)
{
	int detected;
	dprintk("+aic32x:%s Function\n", __func__);
	if (sel_detect_active_high)
		detected = gpio_get_value(JACK_DETECT_GPIO) ? 1 : 0;
	else
		detected = gpio_get_value(JACK_DETECT_GPIO) ? 0 : 1;
	detected = gpio_get_value(JACK_DETECT_GPIO) ? 0 : 1;
	dprintk(KERN_INFO "-aic32x:%s sel_detect=%d\n", __func__, detected);
	return detected;
}

static int report_sel(int sel)
{
	int err;
	char *envp[3], *buf;
	buf = kmalloc(32, GFP_ATOMIC);
	if (!buf) {
		printk(KERN_ERR "aic32x:%s kmalloc failed\n", __func__);
		return -ENOMEM;
	}

	if (sel)
		snprintf(buf, 32, "STATE=%d", 1);
	else
		snprintf(buf, 32, "STATE=%d", 0);

	envp[0] = "NAME=Sel";
	envp[1] = buf;
	envp[2] = NULL;
	err = kobject_uevent_env(&omap3evm_snd_device->dev.kobj,
							 KOBJ_CHANGE, envp);
	if (err < 0) {
		printk(KERN_ERR "aic32x:%s kobject_uevent_env Failed. Error=%d\n",
			   __func__, err);
	}
	kfree(buf);

	return err;
}

static void jack_detect_work(struct work_struct *work)
{
	int sel_detected;
	struct aic32xx_adc31xx_work *data;
	if (!work) {
	    printk (KERN_ERR "aic32x: %s parameter NULL.\n", __FUNCTION__);
	    return;
	}

	data = container_of(work, struct aic32xx_adc31xx_work, work);
	if (!data) {
	    printk (KERN_ERR "aic32x: %s failed to cast parameter.\n", __FUNCTION__);
	    return
	}
	dprintk("+aic32x:%s Function\n", __func__);

	sel_detected = sel_detect();
	printk(KERN_INFO "aic32x:%s SEL = %d\n",
		   __func__, sel_detected);
	aic32xx_set_sel(data->codec, sel_detected);
	report_sel(sel_detected);

	work_completion_flag = 0;
	dprintk("-aic32x:%s Function\n", __func__);
}


/* Interrupt Handler for Headset Jack */
static irqreturn_t jackdetect_interrupt_handler(int irq, void *data)
{
	struct aic32xx_adc31xx_work *private_data;
	if (!data) {
	    printk (KERN_ERR "aic32x: %s parameter NULL.\n", __FUNCTION__);
	    return IRQ_NONE;
	}
	private_data = data;

	dprintk("+aic32x:%s Function\n", __func__);
	printk(KERN_ALERT "Jack Interrupt !!!!!\n");

	if (!work_completion_flag) {
		schedule_work(&private_data->work);
		work_completion_flag = 1;
	}
	dprintk("-aic32x:%s Function\n", __func__);
	return IRQ_HANDLED;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_ext_control
 * Purpose  : This function is to enable or disable input/output pin and
 *	      its parent's or children widgets iff there is a valid audio
 *	      route and active audio stream.
 *
 *----------------------------------------------------------------------------
 */
static void omap3evm_ext_control(struct snd_soc_codec *codec)
{
	dprintk("+%s Function\n", __func__);

	if (omap3evm_jack_func)
		snd_soc_dapm_enable_pin(codec, "Headphone Jack");
	else
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");

	if (omap3evm_power_amp_func)
		snd_soc_dapm_enable_pin(codec, "Power Amp");
	else
		snd_soc_dapm_disable_pin(codec, "Power Amp");

	if (omap3evm_micin_a_p8_func)
		snd_soc_dapm_enable_pin(codec, "Mic Input_A_P8");
	else
		snd_soc_dapm_disable_pin(codec, "Mic Input_A_P8");

	if (omap3evm_micin_b_p9_func)
		snd_soc_dapm_enable_pin(codec, "Mic Input_B_P9");
	else
		snd_soc_dapm_disable_pin(codec, "Mic Input_B_P9");

	if (omap3evm_micin_c_p10_func)
		snd_soc_dapm_enable_pin(codec, "Mic Input_C_P10");
	else
		snd_soc_dapm_disable_pin(codec, "Mic Input_C_P10");

	if (omap3evm_micin_d_p11_func)
		snd_soc_dapm_enable_pin(codec, "Mic Input_D_P11");
	else
		snd_soc_dapm_disable_pin(codec, "Mic Input_D_P11");

	if (omap3evm_micin_e_p15_func)
		snd_soc_dapm_enable_pin(codec, "Mic Input_E_P15");
	else
		snd_soc_dapm_disable_pin(codec, "Mic Input_E_P15");

	if (omap3evm_micin_f_p19_func)
		snd_soc_dapm_enable_pin(codec, "Mic Input_F_P19");
	else
		snd_soc_dapm_disable_pin(codec, "Mic Input_F_P19");
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
	if (omap3evm_micin_g_all_func)
		snd_soc_dapm_enable_pin(codec, "Mic Input_G_ALL");
	else
		snd_soc_dapm_disable_pin(codec, "Mic Input_G_ALL");
#endif
	snd_soc_dapm_sync(codec);

	dprintk("-%s Function\n", __func__);
}

/*
 *--------------------------------------------------------------------------
 * Function : omap3evm_hw_params
 * Purpose  : This function is to set the hardware parameters for
 *            aic32xx_adc31xx machine driver. It sets Codec's and Cpu's Dai
 *            Configuration.
 *-------------------------------------------------------------------------
 */
static int omap3evm_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int err;

	dprintk(KERN_INFO "+Function : %s\n", __func__);

#ifdef AIC32xx_MCBSP_SLAVE
	/* Set codec DAI configuration */

	err = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_DSP_A |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);

	if (err < 0) {
		printk(KERN_ERR "Can't set codec DAI configuration\n");
		return err;
	}

	/* Set cpu DAI configuration */
	err = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_A |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (err < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration\n");
		return err;
	}

#else
	/* Set codec DAI configuration */

	err = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFM);

	if (err < 0) {
		printk(KERN_ERR "Can't set codec DAI configuration\n");
		return err;
	}

	/* Set cpu DAI configuration */

	err = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBS_CFM);
	if (err < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration\n");
		return err;
	}

	/* Set McBSP clock */
	err = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKX_EXT, 0,
				     SND_SOC_CLOCK_IN);
	if (err < 0) {
		printk(KERN_ERR "can't set cpu system clock\n");
		return err;
	}
#endif




	/* Set the codec system clock for DAC and ADC */
	err = snd_soc_dai_set_sysclk(codec_dai, 0, 19200000,
				     SND_SOC_CLOCK_IN);
	/* err = snd_soc_dai_set_sysclk(codec_dai, 0, 12000000,
	   SND_SOC_CLOCK_IN); */
	/* err = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
	   SND_SOC_CLOCK_IN); */
	dprintk(KERN_INFO "-Function : %s\n", __func__);
	return err;
}

/* SoC audio ops */
static struct snd_soc_ops omap3evm_ops = {
	.hw_params = omap3evm_hw_params,
};

static int omap3evm_bt_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	if (params_channels(params) != 1)
		return -EINVAL;

	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_A |
				  SND_SOC_DAIFMT_IB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	return 0;
}

/* BT audio ops */
static struct snd_soc_ops omap3evm_bt_ops = {
	.hw_params = omap3evm_bt_hw_params,
};

/* Widget structure for audio_map */
static const struct snd_soc_dapm_widget aic32xx_adc31xx_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_LINE("Power Amp", NULL),

	SND_SOC_DAPM_MIC("Mic Input_A_P8", NULL),
	SND_SOC_DAPM_MIC("Mic Input_B_P9", NULL),
	SND_SOC_DAPM_MIC("Mic Input_C_P10", NULL),
	SND_SOC_DAPM_MIC("Mic Input_D_P11", NULL),
	SND_SOC_DAPM_MIC("Mic Input_E_P15", NULL),
	SND_SOC_DAPM_MIC("Mic Input_F_P19", NULL),
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
	SND_SOC_DAPM_MIC("Mic Input_G_ALL", NULL),
#endif
};

/* DAPM Routing ralated array declaration */
static const struct snd_soc_dapm_route aic32xx_adc31xx_audio_routes[] = {
	/* Headphone connected to HPLOUT, HPROUT */
	{"Headphone Jack", NULL, "HPL"},
	{"Headphone Jack", NULL, "HPR"},

#if 0
	/* Power Amp connected to LOLOUT, LOROUT */
	{"Power Amp", NULL, "LOL"},
	{"Power Amp", NULL, "LOR"},
#endif

	/* Mic In connected to AIC3204 +IN1_L, -IN1_R */
	{"IN1_L", NULL, "Mic Input_A_P8"},
	{"IN1_R", NULL, "Mic Input_A_P8"},

	/*Mic In connected to AIC3204 +IN2_L, -IN2_R */
	{"IN2_L", NULL, "Mic Input_B_P9"},
	{"IN2_R", NULL, "Mic Input_B_P9"},

	/* Mic Input to ADC dapm Inputs C*/
	{"ADC_A_IN2_L", NULL, "Mic Input_C_P10"},
	{"ADC_A_IN3_L", NULL, "Mic Input_C_P10"},

	/* Mic Input to ADC dapm Inputs D*/
	{"ADC_A_IN2_R", NULL, "Mic Input_D_P11"},
	{"ADC_A_IN3_R", NULL, "Mic Input_D_P11"},

	/* Line Input to ADC dapm Inputs E*/
	{"ADC_B_IN2_L", NULL, "Mic Input_E_P15"},
	{"ADC_B_IN3_L", NULL, "Mic Input_E_P15"},

	/* Mic Input to ADC dapm Inputs F*/
	{"ADC_B_IN2_R", NULL, "Mic Input_F_P19"},
	{"ADC_B_IN3_R", NULL, "Mic Input_F_P19"},
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
	/* Mic Input to ADC dapm Inputs G*/
	{"ADC_C_IN2_L", NULL, "Mic Input_G_ALL"},
	{"ADC_C_IN3_L", NULL, "Mic Input_G_ALL"},
	{"ADC_C_IN2_R", NULL, "Mic Input_G_ALL"},
	{"ADC_C_IN3_R", NULL, "Mic Input_G_ALL"},

	{"ADC_D_IN2_L", NULL, "Mic Input_G_ALL"},
	{"ADC_D_IN3_L", NULL, "Mic Input_G_ALL"},
	{"ADC_D_IN2_R", NULL, "Mic Input_G_ALL"},
	{"ADC_D_IN3_R", NULL, "Mic Input_G_ALL"},
#endif
};



#define OMAP3EVM_AIC32xx_ADC31xx_DAPM_ROUTE_NUM \
	ARRAY_SIZE(omap3evm_aic32xx_adc31xx_audio_routes)

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_sel
 * Purpose  : This function is called to check the status of sel
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_sel(struct snd_kcontrol *kcontrol,
		  struct snd_ctl_elem_value *ucontrol)
{
	if (!ucontrol) {
	    printk (KERN_ERR "aic32x: %s parameter NULL.\n", __FUNCTION__);
	    return 1;
	}
	dprintk("+aic32x:%s Function\n", __func__);
	ucontrol->value.integer.value[0] = sel_detect();
	dprintk("-aic32x:%s SEL=%d", __func__,
			ucontrol->value.integer.value[0]);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_sel
 * Purpose  : This function is called to select sel from userspace
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_sel(struct snd_kcontrol *kcontrol,
		  struct snd_ctl_elem_value *ucontrol)
{
	if (!ucontrol) {
	    printk (KERN_ERR "aic32x: %s parameter NULL.\n", __FUNCTION__);
	    return 1;
	}
	dprintk("+aic32x:%s Sel Enable=%d\n", __func__,
		   ucontrol->value.integer.value[0]);
	set_sel((ucontrol->value.integer.value[0] == 0) ? 0 : 1);

	return 1;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_jack
 * Purpose  : This function is called to read data of jack for program
 *	      the aic32xx_adc31xx jack selection.
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_jack(struct snd_kcontrol *kcontrol,
		  struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_jack_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_jack
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_jack(struct snd_kcontrol *kcontrol,
		  struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_jack_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_jack_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_power_amp
 * Purpose  : This function is called to read data of power amp for program
 *	      the aic32xx_adc31xx jack selection. Power amp comes from line
 *	  out on the AIC3204 codec.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_power_amp(struct snd_kcontrol *kcontrol,
		       struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_power_amp_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}


/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_power_amp
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_power_amp(struct snd_kcontrol *kcontrol,
		       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_power_amp_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_power_amp_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);

	dprintk("-%s Function\n", __func__);
	return 1;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_micin_a_p8
 * Purpose  : This function is called to read data of external micin for program
 *	      the aic32xx_adc31xx external mic-in selection.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_micin_a_p8(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_micin_a_p8_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_micin_a_p8
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_micin_a_p8(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_micin_a_p8_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_micin_a_p8_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}


/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_micin_b_p9
 * Purpose  : This function is called to read data of mic B micin for program
 *	      the aic32xx_adc31xx micb mic-in selection.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_micin_b_p9(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_micin_b_p9_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_micin_b_p9
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_micin_b_p9(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_micin_b_p9_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_micin_b_p9_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_micin_c_p10
 * Purpose  : This function is called to read data of mic C micin for program
 *	      the aic32xx_adc31xx micb mic-in selection.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_micin_c_p10(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_micin_c_p10_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_micin_c_p10
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_micin_c_p10(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_micin_c_p10_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_micin_c_p10_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_micin_d_p11
 * Purpose  : This function is called to read data of mic D micin for program
 *	      the aic32xx_adc31xx micb mic-in selection.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_micin_d_p11(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_micin_d_p11_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_micin_d_p11
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_micin_d_p11(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_micin_d_p11_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_micin_d_p11_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_micin_e_p15
 * Purpose  : This function is called to read data of mic E micin for program
 *	      the aic32xx_adc31xx micb mic-in selection.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_micin_e_p15(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_micin_e_p15_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_micin_e_p15
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_micin_e_p15(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_micin_e_p15_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_micin_e_p15_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_micin_f_p19
 * Purpose  : This function is called to read data of mic F micin for program
 *	      the aic32xx_adc31xx micb mic-in selection.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_micin_f_p19(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_micin_f_p19_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_micin_f_p19
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_micin_f_p19(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_micin_f_p19_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_micin_f_p19_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_get_micin_g_all
 * Purpose  : This function is called to read data of mic F micin for program
 *	      the aic32xx_adc31xx micb mic-in selection.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_get_micin_g_all(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	dprintk("+%s Function\n", __func__);
	ucontrol->value.integer.value[0] = omap3evm_micin_g_all_func;
	dprintk("-%s Function\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_set_micin_g_all
 * Purpose  : This function is called to pass data from user/application to the
 *	      machine driver.
 *
 *----------------------------------------------------------------------------
 */
static int
omap3evm_set_micin_g_all(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	dprintk("+%s Function\n", __func__);
	if (omap3evm_micin_g_all_func == ucontrol->value.integer.value[0])
		return 0;

	omap3evm_micin_g_all_func = ucontrol->value.integer.value[0];
	omap3evm_ext_control(codec);
	dprintk("-%s Function\n", __func__);

	return 1;
}
#endif

static const char *jack_function[] = {"Off", "On"};
#if 0
static const char *linein_function_a[] = { "Off", "On" };
static const char *micin_function_a[] = { "Off", "On" };
static const char *linein_function_b[] = { "Off", "On" };
static const char *micin_function_b[] = { "Off", "On" };

/* Creates an array of the Single Ended Widgets */
static const struct soc_enum omap_enum_a[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(linein_function_a), linein_function_a),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micin_function_a), micin_function_a),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micin_function_a), micin_function_a),
};


static const struct soc_enum omap_enum_b[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(linein_function_b), linein_function_b),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micin_function_b), micin_function_b),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micin_function_b), micin_function_b),
};
#endif

/* Creates an array of the Single Ended Widgets */
static const struct soc_enum omap3evm_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(jack_function), jack_function),

#if 0
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(linein_function_a), linein_function_a),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micin_function_a), micin_function_a),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(linein_function_b), linein_function_b),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micin_function_b), micin_function_b),
#endif
};

/* Various Controls For aic32xx_adc31xx */
static const struct snd_kcontrol_new aic32xx_adc31xx_omap3evm_controls[] = {
	SOC_ENUM_EXT("Headphone Function", omap3evm_enum[0],
		     omap3evm_get_jack, omap3evm_set_jack),
	SOC_ENUM_EXT("Power-Amp Function", omap3evm_enum[1],
		     omap3evm_get_power_amp, omap3evm_set_power_amp),
	SOC_ENUM_EXT("Mic Input_A_P8", omap3evm_enum[2],
		     omap3evm_get_micin_a_p8, omap3evm_set_micin_a_p8),
	SOC_ENUM_EXT("Mic Input_B_P9", omap3evm_enum[3],
		     omap3evm_get_micin_b_p9, omap3evm_set_micin_b_p9),
	SOC_ENUM_EXT("Mic Input_C_P10", omap3evm_enum[4],
		     omap3evm_get_micin_c_p10, omap3evm_set_micin_c_p10),
	SOC_ENUM_EXT("Mic Input_D_P11", omap3evm_enum[5],
		     omap3evm_get_micin_d_p11, omap3evm_set_micin_d_p11),
	SOC_ENUM_EXT("Mic Input_E_P15", omap3evm_enum[6],
		     omap3evm_get_micin_e_p15, omap3evm_set_micin_e_p15),
	SOC_ENUM_EXT("Mic Input_F_P19", omap3evm_enum[7],
		     omap3evm_get_micin_f_p19, omap3evm_set_micin_f_p19),
#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
	SOC_ENUM_EXT("Mic Input_G_ALL", omap3evm_enum[8],
		     omap3evm_get_micin_g_all, omap3evm_set_micin_g_all),
#endif
};

static const struct snd_kcontrol_new aic32xx_adc31xx_line_control =
	SOC_ENUM_EXT("Sel Function", omap3evm_enum[0],
				omap3evm_get_sel, omap3evm_set_sel);

/*
 *------------------------------------------------------------------------
 * Function : omap3evm_aic32xx_sel_init
 * Purpose  : This function is to initialize sel
 *
 *------------------------------------------------------------------------
 */
static int omap3evm_aic32xx_sel_init(struct snd_soc_card *card)
{
	int sel, err;
	struct snd_soc_codec *play_codec;
	if (!card) {
		printk (KERN_ERR "aic32x: %s parameter NULL.\n", __FUNCTION__);
		return 1;
	}

	dprintk(KERN_INFO "+aic32x:%s Function\n", __func__);

	err = snd_ctl_add(card->snd_card,
			snd_soc_cnew(&aic32xx_adc31xx_line_control,
						 NULL, "SEL"));
	private_data.card = card;

	play_codec = card->rtd[1].codec_dai->codec;
	private_data.codec = play_codec;

	err = gpio_request(JACK_DETECT_GPIO, "AIC32xx_ADC31xx-sel irq");
	if (err < 0) {
		printk(KERN_ERR "aic32x:%s Failed to request"
			   "JACK_DETECT_GPIO=%d, Error=%d\n",
			   __func__, JACK_DETECT_GPIO, err);
		return err;
	}

	gpio_direction_input(JACK_DETECT_GPIO);
	err = gpio_set_debounce(JACK_DETECT_GPIO, JACK_DETECT_DEBOUNCE_MS);
	if (err < 0) {
		printk(KERN_ERR "aic32x:%s Failed to set debounce"
			   " JACK_DETECT_GPIO=%d, Error=%d\n",
			   __func__, JACK_DETECT_GPIO, err);
	}
	gpio_export(JACK_DETECT_GPIO, false);

	err = gpio_request(SEL_GPIO, "AIC32xx_ADC31xx-Sel");
	if (err < 0) {
		printk(KERN_ERR "aic32x:%s Failed to request"
			   " SEL_GPIO=%d, Error=%d\n",
			   __func__, SEL_GPIO, err);
		goto err_lineout;
	}

	gpio_direction_output(SEL_GPIO, true);
	gpio_export(SEL_GPIO, false);

	INIT_WORK(&private_data.work, jack_detect_work);

	err = devm_request_threaded_irq(&omap3evm_snd_device->dev,
				OMAP_GPIO_IRQ(JACK_DETECT_GPIO), NULL,
				(irq_handler_t) jackdetect_interrupt_handler,
				IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
				"aic32xx_adc31xx", &private_data);
	if (err != 0) {
		printk(KERN_ERR "aic32x:%s request_irq failed for jack detection\n",
			   __func__);
		goto err_irq;
	}

	sel = sel_detect();
	printk(KERN_INFO "aic32x:%s SEL = %d\n",
		   __func__, sel);
	aic32xx_set_sel(play_codec, sel);

	dprintk(KERN_INFO "-aic32x:%s Function\n", __func__);
	return 0;

err_irq:
	gpio_free(SEL_GPIO);

err_lineout:
	gpio_free(JACK_DETECT_GPIO);

	return err;
}

/* SoC machine DAI configuration. glues a codec and cpu together */
static struct snd_soc_dai_link omap3evm_dai[] = {
	{
		.name = "tlv320aic32xx-capture",
		.stream_name = "AIC32xx Capture",
		.cpu_dai_name = "omap-mcbsp-dai.0", /* <-- McBSP1 */
		.codec_dai_name = "tlv320aic32xx-capture",
		.platform_name = "omap-pcm-audio",
		.codec_name = "tlv320aic32xx.3-0018",
		.ops = &omap3evm_ops,
	},
	{
		.name = "tlv320aic32xx-playback",
		.stream_name = "AIC32xx Playback",
		.cpu_dai_name = "omap-mcbsp-dai.1", /* <-- McBSP2 */
		.codec_dai_name = "tlv320aic32xx-playback",
		.platform_name = "omap-pcm-audio",
		.codec_name = "tlv320aic32xx.3-0018",
		.ops = &omap3evm_ops,
	},
	{
		.name = "BT-VX SCO",
		.stream_name = "BT",
		.cpu_dai_name = NULL, /* set in omap3evm_init() */
		.codec_dai_name = "Bluetooth",
		.platform_name = "omap-pcm-audio",
		.codec_name = "dummy-codec-bt.0",
		.ops = &omap3evm_bt_ops,
	},
};

static struct snd_soc_dai_link omap3evm_p_dai[] = {
	{
		.name = "tlv320aic32xx-capture",
		.stream_name = "AIC32xx Capture",
		.cpu_dai_name = "omap-mcbsp-dai.0", /* <-- McBSP1 */
		.codec_dai_name = "tlv320aic32xx-capture",
		.platform_name = "omap-pcm-audio",
		.codec_name = "tlv320aic32xx.3-0018",
		.ops = &omap3evm_ops,
	},
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_omap3evm = {
	.name = "OMAP3EVM",
	.dai_link = omap3evm_dai,
	.num_links = ARRAY_SIZE(omap3evm_dai),
};

static struct snd_soc_card snd_soc_omap3evm_p = {
	.name = "OMAP3EVM",
	.dai_link = omap3evm_p_dai,
	.num_links = ARRAY_SIZE(omap3evm_p_dai),
};


/* Audio private data */
/*static struct aic32xx_adc31xx_setup_data omap3evm_aic32xx_adc31xx_setup = {
	.i2c_address = 0x18,	/ * i2c address for tlv320aic32xx_adc31xx * /
};*/

/*
 *----------------------------------------------------------------------------
 * Function : omap3evm_init
 * Purpose  : This function is to initialize the aic32xx_adc31xx Machine driver.
 *	      The functions resets the chip and add the platform device.
 *
 *----------------------------------------------------------------------------
 */
static int __init omap3evm_init(void)
{
	int ret = 0;
	struct device *dev;
	struct snd_soc_dai_link *bt_dai;
	struct snd_soc_card *card;
	bool set_sel = false;

	void *phymux_bus;
	volatile int reg;

	dprintk(KERN_INFO "+aic32x:%s Function\n", __func__);

	printk(KERN_INFO "MACHINE DRIVER : omap3evm_init function....\n");
	/* converting the 6 pin configuration of mcbsp1 to a 4 pin
	 * configuration.  shorting the FSx to FSr and clkx to clkr */
	phymux_bus = ioremap(0x48002270, 100);
	reg = __raw_readl(phymux_bus+0x4);
	reg = reg | 0x00000018;
	__raw_writel(reg, phymux_bus + 0x4);


	dprintk("aic32x: about to toggle the GPIO's in %s\n", __func__);


	/* Software Reset for AIC3204,AIC3201_A and AIC3201_B as we
	 * dont external reset
	 */

	gpio_request(RESET_GPIO_3204, "aic32xx_adc31xxh-reset");

	gpio_direction_output(RESET_GPIO_3204, 1);
	/* 1uSec RESET pulse needed */
	gpio_set_value(RESET_GPIO_3204, 1);
	udelay(1);
	gpio_set_value(RESET_GPIO_3204, 0);
	udelay(1);
	gpio_set_value(RESET_GPIO_3204, 1);

	gpio_request(RESET_GPIO_3101_A, "adc31xx_a_reset");
	gpio_direction_output(RESET_GPIO_3101_A, 1);
	/* 1uSec RESET pulse needed */
	gpio_set_value(RESET_GPIO_3101_A, 1);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_A, 0);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_A, 1);


	gpio_request(RESET_GPIO_3101_B, "adc3101_b_reset");
	gpio_direction_output(RESET_GPIO_3101_B, 1);
	/* 1uSec RESET pulse needed */
	gpio_set_value(RESET_GPIO_3101_B, 1);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_B, 0);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_B, 1);

#ifdef CONFIG_AIC3XXX_10_CHANNELS_SUPPORT
	/* Add two more ADC 3101 */
	gpio_request(RESET_GPIO_3101_C, "adc3101_c_reset");
	gpio_direction_output(RESET_GPIO_3101_C, 1);
	/* 1uSec RESET pulse needed */
	gpio_set_value(RESET_GPIO_3101_C, 1);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_C, 0);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_C, 1);

	gpio_request(RESET_GPIO_3101_D, "adc3101_d_reset");
	gpio_direction_output(RESET_GPIO_3101_D, 1);
	/* 1uSec RESET pulse needed */
	gpio_set_value(RESET_GPIO_3101_D, 1);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_D, 0);
	udelay(1);
	gpio_set_value(RESET_GPIO_3101_D, 1);
#endif

	/* set the serial connection for bluetooth */
	bt_dai = &omap3evm_dai[2];
	if (0 != strcmp(bt_dai->stream_name, "BT")) {
		printk(KERN_ERR "audio board layout changed, can't find BT DAI\n");
		return -ENODEV;
	}

	switch (get_omap3_misto_rev()) {
	case MISTO_BOARD_EVT_2_0:
	case MISTO_BOARD_EVT_2_1:
		card = &snd_soc_omap3evm;
		bt_dai->cpu_dai_name = "omap-mcbsp-dai.4"; /* McBSP5 */
		break;
	case MISTO_BOARD_P_DVT_1_0:
		sel_detect_active_high = 1;
		/* Fall through */
	case MISTO_BOARD_P_PROTO_3_0:
	case MISTO_BOARD_P_EVT_1_0:
		set_sel = true;
	case MISTO_BOARD_EVT_3_0:
	case MISTO_BOARD_EVT_3_1:
	case MISTO_BOARD_EVT_3_2:
		card = &snd_soc_omap3evm;
		bt_dai->cpu_dai_name = "omap-mcbsp-dai.2"; /* McBSP3 */
		break;
	case MISTO_BOARD_P_PROTO_1_0:
	case MISTO_BOARD_P_PROTO_2_0:
		card = &snd_soc_omap3evm_p;
		break;
	default:
		printk(KERN_ERR "unsupported audio board ID\n");
		return -ENODEV;
	}

	omap3evm_snd_device = platform_device_alloc("soc-audio", -1);
	if (!omap3evm_snd_device) {
		printk(KERN_ERR "aic32x:%s Cant allocate platform device memory\n",
			   __func__);
		return -ENOMEM;
	}

	platform_set_drvdata(omap3evm_snd_device, card);

	ret = platform_device_add(omap3evm_snd_device);

	if (ret) {
		printk(KERN_ERR "aic32x:%s Unable to add platform device\n",
			   __func__);
		goto err_dev;
	}

	if (set_sel) {
		ret = omap3evm_aic32xx_sel_init(card);
		if (ret) /* Error info is logged by previous called function */
			goto err_dev;
	}

	dev = &omap3evm_snd_device->dev;

#if 0
	sys_clkout2 = clk_get(dev, "sys_clkout2");
	if (IS_ERR(sys_clkout2)) {
		printk(KERN_ERR "Could not get sys_clkout2\n");
		return -1;
	}

	sys_clkout2 = clk_get(dev, "sys_clkout2");
	if (IS_ERR(sys_clkout2)) {
		printk(KERN_ERR "Could not get sys_clkout2\n");
		return -1;
	}

	/*
	 * Configure 12 MHz output on SYS_CLKOUT2. Therefore we must use
	 * 96 MHz as its parent in order to get 12 MHz
	 */
	cm_96m_ck = clk_get(dev, "cm_96m_fck");
	if (IS_ERR(cm_96m_ck)) {
		printk(KERN_ERR "Could not get func 96M clock\n");
		ret = -1;
		goto err1;
	}
	clk_set_parent(clkout2_src_ck, cm_96m_ck);
	/* clk_set_rate(sys_clkout2, 12000000); */
	clk_set_rate(sys_clkout2, 19200000);


	omap_writel(0x1a, 0x48004d70);

	clk_enable(sys_clkout2);
#endif
	sys_clkout1 = clk_get(dev, "sys_clkout1");
	if (IS_ERR(sys_clkout1)) {
		printk(KERN_ERR "Could not get sys_clkout1\n");
		return -1;
	}
	clk_enable(sys_clkout1);

	dprintk(KERN_INFO "-aic32x:%s: Function\n", __func__);

	return 0;
#if 0
err1:
	clk_put(sys_clkout2);

	dprintk(KERN_INFO "-Function : %s\n", __func__);
	return ret;
#endif

err_dev:
	platform_device_put(omap3evm_snd_device);
	return -ENODEV;
}

static void __exit omap3evm_exit(void)
{
	printk(KERN_INFO "Function: %s\n", __func__);

	if (get_omap3_misto_rev() & MISTO_BOARD_IS_P) {
		gpio_free(SEL_GPIO);
		gpio_free(JACK_DETECT_GPIO);
	}
	platform_device_unregister(omap3evm_snd_device);
}

module_init(omap3evm_init);
module_exit(omap3evm_exit);

MODULE_AUTHOR("Sugumaran <sugumaran.l@ti.com>");
MODULE_DESCRIPTION("ALSA SoC OMAP35xx EVM");
MODULE_LICENSE("GPL");
