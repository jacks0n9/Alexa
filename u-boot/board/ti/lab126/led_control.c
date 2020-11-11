/*
 * led_control.c
 *
 * Copyright 2013 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <malloc.h>
#include <idme.h>
#include "board_id.h"
#include "boot_anim_data.h"
#include "led_patterns.h"

extern u8 get_omap3_board_rev(void);

#define I2C_LED_BUS_NUM		1
#define DEFAULT_GPIO_HOLD_TIME		12000 /* 12 ms */
#define GPT8_RESET_NUM_RETRY		10
#define NUM_CONTROLLERS		4

/* Register Address Mapping */
#define LP5523_REG_ENABLE		0x00
#define LP5523_RESET_REG		0x3d
#define LP5523_REG_CONFIG		0x36
#define LP5523_REG_GPO		0x3B
#define LP5523_AUTO_INC		0x40
#define LP5523_PWR_SAVE		0x20
#define LP5523_PWM_PWR_SAVE		0x04
#define LP5523_CP_AUTO		0x18
#define LP5523_AUTO_CLK		0x02
#define LP5523_REG_LED_PWM_BASE		0x16
#define LP5523_REG_LED_CURRENT_BASE		0x26
#define LP5523_REG_ENABLE_LEDS_MSB		0x04
#define LP5523_REG_ENABLE_LEDS_LSB		0x05
#define LP52232_RESET		0xFF
#define LP5523_ENABLE		0x40
#define LP5523_CMD_LOAD		0x15
#define LP5523_CMD_RUN		0x2a
#define LP5523_CMD_DISABLED		0x00
#define LP5523_REG_OP_MODE		0x01
#define LP5523_REG_PROG_MEM		0x50
#define LP5523_REG_PROG_PAGE_SEL		0x4f
#define LP5523_REG_CH1_PROG_START		0x4c
#define LP5523_REG_CH2_PROG_START		0x4d
#define LP5523_REG_CH3_PROG_START		0x4e
#define LP5523_GPIO_HIGH		0x5

#define GPO_INT_CONF(enable)	((enable) << 2)
#define GPO(enable)				((enable) << 1)
#define GPO_INT_GPO(enable)		((enable) << 0)
#define GPO_SET(int_conf, gpo_value, int_gpo_value) \
	GPO_INT_CONF((int_conf)) | GPO((gpo_value)) | GPO_INT_CONF((int_gpo_value))

#define LP5523_LEDS		9
#define LP5523_PAGE_SIZE		32
#define LED_CURRENT_DEFAULT		150 /* 15 mA */

/* OMAP Specific */
#define OMAP_CM_FCLKEN_PER		0x48005000
#define OMAP_CM_ICLKEN_PER		0x48005010
#define OMAP_CM_CLKSEL_PER		0x48005040
#define OMAP_GPTIMER8_BASE		0x4903E000
#define OMAP_GPTIMER8_OCP_CFG_REG		(OMAP_GPTIMER8_BASE + 0x10)
#define OMAP_GPTIMER8_SYS_STAT_REG		(OMAP_GPTIMER8_BASE + 0x14)
#define OMAP_GPTIMER8_CTRL_REG		(OMAP_GPTIMER8_BASE + 0x24)
#define OMAP_GPTIMER8_IF_CTRL_REG		(OMAP_GPTIMER8_BASE + 0x40)

/// Animation storage
#define BOOT_ANIMATION_FILE	"bootanim.idmedata"
#define VARBUF_SIZE	(1024u)

static char *varbuf = NULL;

/*
 * Enums
 */
typedef enum led_colors {
	LED_PATTERN_INVALID,
	LED_PATTERN_NONE,
	LED_PATTERN_SOLID_RED,
	LED_PATTERN_SOLID_BLUE,
	LED_PATTERN_SOLID_GREEN,
	LED_PATTERN_SOLID_WHITE,
	LED_PATTERN_BOOT_ANIM
} LED_PATTERN_T;

enum led_cmd {
	LED_ON,
	LED_OFF
};

/*
 * Structs
 */
