/*
 * Copyright (c) 2010, The Android Open Source Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Neither the name of The Android Open Source Project nor the names
 *    of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <common.h>
#include <mmc.h>
#include <malloc.h>
#include <fat.h>

#define EFI_VERSION 0x00010000
#define EFI_ENTRIES 128
#define EFI_NAMELEN 36
#define FASTBOOT_PTENTRY_FLAGS_WRITE_ENV              0x00000400

#define UUID_SIZE	16
/* Android-style flash naming */
typedef struct fastboot_ptentry fastboot_ptentry;

/* flash partitions are defined in terms of blocks
 ** (flash erase units)
 */
struct fastboot_ptentry {

	/* The logical name for this partition, null terminated */
	char name[16];
	/* The start wrt the nand part, must be multiple of nand block size */
	unsigned int start;
	/* The length of the partition, must be multiple of nand block size */
	unsigned int length;
	/* Controls the details of how operations are done on the partition
	 See the FASTBOOT_PTENTRY_FLAGS_*'s defined below */
	unsigned int flags;
};

static const u8 partition_type[UUID_SIZE] = { 0xa2, 0xa0, 0xd0, 0xeb, 0xe5,
		0xb9, 0x33, 0x44, 0x87, 0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7, };

/** Random Partition UUID */
static const u8 random_uuid[UUID_SIZE] = { 0xff, 0x1f, 0xf2, 0xf9, 0xd4, 0xa8,
		0x0e, 0x5f, 0x97, 0x46, 0x59, 0x48, 0x69, 0xae, 0xc3, 0x4e, };

/** Xloader Partition UUID - e127d637-263d-4eb2-973c-73345e25db99*/
static const u8 xloader_uuid[UUID_SIZE] = { 0x37, 0xd6, 0x27, 0xe1, 0x3d, 0x26,
		0xb2, 0x4e, 0x97, 0x3c, 0x73, 0x34, 0x5e, 0x25, 0xdb, 0x99, };

/** Recovery Partition UUID - 23adc608-b7b5-42bf-98f8-7db854629358*/
static const u8 recovery_uuid[UUID_SIZE] = { 0x08, 0xc6, 0xad, 0x23, 0xb5, 0xb7,
		0xbf, 0x42, 0x98, 0xf8, 0x7d, 0xb8, 0x54, 0x62, 0x93, 0x58, };

/** Boot Partition UUID - 67bba12a-5bd3-45e0-b5d6-216006110528*/
static const u8 boot_uuid[UUID_SIZE] = { 0x2a, 0xa1, 0xbb, 0x67, 0xd3, 0x5b,
		0xe0, 0x45, 0xb5, 0xd6, 0x21, 0x60, 0x06, 0x11, 0x05, 0x28, };

/** Idme Partition UUID - 4bde7e1f-3119-4868-905d-af6ddcc340d1*/
static const u8 misc_uuid[UUID_SIZE] = { 0x1f, 0x7e, 0xde, 0x4b, 0x19, 0x31,
		0x68, 0x48, 0x90, 0x5d, 0xaf, 0x6d, 0xdc, 0xc3, 0x40, 0xd1, };

/** Mfg/factory Partition UUID - 401f679a-a643-485e-b546-92bc7ebaed1b*/
static const u8 mfg_uuid[UUID_SIZE] = { 0x9a, 0x67, 0x1f, 0x40, 0x43, 0xa6,
		0x5e, 0x48, 0xb5, 0x46, 0x92, 0xbc, 0x7e, 0xba, 0xed, 0x1b, };

/** Main-A Partition UUID - 7c688c84-b864-44ea-910a-d97922ddd5e0*/
static const u8 main_A_uuid[UUID_SIZE] = { 0x84, 0x8c, 0x68, 0x7c, 0x64, 0xb8,
		0xea, 0x44, 0x91, 0x0a, 0xd9, 0x79, 0x22, 0xdd, 0xd5, 0xe0, };

/** Main-B Partition UUID - 7733bee4-42fd-45dd-a77d-e99b9458e5cf*/
static const u8 main_B_uuid[UUID_SIZE] = { 0xe4, 0xbe, 0x33, 0x77, 0xfd, 0x42,
		0xdd, 0x45, 0xa7, 0x7d, 0xe9, 0x9b, 0x94, 0x58, 0xe5, 0xcf, };

