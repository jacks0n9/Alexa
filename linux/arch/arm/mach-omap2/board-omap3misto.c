/*
 * linux/arch/arm/mach-omap2/board-omap3misto.c
 *
 * Copyright (C) 2011 Texas Instruments
 *
 * Modified from mach-omap2/board-omap3evm.c
 *
 * Initial code: Syed Mohammed Khasim
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/input/matrix_keypad.h>
#include <linux/leds.h>
#include <linux/interrupt.h>
#include <linux/mtd/nand.h>
#include <linux/ctype.h>

#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>
#include <linux/i2c/twl.h>
#include <linux/usb/otg.h>
#include <linux/smsc911x.h>

#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/tps65910.h>
#include <linux/mmc/host.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/usb.h>
#include <plat/common.h>
#include <plat/mcspi.h>
#include <plat/display.h>
#include <plat/omap-pm.h>
#include <plat/gpmc.h>
#include <plat/board-omap3misto-id.h>

#include <linux/input.h>
#include <linux/gpio_keys.h>

#include <sound/tlv320dac33-plat.h>

#include "mux.h"
#include "sdram-micron-mt46h32m32lf-6.h"
#include "sdram-hynix-h8kds0un0mer-4em.h"
#include "hsmmc.h"
#include "board-flash.h"
#include "omap_ram_console.h"

#include <linux/rotary_encoder.h>
#include <linux/leds-lp5523.h>
#include <linux/spi/ks8851.h>

/*
 * Global GPIO distribution
 */
#define GPIO_OMAP_END                   191
#define GPIO_LP5523_U24                 192
#define GPIO_LP5523_U25                 193
#define GPIO_LP5523_U26                 194
#define GPIO_LP5523_U27                 195

/*
 * All GPIO's
 */

#define OMAP3MISTO_PA_EN_GPIO           (129)
#define OMAP3MISTO_USB_UART_GPIO           (188)

/* WIFI power reset signal */
#define OMAP3MISTO_WIFI_PWD_L_EVT_2     (13)
#define OMAP3MISTO_WIFI_PWD_L_EVT_3     (21)

#define OMAP3MISTO_BT_RESET_GPIO        (22)
#define OMAP3MISTO_BT_WKUP_BT           (93)
#define OMAP3MISTO_BT_HOST_WAKE         (23)

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
#define ETH_KS8851_POWER_ON             (14)
#define ETH_KS8851_IRQ                  (15)
#define ETH_KS8851_QUART                (16)
#endif // CONFIG_KS8851

#define OMAP3MISTO_ID0                  (27)
#define OMAP3MISTO_ID1                  (28)
#define OMAP3MISTO_ID2                  (29)
#define OMAP3MISTO_ID3                  (39)
#define OMAP3MISTO_ID4                  (40)
#define OMAP3MISTO_ID5                  (41)
#define OMAP3MISTO_ID6                  (42)
#define OMAP3MISTO_LOW_BITS_MASK        0x7
#define OMAP3MISTO_HIGH_BITS_VALID      0x2
#define OMAP3MISTO_HIGH_BITS_VALID_P    0x1

#define OMAP3MISTO_WLAN_IRQ_GPIO        (24)

#define OMAP3MISTO_ROTARY_A             (69)
#define OMAP3MISTO_ROTARY_B             (76)

#define OMAP3MISTO_BTN_MUTE             (70)
#define OMAP3MISTO_BTN_WPS              (86)
#define OMAP3MISTO_BTN_UBER             (72)
#define OMAP3MISTO_BTN_RESET            (77)
#define OMAP3MISTO_NETWORK_LED_GPIO     (86)
#define OMAP3MISTO_LED_DRVR_EN          (109)

#define MISTO_BOARD_EVT_2_0_STR         "EVT-2.0"
#define MISTO_BOARD_EVT_2_1_STR         "EVT-2.1"
#define MISTO_BOARD_EVT_3_0_STR         "EVT-3.0"
#define MISTO_BOARD_EVT_3_1_STR         "EVT-3.1"
#define MISTO_BOARD_EVT_3_2_STR         "EVT-3.2"
#define MISTO_BOARD_P_PROTO_1_0_STR       "P_PROTO-1.0"
#define MISTO_BOARD_P_PROTO_2_0_STR       "P_PROTO-2.0"
#define MISTO_BOARD_P_PROTO_3_0_STR       "P_PROTO-3.0"
#define MISTO_BOARD_P_EVT_1_0_STR       "P_EVT-1.0"
#define MISTO_BOARD_P_DVT_1_0_STR       "P_DVT-1.0"
#define MISTO_BOARD_UNKNOWN_STR         "Unknown"
#define MISTO_PROTO_1_2_DEFAULT_PCBSN   "01E0000000000000"

#define U24_DEVICE_I2C_ADDR             0x32
#define U25_DEVICE_I2C_ADDR             0x33
#define U26_DEVICE_I2C_ADDR             0x34
#define U27_DEVICE_I2C_ADDR             0x35

#define MISTO_EVT_CLEAR_GPIO_IDX        1
#define POWER_GP_TIMER_ID               8
#define NETWORK_GP_TIMER_ID             11

#define EVT2_LED_MAX_CURRENT             200


static u8 omap3_misto_version;

u8 get_omap3_misto_rev(void)
{
	return omap3_misto_version;
}
EXPORT_SYMBOL(get_omap3_misto_rev);

/* Workaround for unpopulated pcbsn on older boards */
void get_default_board_id(char *dst, int len) {

	int default_len = strlen(MISTO_PROTO_1_2_DEFAULT_PCBSN) + 1 /* Null char */;

	if (!dst)
		return;

	if (len > default_len)
		len = default_len;

	strncpy(dst, MISTO_PROTO_1_2_DEFAULT_PCBSN, len);
	dst[len - 1] = '\0';
}
EXPORT_SYMBOL(get_default_board_id);

char* get_omap3_misto_rev_str(void)
{
	char *rev = MISTO_BOARD_UNKNOWN_STR;

	switch (get_omap3_misto_rev()) {
	case MISTO_BOARD_EVT_2_0:
		rev = MISTO_BOARD_EVT_2_0_STR;
		break;
	case MISTO_BOARD_EVT_2_1:
		rev = MISTO_BOARD_EVT_2_1_STR;
		break;
	case MISTO_BOARD_EVT_3_0:
		rev = MISTO_BOARD_EVT_3_0_STR;
		break;
	case MISTO_BOARD_EVT_3_1:
		rev = MISTO_BOARD_EVT_3_1_STR;
		break;
	case MISTO_BOARD_EVT_3_2:
		rev = MISTO_BOARD_EVT_3_2_STR;
		break;
	case MISTO_BOARD_P_PROTO_1_0:
		rev = MISTO_BOARD_P_PROTO_1_0_STR;
		break;
	case MISTO_BOARD_P_PROTO_2_0:
		rev = MISTO_BOARD_P_PROTO_2_0_STR;
		break;
	case MISTO_BOARD_P_PROTO_3_0:
		rev = MISTO_BOARD_P_PROTO_3_0_STR;
		break;
	case MISTO_BOARD_P_EVT_1_0:
		rev = MISTO_BOARD_P_EVT_1_0_STR;
		break;
	case MISTO_BOARD_P_DVT_1_0:
		rev = MISTO_BOARD_P_DVT_1_0_STR;
		break;
	default:
		break;
	}

	return rev;
}
EXPORT_SYMBOL(get_omap3_misto_rev_str);