struct gpio_data {
	u16 gpio_num;
	u16 active_low;
};

struct i2c_data {
	u32 bus_num;
	u8* address_list;
};

struct control_data {
	struct gpio_data gpio_info;
	struct i2c_data i2c_info;
};

struct led_control {
	u16 board_rev;
	struct control_data driver_info;
	struct control_data power_led_info;
	void (*enable_power_led)(struct led_control*);
	void (*show_pattern)(struct led_control*, LED_PATTERN_T color);
};

/* Function prototypes */
static void lp5523_pattern(struct led_control* handle, LED_PATTERN_T color);
static void gpt8_pwm_enable(struct led_control* handle);

/* lp5523 driver addresses - MUST BE NULL TERMINATED */
static u8 lp5523_driver_address[] = {
		LP5523_DRIVER_1_ADDR,
		LP5523_DRIVER_2_ADDR,
		LP5523_DRIVER_3_ADDR,
		LP5523_DRIVER_4_ADDR,
		0x0
};


static struct led_control evt2_led_control = {
		.board_rev = BOARD_LED_LP5523,
		.enable_power_led = gpt8_pwm_enable,
		.show_pattern = lp5523_pattern,
		.driver_info.gpio_info = {109, 0},
		.driver_info.i2c_info = {I2C_LED_BUS_NUM, lp5523_driver_address},
		.power_led_info.gpio_info = {0, 0},
		.power_led_info.i2c_info = {0, 0},
};

static struct led_control* driver_handle_list[] = {
		&evt2_led_control,
		NULL
};

/*
 * get_driver_handle - Return the handle / index for the controller
 */
static struct led_control* get_driver_handle(int board_rev)
{
	int i = 0;
	u32 search_rev = BOARD_LED_LP5523;

	while (driver_handle_list[i] != NULL) {
		if (driver_handle_list[i]->board_rev == search_rev)
			return driver_handle_list[i];
		i++;
	}

	return (struct led_control*) NULL;
}

/*
 * i2c_initialize - Initialize the I2C controller for particular bus
 */
static void i2c_initialize(struct i2c_data* const i2c_handle)
{
	if (NULL == i2c_handle)
		return;
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	/* Selecting the bus num will initialize this bus */
	I2C_SET_BUS(i2c_handle->bus_num);

}

/*
 * send_i2c_cmd - Send I2C command
 */
static int send_i2c_cmd(u8* const address_list, u8 reg, u8 value)
{
	int ret = -1;
	int i = 0;

	if(!address_list)
		return ret;

	while (address_list[i]) {
		if ((ret = i2c_write(address_list[i], reg, 1, &value, 1)) != 0)
			return ret;
		i++;
	}

	return ret;
}

/*
 * read_i2c - Read I2C data
 */
static int read_i2c_reg(u8* const address_list, u8 reg, u8* data, u32 data_len)
{
	int ret = -1;
	int i = 0;

	if(!address_list || !data_len)
		return ret;

	/* Read reg for each address in list */
	while (address_list[i]) {
		if (i >= data_len)
			return -1;

		if ((ret = i2c_read(address_list[i], reg, 1, &data[i], 1)) != 0)
			return ret;

		i++;
	}

	return ret;
}

/*
 * send_i2c_cmd_no_ack - Send I2C command, expect no ack and reset the controller
 */
static void send_i2c_cmd_no_ack(u8* address_list, u8 reg, u8 value)
{
	int i = 0;

	if(!address_list)
		return;

	while (address_list[i]) {
		// Ignore errors
		i2c_write(address_list[i], reg, 1, &value, 1);
		i++;
	}
}
/*
 * gpio_toggle - Toggle gpio
 */
static void gpio_toggle(struct gpio_data* const gp_handle, u32 hold_time_us)
{
	u8 active_high = 1;

	if (NULL == gp_handle)
		return;

	if (gp_handle->gpio_num) {
		if (gp_handle->active_low)
			active_high = 0;

		omap_request_gpio(gp_handle->gpio_num);
		omap_set_gpio_direction(gp_handle->gpio_num, 0);
		omap_set_gpio_dataout(gp_handle->gpio_num, active_high ? 1 : 0);
		omap_set_gpio_dataout(gp_handle->gpio_num, active_high ? 0 : 1);
		udelay(hold_time_us);
		omap_set_gpio_dataout(gp_handle->gpio_num, active_high ? 1 : 0);
	}
}

