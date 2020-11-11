/*
 * (C) Copyright 2004-2011
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Benoit Parrot <bparrot@ti.com>
 *
 * Derived from Omap3 EVM Board code by
 *	Manikandan Pillai <mani.pillai@ti.com>
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
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
#include <common.h>
#include <spi.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <linux/ctype.h>
#include <mmc.h>
#include <asm/omap_mmc.h>
#include "evm.h"
#include <twl4030.h>
#include <idme.h>
#include <malloc.h>
#include <watchdog.h>
#include "board_id.h"

#define CONTROL_PBIAS_LITE		0x48002520
#define PBIASLITESUPPLYHIGH1 (1<<15)
#define PBIASLITEVMODEERROR1 (1<<11)
#define PBIASLITEPWRDNZ1	 (1<<9)
#define PBIASLITEVMODE1	  (1<<8)
#define PBIASLITESUPPLYHIGH0 (1<<7)
#define PBIASLITEVMODEERROR0 (1<<3)
#define PBIASSPEEDCTRL0	  (1<<2)
#define PBIASLITEPWRDNZ0	 (1<<1)
#define PBIASLITEVMODE0	  (1<<0)
#define BOOTMODE_KERNEL_START		0xCCCC
#define IDME_MAX_SIZE	20 /* bytes */
#define IDME_RESTORE_STR	"restore"
#define IDME_RESTORE_STR_SZ	7
#define IDME_BACKUP_STR	"backup"
#define IDME_BACKUP_STR_SZ	6
#define NOT_DETERMINED	0xFF

static u8 omap3_board_version = 0;
/* board id and serial number. */
static u8 serial_number[MAX_VAR_SIZE+1];
static u8 board_id[MAX_VAR_SIZE+1];

static void late_muxconf_regs(void);
extern void initialize_leds(int board_rev);
extern void turn_off_leds(void);
extern int get_partition_num(const char *);
extern char * get_partition_uuid(const char *);


int setup_board_info(void)
{
#if defined(CONFIG_CMD_IDME)
	if (idme_get_var("pcbsn", (char *) board_id, sizeof(board_id)))
#endif
	{
		/* not found: clean up garbage characters. */
		memset(board_id, 0, sizeof(board_id));
	}

#if defined(CONFIG_CMD_IDME)
	if (idme_get_var("dsn", (char *) serial_number, sizeof(serial_number)))
#endif
	{
		/* not found: clean up garbage characters. */
		memset(serial_number, 0, sizeof(serial_number));
	}

	return 0;
}

u8 get_omap3_board_rev(void)
{
	return omap3_board_version;
}

int board_is_p(void) {
	return (omap3_board_version & BOARD_P_ID);
}

int board_info_valid(u8 *info, int len) {
	int i = 0;
	if (!info) {
		return 0;
	}

	while (info[i] != '\0' && i < len) {
		if ((info[i] < '0') && (info[i] > '9') && (info[i] < 'A')
				&& (info[i] > 'Z'))
			return 0;

		i++;
	}

	return 1;
}

/*************************************************************************
 * get_board_id() - setup to pass kernel board revision information
 *	  16-byte alphanumeric containing the board revision.
 *************************************************************************/
const u8 *get_board_id(void)
{
	if (!board_info_valid(board_id, sizeof(board_id)))
		return (u8 *) "0000000000000000";
	else
		return board_id;
}

/*************************************************************************
 * get_board_serial() - setup to pass kernel serial number information
 *	  return: alphanumeric containing the serial number.
 *************************************************************************/
const u8 *get_board_serial(void)
{
	if (!board_info_valid(serial_number, sizeof(serial_number)))
		return (u8 *) "0000000000000000";
	else
		return serial_number;
}

/* belongs in utility.c */
/* simple_itoa() was lifted from boa-0.93.15 */
static char *simple_itoa(unsigned int i)
{
	/* 21 digits plus null terminator, good for 64-bit or smaller ints */
	static char local[22];
	char *p = &local[21];
	*p-- = '\0';
	do {
		*p-- = '0' + i % 10;
		i /= 10;
	} while (i > 0);
	return p + 1;
}

/*
 * Some constant strings describing the values expected in
 * bootmode and knowngood idme values for reliable boot.
 */
const char main_a_label[] = "main-A";
const char main_b_label[] = "main-B";