/** Data Partition UUID - e04cfc88-c905-48ee-8fbf-0ac73bf70eda*/
static const u8 data_uuid[UUID_SIZE] = { 0x88, 0xfc, 0x4c, 0xe0, 0x05, 0xc9,
		0xee, 0x48, 0x8f, 0xbf, 0x0a, 0xc7, 0x3b, 0xf7, 0x0e, 0xda, };

struct efi_entry {
	u8 type_uuid[16];
	u8 uniq_uuid[16];
	u64 first_lba;
	u64 last_lba;
	u64 attr;
	u16 name[EFI_NAMELEN];
};

struct efi_header {
	u8 magic[8];

	u32 version;
	u32 header_sz;

	u32 crc32;
	u32 reserved;

	u64 header_lba;
	u64 backup_lba;
	u64 first_lba;
	u64 last_lba;

	u8 volume_uuid[16];

	u64 entries_lba;

	u32 entries_count;
	u32 entries_size;
	u32 entries_crc32;
}__attribute__((packed));

struct ptable {
	u8 mbr[512];
	union {
		struct efi_header header;
		u8 block[512];
	};
	struct efi_entry entry[EFI_ENTRIES];
};

struct fbt_partition {
	const char *name;
	unsigned size_kb;
	const char *fs_type;
	const u8 *uuid;
	struct efi_entry * entry;
};


#define MB 1024
// TODO: Read the partition table from mmc-1
static struct fbt_partition fbt_partitions[] = {
		{ "-", 128, NULL, NULL },
		{ "xloader", 896, "raw", xloader_uuid },
		{ "recovery", MB * 16, "fat", recovery_uuid },
		{ "boot", MB * 16, "fat", boot_uuid },
		{ "idme", MB * 16, "ext3", misc_uuid },
		{ "diags", MB * 128, "ext3", mfg_uuid },
		{ "main-A", MB * 512, "ext3", main_A_uuid },
		{ "main-B", MB * 512, "ext3", main_B_uuid },
		{ "data", 0, "ext3", data_uuid },
		{ 0, 0, NULL, NULL },
};


int get_partition_num(const char* name) {
	int part_num = 0; /* skip the "-" partition */
	struct fbt_partition * partition = fbt_partitions;

	if (!name)
		return -1;

	while (partition->name) {
		if (strncmp(name, partition->name, strlen(partition->name)) == 0)
			return part_num;
		part_num++;
		partition++;
	}

	return -1;
}

const char * get_partition_type(const char* name) {
	struct fbt_partition * partition = fbt_partitions;

	if (!name)
		return NULL;

	while (partition->name) {
		if (strncmp(name, partition->name, strlen(partition->name)) == 0)
			return partition->fs_type;
		partition++;
	}

	return NULL;
}

char* get_partition_uuid(const char* name) {
	char * uuid = NULL;

	// TODO: Return NULL for now
	return uuid;
}

#ifdef CONFIG_CMD_PARTITION

static struct ptable the_ptable;

static void init_mbr(u8 *mbr, u32 blocks) {
	mbr[0x1be] = 0x00; /* nonbootable */
	mbr[0x1bf] = 0xFF; /* bogus CHS */
	mbr[0x1c0] = 0xFF;
	mbr[0x1c1] = 0xFF;

	mbr[0x1c2] = 0xEE; /* GPT partition */
	mbr[0x1c3] = 0xFF; /* bogus CHS */
	mbr[0x1c4] = 0xFF;
	mbr[0x1c5] = 0xFF;

	mbr[0x1c6] = 0x01; /* start */
	mbr[0x1c7] = 0x00;
	mbr[0x1c8] = 0x00;
	mbr[0x1c9] = 0x00;

	memcpy(mbr + 0x1ca, &blocks, sizeof(u32));

	mbr[0x1fe] = 0x55;
	mbr[0x1ff] = 0xaa;
}