/*
 * gpt8_pwm_enable - Enable GPT8 PWM
 */
static void gpt8_pwm_enable(struct led_control* const handle)
{
	u32 reg = 0;
	int retry_count = 0;

	if (NULL == handle)
		return;

	/* Enabled ICLK, FCLK and set sys clock for gpt8 */
	reg = readl(OMAP_CM_CLKSEL_PER);
	reg |= 0x40; /* Clock select for GPT8, use sys clk */
	writel(reg, OMAP_CM_CLKSEL_PER);

	reg = readl(OMAP_CM_ICLKEN_PER);
	reg |= 0x200; /* Enable GPT8 Interface clock */
	writel(reg, OMAP_CM_ICLKEN_PER);

	reg = readl(OMAP_CM_FCLKEN_PER);
	reg |= 0x200; /* Enable GPT8 functional clock */
	writel(reg, OMAP_CM_FCLKEN_PER);

	reg = 0x6; /* Reset gpt8 timer, select posted mode */
	writel(reg, OMAP_GPTIMER8_IF_CTRL_REG);

	/* Reset gpt8 timer */
	while ((readl(OMAP_GPTIMER8_SYS_STAT_REG) & 0x1) != 1) {
		// Wait for reset to complete
		udelay(1000); /* wait ~1ms */
		if (++retry_count == GPT8_RESET_NUM_RETRY) {
			printf("gpt8_pwm_enable - gpt8 reset timeout!\n");
			break;
		}
	}

	reg = readl(OMAP_GPTIMER8_OCP_CFG_REG);
	reg = 0x00000214; /* Wake up enabled in smart idle, smart idle, fclk maintined */
	writel(reg, OMAP_GPTIMER8_OCP_CFG_REG);

	reg = 0x00001880; /* PWM out enabled, overflow & match trigger, toggle modulation */
	writel(reg, OMAP_GPTIMER8_CTRL_REG);
}

static void copy_program_data(const EngineDataT* from, struct pattern_info *to)
{
	to->data = (u8*)from->data;
	to->size = from->size;
	to->engine1_start = from->engine1_offset;
	to->engine2_start = from->engine2_offset;
	to->engine3_start = from->engine3_offset;
}

static int get_pattern_from_storage(struct pattern_info * p1, struct pattern_info * p2,
		struct pattern_info * p3, struct pattern_info * p4)
{
	int ret = -1;
	BootAnimationT *program_data = NULL;

	if (!p1 || !p2 || !p3 || !p4)
		return ret;

	varbuf = malloc(VARBUF_SIZE);
	if (!varbuf)
		return ret;

	ret = idme_get_data_store(BOOT_ANIMATION_FILE, varbuf, VARBUF_SIZE);
	if (ret != 0)
		return ret;

	// Get the pattern info
	program_data = (BootAnimationT *) varbuf;
	// Check header
	if (BOOT_ANIM_HEADER_SEQUENCE != program_data->header) {
		ret = -1;
	}
	else {
		printf("Animation Version = %d\n", program_data->version);
		// Program 1
		copy_program_data(&program_data->program1_data, p1);
		// Program 2
		copy_program_data(&program_data->program2_data, p2);
		// Program 3
		copy_program_data(&program_data->program3_data, p3);
		// Program 4
		copy_program_data(&program_data->program4_data, p4);
	}

	return ret;
}

/*
 * lp5523_reset - Manually reset each controller and re-initialize the I2C bus
 */