/*************************************************************************
 * prepare_eMMC_boot() - Setup boot parameters for eMMC device
 *	return: None
 *************************************************************************/
static void prepare_eMMC_boot(void)
{
	char boot_mode[20];
	char known_good[20];
	char retrycount[20];
	char part[16];
	char root[64];
	int retry;
	int part_num = 0;
	int phy_mmc_dev = 1; /* Physical device: 0 - SDCard; 1-eMMC */
	int logical_mmc_dev = 0; /* Index of detected MMC slot. 0 = eMMC if SDcard absent */

	if (idme_get_var("bootmode", boot_mode, sizeof(boot_mode)))
	{
		printf("failed to get bootmode var - default to main-A\n");
		// TODO: Handle corruption...this should always succeed !
		memcpy(boot_mode, main_a_label, sizeof(main_a_label));
	}

	if (idme_get_var("otaretrycount", retrycount, sizeof(retrycount)))
	{
		printf("Unable to get retry count\n");
		// default to retry of 1
		memcpy(retrycount, "1\0", 2);
	}

	retry = simple_strtoul(retrycount, NULL, 10);
	if (retry >= 0) {
		// Got a valid retry

		// Retrieve the known good partition
		if (idme_get_var("knowngood", known_good, sizeof(known_good))) {
			printf("Unable to get knowngood partition\n");
		}
		else
		{
			int knowngoodisboot = strcmp(known_good, boot_mode) == 0 ? 1 : 0;
			if (!knowngoodisboot && retry > 0) {
				// Update the retry count
				if (idme_update_var("otaretrycount", (const char*)simple_itoa(--retry)))
					printf("Failed to update retry count \n");
			}
			else if (!knowngoodisboot && retry == 0) {
				// swap to knowngood
				if (idme_update_var("bootmode", known_good)) {
					printf(" Failed to switch to known good partition \n");
				}

				if (idme_update_var("knowngood", boot_mode)) {
					printf(" Failed update of known good partition \n");
				}

				memcpy(boot_mode, known_good, sizeof(boot_mode));

				if (idme_update_var("otaretrycount", "1")) {
					printf(" Failed to switch to known good partition \n");
				}
			}
		}
	}

	// Set the boot type for mmc
	setenv("boottype", "mmcboot");

	memset(part, 0, sizeof(part));

	if (mmc_get_dev(0) != 0)
		logical_mmc_dev = 1; // SD card present, prepare to boot from eMMC

	part_num = get_partition_num(boot_mode);
	if (part_num <= 0) {
		printf("Invalid partition number\n");
		// Defaults will be used
		return;
	}

	char * uuid = get_partition_uuid(boot_mode);
	if (uuid) {
		if (sprintf(root, "UUID=%s", uuid) < 0) {
			free(uuid);
			return;
		}
		free(uuid);
	}
	else {
		printf("Invalid uuid. Booting by block dev \n");
		if (sprintf(root, "/dev/mmcblk%dp%d", logical_mmc_dev, part_num) < 0) {
			return;
		}
	}

	if (sprintf(part, "%d:%d", phy_mmc_dev, part_num) < 0)
		return;

	setenv("mmc_part", part);
	setenv("root", root);

	if (!strncmp(boot_mode, "diags", 5)) {
		// set mount_type to RW for diags and factory
		setenv("mount_type", "rw");
	}
	printf("booting ...%s\n", boot_mode);
	return;
}

/*************************************************************************
 * check_boot_mode() - Determine which partition and device to boot
 *	return: None
 *************************************************************************/
void check_boot_mode(void)
{
	prepare_eMMC_boot();
}

