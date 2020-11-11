/*
 * linux/sound/soc/codecs/tlv320aic325x_mini-dsp.c 
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
 *
 * Rev 0.1 	 mini DSP support    		 4-05-2009
 *
 *          The mini DSP programming support is added to codec AIC325x.
 *
 *
 * Rev 1.0 	 mini DSP mixer controls 	         11-05-2009
 *
 *         	 Added mixer controls for aic325x codec, and code cleanup
 * 
 * Rev 2.0      Two miniDSP modes                        20-01-2011
 *              Added the miniDSP support for two miniDSP modes.
 *
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/control.h>
#include <linux/time.h>		/* For timing computations */

#include "tlv320aic325x.h"
#include "tlv320aic325x_mini-dsp.h"

/* enable debug prints in the driver */
#define DEBUG 1
//#undef DEBUG

#ifdef DEBUG
#define dprintk(x...) 	printk(x)
#else
#define dprintk(x...)
#endif

#ifdef CONFIG_MINI_DSP

//#define VERIFY_MINIDSP                                1

/* Function prototypes */
#ifdef REG_DUMP_MINIDSP
static void aic325x_dump_page(struct i2c_client *i2c, u8 page);
#endif

/* externs */
extern int aic325x_read (struct snd_soc_codec *codec, u16 reg);
extern u16 aic325x_read_2byte (struct snd_soc_codec * codec, u16 reg);
extern int aic325x_reset_cache (struct snd_soc_codec *codec);
extern int aic325x_change_page(struct snd_soc_codec *codec, u8 new_page);
extern int aic325x_write(struct snd_soc_codec *codec, u16 reg, u8 value);
extern void aic325x_write_reg_cache(struct snd_soc_codec *codec,
                                    u16 reg, u8 value);
/* Added the minidsp_parser_data type global variables which will
 * parse the miniDSP Header files and keep the burst miniDSP write
 * configuration prepared even before the user wishes to switch from one
 * mode to another.
 */
minidsp_parser_data dsp_parse_data[MINIDSP_PARSER_ARRAY_SIZE*2];

/* For switching between Multiple miniDSP Modes, it is required
 * to update the Codec Registers across Several Pages. The information
 * on the Pages and Register Offsets with values to be written will be
 * stored in the above minidsp_parser_data Arrays. However, for
 * communicating them to the Codec, we still need the i2c_msg structure.
 * so instead of initializing the i2c_msg at run-time, we will also
 * populate a global array of the i2c_msg structures and keep it
 * ready
 */
struct i2c_msg i2c_transaction[MINIDSP_PARSER_ARRAY_SIZE * 2];

/* Actual Count of the above Arrays */
int second_i2c_count = 0;
int first_i2c_count = 0;

minidsp_i2c_page second_i2c_page_array[MINIDSP_PARSER_ARRAY_SIZE];
minidsp_i2c_page first_i2c_page_array[MINIDSP_PARSER_ARRAY_SIZE];

/* Actual Count of the above arrays */
int second_i2c_page_count = 0;
int first_i2c_page_count = 0;

u8 first_mode_enable;

/******************** Debug section ********************/

#ifdef REG_DUMP_MINIDSP
/*
 *----------------------------------------------------------------------------
 * Function : aic325x_dump_page
 * Purpose  : Read and display one codec register page, for debugging purpose
 *----------------------------------------------------------------------------
 */
static void aic325x_dump_page(struct i2c_client *i2c, u8 page)
{
	int i;
	u8 data;
	u8 test_page_array[256];

	aic325x_change_page(codec, page);

	data = 0x0;

	i2c_master_send(i2c, data, 1);
	i2c_master_recv(i2c, test_page_array, 128);

	dprintk("\n------- MINI_DSP PAGE %d DUMP --------\n", page);
	for (i = 0; i < 128; i++) {
		dprintk(" [ %d ] = 0x%x\n", i, test_page_array[i]);
	}
}
#endif

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_minidsp_get_first_burst
 * Purpose  : Format one I2C burst for transfer from mini dsp program array.
 * 	      This function will parse the program array and get next burst
 *            data for doing an I2C bulk transfer for first mode ie first mode.
 *----------------------------------------------------------------------------
 */