static void lp5523_reset(struct led_control* const handle)
{
	int i = 0;
	u8 address[] = { 0, 0};

	for (i = 0; i < NUM_CONTROLLERS; i++) {
		address[0] = handle->driver_info.i2c_info.address_list[i];
		/* Soft reset -  Send reset i2c command */
		send_i2c_cmd_no_ack(address, LP5523_RESET_REG, LP52232_RESET);

		/* Toggle the enable line */
		udelay(20000); /* wait ~20ms */
		gpio_toggle(&handle->driver_info.gpio_info, DEFAULT_GPIO_HOLD_TIME);
		udelay(2000); /* wait 2ms */

		/* lp5523 will not respond with an ACK after a reset.
		 * HACK: Re-initialize the I2C controller to handle other transactions
		 */
		i2c_initialize(&handle->driver_info.i2c_info);
	}
}

static int is_pattern_active(struct led_control* const handle)
{
	u32 read_data_size = ARRAY_SIZE(handle->driver_info.i2c_info.address_list);
	u8 reg_data[read_data_size];
	u32 i = 0;

	if (read_i2c_reg(handle->driver_info.i2c_info.address_list, LP5523_REG_OP_MODE,
			reg_data, read_data_size))
		return 0;

	for (i = 0; i < read_data_size; ++i) {
		if (!(reg_data[i] & LP5523_CMD_RUN))
			return 0;
	}

	/* All engines are running */
	return 1;
}

/*
 * lp5523_init - Initialize the lp5523 led driver
 */
static int lp5523_init(struct led_control* const handle)
{
	int ret = 0;
	int led_count = 0;

	// initialize I2C
	i2c_initialize(&handle->driver_info.i2c_info);

	// Check if we already have a pattern running
	if (is_pattern_active(handle))
		return 1;

	/* Enable line */
	gpio_toggle(&handle->driver_info.gpio_info, DEFAULT_GPIO_HOLD_TIME);
	udelay(2000); /* wait 2ms */

	/* Send soft reset commands */
	lp5523_reset(handle);

	ret = send_i2c_cmd(handle->driver_info.i2c_info.address_list, LP5523_REG_ENABLE,
			LP5523_ENABLE);
	udelay(2000); /* wait 2ms */

	ret |= send_i2c_cmd(handle->driver_info.i2c_info.address_list, LP5523_REG_CONFIG,
			LP5523_AUTO_INC | LP5523_PWR_SAVE | LP5523_CP_AUTO | LP5523_AUTO_CLK
			| LP5523_PWM_PWR_SAVE);
	/* Set the default led current */
	for (led_count = 0; led_count < LP5523_LEDS; ++led_count) {
		ret |= send_i2c_cmd(handle->driver_info.i2c_info.address_list,
				LP5523_REG_LED_CURRENT_BASE + led_count, LED_CURRENT_DEFAULT);
	}
	/* Set INT to GPIO and high */
	ret = send_i2c_cmd(handle->driver_info.i2c_info.address_list, LP5523_REG_GPO,
				LP5523_GPIO_HIGH);

	return ret;
}

/*
 * lp5523_load_pattern - Load pattern data to LP5523 SRAM
 */
static void lp5523_load_pattern(u8 *address_list, struct pattern_info *pattern)
{
	u8 prog_page = 0;
	int offset = 0;
	int i = 0;
	int pattern_size = pattern->size;
	// Disable the engine
	send_i2c_cmd(address_list, LP5523_REG_OP_MODE, LP5523_CMD_DISABLED);

	// Set the program start
	send_i2c_cmd(address_list, LP5523_REG_CH1_PROG_START, pattern->engine1_start);
	send_i2c_cmd(address_list, LP5523_REG_CH2_PROG_START, pattern->engine2_start);
	send_i2c_cmd(address_list, LP5523_REG_CH3_PROG_START, pattern->engine3_start);

	// Set the engine mode to load
	send_i2c_cmd(address_list, LP5523_REG_OP_MODE, LP5523_CMD_LOAD);

	// Send pattern sequence
	for (i = 0; i < pattern_size; i++) {
		// Write the page select
		if (!(i % LP5523_PAGE_SIZE)) {
			send_i2c_cmd(address_list, LP5523_REG_PROG_PAGE_SEL, prog_page);
			prog_page++;
			offset = 0;
		}
		// Write data to selected page
		send_i2c_cmd(address_list, LP5523_REG_PROG_MEM + offset,
				pattern->data[i]);
		offset++;
	}
}