/* REVISION TABLE (in assignment order)
 *
 *  ID6 | ID5 | ID4 | ID3 | ID2 | ID1 | ID0 || Board
 * -----+-----+-----+-----+-----+-----+-----++----------------------
 *   -     -     -     -     0     0     1  || TBD
 *   -     -     -     -     0     0     0  || DO NOT USE
 *   -     -     -     -     1     1     1  || Proto 1.2 (obsolete)
 *   -     -     -     -     1     1     0  || EVT 1.0
 *   -     -     -     -     1     0     1  || EVT 1.1L
 *   -     -     -     -     1     0     0  || EVT 2.0
 *   -     -     -     -     0     1     1  || EVT 2.1
 *   -     -     -     -     0     1     0  || (>= EVT 3, bits 3-6 valid)
 *   -     -     -     -     0     0     1  || (Companion, bits 3-6 valid)
 *   0     0     0     0     0     1     0  || DO NOT USE
 *   0     0     0     1     0     1     0  || EVT 3.0
 *   0     0     1     0     0     1     0  || EVT 3.1
 *   0     0     1     1     0     1     0  || EVT 3.2
 *   0     0     0     1     0     0     1  || P_PROTO 1.0 and 1.1
 *   0     0     1     0     0     0     1  || P_PROTO 2.0
 *   0     1     0     0     0     0     1  || P_PROTO 3.0
 *   0     1     0     1     0     0     1  || P_EVT 1.0
 *   0     1     1     0     0     0     1  || P_DVT 1.0
 * -----+-----+-----+-----+-----+-----+-----++----------------------
 * Key: 0 = low voltage, 1 = high voltage, - = don't care
 *
 */
