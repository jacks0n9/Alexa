/*
 * part_custom.c - Fall back partition type between EFI and DOS
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
#include <config.h>
#include <part.h>
#include <fat.h>

#define GPT_BLOCK_SIZE 512
#define GPT_PRIMARY_PARTITION_TABLE_LBA	1ULL
#define GPT_HEADER_SIGNATURE_LOW	0x20494645ULL
#define GPT_HEADER_SIGNATURE_HIGH	0x54524150ULL
#define DOS_PART_TBL_OFFSET	0x1be
#define DOS_PART_MAGIC_OFFSET	0x1fe
#define DOS_FS_TYPE_OFFSET	0x52

extern int strncmp(const char * cs, const char * ct, size_t count);

/* based on linux/fs/partitions/efi.h */
typedef unsigned short efi_char16_t;

typedef struct {
	unsigned char b[16];
} efi_guid_t;

typedef struct _gpt_entry_attributes {
	unsigned long long required_to_function :1;
	unsigned long long reserved :47;
	unsigned long long type_guid_specific :16;
}__attribute__ ((packed)) gpt_entry_attributes;

typedef struct _gpt_entry {
	efi_guid_t partition_type_guid;
	efi_guid_t unique_partition_guid;
	unsigned char starting_lba[8];
	unsigned char ending_lba[8];
	gpt_entry_attributes attributes;
	efi_char16_t partition_name[72 / sizeof(efi_char16_t)];
}__attribute__ ((packed)) gpt_entry;

typedef struct _gpt_header {
	unsigned char signature[8];
	unsigned char revision[4];
	unsigned char header_size[4];
	unsigned char header_crc32[4];
	unsigned char reserved1[4];
	unsigned char my_lba[8];
	unsigned char alternate_lba[8];
	unsigned char first_usable_lba[8];
	unsigned char last_usable_lba[8];
	efi_guid_t disk_guid;
	unsigned char partition_entry_lba[8];
	unsigned char num_partition_entries[4];
	unsigned char sizeof_partition_entry[4];
	unsigned char partition_entry_array_crc32[4];
	unsigned char reserved2[GPT_BLOCK_SIZE - 92];
}__attribute__ ((packed)) gpt_header;

static int get_partition_custom_dos(block_dev_desc_t * dev_desc, int part,
		disk_partition_t *info) {
	unsigned char buffer[SECTOR_SIZE];
	/* check if we have a MBR (on floppies we have only a PBR) */
	if (dev_desc->block_read(dev_desc->dev, 0, 1, (void*) buffer) != 1) {
		printf("** Can't read from device %d **\n", dev_desc->dev);
		return -1;
	}

	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55
			|| buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) {
		/* no signature found */
		return -1;
	}

	if (!strncmp((char *) &buffer[DOS_FS_TYPE_OFFSET], "FAT", 3)) {
		/* ok, we assume we are on a PBR only */
		info->start = 0;
	} else {
		info->start = buffer[DOS_PART_TBL_OFFSET + 8]
				| buffer[DOS_PART_TBL_OFFSET + 9] << 8
				| buffer[DOS_PART_TBL_OFFSET + 10] << 16
				| buffer[DOS_PART_TBL_OFFSET + 11] << 24;
		return 0;
	}

	return -1;
}

static int get_partition_custom_efi(block_dev_desc_t * dev_desc, int part,
		disk_partition_t *info) {
	gpt_header gpt_head;
	u32 * ptr = NULL;
	u32 num_partitions = 0;
	u32 first_pte = 0;
	u32 pte_size = 0;
	gpt_entry * pentry;
	unsigned char buffer[SECTOR_SIZE];
	unsigned int i = 0;
	unsigned int offset = 0;

	/* Read GPT Header from device */
	if (dev_desc->block_read(dev_desc->dev, GPT_PRIMARY_PARTITION_TABLE_LBA, 1,
			(void*) &gpt_head) != 1) {
		printf("*** ERROR: Can't read GPT header ***\n");
		return -1;
	}

	ptr = (u32*) &gpt_head.signature;
	if (*ptr++ != GPT_HEADER_SIGNATURE_LOW && *ptr != GPT_HEADER_SIGNATURE_HIGH) {
		printf("custom_efi: invalid signature\n");
		return -1;
	}

	// Get number of partition - Note: only 32-bits of 64-bit value is being obtained
	ptr = (u32*) &gpt_head.num_partition_entries;
	num_partitions = *ptr;
	if (part > num_partitions) {
		printf("custom_efi: invalid partition request\n");
		return -1;
	}

	ptr = (u32*) &gpt_head.sizeof_partition_entry;
	pte_size = *ptr;

	// Get the first usable lba - Note: only 32-bits of 64-bit value is being obtained
	ptr = (u32*) &gpt_head.partition_entry_lba;
	first_pte = *ptr;

	// Get the block containing the entry
	for (i = 1; i <= num_partitions && part != i; ++i) {
		if (!(i % (dev_desc->blksz / pte_size))) {
			first_pte++;
		}
	}

	// Get the partition entry
	if (dev_desc->block_read(dev_desc->dev, first_pte, 1, (void*) buffer)
			!= 1) {
		printf("*** ERROR: Can't read GPT partition entry ***\n");
		return -1;
	}
	// Determine offset into the block buffer, where this part gpt entry is located
	offset = (part - 1) % (dev_desc->blksz / pte_size);
	pentry = (gpt_entry*) &buffer[offset * pte_size];
	// Get the starting lba - Last lower 32-bits only
	ptr = (u32*) &pentry->starting_lba;
	info->start = *ptr;
	// Get the ending lba - Last lower 32-bits only
	ptr = (u32*) &pentry->ending_lba;
	info->size = *ptr + 1 - info->start;
	// Set the block size
	info->blksz = dev_desc->blksz;

	return 0;
}

int get_partition_info_custom(block_dev_desc_t * dev_desc, int part,
		disk_partition_t *info) {
	// Read the EFI partition block. If invalid, fallback to DOS partition
	if (!dev_desc || !info || part < 1) {
		printf("%s: Invalid Argument(s)\n", __func__);
		return -1;
	}

	if (get_partition_custom_efi(dev_desc, part, info) != 0) {
		// No EFI paritition table present
		return get_partition_custom_dos(dev_desc, part, info);
	}

	return 0;
}

void print_part_custom(block_dev_desc_t * dev_desc) {
	/* Do nothing */
	printf("print_part_custom \n");
	return;
}

int test_part_custom(block_dev_desc_t * dev_desc) {
	/* Do nothing */
	return 0;
}