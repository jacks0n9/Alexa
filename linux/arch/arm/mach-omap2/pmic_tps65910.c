/**
 * Implements support for TPS65910
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <linux/mfd/tps65910.h>

#include <plat/voltage.h>

#include "pm.h"

#define	TPS65910_VDCDC1_MIN	(VDD1_2_MIN_VOLT*100)	/* 0.6V		*/
#define	TPS65910_VDCDC1_STEP	(VDD1_2_OFFSET*100)	/* 0.0125V	*/


/*
 * Get voltage corresponding to specified vsel value using this formula:
 *	Vout = 0.8V + (25mV x Vsel)
 */
static unsigned long tps65910_vsel_to_uv(const u8 vsel)
{
//	int volt;

//	volt = TPS65910_VDCDC1_MIN +
//				(vsel % VDD1_2_NUM_VOLTS) * TPS65910_VDCDC1_STEP;
//	return  volt;
//	return (TPS65023_VDCDC1_MIN + (TPS65023_VDCDC1_STEP * vsel));
	return (((vsel * 125) + 6000)) * 100;
}

/*
 * Get vsel value corresponding to specified voltage using this formula:
 *	Vsel = (Vout - 0.8V)/ 25mV
 */
static u8 tps65910_uv_to_vsel(unsigned long uv)
{
//	return DIV_ROUND_UP(uv - TPS65910_VDCDC1_MIN, TPS65910_VDCDC1_STEP);
	return DIV_ROUND_UP(uv - 600000, 12500);	
}

/*
 * TPS65910 is currently supported only for DM37x devices.
 * Therefore, implementation below is specific to this device pair.
 */

/**
 * Voltage information related to the MPU voltage domain of the
 * DM37x processors - in relation to the TPS65910.
 */
static struct omap_volt_pmic_info tps65910_dm37xx_mpu_volt_info = {
	.slew_rate		= 4000,
	.step_size		= 12500,
	.on_volt		= 1200000,
	.vsel_to_uv		= tps65910_vsel_to_uv,
	.uv_to_vsel		= tps65910_uv_to_vsel,
};

int __init omap3_tps65910_init(void)
{
	struct voltagedomain *voltdm;

	if (!cpu_is_omap34xx())
		return -ENODEV;

	if ( cpu_is_omap3630()) {
		voltdm = omap_voltage_domain_lookup("mpu");
		omap_voltage_register_pmic(voltdm, &tps65910_dm37xx_mpu_volt_info);
		voltdm = omap_voltage_domain_lookup("core");
		omap_voltage_register_pmic(voltdm, &tps65910_dm37xx_mpu_volt_info);
	} else {
		/* TODO:
		 * Support for other devices that support TPS65910
		 */
	}

	return 0;
}
