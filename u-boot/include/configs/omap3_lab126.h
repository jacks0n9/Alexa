/*
 * (C) Copyright 2006-2011
 * Texas Instruments.
 * Author :
 *	Benoit Parrot <bparrot@ti.com>
 * Derived from EVM Board by
 *	Manikandan Pillai <mani.pillai@ti.com>
 *
 *
 * Configuration settings for the TI OMAP3 LAB126 board.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARCH_CPU_INIT	1

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMCORTEXA8	1	/* This is an ARM V7 CPU core */
#define CONFIG_OMAP		1	/* in a TI OMAP core */
#define CONFIG_OMAP34XX		1	/* which is a 34XX */
#define CONFIG_OMAP3430		1	/* which is in a 3430 */
//#define CONFIG_OMAP3_EVM	1	/* working with EVM */
#define CONFIG_OMAP3_LAB126	1	/* working with LAB126 Board */
#define BOARD_LATE_INIT 1

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>	/* get chip and board defs */
#include <asm/arch/omap3.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

/* Clock Defines */
#define V_OSCK			19200000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#undef CONFIG_USE_IRQ			/* no support for IRQs */
#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1
#define CONFIG_SERIAL16_TAG		1
#define CONFIG_REVISION16_TAG		1
#define CONFIG_MACADDR_TAG		1
#define CONFIG_BOOTMODE_TAG		1
#define CONFIG_IDME_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(8 << 10)	/* 8 KiB */
						/* Sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (512 << 10))
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* bytes reserved for */
						/* initial data */
#undef CONFIG_CMD_DHCP
/*
 * Hardware drivers
 */
#define CONFIG_WATCHDOG		1
#define CONFIG_SYS_WATCHDOG_TIMEOUT		10	/* 10-seconds */
/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1
#define CONFIG_SERIAL1			1	/* UART1 on OMAP3 EVM */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}
#define CONFIG_MMC			1
#define CONFIG_OMAP3_MMC		1
#define CONFIG_GENERIC_MMC		1
#define CONFIG_DOS_PARTITION		1

/* DDR - I use Micron DDR */
#define CONFIG_OMAP3_MICRON_DDR		1

/* USB
 * Disable CONFIG_MUSB_HCD for Host functionalities MSC, keyboard
 * Disable CONFIG_MUSB_UDD for Device functionalities.
 */
#undef CONFIG_USB_OMAP3
#undef CONFIG_MUSB_HCD
/* #define CONFIG_MUSB_UDC		1 */


/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_EFI_PARTITION        1
#define CONFIG_CMD_EXT4     1
#define CONFIG_FS_EXT4      1
#define CONFIG_CMD_EXT4_WRITE       1
#define CONFIG_EXT4_WRITE   1
#define CONFIG_CMD_FAT		1		/* FAT support			*/
#define CONFIG_CMD_MMC		1		/* MMC support			*/
#define CONFIG_CMD_IDME		1 /* Enable IDME support	*/

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_SYS_NO_FLASH
#define CONFIG_I2C_MULTI_BUS	1
#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
#define CONFIG_DRIVER_OMAP34XX_I2C	1

#ifdef CONFIG_CMD_FAT
#define CONFIG_FS_FAT	1
#define CONFIG_FAT_WRITE	1
#endif

/* Environment information */
#define CONFIG_BOOTDELAY	0

#define CONFIG_BOOTFILE		uImage

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"kernaddr=0x280000\0" \
	"usbtty=cdc_acm\0" \
	"mmcdev=0:1\0" \
	"console=ttyO0,115200n8\0" \
	"bootdevice=nand\0" \
	"config_extra=mem=200M@0x80000000 " \
		"mpurate=1000 quiet " \
		"--default-console none\0" \
	"kernel_file=/boot/uImage\0" \
	"mmc_part=1:6\0" \
    "mount_type=ro\0" \
	"root=/dev/mmcblk0p6\0" \
	"boottype=mmcboot\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=${root} ${mount_type} " \
		"rootfstype=ext3 rootwait " \
		"${config_extra}\0"  \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"ext4load mmc ${mmc_part} ${loadaddr} ${kernel_file};" \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"run ${boottype};" \

/*
 * Miscellaneous configurable options
 */
#undef CONFIG_SYS_LONGHELP		/* undef to save memory */
#undef CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#undef CONFIG_SYS_PROMPT_HUSH_PS2
#define CONFIG_SYS_PROMPT		"uboot > "
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		32	/* max number of command */
						/* args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0) /* default load */
								/* address */

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 << 10)	/* regular stack 128 KiB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4 << 10)	/* IRQ stack 4 KiB */
#define CONFIG_STACKSIZE_FIQ	(4 << 10)	/* FIQ stack 4 KiB */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	(32 << 20)	/* at least 32 MiB */
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/* SDRAM Bank Allocation method */
#define SDRC_R_B_C		1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M
#define PISMO1_ONEN_SIZE		GPMC_SIZE_128M

#define CONFIG_SYS_MAX_FLASH_SECT	520	/* max number of sectors */
						/* on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of flash banks */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_SYS_FLASH_BASE		boot_flash_base

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_SYS_ENV_SECT_SIZE	boot_flash_sec
#define CONFIG_ENV_OFFSET		boot_flash_off
#define CONFIG_ENV_ADDR			boot_flash_env_addr
#define HAVE_BLOCK_DEVICE		1
#define CONFIG_PARTITIONS		1
/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
#ifndef __ASSEMBLY__
extern unsigned int boot_flash_base;
extern volatile unsigned int boot_flash_env_addr;
extern unsigned int boot_flash_off;
extern unsigned int boot_flash_sec;
extern unsigned int boot_flash_type;
#endif

/*
 * Enable Boot retry and keys
 */
#undef CONFIG_AUTOBOOT_PROMPT
#undef CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_STOP_STR	"@#$"
#define CONFIG_RESET_TO_RETRY	1
#define CONFIG_BOOT_RETRY_TIME	3 /* 3 -seconds retry timeout */
#define CONFIG_MENUPROMPT	"*"	/* print '*' for a menu prompt */



#endif /* __CONFIG_H */
