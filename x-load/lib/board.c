/*
 * Copyright (C) 2005 Texas Instruments.
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>

extern int misc_init_r (void);
extern u32 get_mem_type(void);
extern void set_bootmode(uint16_t boot_mode);
extern uint16_t get_bootmode(void);
extern u_int32_t omap3_get_time_ms(void);

// Board data
#define BOARD_NAND_MFGDATA_BLOCK_OFFSET		0x200000
#define BOARD_PCBSN_LENGTH		16
#define BOARD_PCBSN_OFFSET		0x70
#define BOARD_POSTMODE_LENGTH		16
#define BOARD_POSTMODE_OFFSET		0x160

/* Boot Mode */
/* Boot the recovery u-boot */
#define BOOTMODE_RECOVERY		0xAAAA
#define BOOTMODE_MAIN		0
/** Alphanumeric PCBSN
 * This contains the Tattoo and Revision code for the board.
 * Format: TTTRRXPYYWWD####, where
 *         TTT = 3 digit Tattoo Code
 *         RR  = 2 digit Board Revision Number
 *         Remaining 11-digits are unique per device
 */
char g_pcbsn[BOARD_PCBSN_LENGTH];
/** Ascii Postmode
 * This contains the mode for Power On Self Test (POST)
 * 'normal' - Perform basic POST tests
 * 'slow'   - Perform more extensive POST tests
 */
char g_postmode[BOARD_POSTMODE_LENGTH];


static void get_board_info(void);
static void run_post(void);
static int mmc_boot_recovery(uchar * buf);

#ifdef CFG_PRINTF
int print_info(void)
{
	printf("\n\nTexas Instruments X-Loader 1.51 ("
			__DATE__ " - " __TIME__ ")\n");
	return 0;
}
#endif

typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
		cpu_init,		/* basic cpu dependent setup */
		board_init,		/* basic board dependent setup */
#ifdef CFG_PRINTF
		serial_init,		/* serial communications setup */
		print_info,
#endif
		NULL,
};

void start_armboot (void)
{
	init_fnc_t **init_fnc_ptr;
	int block, page;
	u32 num_blocks = 0;
	u32 num_pages_remaining = 0;
	uchar *buf;
	u32 uboot_start = NAND_MAIN_UBOOT_START;
	u32 uboot_end = NAND_MAIN_UBOOT_END;
	int partition_num = 1;
	int mmc_dev = 0; /* SD Card */

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

	misc_init_r();
	buf =  (uchar*) CFG_LOADADDR;

	/* Clear first byte at CFG_LOADADDR
	 * Memory write appears to be needed to allow reboot
	 *
	 * This seems to be a result of the LPDDR going into self-refresh on soft boot
	 * Upon memory access, the mode is automatically exited.
	 * Need investigation !!
	 */
	buf[0] = 0;

	/* Always first try mmc without checking boot pins */
#ifndef CONFIG_OMAP3LAB126
	if ((get_mem_type() == MMC_ONENAND) || (get_mem_type() == MMC_NAND))
#endif	/* CONFIG_OMAP3LAB126 */
		buf += mmc_boot(buf, mmc_dev, partition_num);
		if (buf == (uchar *)CFG_LOADADDR) {
			// No boot file in partition 1
			//  Attempt to boot partition 3 of the SD-card
			partition_num = 3;
			buf += mmc_boot(buf, mmc_dev, partition_num);
		}

	if (buf == (uchar *)CFG_LOADADDR) {

		if (get_mem_type() == GPMC_NAND) {
			// Board specific nand init method
			nand_init();

#ifdef CONFIG_OMAP3LAB126
			// Get the pcbsn / board id etc. from nand
			get_board_info();

			// Run post mode test as required
			run_post();

			// Get the bootmode
			if (get_bootmode() == BOOTMODE_RECOVERY) {
				// Prepare to boot the recovery image
				uboot_start = NAND_REC_UBOOT_START;
				uboot_end = NAND_REC_UBOOT_END;
#ifdef CFG_PRINTF
				printf("\n ******** Booting recovery image ********\n");
#endif
			}
#endif //CONFIG_OMAP3LAB126

			num_blocks = CONFIG_UBOOT_MAX_SIZE / NAND_BLOCK_SIZE;
			// Whatever is left in pages
			num_pages_remaining =
					(CONFIG_UBOOT_MAX_SIZE-(num_blocks*NAND_BLOCK_SIZE)) / NAND_PAGE_SIZE;
#ifdef CFG_PRINTF
			printf("Booting from nand. . .(Max u-boot=%d-bytes (blocks=%d, pages=%d)\n",
					CONFIG_UBOOT_MAX_SIZE, num_blocks, num_pages_remaining);
#endif
			for (block = uboot_start; block < uboot_end && num_blocks; block += NAND_BLOCK_SIZE) {
				if (!nand_read_block(buf, block)) {
					buf += NAND_BLOCK_SIZE; /* advance buf ptr */
					num_blocks--;
				}
			}

			// Read any pages left
			// block should be pointing to the next block
			for ( page = 0; page < num_pages_remaining; ++page) {
				if (!nand_read_block_page(buf, block, page*NAND_PAGE_SIZE)) {
					buf += NAND_PAGE_SIZE; /* advance buf ptr */
				}
			}
		}

		if (get_mem_type() == GPMC_ONENAND) {
			// Board specific nand init method
			nand_init();
#ifdef CFG_PRINTF
			printf("Booting from onenand . . .\n");
#endif
			for (block = ONENAND_START_BLOCK; block < ONENAND_END_BLOCK; block++) {
				if (!onenand_read_block(buf, block))
					buf += ONENAND_BLOCK_SIZE;
			}
		}

		if ((get_mem_type() == MMC_ONENAND) || (get_mem_type() == MMC_NAND)) {
#ifdef CFG_PRINTF
			printf("Booting from eMMC . . .\n");
#endif
			// Get the bootmode
			if (get_bootmode() == BOOTMODE_RECOVERY)
				buf += mmc_boot_recovery(buf);
			else {
				mmc_dev = 1; /* eMMC */
				/* Non-recovery u-boot.bin is located in partition-3 */
				partition_num = 3;
				buf += mmc_boot(buf, mmc_dev, partition_num);
				if (buf == (uchar *)CFG_LOADADDR)
					buf += mmc_boot_recovery(buf);
			}
		}
	}

#if defined (CONFIG_AM3517EVM)
	/*
	 * FIXME: Currently coping uboot image,
	 * ideally we should leverage XIP feature here
	 */
	if (get_mem_type() == GPMC_NOR) {
		int size;
		printf("Booting from NOR Flash...\n");
		size = nor_read_boot(buf);
		if (size > 0)
			buf += size;
	}
#endif

#ifdef CONFIG_OMAP3LAB126
	// Set the bootmode to recovery
	set_bootmode(BOOTMODE_RECOVERY);
	// Enable the watchdog timer before running U-boot
	watchdog_set_timeout(CONFIG_WATCHDOG_TIMEOUT_SEC);
	watchdog_enable();
#endif //CONFIG_OMAP3LAB126

	if (buf == (uchar *)CFG_LOADADDR)
		hang();

	/* go run U-Boot and never return */
	printf("Starting OS Bootloader...(time = %d ms)\n",omap3_get_time_ms());
	((init_fnc_t *)CFG_LOADADDR)();

	/* should never come here */
}