static void run_pattern(struct led_control* const handle)
{
	// Run the pattern
	send_i2c_cmd(handle->driver_info.i2c_info.address_list, LP5523_REG_ENABLE,
			(LP5523_CMD_RUN | LP5523_ENABLE));

	send_i2c_cmd(handle->driver_info.i2c_info.address_list, LP5523_REG_OP_MODE,
			LP5523_CMD_RUN);
}

static void load_default_boot_patterns(void)
{
	printf("loading default boot pattern\n");
	// load master pattern
	lp5523_load_pattern(pattern_boot_master.address_list, &pattern_boot_master);
	// load slave 1 pattern
	lp5523_load_pattern(pattern_boot_slave_1.address_list, &pattern_boot_slave_1);
	// load slave 1 pattern
	lp5523_load_pattern(pattern_boot_slave_2.address_list, &pattern_boot_slave_2);
	// load slave 1 pattern
	lp5523_load_pattern(pattern_boot_slave_3.address_list, &pattern_boot_slave_3);

}

/*
 * lp5523_enable_boot_pattern - Enable the boot pattern
 */
static void lp5523_enable_boot_pattern(struct led_control* const handle)
{
	struct pattern_info pattern_program_1;
	struct pattern_info pattern_program_2;
	struct pattern_info pattern_program_3;
	struct pattern_info pattern_program_4;

	if(get_pattern_from_storage(&pattern_program_1, &pattern_program_2,
			&pattern_program_3, &pattern_program_4) != 0)
		// load the default patterns
		load_default_boot_patterns();
	else {
		// load master pattern
		lp5523_load_pattern(pattern_boot_master.address_list, &pattern_program_1);
		// load slave 1 pattern
		lp5523_load_pattern(pattern_boot_slave_1.address_list, &pattern_program_2);
		// load slave 1 pattern
		lp5523_load_pattern(pattern_boot_slave_2.address_list, &pattern_program_3);
		// load slave 1 pattern
		lp5523_load_pattern(pattern_boot_slave_3.address_list, &pattern_program_4);
	}

	// Free the buffer
	if (varbuf) {
		free(varbuf);
		varbuf = NULL;
	}

	// Run the pattern
	run_pattern(handle);
}

/*
 * lp5523_pattern - Display pattern for lp5523 led driver
 */
static void lp5523_pattern(struct led_control* const handle, LED_PATTERN_T color)
{
	int i = 0;
	int num_leds = LP5523_LEDS; /* Max = LP5523_LEDS */
	u8 brightness = 0xFF; /* Max intensity */
	u8 channels[LP5523_LEDS] = {0, 1, 2, 3, 4, 5, 6, 7, 8}; /* LED Channels */

	if (NULL == handle)
		return;

	if (lp5523_init(handle) != 0) {
		return;
	}

	switch (color) {
	case LED_PATTERN_SOLID_RED:
		channels[0] = 6; channels[1] = 7; channels[2] = 8;
		num_leds = 3; /* 3 R-channels */
		break;
	case LED_PATTERN_SOLID_BLUE:
		channels[0] = 1; channels[1] = 3; channels[2] = 5;
		num_leds = 3; /* 3 B-channels */
		break;
	case LED_PATTERN_SOLID_GREEN:
		channels[0] = 0; channels[1] = 2; channels[2] = 4;
		num_leds = 3; /* 3 G-channels */
		break;
	case LED_PATTERN_SOLID_WHITE:
		num_leds = LP5523_LEDS;
		break;
	case LED_PATTERN_NONE:
		num_leds = LP5523_LEDS;
		brightness = 0;
		break;
	case LED_PATTERN_BOOT_ANIM:
		// Handle a non-solid pattern
		lp5523_enable_boot_pattern(handle);
		return;
	default:
		return;
	}
	/* Enable all leds */
	send_i2c_cmd(handle->driver_info.i2c_info.address_list, LP5523_REG_ENABLE_LEDS_MSB,
			0x1);
	send_i2c_cmd(handle->driver_info.i2c_info.address_list, LP5523_REG_ENABLE_LEDS_LSB,
			0xFF);
	/* Drive appropriate pwm per color channel */
	for (i = 0; i < num_leds; i++) {
		send_i2c_cmd(handle->driver_info.i2c_info.address_list,
				LP5523_REG_LED_PWM_BASE + channels[i], brightness);
	}
}