static void start_ptbl(struct ptable *ptbl, unsigned blocks) {
	struct efi_header *hdr = &ptbl->header;

	memset(ptbl, 0, sizeof(*ptbl));

	init_mbr(ptbl->mbr, blocks - 1);

	memcpy(hdr->magic, "EFI PART", 8);
	hdr->version = EFI_VERSION;
	hdr->header_sz = sizeof(struct efi_header);
	hdr->header_lba = 1;
	hdr->backup_lba = blocks - 1;
	hdr->first_lba = 34; /* 34 standard blocks for header and partitions */;
	hdr->last_lba = blocks - 34; /* Enough room for backup header and partitions at the end of MMC*/
	memcpy(hdr->volume_uuid, random_uuid, 16);
	hdr->entries_lba = 2;
	hdr->entries_count = 0;
	hdr->entries_size = sizeof(struct efi_entry);
}

static void end_ptbl(struct ptable *ptbl) {
	struct efi_header *hdr = &ptbl->header;
	u32 n;

	n = crc32(0, 0, 0);
	n = crc32(n, (void *) ptbl->entry, hdr->entries_count * hdr->entries_size);
	hdr->entries_crc32 = n;

	n = crc32(0, 0, 0);
	n = crc32(0, (void *) hdr, hdr->header_sz);
	hdr->crc32 = n;
}

static void backup_ptble(struct ptable *ptbl) {
	struct efi_header *hdr = &ptbl->header;
	u64 tmp;

	tmp = hdr->header_lba;
	hdr->header_lba = hdr->backup_lba;
	hdr->backup_lba = tmp;

	hdr->crc32 = 0;

	end_ptbl(ptbl);
}

int add_ptn(struct ptable *ptbl, u64 first, u64 last,
		struct fbt_partition *part) {
	struct efi_header *hdr = &ptbl->header;
	struct efi_entry *entry = ptbl->entry;
	unsigned int name_size = strlen(part->name);
	unsigned n, i;

	printf("Creating partition - %s ... \n", part->name);

	if (first < 34) {
		printf("partition '%s' overlaps partition table\n", part->name);
		return -1;
	}

	if (last > hdr->last_lba) {
		printf("partition '%s' does not fit\n", part->name);
		return -1;
	}

	// Increment the EFI entry count
	hdr->entries_count++;

	if (name_size > EFI_NAMELEN)
		name_size = EFI_NAMELEN;

	for (n = 0; n < EFI_ENTRIES; n++, entry++) {
		if (entry->last_lba)
			continue;
		memcpy(entry->type_uuid, partition_type, 16);
		memcpy(entry->uniq_uuid, part->uuid, 16);
		for(i = 0; i < name_size; ++i)
			entry->name[i] = (efi_char16_t)part->name[i];
		entry->first_lba = first;
		entry->last_lba = last;
		// Store the entry pointer
		part->entry = entry;
		return 0;
	}
	printf("out of partition table entries\n");
	hdr->entries_count--;
	return -1;
}


int do_partition(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {
	struct ptable *ptbl = &the_ptable;
	unsigned next;
	int n;
	struct mmc *mmc;
	unsigned long blocks_to_write, result;
	int dev = 0;

	if (argc >= 2)
		dev = simple_strtoul(argv[1], NULL, 10);

	mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		printf("mmc init failed\n");
		return -1;
	}

	if (!mmc->block_dev.lba) {
		printf("device %s has no space\n", mmc->name);
		return -1;
	}

	start_ptbl(ptbl, mmc->block_dev.lba);
	for (n = 0, next = 0; fbt_partitions[n].name; n++) {
		u64 sz = fbt_partitions[n].size_kb * 2;
		if (fbt_partitions[n].name[0] == '-') {
			next += sz;
			continue;
		}
		if (sz == 0)
			sz = ptbl->header.last_lba - next;
		if (add_ptn(ptbl, next, next + sz - 1, &fbt_partitions[n]))
			return -1;
		next += sz;
	}
	end_ptbl(ptbl);

	blocks_to_write = DIV_ROUND_UP(sizeof(struct ptable), mmc->block_dev.blksz);
	result = mmc->block_dev.block_write(mmc->block_dev.dev, 0, blocks_to_write,
			ptbl);
	if (result != blocks_to_write) {
		printf("\nFormat failed, block_write() returned %lu instead of %lu\n",
				result, blocks_to_write);
		return -1;
	}

	printf("\nnew partition table of %lu %lu-byte blocks\n", blocks_to_write,
			mmc->block_dev.blksz);

	// Write the backup header
	backup_ptble(ptbl);

	blocks_to_write = DIV_ROUND_UP(ptbl->header.entries_count * ptbl->header.entries_size,
			mmc->block_dev.blksz);
	result = mmc->block_dev.block_write(mmc->block_dev.dev, ptbl->header.last_lba+1, blocks_to_write,
			ptbl->entry);
	if (result != blocks_to_write) {
		printf("\nFormat failed, block_write() returned %lu instead of %lu\n",
				result, blocks_to_write);
		return -1;
	}

	result = mmc->block_dev.block_write(mmc->block_dev.dev, ptbl->header.header_lba, 1,
			&ptbl->header);
	if (result != 1) {
		printf("\nFormat failed, block_write() returned %lu instead of %d\n",
				result, 1);
		return -1;
	}

	return 0;
}