void serial_loopback_mode(void)
{
	const int my_gpio = 188;
	const int my_gpio_lo = 0;
	const int my_gpio_hi = 1;
	const int gpio_settle_us = 50000; // 50 ms
	const int magic_itrs = 3;
	#define TSTC_TIMEOUT_MS 100
	const char magic[] = {14, 25, 165, 52, 166, 235, 227, 175, 75, 162};
	const int magic_len = ARRAY_SIZE(magic);
	int i, j;
	ulong st;

	if (!board_is_p()) {
		return;
	}

	for (st = get_timer(0); get_timer(st) < TSTC_TIMEOUT_MS;) {
		char c;

		if (!tstc()) {
			continue;
		}

		c = getc();
	}

	omap_request_gpio(my_gpio);
	omap_set_gpio_direction(my_gpio, my_gpio_lo);
	omap_set_gpio_dataout(my_gpio, my_gpio_lo);

	udelay(gpio_settle_us);

	for (i = 0; i < magic_itrs; i++) {
		for (j = 0; j < magic_len; j++) {
			putc(magic[j]);
		}
	}

	for (i = 0, j = 0, st = get_timer(0);
			i < magic_len*magic_itrs
			&& j < magic_len
			&& get_timer(st) < TSTC_TIMEOUT_MS;
		 i++, j++) {
		char c;

		if (!tstc()) {
			i--;
			j--;
			continue;
		}

		c = getc();
		if (magic[j] != c) {
			j = 0;
		}
	}

	if (j == magic_len) {
		omap_set_gpio_dataout(my_gpio, my_gpio_hi);
		udelay(gpio_settle_us);
		while (tstc()) { (void) getc(); }
		printf("\nELdetected\n");
		return;
	}

	printf("\nSEL NOT detected\n");
	idme_update_var("postmode", "usb-uart-enable");
}
#define PMIC_LDO_REG_VDIG 0x30
#define PMIC_LDO_REG_VDAC 0x37
#define PMIC_I2C_ADDRESS 0x2d
#define PMIC_LDO_STATE_MASK 0x3

#if defined(CONFIG_CMD_IDME)
static int pmic_ldo_disable(int ldo_reg)
{
	int ret = 0;
	uchar byte = 0;

	ret = i2c_read(PMIC_I2C_ADDRESS, ldo_reg, 1, &byte, 1);
	if (ret != 0) {
		puts("Error reading PMIC.\n");
		return ret;
	}

	byte &= ~PMIC_LDO_STATE_MASK;
	ret = i2c_write(PMIC_I2C_ADDRESS, ldo_reg, 1, &byte, 1);
	if (ret != 0) {
		puts("Error writing PMIC reg\n");
		return ret;
	}

	return ret;
}

static int set_pmic_powersave(void)
{
	int ret = 0;
	int i;
	uchar byte = 0;

	/* Disable programmable LDOs */
	for (i = PMIC_LDO_REG_VDIG; i <= PMIC_LDO_REG_VDAC; i++) {
		ret = pmic_ldo_disable(i);
		if (ret != 0) {
			return ret;
		}
	}

	/* Turn on sleep mode in the PMIC */
	byte = 0x12;
	ret = i2c_write(PMIC_I2C_ADDRESS, 0x3f, 1, &byte, 1);
	if (ret != 0) {
		puts("Error writing the chip.\n");
		return ret;
	}

	return ret;
}

/**
 * Check to see if low-power ("off") mode is specified
 */
static void check_powersave_mode(void)
{
	char powersave[IDME_MAX_SIZE];

	if (board_is_p()) {
		printf("Powersave mode not supported\n");
		return;
	}

	if (idme_get_var("powersave", powersave, sizeof(powersave))) {
		printf("failed to get powersave var\n");
		return;
	}

	// Clear power save mode
	idme_update_var("powersave", NULL);

	if (strncmp(powersave, "on", 2) == 0) {
		printf("entering powersave mode\n");

		initialize_leds(get_omap3_board_rev());
		turn_off_leds();
		I2C_SET_BUS(0);
		udelay(5000000);

		set_pmic_powersave();

		/* Loop until power removed */
		while (1) {
			udelay(100000);
			watchdog_reset();
		}
	}
}
#endif

/**
 * Run any Power-On-Self-Tests (POST) specified
 */
static void check_post_mode(void)
{
#if defined(CONFIG_CMD_IDME)
	char post_mode[IDME_MAX_SIZE];

	if (idme_get_var("postmode", post_mode, sizeof(post_mode)))
	{
		printf("failed to get postmode var\n");
		return;
	}

	// Clear the postmode
	idme_update_var("postmode", NULL);

	if (!strncmp(post_mode, IDME_RESTORE_STR, IDME_RESTORE_STR_SZ)) {
		if (idme_restore()) {
			printf("failed to restore idme\n");
			return;
		}
		printf("Restored IDME data !\n");
	} else if (!strncmp(post_mode, IDME_BACKUP_STR, IDME_BACKUP_STR_SZ)) {
		if (idme_backup()) {
			printf("failed to backup idme\n");
			return;
		}
		printf("Backed up IDME data !\n");
	}

#endif
}