static void
aic325x_minidsp_get_first_burst(first_reg_value * program_ptr, int program_size,
                                minidsp_parser_data * parse_data)
{
	int index = parse_data->current_loc;
	int burst_write_count = 0;

	/* check if first location is page register, and populate page addr */
	if (program_ptr[index].reg_off == 0) {
		parse_data->page_num = program_ptr[index].reg_val;
		parse_data->burst_array[burst_write_count++] =
                        program_ptr[index].reg_off;
		parse_data->burst_array[burst_write_count++] =
                        program_ptr[index].reg_val;
		index++;
		goto finish_out;
	}

	parse_data->burst_array[burst_write_count++] =
                program_ptr[index].reg_off;
	parse_data->burst_array[burst_write_count++] =
                program_ptr[index].reg_val;
	index++;

	for (; index < program_size; index++) {
		if (program_ptr[index].reg_off !=
		    (program_ptr[index - 1].reg_off + 1))
			break;
		else {
			parse_data->burst_array[burst_write_count++] =
                                program_ptr[index].reg_val;
		}
	}

finish_out :
        parse_data->burst_size = burst_write_count;

        if (index == program_size) {
                /* parsing completed */
                parse_data->current_loc = MINIDSP_PARSING_END;
        } else
                parse_data->current_loc = index;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_minidsp_get_second_burst
 * Purpose  : Format one I2C burst for transfer from mini dsp program array.
 *            This function will parse the program array and get next burst
 *            data for doing an I2C bulk transfer for second mode ie second mode
 *----------------------------------------------------------------------------
 */
static void
aic325x_minidsp_get_second_burst (second_reg_value * program_ptr,
                                  int program_size,
                                  minidsp_parser_data * parse_data)
{
        int index = parse_data->current_loc;
        int burst_write_count = 0;

        /* check if first location is page register, and populate page addr */
        if (program_ptr[index].reg_off == 0)
        {
                parse_data->page_num = program_ptr[index].reg_val;
                parse_data->burst_array[burst_write_count++] =
                        program_ptr[index].reg_off;
                parse_data->burst_array[burst_write_count++] =
                        program_ptr[index].reg_val;
                index++;
                goto finish_out;
        }

        parse_data->burst_array[burst_write_count++] = program_ptr[index].reg_off;
        parse_data->burst_array[burst_write_count++] = program_ptr[index].reg_val;
        index++;

        for (; index < program_size; index++)
        {
                if (program_ptr[index].reg_off != (program_ptr[index - 1].reg_off + 1))
                        break;
                else
                {
                        parse_data->burst_array[burst_write_count++] =
                                program_ptr[index].reg_val;
                }
        }
finish_out:
        parse_data->burst_size = burst_write_count;
        if (index == program_size)
        {
                /* parsing completed */
                parse_data->current_loc = MINIDSP_PARSING_END;
        }
        else
                parse_data->current_loc = index;
}
/*
 *----------------------------------------------------------------------------
 * Function : aic325x_minidsp_burst_write_second_program
 * Purpose  : Configures the AIC325x register map as per the PPS generated
 *            second mode ie for second Header file settings.
 *	      It returns 0 on success and -1 on failure
 *----------------------------------------------------------------------------
 */
static int
aic325x_minidsp_burst_write_second_program (struct snd_soc_codec *codec,
                                            second_reg_value * program_ptr, int program_size)
{
	struct i2c_client *client = codec->control_data;

	minidsp_parser_data parse_data;
	int count=0;

	/* point the current location to start of program array */
	parse_data.current_loc = 0;
	parse_data.page_num = 0;
	do
	{
		/* Get first burst data */
		aic325x_minidsp_get_second_burst (program_ptr, program_size, &parse_data);
		dsp_parse_data[count] = parse_data;
		dprintk ("Burst,PAGE=0x%x Size=%d\n", parse_data.page_num,
                         parse_data.burst_size);

		i2c_transaction[count].addr = client->addr;
		i2c_transaction[count].flags = client->flags & I2C_M_TEN;
		i2c_transaction[count].len = dsp_parse_data[count].burst_size;
		i2c_transaction[count].buf = dsp_parse_data[count].burst_array;

		count++;
		/* Proceed to the next burst reg_addr_incruence */
	}
	while (parse_data.current_loc != MINIDSP_PARSING_END);
	if(count>0)
	{
		if(i2c_transfer(client->adapter, i2c_transaction, count) != count)
		{
			dprintk ("Write burst i2c data error!\n");
		}
		//printk("%s: transfer count=%d\n", __func__, count);
	}
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_minidsp_burst_write_second_program
 * Purpose  : Configures the AIC325x register map as per the PPS generated
 *            first mode ie first Header file settings.
 *	      It returns 0 on success and -1 on failure
 *----------------------------------------------------------------------------
 */
static int
aic325x_minidsp_burst_write_first_program (struct snd_soc_codec *codec,
                                           first_reg_value * program_ptr, int program_size)
{
	struct i2c_client *client = codec->control_data;

	minidsp_parser_data parse_data;
	int count=0;

	/* point the current location to start of program array */
	parse_data.current_loc = 0;
	parse_data.page_num = 0;

	do
	{
		/* Get first burst data */
		aic325x_minidsp_get_first_burst (program_ptr, program_size, &parse_data);
		dsp_parse_data[count] = parse_data;
		dprintk ("Burst,PAGE=0x%x Size=%d\n", parse_data.page_num,
                         parse_data.burst_size);

		i2c_transaction[count].addr = client->addr;
		i2c_transaction[count].flags = client->flags & I2C_M_TEN;
		i2c_transaction[count].len = dsp_parse_data[count].burst_size;
		i2c_transaction[count].buf = dsp_parse_data[count].burst_array;

		count++;
		/* Proceed to the next burst reg_addr_incruence */
	}
	while (parse_data.current_loc != MINIDSP_PARSING_END);
	if(count>0)
	{
		if(i2c_transfer(client->adapter, i2c_transaction, count) != count)
		{
			dprintk ("Write burst i2c data error!\n");
		}
		//printk("%s: transfer count=%d\n", __func__, count);
	}
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : set_minidsp_first_mode
 * Purpose  : Switch to the first minidsp mode.
 *----------------------------------------------------------------------------
 */
static int
set_minidsp_first_mode(struct snd_soc_codec *codec)
{
#if defined(EN_REG_CACHE)
	int i=0;
	int page=0;
	int reg = 0;
#endif

	printk("%s: switch first mode start\n", __func__);

	aic325x_reset_cache(codec);
	/* Array size should be greater than 1 to start programming,
         *	*		   * since first write command will be the page register
         *	   *				   */
	if (ARRAY_SIZE (REG_first_Section_init_program) > 1)
	{
		aic325x_minidsp_burst_write_first_program (codec, REG_first_Section_init_program,
                                                           ARRAY_SIZE
                                                           (REG_first_Section_init_program));
#if defined(EN_REG_CACHE)
                for(i=0; i<ARRAY_SIZE(REG_first_Section_init_program); i++)
                {
                        if(0 == REG_first_Section_init_program[i].reg_off)
                        {
                                page = REG_first_Section_init_program[i].reg_val;
                                continue;
                        }
                        reg = page*128 + REG_first_Section_init_program[i].reg_off;
                        aic325x_write_reg_cache (codec, reg, REG_first_Section_init_program[i].reg_val);
                }
#endif

	}
	else
	{
		dprintk ("FIRST/second_CODEC_REGS: Insufficient data for programming\n");
	}
	if (ARRAY_SIZE (miniDSP_A_first_reg_values) > 1)
	{
		aic325x_minidsp_burst_write_first_program(codec, miniDSP_A_first_reg_values,
                                                          ARRAY_SIZE (miniDSP_A_first_reg_values));
	}
	else
	{
		dprintk ("MINI_DSP_A_second: Insufficient data for programming\n");
	}

	if (ARRAY_SIZE (miniDSP_D_first_reg_values) > 1)
	{
		aic325x_minidsp_burst_write_first_program(codec, miniDSP_D_first_reg_values,
                                                          ARRAY_SIZE (miniDSP_D_first_reg_values));
	}
	else
	{
		dprintk ("MINI_DSP_D_second: Insufficient data for programming\n");
	}

	if (ARRAY_SIZE (REG_first_Section_post_program) > 1)
	{
		aic325x_minidsp_burst_write_first_program (codec, REG_first_Section_post_program,
                                                           ARRAY_SIZE
                                                           (REG_first_Section_post_program));
#if defined(EN_REG_CACHE)
                for(i=0; i<ARRAY_SIZE(REG_first_Section_post_program); i++)
                {
                        if(0 == REG_first_Section_post_program[i].reg_off)
                        {
                                page = REG_first_Section_post_program[i].reg_val;
                                continue;
                        }
                        reg = page*128 + REG_first_Section_post_program[i].reg_off;
                        aic325x_write_reg_cache (codec, reg, REG_first_Section_post_program[i].reg_val);
                }
#endif
	}
	else
	{
		dprintk ("second_CODEC_REGS: Insufficient data for programming\n");
	}
	dprintk("%s: switch first mode finished\n", __func__);
	return 0;

}

/*
 *----------------------------------------------------------------------------
 * Function : set_minidsp_second_mode
 * Purpose  : Switch to the first minidsp mode.
 *----------------------------------------------------------------------------
 */
int
set_minidsp_second_mode(struct snd_soc_codec *codec)
{
#if defined(EN_REG_CACHE)
	int i=0;
	int page=0;
	int reg = 0;
#endif
	dprintk("%s: switch second mode start\n", __func__);

	aic325x_reset_cache(codec);
	if (ARRAY_SIZE (REG_second_Section_program) > 1)
	{
		aic325x_minidsp_burst_write_second_program(codec, REG_second_Section_init_program,
                                                           ARRAY_SIZE(REG_second_Section_init_program));
#if defined(EN_REG_CACHE)
                for(i=0; i<ARRAY_SIZE(REG_second_Section_init_program); i++)
                {
                        if(0 == REG_second_Section_init_program[i].reg_off)
                        {
                                page = REG_second_Section_init_program[i].reg_val;
                                continue;
                        }
                        reg = page*128 + REG_second_Section_init_program[i].reg_off;
                        aic325x_write_reg_cache (codec, reg, REG_second_Section_init_program[i].reg_val);
                }
#endif
	}
	else
	{
		dprintk ("second_CODEC_REGS: Insufficient data for programming\n");
	}

	if (ARRAY_SIZE (miniDSP_A_second_reg_values) > 1)
	{
		aic325x_minidsp_burst_write_second_program (codec, miniDSP_A_second_reg_values,
                                                            ARRAY_SIZE (miniDSP_A_second_reg_values));
	}
	else
	{
		dprintk ("MINI_DSP_A_second: Insufficient data for programming\n");
	}

	if (ARRAY_SIZE (miniDSP_D_second_reg_values) > 1)
	{
		aic325x_minidsp_burst_write_second_program (codec, miniDSP_D_second_reg_values,
                                                            ARRAY_SIZE (miniDSP_D_second_reg_values));
	}
	else
	{
		dprintk ("MINI_DSP_D_second: Insufficient data for programming\n");
	}

	if (ARRAY_SIZE (REG_second_Section_post_program) > 1)
	{
		aic325x_minidsp_burst_write_second_program(codec, REG_second_Section_post_program,
                                                           ARRAY_SIZE(REG_second_Section_post_program));
#if defined(EN_REG_CACHE)
                for(i=0; i<ARRAY_SIZE(REG_second_Section_post_program); i++)
                {
                        if(0 == REG_second_Section_post_program[i].reg_off)
                        {
                                page = REG_second_Section_post_program[i].reg_val;
                                continue;
                        }
                        reg = page*128 + REG_second_Section_post_program[i].reg_off;
                        aic325x_write_reg_cache (codec, reg, REG_second_Section_post_program[i].reg_val);
                }
#endif
	}
	else
	{
		dprintk ("second_CODEC_REGS: Insufficient data for programming\n");
	}
	dprintk("%s: switch second mode finished\n", __func__);
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : aic325x_minidsp_program
 * Purpose  : Program mini dsp for AIC325x codec chip. This routine is
 * 			  called from the aic325x codec driver, if mini dsp programming
 * 			  is enabled.
 *----------------------------------------------------------------------------
 */
int aic325x_minidsp_program(struct snd_soc_codec *codec)
{
	int  ret;
	printk("AIC325x: programming mini dsp\n");

	printk("The register sections found in pps header file:\n");

#ifdef CODEC_REG_NAMES
        for (i = 0; i < ARRAY_SIZE (REG_second_Section_names); i++)
        {
                dprintk ("%s\n", REG_second_Section_names[i]);
        }
        for (i = 0; i < ARRAY_SIZE (REG_first_Section_names); i++)
        {
                dprintk ("%s\n", REG_first_Section_names[i]);
        }
#endif

#ifdef PROGRAM_MINI_DSP_second
	set_minidsp_second_mode(codec);
#endif
#ifdef PROGRAM_MINI_DSP_first
	set_minidsp_first_mode(codec);
#endif
	return 0;
}

/********************* AMIXER Controls for mini dsp *************************/

#ifdef ADD_MINI_DSP_CONTROLS

/* Volume Lite coefficents table */
static int volume_lite_table[] = {
	0x00000D, 0x00000E, 0x00000E, 0x00000F,
	0x000010, 0x000011, 0x000012, 0x000013,
	0x000015, 0x000016, 0x000017, 0x000018,
	0x00001A, 0x00001C, 0x00001D, 0x00001F,
	0x000021, 0x000023, 0x000025, 0x000027,
	0x000029, 0x00002C, 0x00002F, 0x000031,
	0x000034, 0x000037, 0x00003B, 0x00003E,
	0x000042, 0x000046, 0x00004A, 0x00004F,
	0x000053, 0x000058, 0x00005D, 0x000063,
	0x000069, 0x00006F, 0x000076, 0x00007D,
	0x000084, 0x00008C, 0x000094, 0x00009D,
	0x0000A6, 0x0000B0, 0x0000BB, 0x0000C6,
	0x0000D2, 0x0000DE, 0x0000EB, 0x0000F9,
	0x000108, 0x000118, 0x000128, 0x00013A,
	0x00014D, 0x000160, 0x000175, 0x00018B,
	0x0001A3, 0x0001BC, 0x0001D6, 0x0001F2,
	0x000210, 0x00022F, 0x000250, 0x000273,
	0x000298, 0x0002C0, 0x0002E9, 0x000316,
	0x000344, 0x000376, 0x0003AA, 0x0003E2,
	0x00041D, 0x00045B, 0x00049E, 0x0004E4,
	0x00052E, 0x00057C, 0x0005D0, 0x000628,
	0x000685, 0x0006E8, 0x000751, 0x0007C0,
	0x000836, 0x0008B2, 0x000936, 0x0009C2,
	0x000A56, 0x000AF3, 0x000B99, 0x000C49,
	0x000D03, 0x000DC9, 0x000E9A, 0x000F77,
	0x001062, 0x00115A, 0x001262, 0x001378,
	0x0014A0, 0x0015D9, 0x001724, 0x001883,
	0x0019F7, 0x001B81, 0x001D22, 0x001EDC,
	0x0020B0, 0x0022A0, 0x0024AD, 0x0026DA,
	0x002927, 0x002B97, 0x002E2D, 0x0030E9,
	0x0033CF, 0x0036E1, 0x003A21, 0x003D93,
	0x004139, 0x004517, 0x00492F, 0x004D85,
	0x00521D, 0x0056FA, 0x005C22, 0x006197,
	0x006760, 0x006D80, 0x0073FD, 0x007ADC,
	0x008224, 0x0089DA, 0x009205, 0x009AAC,
	0x00A3D7, 0x00B7D4, 0x00AD8C, 0x00C2B9,
	0x00CE43, 0x00DA7B, 0x00E76E, 0x00F524,
	0x0103AB, 0x01130E, 0x01235A, 0x01349D,
	0x0146E7, 0x015A46, 0x016ECA, 0x018486,
	0x019B8C, 0x01B3EE, 0x01CDC3, 0x01E920,
	0x02061B, 0x0224CE, 0x024553, 0x0267C5,
	0x028C42, 0x02B2E8, 0x02DBD8, 0x030736,
	0x033525, 0x0365CD, 0x039957, 0x03CFEE,
	0x0409C2, 0x044703, 0x0487E5, 0x04CCA0,
	0x05156D, 0x05628A, 0x05B439, 0x060ABF,
	0x066666, 0x06C77B, 0x072E50, 0x079B3D,
	0x080E9F, 0x0888D7, 0x090A4D, 0x09936E,
	0x0A24B0, 0x0ABE8D, 0x0B6188, 0x0C0E2B,
	0x0CC509, 0x0D86BD, 0x0E53EB, 0x0F2D42,
	0x101379, 0x110754, 0x1209A3, 0x131B40,
	0x143D13, 0x157012, 0x16B543, 0x180DB8,
	0x197A96, 0x1AFD13, 0x1C9676, 0x1E481C,
	0x201373, 0x21FA02, 0x23FD66, 0x261F54,
	0x28619A, 0x2AC625, 0x2D4EFB, 0x2FFE44,
	0x32D646, 0x35D96B, 0x390A41, 0x3C6B7E,
	0x400000, 0x43CAD0, 0x47CF26, 0x4C106B,
	0x50923B, 0x55586A, 0x5A6703, 0x5FC253,
	0x656EE3, 0x6B7186, 0x71CF54, 0x788DB4,
	0x7FB260,
};

/************************ VolumeLite control section ************************/

static struct snd_kcontrol_new snd_vol_first_controls[MAX_VOLUME_CONTROLS];
static struct snd_kcontrol_new snd_vol_second_controls[MAX_VOLUME_CONTROLS];

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_info_second_minidsp_volume
 * Purpose  : info routine for volumeLite amixer kcontrols
 *----------------------------------------------------------------------------
 */
static int
__new_control_info_second_minidsp_volume(struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_info *uinfo)
{
	int index;
	int ret_val = -1;

	for (index = 0; index < ARRAY_SIZE(VOLUME_second_controls); index++) {
		if (strstr(kcontrol->id.name,
                           VOLUME_second_control_names[index]))
			break;
	}

	if (index < ARRAY_SIZE(VOLUME_second_controls)) {
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = MIN_VOLUME;
		uinfo->value.integer.max = MAX_VOLUME;
		ret_val = 0;
	}
	return ret_val;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_get_second_minidsp_vol
 * Purpose  : get routine for amixer kcontrols, read current register
 * 			  values. Used for for mini dsp 'VolumeLite' amixer controls.
 *----------------------------------------------------------------------------
 */
static int
__new_control_get_second_minidsp_volume(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = kcontrol->private_value;
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_put_second_minidsp_volume
 * Purpose  : put routine for amixer kcontrols, write user values to registers
 * 			  values. Used for for mini dsp 'VolumeLite' amixer controls.
 *----------------------------------------------------------------------------
 */
static int
__new_control_put_second_minidsp_volume(struct snd_kcontrol *kcontrol,
                                        struct snd_ctl_elem_value *ucontrol)
{
	u8 data[5];
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	int index;
	int user_value = ucontrol->value.integer.value[0];
	struct i2c_client *i2c;
	int ret_val = -1;
	int coeff;
	u8 value[2];
	i2c = codec->control_data;

	dprintk("user value = 0x%x\n", user_value);

	for (index = 0; index < ARRAY_SIZE(VOLUME_second_controls); index++) {
		if (strstr(kcontrol->id.name,
                           VOLUME_second_control_names[index]))
			break;
	}

	if (index < ARRAY_SIZE(VOLUME_second_controls)) {
		aic325x_change_page(codec,
                                    VOLUME_second_controls[index].control_page);

		coeff = volume_lite_table[user_value << 1];

		data[1] = (u8) ((coeff >> 16) & AIC325x_8BITS_MASK);
		data[2] = (u8) ((coeff >> 8) & AIC325x_8BITS_MASK);
		data[3] = (u8) ((coeff) & AIC325x_8BITS_MASK);

		/* Start register address */
		data[0] = VOLUME_second_controls[index].control_base;

		ret_val = i2c_master_send(i2c, data, VOLUME_REG_SIZE + 1);

		if (ret_val != VOLUME_REG_SIZE + 1) {
			dprintk("i2c_master_send transfer failed\n");
		} else {
			/* store the current level */
			kcontrol->private_value = user_value;
			ret_val = 0;
		}

                aic325x_change_page (codec, 44);

                value[0] = 1;

                if (i2c_master_send (i2c, value, 1) != 1)
                {
                        dprintk ("Can not write register address\n");
                }

                if (i2c_master_recv (i2c, value, 1) != 1)
                {
                        dprintk ("Can not read codec registers\n");
                }

                value[1] = value[0] | 1;
                value[0] = 1;

                if (i2c_master_send (i2c, value, 2) != 2)
                {
                        dprintk ("Can not write register address\n");
                }

                aic325x_change_page (codec, VOLUME_second_controls[index].control_page);

                ret_val = i2c_master_send (i2c, data, VOLUME_REG_SIZE + 1);
                ret_val = 0;

	}

	aic325x_change_page(codec, 0);
	return (ret_val);
}

/*
 *----------------------------------------------------------------------------
 * Function : minidsp_volume_second_mixer_controls
 * Purpose  : Add amixer kcontrols for mini dsp volume Lite controls,
 *----------------------------------------------------------------------------
 */
static int
minidsp_volume_second_mixer_controls(struct snd_soc_codec *codec)
{
	int i, err, no_volume_controls;
	static char volume_control_name[MAX_VOLUME_CONTROLS][40];

	no_volume_controls = ARRAY_SIZE(VOLUME_second_controls);

	dprintk(" %d mixer controls for mini dsp 'volumeLite' \n",
		no_volume_controls);

	if (no_volume_controls) {
                for (i = 0; i < no_volume_controls; i++) {
                        strcpy(volume_control_name[i], VOLUME_second_control_names[i]);
                        strcat(volume_control_name[i], VOLUME_KCONTROL_NAME);

                        dprintk("Volume controls: %s\n", volume_control_name[i]);

                        snd_vol_second_controls[i].name = volume_control_name[i];
                        snd_vol_second_controls[i].iface = SNDRV_CTL_ELEM_IFACE_MIXER;
                        snd_vol_second_controls[i].access =			    SNDRV_CTL_ELEM_ACCESS_READWRITE;
                        snd_vol_second_controls[i].info =
                                __new_control_info_second_minidsp_volume;
                        snd_vol_second_controls[i].get =
                                __new_control_get_second_minidsp_volume;
                        snd_vol_second_controls[i].put =
                                __new_control_put_second_minidsp_volume;
		 	/*
			 *      TBD: read volume reg and update the index number
			 */
                        snd_vol_second_controls[i].private_value = 0;
                        snd_vol_second_controls[i].count = 0;

                        err = snd_ctl_add (codec->card,
                                           snd_soc_cnew (&snd_vol_second_controls[i],
                                                         codec, NULL));
                        if (err < 0)
                        {
                                printk ("%s:Invalid control %s\n", __FILE__,
                                        snd_vol_second_controls[i].name);
                        }
                }
        }
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_info_first_minidsp_volume
 * Purpose  : info routine for volumeLite amixer kcontrols
 *----------------------------------------------------------------------------
 */
static int
__new_control_info_first_minidsp_volume (struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_info *uinfo)
{
        int index;
        int ret_val = -1;

        for (index = 0; index < ARRAY_SIZE (VOLUME_first_controls); index++)
        {
                if (strstr (kcontrol->id.name, VOLUME_first_control_names[index]))
                        break;
        }

        if (index < ARRAY_SIZE (VOLUME_first_controls))
        {
                uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
                uinfo->count = 1;
                uinfo->value.integer.min = MIN_VOLUME;
                uinfo->value.integer.max = MAX_VOLUME;
                ret_val = 0;
        }
        return ret_val;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_get_first_minidsp_vol
 * Purpose  : get routine for amixer kcontrols, read current register
 * 			  values. Used for for mini dsp 'VolumeLite' amixer controls.
 *----------------------------------------------------------------------------
 */
static int
__new_control_get_first_minidsp_volume (struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
        ucontrol->value.integer.value[0] = kcontrol->private_value;
        return 0;
}
/*
 *----------------------------------------------------------------------------
 * Function : __new_control_put_first_minidsp_volume
 * Purpose  : put routine for amixer kcontrols, write user values to registers
 * 			  values. Used for for mini dsp 'VolumeLite' amixer controls.
 *----------------------------------------------------------------------------
 */
static int
__new_control_put_first_minidsp_volume (struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
        u8 data[4];
        struct snd_soc_codec *codec = snd_kcontrol_chip (kcontrol);
        int index;
        int user_value = ucontrol->value.integer.value[0];
        struct i2c_client *i2c = codec->control_data;
        int ret_val = -1;
        int coeff;
        u8 value[2];

        dprintk ("user value = 0x%x\n", user_value);

        for (index = 0; index < ARRAY_SIZE (VOLUME_first_controls); index++)
        {
                if (strstr (kcontrol->id.name, VOLUME_first_control_names[index]))
                        break;
        }

        if (index < ARRAY_SIZE (VOLUME_first_controls))
        {
                aic325x_change_page (codec, VOLUME_first_controls[index].control_page);

                coeff = volume_lite_table[user_value << 1];

                data[1] = (u8) ((coeff >> 8) & AIC325x_8BITS_MASK);
                data[2] = (u8) ((coeff) & AIC325x_8BITS_MASK);

                /* Start register address */
                data[0] = VOLUME_first_controls[index].control_base;

                ret_val = i2c_master_send (i2c, data, VOLUME_REG_SIZE + 1);

                if (ret_val != VOLUME_REG_SIZE + 1)
                {
                        dprintk ("i2c_master_send transfer failed\n");
                }
                else
                {
                        /* store the current level */
                        kcontrol->private_value = user_value;
                        ret_val = 0;
                }

                aic325x_change_page (codec, 8);
                value[0] = 1;

                if (i2c_master_send (i2c, value, 1) != 1)
                {
                        dprintk ("Can not write register address\n");
                }

                if (i2c_master_recv (i2c, value, 1) != 1)
                {
                        dprintk ("Can not read codec registers\n");
                }

                value[1] = value[0] | 1;
                value[0] = 1;

                if (i2c_master_send (i2c, value, 2) != 2)
                {
                        dprintk ("Can not write register address\n");
                }

                aic325x_change_page (codec, VOLUME_first_controls[index].control_page);

                ret_val = i2c_master_send (i2c, data, VOLUME_REG_SIZE + 1);
                ret_val = 0;

        }

        aic325x_change_page (codec, 0);
        return (ret_val);
}

/*
 *----------------------------------------------------------------------------
 * Function : minidsp_volume_first_mixer_controls
 * Purpose  : Add amixer kcontrols for mini dsp volume Lite controls,
 *----------------------------------------------------------------------------
 */
static int
minidsp_volume_first_mixer_controls (struct snd_soc_codec *codec)
{
        int i, err, no_volume_controls;
        static char volume_control_name[MAX_VOLUME_CONTROLS][40];

        no_volume_controls = ARRAY_SIZE (VOLUME_first_controls);

        dprintk (" %d mixer controls for mini dsp 'volumeLite' \n",
                 no_volume_controls);

        if (no_volume_controls)
        {

                for (i = 0; i < no_volume_controls; i++)
                {
                        strcpy (volume_control_name[i], VOLUME_first_control_names[i]);
                        strcat (volume_control_name[i], VOLUME_KCONTROL_NAME);

                        dprintk ("Volume controls: %s\n", volume_control_name[i]);

                        snd_vol_first_controls[i].name = volume_control_name[i];
                        snd_vol_first_controls[i].iface = SNDRV_CTL_ELEM_IFACE_MIXER;
                        snd_vol_first_controls[i].access = SNDRV_CTL_ELEM_ACCESS_READWRITE;
                        snd_vol_first_controls[i].info =
                                __new_control_info_first_minidsp_volume;
                        snd_vol_first_controls[i].get =
                                __new_control_get_first_minidsp_volume;
                        snd_vol_first_controls[i].put =
                                __new_control_put_first_minidsp_volume;
                        /*
                         *      TBD: read volume reg and update the index number
                         */
                        snd_vol_first_controls[i].private_value = 0;
                        snd_vol_first_controls[i].count = 0;

                        err = snd_ctl_add (codec->card,
                                           snd_soc_cnew (&snd_vol_first_controls[i],
                                                         codec, NULL));
                        if (err < 0)
                        {
                                printk ("%s:Invalid control %s\n", __FILE__,
                                        snd_vol_first_controls[i].name);
                        }
                }
        }
        return 0;
}


/************************** MUX CONTROL section *****************************/
static struct snd_kcontrol_new snd_mux_controls[MAX_MUX_CONTROLS];

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_info_minidsp_mux
 * Purpose  : info routine for mini dsp mux control amixer kcontrols
 *----------------------------------------------------------------------------
 */
static int __new_control_info_minidsp_mux(struct snd_kcontrol *kcontrol,
					  struct snd_ctl_elem_info *uinfo)
{
	int index;
	int ret_val = -1;

	for (index = 0; index < ARRAY_SIZE(MUX_second_controls); index++) {
		if (strstr(kcontrol->id.name, MUX_second_control_names[index]))
			break;
	}

	if (index < ARRAY_SIZE(MUX_second_controls)) {
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = MIN_MUX_CTRL;
		uinfo->value.integer.max = MAX_MUX_CTRL;
		ret_val = 0;
	}
	return ret_val;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_get_minidsp_mux
 *
 * Purpose  : get routine for  mux control amixer kcontrols,
 * 			  read current register values to user.
 * 			  Used for for mini dsp 'MUX control' amixer controls.
 *----------------------------------------------------------------------------
 */
static int __new_control_get_minidsp_mux(struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = kcontrol->private_value;
	return 0;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_put_minidsp_mux
 *
 * Purpose  : put routine for amixer kcontrols, write user values to registers
 *            values. Used for for mini dsp 'MUX control' amixer controls.
 *----------------------------------------------------------------------------
 */
static int __new_control_put_minidsp_mux(struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_value *ucontrol)
{
	u8 data[MUX_CTRL_REG_SIZE + 1];
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	int index;
	int user_value = ucontrol->value.integer.value[0];
	struct i2c_client *i2c;
#if 1
	u8 value[2];
  	u8 page;

        /* Enable the below line only when reading the I2C Transactions */
        /*u8 read_data[10]; */
#endif
	int ret_val = -1;
	i2c = codec->control_data;
#if 1
	page = MUX_second_controls[index].control_page;
	dprintk("user value = 0x%x\n", user_value);

	for (index = 0; index < ARRAY_SIZE(MUX_second_controls); index++) {
		if (strstr(kcontrol->id.name, MUX_second_control_names[index]))
			break;
	}
        if (index < ARRAY_SIZE (MUX_second_controls))
        {
                dprintk ("Index %d Changing to Page %d\r\n", index,
                         MUX_second_controls[index].control_page);
                aic325x_change_page (codec, MUX_second_controls[index].control_page);

                data[1] = (u8) ((user_value >> 8) & AIC325x_8BITS_MASK);
                data[2] = (u8) ((user_value) & AIC325x_8BITS_MASK);

                /* start register address */
                data[0] = MUX_second_controls[index].control_base;

                dprintk ("Writing %d %d %d \r\n", data[0], data[1], data[2]);

                ret_val = i2c_master_send (i2c, data, MUX_CTRL_REG_SIZE + 1);

                if (ret_val != MUX_CTRL_REG_SIZE + 1)
                {
                        dprintk ("i2c_master_send transfer failed\n");
                }
                else
                {
                        /* store the current level */
                        kcontrol->private_value = user_value;
                        ret_val = 0;
                        /* Enable adaptive filtering for ADC/DAC */

                }

                /* Perform a BUFFER SWAP Command. Check if we are currently not in Page 8,
                 * if so, swap to Page 8 first
                 */
                aic325x_change_page (codec, 8);

                value[0] = 1;

                if (i2c_master_send (i2c, value, 1) != 1)
                {
                        dprintk ("Can not write register address\n");
                }
                /* Read the Value of the Page 8 Register 1 which controls the Adaptive Switching Mode */
                if (i2c_master_recv (i2c, value, 1) != 1)
                {
                        dprintk ("Can not read codec registers\n");
                }

                /* Write the Register bit updates */
                value[1] = value[0] | 1;
                value[0] = 1;

                if (i2c_master_send (i2c, value, 2) != 2)
                {
                        dprintk ("Can not write register address\n");
                }

        }
        aic325x_change_page (codec, MUX_second_controls[index].control_page);
        ret_val = i2c_master_send (i2c, data, MUX_CTRL_REG_SIZE + 1);
        ret_val = 0;
#endif /* End of #if 0 */

	aic325x_change_page(codec, 0);
	return (ret_val);
}

/*
 *----------------------------------------------------------------------------
 * Function : minidsp_mux_ctrl_mixer_controls
 *
 * Purpose  : Add amixer kcontrols for mini dsp mux controls,
 *----------------------------------------------------------------------------
 */
static int minidsp_mux_ctrl_mixer_controls(struct snd_soc_codec *codec)
{
	int i, err, no_mux_controls;

	no_mux_controls = ARRAY_SIZE(MUX_second_controls);

	dprintk(" %d mixer controls for mini dsp MUX \n", no_mux_controls);

	if (no_mux_controls) {
		for (i = 0; i < no_mux_controls; i++) {

			snd_mux_controls[i].name = MUX_second_control_names[i];
			snd_mux_controls[i].iface = SNDRV_CTL_ELEM_IFACE_MIXER;
			snd_mux_controls[i].access =
                                SNDRV_CTL_ELEM_ACCESS_READWRITE;
			snd_mux_controls[i].info =
                                __new_control_info_minidsp_mux;
			snd_mux_controls[i].get = __new_control_get_minidsp_mux;
			snd_mux_controls[i].put = __new_control_put_minidsp_mux;
			/*
			 *  TBD: read volume reg and update the index number
			 */
			snd_mux_controls[i].private_value = 0;
			snd_mux_controls[i].count = 0;

			err = snd_ctl_add(codec->card,
					  snd_soc_cnew(&snd_mux_controls[i],
						       codec, NULL));
			if (err < 0) {
				printk("%s:Invalid control %s\n", __FILE__,
				       snd_mux_controls[i].name);
			}
		}
	}
	return 0;
}

/************************** Adaptive filtering section **********************/

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_info_minidsp_filter
 * Purpose  : info routine for adaptive filter control amixer kcontrols
 *	      ie Informs the caller about the valid range of values that can be
 *            configured for the Adaptive Mode AMIXER Control. The valid values
 *            are 0 [Adaptive Mode OFF] and 1[Adaptive Mode ON].
 *
 * 	      It returns 0 on success and -1 on failure.
 *----------------------------------------------------------------------------
 */
static int __new_control_info_minidsp_adaptive(struct snd_kcontrol *kcontrol,
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
 * Function : __new_control_get_minidsp_adaptive
 *
 * Purpose  : get routine for  adaptive filter control amixer kcontrols,
 *            reads to user if adaptive filtering is enabled or disabled.
 * 	      ie informs the caller about the current settings of the miniDSP
 *            Adaptive Mode for the Adaptive Mode AMIXER Control. The valid
 *            values are 0 [Adaptive Mode OFF] and 1[Adaptive Mode ON].
 *
 *            The current configuration is updated in the
 *            ucontrol->value.integer.value.
 * 	      It returns 0 on success and -1 on failure
 *----------------------------------------------------------------------------
 */
static int __new_control_get_minidsp_adaptive(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value
					      *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct i2c_client *i2c;
	char data[2];
	int ret = 0;

	/* The kcontrol->private_value is a pointer value which is configured
         * by the driver to hold the following:
         * Page NO for the Adaptive Mode configuration.
         * Register No holding the Adaptive Mode Configuration and
         * BitMask signifying the bit mask required to write it the Register.
  	 */
	u8 page = (kcontrol->private_value) & AIC325x_8BITS_MASK;
	u8 reg = (kcontrol->private_value >> 8) & AIC325x_8BITS_MASK;
	u8 rmask = (kcontrol->private_value >> 16) & AIC325x_8BITS_MASK;

	i2c = codec->control_data;

	dprintk("page %d, reg %d, mask 0x%x\n", page, reg, rmask);

	/* Read the register value */
	aic325x_change_page(codec, page);

	/* write register addr to read */
	data[0] = reg;

	if (i2c_master_send(i2c, data, 1) != 1) {
		printk("Can not write register address\n");
		ret = -1;
		goto revert;
	}
	/* read the codec/minidsp registers */
	if (i2c_master_recv(i2c, data, 1) != 1) {
		printk("Can not read codec registers\n");
		ret = -1;
		goto revert;
	}

	dprintk("read: 0x%x\n", data[0]);

	/* return the read status to the user */
	if (data[0] & rmask) {
		printk("get value 1\n");
		ucontrol->value.integer.value[0] = 1;
	} else {
		printk("get value 0\n");
		ucontrol->value.integer.value[0] = 0;
	}

revert:
	/* put page back to zero */
	aic325x_change_page(codec, 0);
	return ret;
}

/*
 *----------------------------------------------------------------------------
 * Function : __new_control_put_minidsp_adaptive
 *
 * Purpose  : Put routine for adaptive filter controls amixer kcontrols.
 * 	      This routine will enable/disable adaptive filtering.
 *            ie it configures the Adaptive Mode settings as per the User
 *            specification. The User can specify 0 or 1 as the Adaptive AMIXER
 *            Control Value.  The user configuration is specified in the
 *            ucontrol->value.integer.value member of the
 *            struct snd_ctl_elem_value member passed as argument to this
 *            routine from user space. The current configuration is updated in
 *            the ucontrol->value.integer.value.
 *            It returns 0 on success and -1 on failure
 *----------------------------------------------------------------------------
 */
static int __new_control_put_minidsp_adaptive(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value
					      *ucontrol)
{

	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	int user_value = ucontrol->value.integer.value[0];
	struct i2c_client *i2c;
	char data[2];
	int ret = 0;

	u8 page = (kcontrol->private_value) & AIC325x_8BITS_MASK;
	u8 reg = (kcontrol->private_value >> 8) & AIC325x_8BITS_MASK;
	u8 wmask = (kcontrol->private_value >> 24) & AIC325x_8BITS_MASK;
	//u8 rmask = (kcontrol->private_value >> 16) & AIC325x_8BITS_MASK;

	i2c = codec->control_data;

	dprintk("page %d, reg %d, mask 0x%x, user_value %d\n",
		page, reg, wmask, user_value);

	/* Program the register value */
	aic325x_change_page(codec, page);

	/* read register addr to read */
	data[0] = reg;

	if (i2c_master_send(i2c, data, 1) != 1) {
		printk("Can not write register address\n");
		ret = -1;
		goto revert;
	}
	/* read the codec/minidsp registers */
	if (i2c_master_recv(i2c, data, 1) != 1) {
		printk("Can not read codec registers\n");
		ret = -1;
		goto revert;
	}

	dprintk("read: 0x%x\n", data[0]);

	/* set the bitmask and update the register */
	if (user_value == 0) {
		data[1] = (data[0]) & (~wmask);
		printk("set value 0\n");
	} else {
		data[1] = (data[0]) | wmask;
		printk("set value 1\n");
	}
	data[0] = reg;

	if (i2c_master_send(i2c, data, 2) != 2) {
		dprintk("Can not write register address\n");
		ret = -1;
	}

revert:
	/* put page back to zero */
	aic325x_change_page(codec, 0);
	return ret;
}

/*
 * AMIXER Control Interface definition. The definition is done using the
 * below macro SOC_ADAPTIVE_CTL_AIC325x which configures the .iface, .access
 * .info, .get and .put function pointers.
 * The private_value member informs the register/page details to be used
 * when the AMIXER control is used at run-time.
 */
#define SOC_ADAPTIVE_CTL_AIC325x(xname, page, reg, read_mask, write_mask) \
        {   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),       \
                        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,      \
                        .info = __new_control_info_minidsp_adaptive,    \
                        .get = __new_control_get_minidsp_adaptive, 	\
                        .put = __new_control_put_minidsp_adaptive,      \
                        .count = 0,                                     \
                        .private_value = (page) | (reg << 8) |          \
                        ( read_mask << 16) | (write_mask << 24)         \
                        }

/* Adaptive filtering control and buffer swap  mixer kcontrols */
static struct snd_kcontrol_new snd_adaptive_controls[] = {
	SOC_ADAPTIVE_CTL_AIC325x(FILT_CTL_NAME_ADC, BUFFER_PAGE_ADC, 0x1, 0x4,
				 0x4),
	SOC_ADAPTIVE_CTL_AIC325x(FILT_CTL_NAME_DAC, BUFFER_PAGE_DAC, 0x1, 0x4,
				 0x4),
	SOC_ADAPTIVE_CTL_AIC325x(COEFF_CTL_NAME_ADC, BUFFER_PAGE_ADC, 0x1, 0x2,
				 0x1),
	SOC_ADAPTIVE_CTL_AIC325x(COEFF_CTL_NAME_DAC, BUFFER_PAGE_DAC, 0x1, 0x2,
				 0x1),
};

/*
 *-------------------------------------------------------------------------
 * Function : minidsp_adaptive_filter_mixer_controls
 *
 * Purpose  : Registers adaptive filter mixer kcontrols ie Configures the
 *            AMIXER Control Interfaces that can be exercised by the user
 *            at run-time. Utilizes the  the snd_adaptive_controls[]  array to
 *            specify two run-time controls.
 *            It returns 0 on success and -1 on failure
 *----------------------------------------------------------------------------
 */
static int minidsp_adaptive_filter_mixer_controls(struct snd_soc_codec *codec)
{
	int i;
	int err = 0;

	for (i = 0; i < ARRAY_SIZE(snd_adaptive_controls); i++) {
		err = snd_ctl_add(codec->card,
				  snd_soc_cnew(&snd_adaptive_controls[i], codec,
					       NULL));

		if (err < 0) {
			printk("%s:Invalid control %s\n", __FILE__,
			       snd_adaptive_controls[i].name);
			return err;
		}
	}
	return 0;
}

#endif


/****************** first/second MODE CONTROL section********************/
static struct snd_kcontrol_new snd_mode_controls;
/*
 *----------------------------------------------------------------------------
 * Function : __new_control_get_minidsp_mode
 * Purpose  : Add amixer kcontrols for mini dsp volume Lite controls,
 *----------------------------------------------------------------------------
 */
static int
__new_control_get_minidsp_mode (struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
        ucontrol->value.integer.value[0] = first_mode_enable;
        return 0;
}

int put_burst_minidsp_mode (struct snd_soc_codec *codec,u8 ufirst)
{

        int ret_val = -1;
        int user_value = ufirst;
	unsigned long time1,time2;
	
        if( user_value == first_mode_enable )
        {
                return 0;
        }
        printk ("miniDSP: Change miniDSP mode from %s to %s \r\n", first_mode_enable?"first":"second",user_value?"first":"second");

        if (user_value == 0)
        {
                printk("Start switching from first to second mode\n");
                first_mode_enable = 0;
#ifndef MINIDSP_SINGLE_MODE
		time1 = jiffies;
                set_minidsp_second_mode(codec);
		time2 = jiffies;
		printk("Time taken to change the mode %ld clock ticks\n",(time2-time1));
                printk("Switching from first to second mode finished\n");
#endif
        }
        else
        {
                printk("Start switching from second to first mode\n");
                first_mode_enable = 1;
#ifndef MINIDSP_SINGLE_MODE
		time1 = jiffies;
                set_minidsp_first_mode(codec);
		time2 = jiffies;
		printk("Time taken to change the mode %ld clock ticks\n",(time2-time1));
                printk("Switching from second to first mode finished \n");
#endif
        }

        ret_val = 0;

        return (ret_val);
}

/*
 *-------------------------------------------------------------------------------
 * Function : __new_control_put_burst_minidsp_mode
 * Purpose : amixer control callback function invoked when user performs the cset
 * 	     option for the miniDSP Mode switch. This is the burst
 *           implementation of miniDSP mode switching logic and it internally
 *           refers to the global second_dsp_A_parse_data[],
 *           second_dsp_D_parse_data[], and first_dsp_A_parse_data[],
 *           first_dsp_D_parse_data[] arrays for second and first Mode switching
 *           respectively. These arrays will be pre-populated during the Driver
 *           Initialization sequence and they will contain the list of
 *           programming required for switching the modes.
 *
 *           It returns 0 on success and -1 on failure
 *-------------------------------------------------------------------------------
 */
static int
__new_control_put_burst_minidsp_mode (struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
        struct snd_soc_codec *codec = snd_kcontrol_chip (kcontrol);
        int user_value = ucontrol->value.integer.value[0];
        return put_burst_minidsp_mode(codec, user_value);
}
static const char *miniDSP_Mode_Str[] = {"second", "first"};

static const struct soc_enum miniDSP_Mode_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(miniDSP_Mode_Str), miniDSP_Mode_Str),
};

/*
 *-------------------------------------------------------------------------------
 * Function : minidsp_mode_mixer_controls
 * Purpose :  Configures the AMIXER Controls for the miniDSP run-time
 * 	      configuration. This routine configures the function pointers for
 * 	      the get put and the info members of the snd_mode_controls and
 *	      calls the snd_ctl_add routine to register this with the ALSA
 *	      Library.
 *
 * 	      It returns 0 on success and -1 on failure
 *-------------------------------------------------------------------------------
 */
static int
minidsp_mode_mixer_controls (struct snd_soc_codec *codec)
{
        int err;

        snd_mode_controls.name = "miniDSP Mode Selection";
        snd_mode_controls.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
        snd_mode_controls.access = SNDRV_CTL_ELEM_ACCESS_READWRITE;
        snd_mode_controls.info = snd_soc_info_enum_ext;//__new_control_info_minidsp_mode;
        snd_mode_controls.get = __new_control_get_minidsp_mode;
#ifdef OLD_MINIDSP_MODE_SWITCH
        snd_mode_controls.put = __new_control_put_minidsp_mode;
#else
        snd_mode_controls.put = __new_control_put_burst_minidsp_mode;
#endif

        snd_mode_controls.private_value = (unsigned long)&miniDSP_Mode_enum[0];
        snd_mode_controls.count = 0;

        err = snd_ctl_add (codec->card,
                           snd_soc_cnew (&snd_mode_controls, codec, NULL));
        if (err < 0)
        {
                printk ("%s:Invalid control %s\n", __FILE__, snd_mode_controls.name);
        }

        return err;
}

/*
 *--------------------------------------------------------------------------
 * Function : aic325x_add_minidsp_controls
 * Purpose :  Configures the AMIXER Control Interfaces that can be exercised by
 *            the user at run-time. Utilizes the  the snd_adaptive_controls[]
 *            array to specify two run-time controls.
 *---------------------------------------------------------------------------
 */
void aic325x_add_minidsp_controls(struct snd_soc_codec *codec)
{
#ifdef ADD_MINI_DSP_CONTROLS
        if (minidsp_volume_second_mixer_controls (codec))
        {
                printk("mini DSP second volumeLite mixer control registration failed\n");
        }
        if (minidsp_volume_first_mixer_controls (codec))
        {
                printk("mini DSP first volumeLite mixer control registration failed\n");
        }

        if (minidsp_mux_ctrl_mixer_controls (codec))
        {
                printk ("mini DSP mux selection mixer control registration failed\n");
        }

        if (minidsp_adaptive_filter_mixer_controls (codec))
        {
                printk ("Adaptive filter mixer control registration failed\n");
        }

        if (minidsp_mode_mixer_controls (codec))
        {
                printk ("mini DSP mode mixer control registration failed\n");
        }
/* if this is used in Android mode, the system hangs, so not using this*/
/*
  if (minidsp_multibyte_mixer_controls (codec))
  {
  printk ("mini DSP multibyte write mixer control registration failed\n");
  }
*/
#endif /* ADD_MINI_DSP_CONTROLS */
}
MODULE_DESCRIPTION("ASoC TLV320AIC325x miniDSP driver");
MODULE_AUTHOR("Shahina Shaik ");
MODULE_LICENSE("GPL");
#endif /* End of CONFIG_MINI_DSP */