/**
 * Boot the recovery u-boot.bin image
 * return size of u-boot.bin
 */
static int mmc_boot_recovery(uchar * buf)
{
	int partition_num = 2; /* Recovery partition */
	int mmc_dev = 1; /* eMMC  */

#ifdef CFG_PRINTF
	printf("\n ******** Booting recovery image ********\n");
#endif

	return mmc_boot(buf, mmc_dev, partition_num);
}

/*
 * Read the PCB serial number from serial
 */
void get_PCB_info_from_serial(char *pcbsn, unsigned int len)
{
	char c = ' ';
	unsigned int count = 0;
#ifdef CFG_PRINTF
	printf("Enter PCB Serial Number > ");
#endif
	while (c != '\n' && c != '\r') {
		c = serial_getc();
		if (count < len) {
			pcbsn[count] = c;
			printf("%c", pcbsn[count]);
		}
		count++;
	}

#ifdef CFG_PRINTF
	printf("\n");
#endif
}

int validate_pcbsn(const char *info, unsigned int len)
{
	int ret = -1;
	unsigned int i = 0;

	if (!info || !len) {
		return ret;
	}

	while (info[i] != '\0' && i < len) {
		if ((info[i] >= '0') && (info[i] <= '9')) {
			// Valid numerical data between 0 - 9
		} else if ((info[i] >= 'A') && (info[i] <= 'Z')) {
			// Valid alphabetical data between A - Z
			// No need to support lower case, since it's not used
		} else {
			// Non alpanumeric character, return error
			return ret;
		}
		i++;
	}

	return (i == len) ? 0 : ret;
}

static void get_board_info()
{
	unsigned char mfgdata_buf[NAND_PAGE_SIZE];
	unsigned int i = 0;

	// Invalidate the global data for board info
	g_pcbsn[0] = '\0';
	g_postmode[0] = '\0';

	// This function assumes that the nand has been initialized
	// Read the page containing IDME info
	if(nand_read_block_page(mfgdata_buf, BOARD_NAND_MFGDATA_BLOCK_OFFSET, 0) == 0) {
		// Extract the pcb serial number
		for (i = 0; i < BOARD_PCBSN_LENGTH; ++i) {
			g_pcbsn[i] = mfgdata_buf[BOARD_PCBSN_OFFSET+i];
		}

		// Extract the postmode
		for (i = 0; i < BOARD_POSTMODE_LENGTH; ++i) {
			g_postmode[i] = mfgdata_buf[BOARD_POSTMODE_OFFSET+i];
		}
	}

	// Check that we have a valid pcb serial number
	// Prevent booting until a valid pcbsn is provided
	while (validate_pcbsn(g_pcbsn, BOARD_PCBSN_LENGTH) != 0) {
		// Invalid pcbsn
		// Request input from the serial
		get_PCB_info_from_serial(g_pcbsn, BOARD_PCBSN_LENGTH);
	}
}

static void run_post()
{
	int ret = 1;
	u_int32_t start_time = omap3_get_time_ms();
	if (strncmp(g_postmode, "normal", 6) == 0) {
		// Perform standard Power On Self Test
#ifdef CFG_PRINTF
		printf("\nRunning fast POST test...\n");
#endif
		ret = memory_post_test (0);
	} else if (strncmp(g_postmode, "slow", 4) == 0) {
		// Perform extensive Power On Self Test
#ifdef CFG_PRINTF
		printf("\nRunning full POST test...\n");
#endif
		ret = memory_post_test (1);
	} else {
		// Initial power up / factory
		// Do nothing
	}
	u_int32_t end_time = omap3_get_time_ms();
	if (0 == ret) {
#ifdef CFG_PRINTF
		printf("\nPASSED \n");
		printf ("\nTime Taken = %d ms \n", (end_time - start_time));
#endif
	}

}

void hang (void)
{
	/* call board specific hang function */
	board_hang();

	/* if board_hang() returns, hange here */
	printf("X-Loader hangs\n");
	for (;;);
}