int do_clone(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {
	unsigned int dev_from = 0;
	unsigned int dev_to = 0;
	struct mmc *mmc_from;
	struct mmc *mmc_to;
	unsigned int n;
	u64 next = 0;
	unsigned long num_blocks = 0;
	char * s = NULL;
	unsigned long load_addr = 0;
	int start_block = (128 * 1024) / 512; // skip the first 128K
	unsigned int min_clone_count = (1024 * 1024) / 512; // 1 MB chunks

	if (argc >= 3) {
		dev_from = simple_strtoul(argv[1], NULL, 10);
		dev_to = simple_strtoul(argv[2], NULL, 10);
	}

	if (dev_from == dev_to) {
		printf("Cannot clone to self\n");
		return -1;
	}

	// Get the corresponding devices
	mmc_from = find_mmc_device(dev_from);
	if (!mmc_from) {
		printf("no mmc device at slot %d\n", dev_from);
		return -1;
	}

	if (mmc_init(mmc_from)) {
		printf("mmc init failed for %d\n", dev_from);
		return -1;
	}

	mmc_to = find_mmc_device(dev_to);
	if (!mmc_to) {
		printf("no mmc device at slot %d\n", dev_to);
		return -1;
	}

	if (mmc_init(mmc_to)) {
		printf("mmc init failed for %d\n", dev_to);
		return -1;
	}

	if (mmc_to->block_dev.blksz != mmc_from->block_dev.blksz) {
		printf("block size mismatch to-%lu vs. from-%lu\n",
				mmc_to->block_dev.blksz, mmc_from->block_dev.blksz);
		return -1;
	}

	// Calculate the partition size
	for (n = 0; fbt_partitions[n].name; n++)
		next += fbt_partitions[n].size_kb;

	num_blocks = (next * 1024) / mmc_to->block_dev.blksz;

	if (mmc_to->block_dev.lba < num_blocks
			|| num_blocks > mmc_from->block_dev.lba) {
		printf("Data size mis-match !\n");
		return -1;
	}

	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	if (load_addr == 0) {
		printf("invalid load address\n");
		return -1;
	}

	printf("cloning %lu blocks...\n", num_blocks);

	for (n = start_block; n < num_blocks; n += min_clone_count) {
		if (!(n % (start_block+min_clone_count)))
			printf("%d / %lu\n", n, num_blocks);
		// Read data from the from mmc
		if (mmc_from->block_dev.block_read(mmc_from->block_dev.dev, n,
				min_clone_count, (void*) load_addr) == min_clone_count) {
			/* flush cache after read */
			flush_cache((ulong) load_addr, mmc_from->block_dev.blksz);
			// Write the data to the to-mmc
			if (mmc_to->block_dev.block_write(mmc_to->block_dev.dev, n,
					min_clone_count, (void*) load_addr) != min_clone_count) {
				printf("failed to write block @ %d\n", n);
				return -1;
			}
		} else {
			printf("Failed to read data @ block %d\n", n);
			return -1;
		}
	}

	printf("\n done !\n");

	return 0;
}

static struct fbt_partition * get_partition_entry_info(const char* name) {

	struct fbt_partition * partition = fbt_partitions;

	if (!name)
		return NULL;

	while (partition->name) {
		if (strcmp(name, partition->name) == 0)
			return partition;
		partition++;
	}

	return NULL;
}

static unsigned int flasheMMC(unsigned long load_size, u64 lba_offset) {

	char * s = NULL;
	unsigned long load_addr = 0;
	unsigned long write_size = 0;
	struct mmc *mmc_to = find_mmc_device(1); /*eMMC*/;

	if ((s = getenv("loadaddr")) == NULL)
		return 0;

	load_addr = simple_strtoul(s, NULL, 16);

	if (!mmc_to) {
		printf("no eMMC device\n");
		return 0;
	}

	if (mmc_init(mmc_to)) {
		printf("mmc init failed\n");
		return 0;
	}

	if ( load_size % mmc_to->block_dev.blksz) {
		printf("load size %lu must be sector (%lu) aligned \n", load_size, mmc_to->block_dev.blksz);
		return 0;
	}

	write_size = mmc_to->block_dev.block_write(mmc_to->block_dev.dev, lba_offset,
			load_size / mmc_to->block_dev.blksz, (void*) load_addr);

	return write_size;
}

static int load_file(const char* file, unsigned long offset, unsigned long file_size) {

	int err = 0;
	unsigned long load_addr = 0;
	unsigned long file_part = 2; /* default second partition "files" of SD-card */
	const char * s = getenv("loadaddr");
	block_dev_desc_t *dev_desc = NULL;

	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	if ((s = getenv("filepart")) != NULL) {
		file_part = simple_strtoul(s, NULL, 16);
	}

	if (!file || !load_addr) {
		printf("invalid file \n");
		return -1;
	}

	dev_desc = get_dev("mmc", 0); /* SD-card */
	if (dev_desc == NULL) {
		printf("Failed to find mmc-0\n");
		return -1;
	}

	err = fat_register_device(dev_desc, file_part);
	if (err) {
		printf("Failed to register mmc-0:2\n");
		return -1;
	}

	err = fat_read_file(file, (uchar *)load_addr, offset, file_size);
	if (err == -1) {
		printf("\n** Unable to read \"%s\" **\n",
				file);
		return -1;
	}

	return err;
}

static unsigned int load_and_flash_file(const char * file,
		unsigned long offset, unsigned long size, u64 lba_offset) {

	// load the file
	long file_size = load_file(file, offset, size);
	if (file_size < 0) {
		printf("failed to load file - %s\n", file);
		// write the file into the eMMC
		return 0;
	}

	return flasheMMC(file_size, lba_offset);
}

static int flash_file_in_parts(const char* file, unsigned long size, u64 lba_offset) {

	unsigned int i = 0;
	unsigned int num_blocks = 0;
	unsigned int read_size = 128 * 1024 * 1024; /* 128 MB at a time*/

	for (i=0; i <= size; i += read_size) {

		num_blocks = load_and_flash_file(file, i, read_size, lba_offset);
		if (!num_blocks)
			break;
		lba_offset += num_blocks;
	}

	return !i;
}


int do_flash(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) {

	struct fbt_partition * part_info = NULL;

	// Verify that we have a valid
	if (argc < 3) {
		printf("invalid argument \n");
		return -1;
	}

	part_info = get_partition_entry_info(argv[1]);

	if (NULL == part_info) {
		printf("unknown partition - %s\n", argv[1]);
		return -1;
	}

	if (NULL == part_info->entry) {
		printf("Partition table layout error. Run partition command \n");
		return -1;
	}

	// load the file
	if ( part_info->size_kb <= (128 * MB)) {
		if (load_and_flash_file(argv[2], 0, part_info->size_kb*MB, part_info->entry->first_lba))
			return 0;
		return -1;
	}
	else
		return flash_file_in_parts(argv[2], part_info->size_kb*MB, part_info->entry->first_lba);
}

/***************************************************/
/// Format boot command
U_BOOT_CMD(
		partition, 3, 1, do_partition,
		"Partition MMC device",
		"MMC device id (0 = SDCard, 1 = eMMC)\n"
);
/// Clone boot command
U_BOOT_CMD(
		clone, 3, 1, do_clone,
		"Clone eMMC/sd-card",
		" <from-dev> <to-dev>\n"
);
/// Flash partition image
U_BOOT_CMD(
		flash, 3, 1, do_flash,
		"Flash file to eMMC",
		" <partition-name> <file-name>\n"
);
#endif //CONFIG_CMD_PARTITION