static void __init omap3_misto_init_rev(void)
{
	int ret;
	u8 board_rev = 0xFF;
	u8 masked_rev;

	omap_mux_init_gpio(OMAP3MISTO_ID0, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(OMAP3MISTO_ID1, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(OMAP3MISTO_ID2, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(OMAP3MISTO_ID3, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(OMAP3MISTO_ID4, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(OMAP3MISTO_ID5, OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_gpio(OMAP3MISTO_ID6, OMAP_PIN_INPUT_PULLUP);

	ret = gpio_request(OMAP3MISTO_ID0, "rev_id_0");
	if (ret < 0)
		goto fail0;

	ret = gpio_request(OMAP3MISTO_ID1, "rev_id_1");
	if (ret < 0)
		goto fail1;

	ret = gpio_request(OMAP3MISTO_ID2, "rev_id_2");
	if (ret < 0)
		goto fail2;

	ret = gpio_request(OMAP3MISTO_ID3, "rev_id_3");
	if (ret < 0)
		goto fail3;

	ret = gpio_request(OMAP3MISTO_ID4, "rev_id_4");
	if (ret < 0)
		goto fail4;

	ret = gpio_request(OMAP3MISTO_ID5, "rev_id_5");
	if (ret < 0)
		goto fail5;

	ret = gpio_request(OMAP3MISTO_ID6, "rev_id_6");
	if (ret < 0)
		goto fail6;

	gpio_direction_input(OMAP3MISTO_ID0);
	gpio_direction_input(OMAP3MISTO_ID1);
	gpio_direction_input(OMAP3MISTO_ID2);
	gpio_direction_input(OMAP3MISTO_ID3);
	gpio_direction_input(OMAP3MISTO_ID4);
	gpio_direction_input(OMAP3MISTO_ID5);
	gpio_direction_input(OMAP3MISTO_ID6);

	board_rev = 0;
	board_rev = gpio_get_value(OMAP3MISTO_ID0)
		|  (gpio_get_value(OMAP3MISTO_ID1) << 1)
		|  (gpio_get_value(OMAP3MISTO_ID2) << 2)
		|  (gpio_get_value(OMAP3MISTO_ID3) << 3)
		|  (gpio_get_value(OMAP3MISTO_ID4) << 4)
		|  (gpio_get_value(OMAP3MISTO_ID5) << 5)
		|  (gpio_get_value(OMAP3MISTO_ID6) << 6);

	masked_rev = board_rev & OMAP3MISTO_LOW_BITS_MASK;
	if ((masked_rev != OMAP3MISTO_HIGH_BITS_VALID)
	    && (masked_rev != OMAP3MISTO_HIGH_BITS_VALID_P)) {
		board_rev &= OMAP3MISTO_LOW_BITS_MASK;
	}

	/* Errata: EVT 3.0 prototype boards had ID of 0x02 */
	if (board_rev == 0x02)
		board_rev = MISTO_BOARD_EVT_3_0;

	omap3_misto_version = board_rev;
	printk(KERN_INFO "OMAP3 Lab126 Rev: r:0x%x f:0x%x\n", board_rev, omap3_misto_version);

	printk(KERN_INFO "Lab126 Board Revision detected : %s",get_omap3_misto_rev_str());

	gpio_free(OMAP3MISTO_ID6);
fail6:
	gpio_free(OMAP3MISTO_ID5);
fail5:
	gpio_free(OMAP3MISTO_ID4);
fail4:
	gpio_free(OMAP3MISTO_ID3);
fail3:
	gpio_free(OMAP3MISTO_ID2);
fail2:
	gpio_free(OMAP3MISTO_ID1);
fail1:
	gpio_free(OMAP3MISTO_ID0);
fail0:
	if(board_rev == 0xFF) {
		printk(KERN_ERR "Unable to get revision detection GPIO pins\n");
		omap3_misto_version = 0xFF;
	}

	return;
}

static void omap3_misto_set_pin_debounce (int pin, int debounceval)
{
	gpio_request(pin, "temp");
	gpio_set_debounce(pin, debounceval);
	gpio_free(pin);
}

/* Rotary Encoder Input Device */

static struct rotary_encoder_platform_data rotary_encoder_pdata = {
	.steps		= 24,
	.axis		= REL_X,
	.relative_axis	= 1,
	.gpio_a		= OMAP3MISTO_ROTARY_A,
	.gpio_b		= OMAP3MISTO_ROTARY_B,
	.inverted_a	= 0,
	.inverted_b	= 0,
};

static struct platform_device rotary_encoder_device = {
	.name	= "rotary-encoder",
	.id	= 0,
	.dev	= {
		.platform_data = &rotary_encoder_pdata,
	}
};

static void __init omap3_misto_init_encoder(void)
{
	// Set up debounce for rotary gpio's
	omap3_misto_set_pin_debounce(OMAP3MISTO_ROTARY_A, 0x80);
	omap3_misto_set_pin_debounce(OMAP3MISTO_ROTARY_B, 0x80);
	platform_device_register(&rotary_encoder_device);
}

static struct platform_device dummy_codec_bt_device = {
	.name = "dummy-codec-bt",
};

static void __init omap3_misto_init_dummy_codec_bt(void)
{
	platform_device_register(&dummy_codec_bt_device);
}

/* Note, update omap3_misto_init_keys() if table is changed */
static struct gpio_keys_button gpio_buttons[] = {
	{
		.code			= KEY_MUTE,
		.gpio			= OMAP3MISTO_BTN_MUTE,
		.desc			= "VOLUME_MUTE",
		.wakeup			= 1,
	},
	{
		.code			= KEY_WPS_BUTTON,
		.gpio			= OMAP3MISTO_BTN_WPS,
		.desc			= "WPS",
		.wakeup			= 1,
	},
	{
		.code			= KEY_HELP,
		.gpio			= OMAP3MISTO_BTN_UBER,
		.desc			= "UBER",
		.wakeup			= 1,
	},
};

static struct gpio_keys_platform_data gpio_keys_pdata = {
	.buttons	= gpio_buttons,
	.nbuttons	= ARRAY_SIZE(gpio_buttons),
};

static struct platform_device gpio_keys_device = {
	.name	= "gpio-keys",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_keys_pdata,
	},
};

/* Initialize the keys based on board */
/* This method may update the default table entries for gpio */
static void __init omap3_misto_init_keys(void)
{
	omap3_misto_set_pin_debounce(OMAP3MISTO_BTN_UBER, 0x80);
	omap3_misto_set_pin_debounce(OMAP3MISTO_BTN_MUTE, 0x80);
	omap3_misto_set_pin_debounce(OMAP3MISTO_BTN_RESET, 0x80);

	/* Overwrite default gpio table entry */
	gpio_buttons[MISTO_EVT_CLEAR_GPIO_IDX].code = KEY_CLEAR;
	gpio_buttons[MISTO_EVT_CLEAR_GPIO_IDX].gpio = OMAP3MISTO_BTN_RESET;
	gpio_buttons[MISTO_EVT_CLEAR_GPIO_IDX].desc = "Reset";

	platform_device_register(&gpio_keys_device);
}

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
/* Important: This structure must be initialized in
 * omap3_misto_init_ks8851(). */
static struct ks8851_platform_data __initdata ks8851_pdata = {
	.have_mac = 0,
	.mac = { 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, },
};

static struct omap2_mcspi_device_config ksz8851_mcspi_config = {
		.turbo_mode	= 1,
		.single_channel	= 1,  /* 0: slave, 1: master */
};

static struct spi_board_info omap3_misto_spi_board_info[] __initdata = {
	{
		.modalias = "ks8851",
		.bus_num = 1,
		.chip_select = 0,
		.max_speed_hz = 24000000,
		.mode = SPI_MODE_3,
		.controller_data = &ksz8851_mcspi_config,
		.irq = ETH_KS8851_IRQ,
		.platform_data = &ks8851_pdata,
	},
};

static int omap3_misto_init_ks8851(void)
{
	int status;
	int n;

	/* Initialize MAC.
	 *
	 * Convert MAC from the ASCII representation in the IDME data,
	 * then set the "Local Administration" bit.
	 *
	 * If IDME data is invalid, then the data will not be ASCII,
	 * but 0xFF's for all the chars.
	 */
	status = 0;
	for (n = 0 ; n < 6 ; ++n) {
		char hex[] = "XX";
		hex[0] = omap3_bt_mac_address[2*n];
		hex[1] = omap3_bt_mac_address[2*n + 1];
		if (!isxdigit(hex[0]) || !isxdigit(hex[1])) {
			status = 1;
			break;
		}
		ks8851_pdata.mac[n] = simple_strtoul(hex, 0, 16);
	}
	if (!status) {
		ks8851_pdata.mac[0] |= (1 << 1); /* locally administered */
		ks8851_pdata.have_mac = 1;
	} else {
		pr_err("BTMAC from IDME is invalid, can not set ks8851 MAC\n");
		ks8851_pdata.have_mac = 0;
	}

	/* Request of GPIO lines */
	status = gpio_request(ETH_KS8851_POWER_ON, "eth_power");
	if (status) {
		pr_err("Cannot request GPIO %d\n", ETH_KS8851_POWER_ON);
		return status;
	}

	status = gpio_request(ETH_KS8851_QUART, "quart");
	if (status) {
		pr_err("Cannot request GPIO %d\n", ETH_KS8851_QUART);
		goto error1;
	}

	status = gpio_request(ETH_KS8851_IRQ, "eth_irq");
	if (status) {
		pr_err("Cannot request GPIO %d\n", ETH_KS8851_IRQ);
		goto error2;
	}

	/* Configuration of requested GPIO lines */

	status = gpio_direction_output(ETH_KS8851_POWER_ON, 1);
	if (status) {
		pr_err("Cannot set output GPIO %d\n", ETH_KS8851_IRQ);
		goto error3;
	}

	status = gpio_direction_output(ETH_KS8851_QUART, 1);
	if (status) {
		pr_err("Cannot set output GPIO %d\n", ETH_KS8851_QUART);
		goto error3;
	}

	status = gpio_direction_input(ETH_KS8851_IRQ);
	if (status) {
		pr_err("Cannot set input GPIO %d\n", ETH_KS8851_IRQ);
		goto error3;
	}

	return 0;

error3:
	gpio_free(ETH_KS8851_IRQ);
error2:
	gpio_free(ETH_KS8851_QUART);
error1:
	gpio_free(ETH_KS8851_POWER_ON);
	return status;
}

#else
static int omap3_misto_init_ks8851(void) { return 0; }
#endif // CONFIG_KS8851

static struct regulator_consumer_supply omap3evm_vmmc1_supply =
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.0");

static struct regulator_consumer_supply omap3evm_mpu_supply =
	REGULATOR_SUPPLY("mpu", "mpu.0");

/*
 * NOTE: To prevent turning off regulators not explicitly consumed by drivers
 * depending on it, ensure following:
 *	1) Set always_on = 1 for them OR
 *	2) Avoid calling regulator_has_full_constraints()
 *
 * With just (2), there will be a warning about incomplete constraints.
 * E.g., "regulator_init_complete: incomplete constraints, leaving LDO8 on"
 *
 * In either cases, the supply won't be disabled.
 *
 * We are taking approach (1).
 */
static struct regulator_init_data tps65910_reg_data[] = {
	/* VRTC */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VIO -VDDA 1.8V */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.apply_uV = 1,
			.always_on = 1,
		},
	},

	/* VDD1 - MPU */
	{
		.constraints = {
			.min_uV = 600000,
			.max_uV = 1500000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
		},
		.num_consumer_supplies	= 1,
		.consumer_supplies	= &omap3evm_mpu_supply,
	},

	/* VDD2 - DSP */
	{
		.constraints = {
			.min_uV = 600000,
			.max_uV = 1500000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VDD3 - CORE */
	{
		.constraints = {
			.min_uV = 5000000,
			.max_uV = 5000000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 0,
		},
	},

	/* VDIG1 */
	{
		.constraints = {
			.min_uV = 1200000,
			.max_uV = 2700000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VDIG2 */
	{
		.constraints = {
			.min_uV = 1000000,
			.max_uV = 1800000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VPLL */
	{
		.constraints = {
			.min_uV = 1000000,
			.max_uV = 2500000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VDAC */
	{
		.constraints = {
			.min_uV = 1100000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VAUX1 */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 2850000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VAUX2 */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
						REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
	},

	/* VAUX33 */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 3300000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
			.always_on = 1,
		},
	},

	/* VMMC */

	/* This rail Powers the Dac3203 . The Sdio card is not the only consumer.
	 * Always on.
	 */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 3000000,
			.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
			.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
			.always_on = 1,
		},
        .num_consumer_supplies = 1,
        .consumer_supplies = &omap3evm_vmmc1_supply,
	},
};

static struct omap2_hsmmc_info mmc_evt2[] = {
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},

	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},
	{}	/* Terminator */
};

static struct omap2_hsmmc_info mmc_evt3[] = {
	{
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},
	{
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},
	{
		.mmc		= 3,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_cd	= -EINVAL,
		.gpio_wp	= -EINVAL,
	},
	{}	/* Terminator */
};

/* The WLAN module (QCA6134) is a fixed supply and supplies power to
 * the "vmmc" pin of the MMC interface that it uses.  This is being
 * modeled as a fixed regulator.
 */

static struct regulator_consumer_supply omap3evm_vmmc2_supply =
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.1");

static struct regulator_consumer_supply omap3evm_vmmc3_supply =
	REGULATOR_SUPPLY("vmmc", "mmci-omap-hs.2");

static struct regulator_init_data omap3evm_wlan_sdio_regulator_evt2 = {
	.constraints = {
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies = &omap3evm_vmmc2_supply,
};

static struct regulator_init_data omap3evm_wlan_sdio_regulator_evt3 = {
	.constraints = {
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies = &omap3evm_vmmc3_supply,
};

static struct fixed_voltage_config omap3evm_vwlan_evt2 = {
	.supply_name		= "QCA6134",
	.microvolts		= 2000000, /* 2.00V */
	.gpio			= -EINVAL,
	.startup_delay		= 0, /* 0ms */
	.enable_high		= 1,
	.enabled_at_boot	= 1,
	.init_data		= &omap3evm_wlan_sdio_regulator_evt2,
};

static struct fixed_voltage_config omap3evm_vwlan_evt3 = {
	.supply_name		= "QCA6134",
	.microvolts		= 2000000, /* 2.00V */
	.gpio			= -EINVAL,
	.startup_delay		= 0, /* 0ms */
	.enable_high		= 1,
	.enabled_at_boot	= 1,
	.init_data		= &omap3evm_wlan_sdio_regulator_evt3,
};

static struct platform_device omap3evm_wlan_regulator_evt2 = {
	.name		= "reg-fixed-voltage",
	.id		= 1,
	.dev = {
		.platform_data = &omap3evm_vwlan_evt2,
	},
};

static struct platform_device omap3evm_wlan_regulator_evt3 = {
	.name		= "reg-fixed-voltage",
	.id		= 1,
	.dev = {
		.platform_data = &omap3evm_vwlan_evt3,
	},
};

#if defined(CONFIG_WLAN)
/*
 * Not sure what the specific Atheros CONFIG label is so I
 * am using the generic one for now
 */

static inline void __init omap3_wifi_init(void)
{
	int ret;
	int gpio_wifi_pwd_l = OMAP3MISTO_WIFI_PWD_L_EVT_3;
	struct platform_device *sdio_regulator = &omap3evm_wlan_regulator_evt3;

	switch (get_omap3_misto_rev()) {
	case MISTO_BOARD_EVT_2_0:
	case MISTO_BOARD_EVT_2_1:
		gpio_wifi_pwd_l = OMAP3MISTO_WIFI_PWD_L_EVT_2;
		sdio_regulator = &omap3evm_wlan_regulator_evt2;
		break;
	case MISTO_BOARD_EVT_3_0:
	case MISTO_BOARD_EVT_3_1:
	case MISTO_BOARD_EVT_3_2:
	case MISTO_BOARD_P_PROTO_1_0:
	case MISTO_BOARD_P_PROTO_2_0:
	case MISTO_BOARD_P_PROTO_3_0:
	case MISTO_BOARD_P_EVT_1_0:
	case MISTO_BOARD_P_DVT_1_0:
		gpio_wifi_pwd_l = OMAP3MISTO_WIFI_PWD_L_EVT_3;
		sdio_regulator = &omap3evm_wlan_regulator_evt3;
		break;
	default:
		pr_err("Failed to assign WIFI_PWD_L GPIO for board rev\n");
	}

	/* ATHEROS RESET */
	omap_mux_init_gpio(gpio_wifi_pwd_l, OMAP_PIN_OUTPUT);

	ret = gpio_request(gpio_wifi_pwd_l, "WIFI_reset");
	if (ret < 0){
		pr_err("Failed to request GPIO13 for WiFI reset\n");
		goto fail0;
	}

	gpio_direction_output(gpio_wifi_pwd_l, 1);
	gpio_set_value(gpio_wifi_pwd_l, 1);
	usleep_range(30000, 40000);
	gpio_set_value(gpio_wifi_pwd_l, 0);
	usleep_range(30000, 40000);
	gpio_set_value(gpio_wifi_pwd_l, 1);
	pr_info("WiFi RESET\n");

	/* BLUETOOTH RESET */
	omap_mux_init_gpio(OMAP3MISTO_BT_RESET_GPIO, OMAP_PIN_OUTPUT);

	ret = gpio_request(OMAP3MISTO_BT_RESET_GPIO, "Bluetooth_reset");
	if (ret < 0){
		pr_err("Failed to request GPIO22 for Bluetooth reset\n");
		goto fail1;
	}

	omap_mux_init_gpio(OMAP3MISTO_BT_WKUP_BT, OMAP_PIN_OUTPUT);
	ret = gpio_request(OMAP3MISTO_BT_WKUP_BT, "bluetooth_wakeup");
	if (ret < 0) {
		pr_err("Failed to request GPIO%d for bluetooth wake-up\n",
		       OMAP3MISTO_BT_WKUP_BT);
	} else {
		gpio_direction_output(OMAP3MISTO_BT_WKUP_BT, 1);
		gpio_set_value(OMAP3MISTO_BT_WKUP_BT, 1);
	}

	omap_mux_init_gpio(OMAP3MISTO_BT_HOST_WAKE, OMAP_PIN_INPUT_PULLDOWN | OMAP_WAKEUP_EN);
	ret = gpio_request(OMAP3MISTO_BT_HOST_WAKE, "bluetooth_host_wakeup");
	if (ret < 0) {
		pr_err("Failed to request GPIO%d for bluetooth host wake-up signal\n",
		       OMAP3MISTO_BT_HOST_WAKE);
	}

	gpio_direction_output(OMAP3MISTO_BT_RESET_GPIO, 1);
	gpio_set_value(OMAP3MISTO_BT_RESET_GPIO, 1);
	usleep_range(30000, 40000);
	gpio_set_value(OMAP3MISTO_BT_RESET_GPIO, 0);
	usleep_range(30000, 40000);
	gpio_set_value(OMAP3MISTO_BT_RESET_GPIO, 1);

	pr_info("Bluetooth RESET\n");

	/*
	 * The wifi regulator described by sdio_regulator is the external
	 * fixed regulator powering the Wifi chip. Resetting the wifi chip
	 * results in no SDIO VDD
	 */
	platform_device_register(sdio_regulator);

	/* Allow WiFi to be reset from userspace via /sys/class/gpio/gpioNN/value */
	gpio_export(gpio_wifi_pwd_l, 0);
	/* Allow BT to be reset from userspace via /sys/class/gpio/gpioNN/value */
	gpio_export(OMAP3MISTO_BT_RESET_GPIO, 0);

	return;

fail2:
	gpio_free(OMAP3MISTO_BT_RESET_GPIO);
fail1:
	gpio_free(gpio_wifi_pwd_l);
fail0:
	return;
}

#else
static inline void __init omap3_wifi_init(void) { return; }
#endif

static struct regulator_init_data regulator_emmc_data = {
	.constraints = {
		.min_uV = 1800000,
		.max_uV = 1800000,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &omap3evm_vmmc2_supply,
};

static struct fixed_voltage_config regulator_fixed_emmc_device = {
	.supply_name = "KLM4G1FEAC",
	.microvolts  = 1800000,
	.gpio = -EINVAL,
	.startup_delay = 0,
	.enable_high = 1,
	.enabled_at_boot = 1,
	.init_data = &regulator_emmc_data,
};

static struct platform_device regulator_fixed_emmc_pdev = {
	.name		= "reg-fixed-voltage",
	.id		= 2,
	.dev = {
		.platform_data = &regulator_fixed_emmc_device,
	},
};

static struct tps65910_board __refdata tps65910_pdata = {
	.irq				= INT_34XX_SYS_NIRQ,
	.irq_base			= TPS65910_IRQ_BASE,
	.gpio_base			= 0,	/* No support currently */
	.tps65910_pmic_init_data	= tps65910_reg_data,
};

static struct i2c_board_info __initdata omap3evm_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("tps65910", 0x2D),
		.flags = I2C_CLIENT_WAKE,
		.irq = INT_34XX_SYS_NIRQ,
		.platform_data = &tps65910_pdata,
	},
};

static struct omap_pwm_led_platform_data board_power_led_data;
static struct omap_pwm_led_platform_data board_network_led_data;

static struct platform_device board_power_led_device = {
		.name	= "omap_pwm_led",
		.id	= 0,
		.dev.platform_data	= &board_power_led_data,
};

static struct platform_device board_network_led_device = {
		.name	= "omap_pwm_led",
		.id	= 1,
		.dev.platform_data	= &board_network_led_data,
};

static struct lp5523_led_config lp5523_led_config[] = {
	{
		.chan_nr	= 0,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 1,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 2,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 3,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 4,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 5,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 6,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 7,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	},
	{
		.chan_nr	= 8,
		.led_current	= 150,	// mA * 10
		.max_current	= 150,
	}
};

static int lp5523_setup(void)
{
	static int once = 0;
	int ret;

	if (once != 0) {
		return 1;
	}

	once = 1;

	/* Setup HW resources */
	ret = gpio_request(OMAP3MISTO_LED_DRVR_EN, "ti_led");
	if (ret < 0) {
		pr_err("Failed to request GPIO for TI LED\n");
	}
	else {
		gpio_direction_output(OMAP3MISTO_LED_DRVR_EN, 1);
	}

	return ret;
}

static void lp5523_release(void)
{
	static int once = 0;

	if (once != 0) {
		return;
	}

	once = 1;

	/* Release HW resources */
	gpio_free(OMAP3MISTO_LED_DRVR_EN);
}

/* Note: the INT pin is N.C. for EVT < 3.0 */
static struct lp5523_platform_data lp5523_platform_data_u24 = {
	.led_config		= lp5523_led_config,
	.num_channels		= ARRAY_SIZE(lp5523_led_config),
	.clock_mode		= LP5523_CLOCK_INT,
	.setup_resources	= lp5523_setup,
	.release_resources	= lp5523_release,
	.enable		        = 0,
	.gpio                   = GPIO_LP5523_U24,
	.int_pin_mode           = INT_ADC,
};

/* Note: UNMUTE_SFWR_L only works on EVT >= 3.0, otherwise is N.C. */
static struct lp5523_platform_data lp5523_platform_data_u25 = {
	.led_config		= lp5523_led_config,
	.num_channels		= ARRAY_SIZE(lp5523_led_config),
	.clock_mode		= LP5523_CLOCK_INT,
	.setup_resources	= lp5523_setup,
	.release_resources	= lp5523_release,
	.enable		        = 0,
	.gpio                   = GPIO_LP5523_U25, /* UNMUTE_SFWR_L */
	.int_pin_mode           = INT_GPO,
};

/* Note: the INT pin is N.C. for EVT <3.0 and 3.1, connected to a secondary ALS
   on some EVT 3.0s, and connected to the mute button led brightness gpio on
   EVT >=3.2. */
static struct lp5523_platform_data lp5523_platform_data_u26 = {
	.led_config		= lp5523_led_config,
	.num_channels		= ARRAY_SIZE(lp5523_led_config),
	.clock_mode		= LP5523_CLOCK_INT,
	.setup_resources	= lp5523_setup,
	.release_resources	= lp5523_release,
	.enable		        = 0,
	.gpio                   = GPIO_LP5523_U26,
	.int_pin_mode           = INT_GPO,
};

/* Note: MUTE_SFWR_L only works on EVT >= 2.1, otherwise is N.C. */
static struct lp5523_platform_data lp5523_platform_data_u27 = {
	.led_config		= lp5523_led_config,
	.num_channels		= ARRAY_SIZE(lp5523_led_config),
	.clock_mode		= LP5523_CLOCK_INT,
	.setup_resources	= lp5523_setup,
	.release_resources	= lp5523_release,
	.enable		        = 0,
	.gpio                   = GPIO_LP5523_U27, /* MUTE_SFWR_L */
	.int_pin_mode           = INT_GPO,
};

static struct i2c_board_info __initdata omap3evm_i2c_boardinfo2[] = {
	{
		I2C_BOARD_INFO("tlv320adc31xx_A", 0x18),
	},
	{
		I2C_BOARD_INFO("tlv320adc31xx_B", 0x19),
	},
	{
		I2C_BOARD_INFO("tlv320adc31xx_C", 0x1a),
	},
	{
		I2C_BOARD_INFO("tlv320adc31xx_D", 0x1b),
	},
	{
		I2C_BOARD_INFO("lp5523", U24_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u24,
	},
	{
		I2C_BOARD_INFO("lp5523", U25_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u25,
	},
	{
		I2C_BOARD_INFO("lp5523", U26_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u26,
	},
	{
		I2C_BOARD_INFO("lp5523", U27_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u27,
	},
};

static struct i2c_board_info __initdata omap3evm_i2c_p_boardinfo2[] = {
	{
		I2C_BOARD_INFO("tlv320adc31xx_A", 0x18),
	},
	{
		I2C_BOARD_INFO("tlv320adc31xx_B", 0x19),
	},
	{
		I2C_BOARD_INFO("tlv320adc31xx_C", 0x1a),
	},
	{
		I2C_BOARD_INFO("tlv320adc31xx_D", 0x1b),
	},
	{
		I2C_BOARD_INFO("lp5523", U24_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u24,
	},
	{
		I2C_BOARD_INFO("lp5523", U25_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u25,
	},
	{
		I2C_BOARD_INFO("lp5523", U26_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u26,
	},
	{
		I2C_BOARD_INFO("lp5523", U27_DEVICE_I2C_ADDR),
				.platform_data = &lp5523_platform_data_u27,
	},
	{
		I2C_BOARD_INFO("tmp103", 0x70),
	},
};

static struct i2c_board_info __initdata omap3evm_i2c_boardinfo3[] = {
	{
		I2C_BOARD_INFO("tlv320aic32xx", 0x18),
	},
};

static struct i2c_board_info __initdata omap3evm_i2c_p_boardinfo3[] = {
	{
		I2C_BOARD_INFO("tlv320aic32xx", 0x18),
	},
	{
		I2C_BOARD_INFO("tmp103", 0x70),
	},
};

static void set_max_led_current(unsigned int val) {
	unsigned int i = 0;

	for (i=0; i<ARRAY_SIZE(lp5523_led_config); i++) {
		lp5523_led_config[i].led_current = val;
		lp5523_led_config[i].max_current = val;
	}
}

/* Initialize all LEDS for misto per board revision */
static int __init omap3_misto_init_leds(void)
{
	int ret = 0;

	/* Power and network GPIOs are pwm controlled */
	memset(&board_power_led_data, 0 , sizeof(struct omap_pwm_led_platform_data));

	/* Populate power led data */
	board_power_led_data.timer_type = OMAP_PWM_TIMER_PULSE;
	board_power_led_data.timer_id = POWER_GP_TIMER_ID;
	board_power_led_data.max = 255;
	board_power_led_data.brightness = 255; /* Enable the led */
	board_power_led_data.name = "power-led";
	board_power_led_data.default_trigger = "power";
	/* Register the power led device */
	ret = platform_device_register(&board_power_led_device);
	if (ret < 0)
		pr_err("unable to register power LED\n");

	/* Populate network led data */
	memset(&board_network_led_data, 0 , sizeof(struct omap_pwm_led_platform_data));
	/* TODO - Use blink timer for Network LED */
	board_network_led_data.timer_type = OMAP_PWM_TIMER_PULSE;
	board_network_led_data.timer_id = NETWORK_GP_TIMER_ID;
	board_network_led_data.max = 255;
	board_network_led_data.brightness = 0; /* Disable the led */
	board_network_led_data.name = "network-led";
	board_network_led_data.default_trigger = "network";
	/* Register the network led device */
	ret = platform_device_register(&board_network_led_device);
	if (ret < 0)
		pr_err("unable to register network LED\n");

	// Update the current for EVT-2.0 and EVT-2.1; use defaults for newer devices
	switch (get_omap3_misto_rev()) {
	case MISTO_BOARD_EVT_2_0:
	case MISTO_BOARD_EVT_2_1:
		set_max_led_current(EVT2_LED_MAX_CURRENT);
		break;
	}

	return ret;
}

static int __init omap3_misto_i2c_init(void)
{
	struct i2c_board_info * bus2_info = omap3evm_i2c_boardinfo2;
	int bus2_size = ARRAY_SIZE(omap3evm_i2c_boardinfo2);

	struct i2c_board_info * bus3_info = omap3evm_i2c_boardinfo3;
	int bus3_size = ARRAY_SIZE(omap3evm_i2c_boardinfo3);

	switch (get_omap3_misto_rev()) {
	case MISTO_BOARD_P_PROTO_3_0:
		bus3_info = omap3evm_i2c_p_boardinfo3;
		bus3_size = ARRAY_SIZE(omap3evm_i2c_p_boardinfo3);
		break;
	case MISTO_BOARD_P_EVT_1_0:
		bus2_info = omap3evm_i2c_p_boardinfo2;
		bus2_size = ARRAY_SIZE(omap3evm_i2c_p_boardinfo2);
		break;
	}
	/* Bus 1 is used for PMIC */
	omap_register_i2c_bus(1, 400, omap3evm_i2c_boardinfo,
			ARRAY_SIZE(omap3evm_i2c_boardinfo));

	/* Bus 3 is used for DAC */
	omap_register_i2c_bus(3, 400, bus3_info, bus3_size);

	// Make sure that the 3203 gets initialized before the 3101s
	/* Bus 2 is used for 31xx adc codecs */
	omap_register_i2c_bus(2, 400, bus2_info, bus2_size);

	return 0;
}


static struct omap_board_config_kernel omap3_misto_config[] __initdata = {
};

static void __init omap3_misto_init_irq(void)
{
	omap_board_config = omap3_misto_config;
	omap_board_config_size = ARRAY_SIZE(omap3_misto_config);
	omap2_init_common_infrastructure();
	if (cpu_is_omap3630())
		omap2_init_common_devices(h8kds0un0mer4em_sdrc_params, NULL);
	else
		omap2_init_common_devices(mt46h32m32lf6_sdrc_params, NULL);

	omap_init_irq();
	gpmc_init();
}
static struct platform_device omap3_misto_aic325x = {
	.name	= "tlv320aic325x",
	.id	= -1,
};


static struct platform_device *omap3_misto_devices[] __initdata = {
	&omap3_misto_aic325x,
};

/*
 * NAND partition tables.
 */

#define NAND_BLOCK_SIZE (64 * 2048)

static struct mtd_partition omap3_misto_nand_partitions[] = {
	/* All the partition sizes are listed in terms of NAND block size */
	{
		.name		= "X-Loader",
		.offset		= 0,
		.size		= 4 * (NAND_BLOCK_SIZE),
		.mask_flags	= MTD_WRITEABLE,	/* force read-only */
	},
	{
		.name		= "U-Boot-Main",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x8 0000 */
		.size		= 6 * (NAND_BLOCK_SIZE),/* TODO: Make read-only */
	},
	{
		.name		= "U-Boot-Rec",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0xC 0000  */
		.size		= 6 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "Data-MFG",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x0020 0000 */
		.size		= 4 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "Kernel-DIAG",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x0028 0000 */
		.size		= 40 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "FS-DIAG",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x0078 0000 */
		.size		= 256 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "Kernel-Main",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x0278 0000 */
		.size		= 40 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "FS-Main",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x02C8 0000 */
		.size		= 2600 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "Data",
		.offset		= MTDPART_OFS_APPEND,	/* Offset = 0x1718 0000 */
		.size		= MTDPART_SIZ_FULL,
	},
	{
		.name		= "Kernel-Main-A",
		.offset		= 0x02780000,		/* Offset = 0x0278 0000 */
		.size		= 40 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "FS-Main-A",
		.offset		= 0x02C80000,		/* Offset = 0x02C8 0000 */
		.size		= 1280 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "Kernel-Main-B",
		.offset		= 0x0CC80000,		/* Offset = 0x0CC8 0000 */
		.size		= 40 * (NAND_BLOCK_SIZE),
	},
	{
		.name		= "FS-Main-B",
		.offset		= 0x0D180000,		/* Offset = 0x0D18 0000 */
		.size		= 1280 * (NAND_BLOCK_SIZE),
	},
};

static struct omap_board_mux omap36x_board_mux_evt_2[] __initdata = {
	OMAP3_MUX(SYS_NIRQ, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
				OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
				OMAP_PIN_OFF_WAKEUPENABLE),
	OMAP3_MUX(MCSPI1_CS1, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
				OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
				OMAP_PIN_OFF_WAKEUPENABLE),
	OMAP3_MUX(MCBSP1_FSR, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
				OMAP_PIN_OFF_NONE),
	/* AM/DM37x EVM: DSS data bus muxed with sys_boot */
	OMAP3_MUX(DSS_DATA18, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA19, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA22, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA21, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA22, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA23, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_ACBIAS, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 69 - VOL_UP */
	OMAP3_MUX(DSS_DATA6,  OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 76 - VOL_DN */
	OMAP3_MUX(DSS_DATA2,  OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 72 - Uber */
	OMAP3_MUX(DSS_DATA7, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 77 - Reset */
	OMAP3_MUX(UART1_CTS, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),	/* GPIO 70 - MIC_MUTE */

	/* MMC2 SDIO pin muxes for WLAN */
	OMAP3_MUX(SDMMC2_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(SDMMC2_CMD, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(SDMMC2_DAT0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(SDMMC2_DAT1, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(SDMMC2_DAT2, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(SDMMC2_DAT3, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),

	/* MCBSP5 for Bluetooth PCM */
	OMAP3_MUX(ETK_CLK, OMAP_MUX_MODE1 | OMAP_PIN_INPUT),    /* MCBSP5_CLK */
	OMAP3_MUX(ETK_D4,  OMAP_MUX_MODE1 | OMAP_PIN_INPUT),    /* MCBSP5_DR  */
	OMAP3_MUX(ETK_D5,  OMAP_MUX_MODE1 | OMAP_PIN_INPUT),    /* MCBSP5_FSX */
	OMAP3_MUX(ETK_D6,  OMAP_MUX_MODE1 | OMAP_PIN_OUTPUT),   /* MCBSP5_DX  */

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	/* McSPI1_CS0 */
	OMAP3_MUX(MCSPI1_CS0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* McSPI1_CLK */
	OMAP3_MUX(MCSPI1_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* McSPI1_SIMO */
	OMAP3_MUX(MCSPI1_SIMO, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* McSPI1_SOMI */
	OMAP3_MUX(MCSPI1_SOMI, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* INTR - GPIO_15 */
	OMAP3_MUX(ETK_D1, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* PME - GPIO_14 */
	OMAP3_MUX(ETK_D0, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_NONE),

	/* RSTN - GPIO_16 */
	OMAP3_MUX(ETK_D2, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_NONE),
#endif /* #if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)*/
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};

static struct omap_board_mux omap36x_board_mux_evt_3[] __initdata = {
	OMAP3_MUX(SYS_NIRQ, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
				OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
				OMAP_PIN_OFF_WAKEUPENABLE),
	OMAP3_MUX(MCSPI1_CS1, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
				OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
				OMAP_PIN_OFF_WAKEUPENABLE),
	OMAP3_MUX(MCBSP1_FSR, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
				OMAP_PIN_OFF_NONE),
	/* AM/DM37x EVM: DSS data bus muxed with sys_boot */
	OMAP3_MUX(DSS_DATA18, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA19, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA22, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA21, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA22, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_DATA23, OMAP_MUX_MODE3 | OMAP_PIN_OFF_NONE),
	OMAP3_MUX(DSS_ACBIAS, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 69 - VOL_UP */
	OMAP3_MUX(DSS_DATA6,  OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 76 - VOL_DN */
	OMAP3_MUX(DSS_DATA2,  OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 72 - Uber */
	OMAP3_MUX(DSS_DATA7, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),	/*GPIO 77 - Reset */
	OMAP3_MUX(UART1_CTS, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),	/* GPIO 70 - MIC_MUTE */

	/* MMC3 SDIO pin muxes for WLAN */
	OMAP3_MUX(ETK_CLK, OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP), /* MMC3_CLK */
	OMAP3_MUX(ETK_CTL, OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP), /* MMC3_CMD */
	OMAP3_MUX(ETK_D4, OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP), /* MMC3_DAT0 */
	OMAP3_MUX(ETK_D5, OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP), /* MMC3_DAT1 */
	OMAP3_MUX(ETK_D6, OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP), /* MMC3_DAT2 */
	OMAP3_MUX(ETK_D3, OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP), /* MMC3_DAT3 */

	/* MCBSP3 for Bluetooth PCM */
	OMAP3_MUX(MCBSP3_CLKX, OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP3_FSX,  OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP3_DR,   OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(MCBSP3_DX,   OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),

	OMAP3_MUX(HSUSB0_DIR, OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(DSS_DATA1, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),

	OMAP3_MUX(HSUSB0_DATA4, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	/* McSPI1_CS0 */
	OMAP3_MUX(MCSPI1_CS0, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* McSPI1_CLK */
	OMAP3_MUX(MCSPI1_CLK, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* McSPI1_SIMO */
	OMAP3_MUX(MCSPI1_SIMO, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* McSPI1_SOMI */
	OMAP3_MUX(MCSPI1_SOMI, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* INTR - GPIO_15 */
	OMAP3_MUX(ETK_D1, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW |
			OMAP_PIN_OFF_WAKEUPENABLE),

	/* PME - GPIO_14 */
	OMAP3_MUX(ETK_D0, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_NONE),

	/* RSTN - GPIO_16 */
	OMAP3_MUX(ETK_D2, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP |
			OMAP_PIN_OFF_NONE),
#endif /* #if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)*/
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type		= MUSB_INTERFACE_ULPI,
	.mode			= MUSB_OTG,
	.power			= 100,
};

static void __init omap3_misto_export_gpio(void)
{
	int ret;

	/* Export PA gpio so user space can toggle on/off */
	omap_mux_init_gpio(OMAP3MISTO_PA_EN_GPIO, OMAP_PIN_OUTPUT);
	ret = gpio_request(OMAP3MISTO_PA_EN_GPIO, "PA_Enable");
	if (ret < 0) {
		pr_err("Failed to request GPIO for PA enable\n");
	} else {
		gpio_direction_output(OMAP3MISTO_PA_EN_GPIO, 0);
		gpio_export(OMAP3MISTO_PA_EN_GPIO, false);
	}

	if (get_omap3_misto_rev() & MISTO_BOARD_IS_P) {

		omap_mux_init_gpio(OMAP3MISTO_USB_UART_GPIO, OMAP_PIN_OUTPUT);
		ret = gpio_request(OMAP3MISTO_USB_UART_GPIO, "usb_uart");
		if (ret < 0) {
			pr_err("Failed to request GPIO for sel select\n");
		} else {
			pr_info("postmode - %s\n", omap3_postmode);
			if (strcmp(omap3_postmode, "usb-uart-enable") == 0)
				gpio_direction_output(OMAP3MISTO_USB_UART_GPIO, 0);
			else
				gpio_direction_output(OMAP3MISTO_USB_UART_GPIO, 1);

			gpio_export(OMAP3MISTO_USB_UART_GPIO, false);
		}
	}
}

/* See commit message */
static void __init omap3_misto_do_magic(void)
{
	asm ("MCR p15, 0, %0, C9, C14, 0\n\t" : : "r" (1));
	asm ("MCR p15, 0, %0, C9, C14, 2\n\t" : : "r" (0x8000000f));
}

static void __init omap3_misto_init(void)
{
	struct omap_board_mux *board_mux = omap36x_board_mux_evt_3;
	struct omap2_hsmmc_info *mmc = mmc_evt3;

	omap3_misto_init_rev();

	/* Setup the board id and IDME info */
	omap3_misto_init_boardid();

	switch (get_omap3_misto_rev()) {
	case MISTO_BOARD_EVT_2_0:
	case MISTO_BOARD_EVT_2_1:
		board_mux = omap36x_board_mux_evt_2;
		mmc = mmc_evt2;
		break;
	case MISTO_BOARD_EVT_3_0:
	case MISTO_BOARD_EVT_3_1:
	case MISTO_BOARD_EVT_3_2:
	case MISTO_BOARD_P_PROTO_1_0:
	case MISTO_BOARD_P_PROTO_2_0:
	case MISTO_BOARD_P_PROTO_3_0:
	case MISTO_BOARD_P_EVT_1_0:
	case MISTO_BOARD_P_DVT_1_0:
		board_mux = omap36x_board_mux_evt_3;
		mmc = mmc_evt3;
		break;
	default:
		pr_err("Unsupported/Unknown Misto Board Revision. Using EVT-3 Default\n");
	}

	omap3_mux_init(board_mux, OMAP_PACKAGE_CUS);

	// Perform LED initializations prior to i2c init
	omap3_misto_init_leds();
	omap3_misto_i2c_init();

	platform_add_devices(omap3_misto_devices, ARRAY_SIZE(omap3_misto_devices));

	// TODO: Reset LED Drivers?

	omap_serial_init();

	/* gpio + 0 is "mmc0_cd" (input/IRQ) */
//	omap_mux_init_gpio(63, OMAP_PIN_INPUT);
//	mmc[0].gpio_cd = gpio + 0;
	omap2_hsmmc_init(mmc);

	/* OMAP3EVM uses ISP1504 phy and so register nop transceiver */
	usb_nop_xceiv_register(0);

	/* EVM REV >= E can supply 500mA with EXTVBUS programming */
	musb_board_data.power = 500;
	musb_board_data.extvbus = 1;

	usb_musb_init(&musb_board_data);
#if defined(CONFIG_KS8851)
	omap3_misto_init_ks8851();
	omap3_misto_spi_board_info[0].irq = gpio_to_irq(ETH_KS8851_IRQ);

	spi_register_board_info(omap3_misto_spi_board_info,
				ARRAY_SIZE(omap3_misto_spi_board_info));
#endif
	omap3_misto_init_encoder();
	omap3_misto_init_dummy_codec_bt();
	omap3_misto_init_keys();
	omap3_misto_export_gpio();

	/* NAND
	 *
	 * EVT1.0 and later have a new partition layout while Proto 1.2 will
	 * retain its layout. Which table is presented to the kernel depends on
	 * the board revision.
	 */
	switch (get_omap3_misto_rev()) {
	case MISTO_BOARD_EVT_2_0:
	case MISTO_BOARD_EVT_2_1:
		board_nand_init(omap3_misto_nand_partitions,
				ARRAY_SIZE(omap3_misto_nand_partitions),
				0, NAND_BUSWIDTH_16);
		break;
	case MISTO_BOARD_EVT_3_0:
	case MISTO_BOARD_EVT_3_1:
	case MISTO_BOARD_EVT_3_2:
	case MISTO_BOARD_P_PROTO_1_0:
	case MISTO_BOARD_P_PROTO_2_0:
	case MISTO_BOARD_P_PROTO_3_0:
	case MISTO_BOARD_P_EVT_1_0:
	case MISTO_BOARD_P_DVT_1_0:
	default:
		platform_device_register(&regulator_fixed_emmc_pdev);
		break;
	}

	omap3_wifi_init();
	regulator_has_full_constraints();

	omap3_misto_do_magic();
}

static void __init omap3_misto_reserve(void)
{
	omap_ram_console_init(OMAP_RAM_CONSOLE_START_DEFAULT,
			      OMAP_RAM_CONSOLE_SIZE_DEFAULT);

	omap_reserve();
}

// TODO: Figure out why we have to be an EVM

MACHINE_START(OMAP3EVM, "OMAP3 EVM")
	/* Maintainer: Misto team, Lab126 */
	.boot_params	= 0x80000100,
	.map_io		= omap3_map_io,
	.reserve	= omap3_misto_reserve,
	.init_irq	= omap3_misto_init_irq,
	.init_machine	= omap3_misto_init,
	.timer		= &omap_timer,
MACHINE_END