/*
 * initialize_leds - Perform any leds related initialization
 */
void initialize_leds(int board_rev) {
	struct led_control* handle = get_driver_handle(board_rev);

	if (NULL == handle) {
		printf("initialize_leds - Invalid control handle\n");
		return;
	}

	if (handle->enable_power_led)
		handle->enable_power_led(handle);

	if (handle->show_pattern)
		handle->show_pattern(handle, LED_PATTERN_BOOT_ANIM);
}

/*
 * Turns off (or dims) all the leds on the platform.  We use this in powersave
 * mode to draw as little power as possible.
 */
void turn_off_leds()
{
	// Turn off power and net lights
	omap_set_gpio_dataout(57, 0);	// LED_NETWORK
	omap_set_gpio_dataout(58, 0);	// LED_POWER

	// Force mute button off
	const u8 MUTE_DISABLE_GPO[] = {LP5523_DRIVER_2_ADDR, 0};
	send_i2c_cmd(MUTE_DISABLE_GPO, LP5523_REG_GPO, GPO_SET(1, 0, 0));

	// Wait for i2c traffic to finish
	udelay(100000);

	// Turn off LED driver chips
	omap_set_gpio_dataout(109, 0);	// LED_DRVR_EN
}


/* ------------------------------------------------------------------
 *                           COMMAND CONTROL
 * ------------------------------------------------------------------
 */

/*
 * trigger_leds - Trigger necessary sequence to enable LED color
 */
static int trigger_leds(LED_PATTERN_T color)
{
	struct led_control* handle = get_driver_handle(get_omap3_board_rev());

	if (NULL == handle) {
		printf("initialize_leds - Invalid control handle\n");
		return -1;
	}

	if (handle->show_pattern)
		handle->show_pattern(handle, color);

	return 0;
}

/*
 * get_led_cmd - Return the led command
 */
static enum led_cmd get_led_cmd(char *var)
{
	if (!var)
		return -1;

	if (strcmp(var, "off") == 0)
		return LED_OFF;

	if (strcmp(var, "on") == 0)
		return LED_ON;

	return -1;
}

/*
 * get_color - Return the led color
 */
static LED_PATTERN_T get_color(char* color)
{
	if (color == NULL)
		return LED_PATTERN_INVALID;

	switch(color[0]){
	case 'r':
	case 'R':
		return LED_PATTERN_SOLID_RED;
	case 'g':
	case 'G':
		return LED_PATTERN_SOLID_GREEN;
	case 'b':
	case 'B':
		return LED_PATTERN_SOLID_BLUE;
	case 'a':
	case 'A':
		return LED_PATTERN_SOLID_WHITE;
	default:
		return LED_PATTERN_INVALID;
	}
}

/*
 * do_led - Command line run routine
 */
int do_led (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	enum led_cmd cmd;
	LED_PATTERN_T color;

	/* Validate arguments */
	if (argc != 3) {
		printf("invalid argument(s)\n");
		return -1;
	}
	cmd = get_led_cmd(argv[2]);
	color = get_color(argv[1]);
	if (cmd < 0) {
		printf("invalid command\n");
		return -1;
	}
	if (color == LED_PATTERN_INVALID) {
		printf("invalid color\n");
		return -1;
	}

	/* set leds */
	if(cmd == LED_OFF){
		color = LED_PATTERN_NONE;
	}
	if(trigger_leds(color) != 0){
		printf("failed to set LEDs\n");
		return -1;
	}

	return 0;
}


U_BOOT_CMD(
	led, 3, 1, do_led,
	"Turn on/off leds",
	"[red|green|blue|all] [on|off]\n"
);