static void omap3_board_get_revision(void)
{
	omap_request_gpio(42);
	omap_request_gpio(41);
	omap_request_gpio(40);
	omap_request_gpio(39);
	omap_request_gpio(29);
	omap_request_gpio(28);
	omap_request_gpio(27);

	omap_set_gpio_direction(42, 1);
	omap_set_gpio_direction(41, 1);
	omap_set_gpio_direction(40, 1);
	omap_set_gpio_direction(39, 1);
	omap_set_gpio_direction(29, 1);
	omap_set_gpio_direction(28, 1);
	omap_set_gpio_direction(27, 1);

	omap3_board_version = omap_get_gpio_datain(27) |
			  omap_get_gpio_datain(28) << 1 |
			  omap_get_gpio_datain(29) << 2 |
			  omap_get_gpio_datain(39) << 3 |
			  omap_get_gpio_datain(40) << 4 |
			  omap_get_gpio_datain(41) << 5 |
			  omap_get_gpio_datain(42) << 6;

	/* If lower 3-bits are NOT = 0x2 or 0x1, then ignore the high 4-bits */
	if ((omap3_board_version & 0x7) != 0x2 &&
			(omap3_board_version & 0x7) != 0x1)
		omap3_board_version &= 0x7;

	/* Temp hack for EVT3-0 proto */
	if (0x02 == omap3_board_version)
		omap3_board_version = BOARD_IS_EVT_3_0;

	omap_free_gpio(42);
	omap_free_gpio(41);
	omap_free_gpio(40);
	omap_free_gpio(39);
	omap_free_gpio(29);
	omap_free_gpio(28);
	omap_free_gpio(27);


	printf("OMAP3 Lab126 Rev: 0x%x\n", omap3_board_version);
}


/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3EVM;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */

int misc_init_r(void)
{
	uchar	byte = 0;
	int flash_dev = EMMC_FLASH_DEVICE;

	/* determine omap3evm revision */
	omap3_board_get_revision();

#ifdef CONFIG_DRIVER_OMAP34XX_I2C
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif

	dieid_num_r();

#if defined(CONFIG_CMD_IDME)
	/* Initialize idme */
	if (idme_init(flash_dev) != 0)
		printf("IDME init failed\n");

	check_powersave_mode();
#endif

	if (is_cpu_family(CPU_OMAP36XX)) {
// Might need to adapt to the TPS65910 if we need to raise mpu power in u-boot
// We'll used default for now and let Linux handle it later.
//			twl4030_power_mpu_init();

		/* Mask the PWR_HOLD Interrupt
		 * EVT2.1 and prior, physically connect the RST_PWR_ON to the PWR_HOLD.
		 * Prevent the PWR_HOLD from keeping the device in the OFF state
		 */
		byte = 0x02; /* default reset value for INT_MSK_REG */
		if (i2c_read(0x2d, 0x51, 1, &byte, 1) != 0)
			puts("Error reading the chip.\n");
		byte |= 0x10; /*PWRHOLD_IT_MASK*/
		if (i2c_write(0x2d, 0x51, 1, &byte, 1) != 0)
			puts("Error writing the chip IT mask.\n");

		/* Initialize the PMIC to desired state */
		/* 1. Set Vmmc to 3.0V */
		byte = 0x9;
		if (i2c_write(0x2d, 0x35, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */
		/* 2. Set Vaux33 to 2.0V */
		byte = 0x5;
		if (i2c_write(0x2d, 0x34, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */
		/* 3. Set Vdd3 to 5.0V */
		byte = 0x5;
		if (i2c_write(0x2d, 0x27, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		/* 4. Set Vaux1 to 2.85V */
		byte = 0xD;
		if (i2c_write(0x2d, 0x32, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */
		/* 5. Set Vaux2 to 3.0V */
		byte = 0x9;
		if (i2c_write(0x2d, 0x33, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */
		/* 6. Set Vpll to 1.8V */
		byte = 0x9;
		if (i2c_write(0x2d, 0x36, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */
		/* 7. Set Vdig1 to 1.8V */
		byte = 0x9;
		if (i2c_write(0x2d, 0x30, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */
		/* 8. Set Vdig2 to 1.8V */
		byte = 0xd;
		if (i2c_write(0x2d, 0x31, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */
		/* 9. Set Vdac to 2.85V */
		byte = 0xD;
		if (i2c_write(0x2d, 0x37, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */

		/* 10. Enable SR and DEV_ON from the control interface*/
		byte = 0x54;
		if (i2c_write(0x2d, 0x3f, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */

		/* 11. Set Vdd1 to 1.36V to allow MPU to 1Ghz*/
		byte = 0x3f;
		if (i2c_write(0x2d, 0x22, 1, &byte, 1) != 0)
			puts("Error writing the chip.\n");
		udelay(11000); /* wait ~10ms for the write to complete */

		/* power up gpio_129 (and other) */
		writel(0x40, CONTROL_WKUP_CTRL);

		/* Setup PBIAS_LITE properly even during NAND boot so that GPIO 129 is fully functional */
		writel(readl(CONTROL_PBIAS_LITE) | PBIASLITESUPPLYHIGH1 |PBIASLITEPWRDNZ1, CONTROL_PBIAS_LITE);

		/* Initialize and enable the LED pattern */
		initialize_leds(get_omap3_board_rev());
	}

	return 0;
}

int board_late_init(void)
{
	late_muxconf_regs();
#if defined(CONFIG_CMD_IDME)
	check_post_mode();
	idme_check_update();
#endif
	setup_board_info();
	check_boot_mode();

	omap3_board_get_revision();
	serial_loopback_mode();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_BOARD();

#ifdef CONFIG_KS8851SNL
	/* SPI2_CLK */
	MUX_VAL(CP(MCSPI1_CLK),		(IEN  | PTD | DIS | M0)) /*McSPI1_CLK */\
	/* SPI2_SIMO */
	MUX_VAL(CP(MCSPI1_SIMO),	(IEN  | PTU | EN  | M0)) /*McSPI1_SIMO */\
	/* SPI2_SOMI */
	MUX_VAL(CP(MCSPI1_SOMI),	(IEN  | PTU | EN  | M0)) /*McSPI1_SOMI */\
	/* SPI2_CS0 */
	MUX_VAL(CP(MCSPI1_CS0),		(IDIS | PTD | DIS | M0)) /*McSPI1_CS0 */\
	MUX_VAL(CP(MCSPI1_CS1),		(IEN  | PTD | DIS | M0)) /*McSPI1_CS1*/\
	;
#endif
}

static void late_muxconf_regs(void)
{
	/* gpt_11_pwm_evt - Network LED */
	MUX_VAL(CP(GPMC_NCS6),	(IDIS  | PTD | EN | M3)) \
	/* gpt_8_pwm_evt - Power LED*/
	MUX_VAL(CP(GPMC_NCS7),	(IDIS  | PTD | EN  | M3))
	;
}

/*
 * Routine: set_bootmode
 * Description: Store the bootmode in OMAP Save Restore Mem
 */
void set_bootmode(uint16_t boot_mode)
{
	u_int32_t val;

	val = readl(SCM_BASE + SCM_SCRATCHPAD_OFFSET);
	writel((val & ~0xFFFF) | boot_mode , SCM_BASE + SCM_SCRATCHPAD_OFFSET);
}

/*
 * Routine: get_bootmode
 * Description: Obtain the bootmode in OMAP Save Restore Mem
 */
uint16_t get_bootmode(void)
{
	u_int32_t val;
	/* Read bootmode from scratch pad */
	val = readl(SCM_BASE + SCM_SCRATCHPAD_OFFSET);

	return val & (0xFFFF);
}

/*
 * Routine: set_kernel_bootmode
 * Description: Store the bootmode in OMAP Save Restore Mem
 */
void set_kernel_bootmode(void)
{
	set_bootmode(BOOTMODE_KERNEL_START);
}

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
#ifdef CONFIG_KS8851SNL
	struct gpio *gpio1_base = (struct gpio *)OMAP34XX_GPIO1_BASE;
#endif

#ifdef CONFIG_SMC911X
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;
	struct gpio *gpio3_base = (struct gpio *)OMAP34XX_GPIO3_BASE;
	/* Configure GPMC registers */
	writel(NET_GPMC_CONFIG1, &gpmc_cfg->cs[3].config1);
	writel(NET_GPMC_CONFIG2, &gpmc_cfg->cs[3].config2);
	writel(NET_GPMC_CONFIG3, &gpmc_cfg->cs[3].config3);
	writel(NET_GPMC_CONFIG4, &gpmc_cfg->cs[3].config4);
	writel(NET_GPMC_CONFIG5, &gpmc_cfg->cs[3].config5);
	writel(NET_GPMC_CONFIG6, &gpmc_cfg->cs[3].config6);
	writel(NET_GPMC_CONFIG7, &gpmc_cfg->cs[3].config7);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

	// OMAP3EVM_BOARD_GEN_2
	/* Make GPIO 16 as output pin */
	writel(readl(&gpio1_base->oe) & ~(GPIO16), &gpio1_base->oe);

	/* Now send a pulse on the GPIO pin */
	writel(GPIO16, &gpio1_base->setdataout);
	udelay(100000);
	writel(GPIO16, &gpio1_base->cleardataout);
	udelay(100000);
	writel(GPIO16, &gpio1_base->setdataout);

#endif

#ifdef CONFIG_KS8851SNL
	/* GPIO16 is connected to KS8851 resetn pin */
	/* Make GPIO 16 as output pin */
	writel(readl(&gpio1_base->oe) & ~(GPIO16), &gpio1_base->oe);
	writel(readl(&gpio1_base->oe) & (GPIO15), &gpio1_base->oe); // input
	writel(readl(&gpio1_base->oe) & ~(GPIO14), &gpio1_base->oe);

	/* Now send a reset pulse on the GPIO pin */
	writel(GPIO16, &gpio1_base->setdataout);
	udelay(1);
	writel(GPIO16, &gpio1_base->cleardataout);
	udelay(1);
	writel(GPIO16, &gpio1_base->setdataout);

#endif
}

/* If BT MAC is available, then set the ETH MAC to the same thing,
 * except also set the "locally administered" bit.
 */
static void set_enetaddr(void)
{
#if defined(CONFIG_CMD_IDME)
	u8 btmac[MAX_VAR_SIZE+1]; /* ASCII representation */
	u8 bin_mac[6]; /* binary representation of MAC-48 */
	unsigned n;

	if (idme_get_var("btmac", (char *)btmac, sizeof(btmac))) {
		return;
	}

	/* Convert to binary, with validation */
	for (n = 0 ; n < ARRAY_SIZE(bin_mac) ; ++n) {
		char hex[] = "XX";
		hex[0] = btmac[2*n];
		hex[1] = btmac[2*n + 1];
		if (!isxdigit(hex[0]) || !isxdigit(hex[1])) {
			return;
		}
		bin_mac[n] = simple_strtoul(hex, NULL, 16);
	}
	bin_mac[0] |= (1 << 1); /* locally administered bit */

	/* Convert back to ASCII */
	sprintf((char*)btmac, "%02X%02X%02X%02X%02X%02X",
		bin_mac[0], bin_mac[1], bin_mac[2],
		bin_mac[3], bin_mac[4], bin_mac[5]);

	/* The standard net driver looks for enetaddr to set the MAC.
	 * See net/eth.c
	 */
	setenv("enetaddr", (char *)btmac);
	setenv("ethaddr", (char *)btmac);

	return;
#endif /* defined(CONFIG_CMD_IDME) */
}

int board_eth_init(bd_t *bis)
{
	int num = 0;

	set_enetaddr();

#ifdef CONFIG_KS8851SNL
	if (!ks8851snl_initialize(KS_SPI_BUS, KS_CS_NUM, KS_SPI_CLOCK, KS_SPI_MODE))
		num++;
#endif

#ifdef CONFIG_SMC911X
	if (!smc911x_initialize(0, CONFIG_SMC911X_BASE))
		num++;
#endif

	return num;
}

int board_mmc_init(bd_t *bis)
{
	int ret = omap_mmc_init(0, 0, 0, -1, -1);
	ret |= omap_mmc_init(1, 0, 0, -1, -1);

	return ret;
}

u32 get_cpu_family(void)
{
	return CPU_OMAP36XX;
}

