/*
 * cmd_idme_nand.c
 *
 * Copyright 2012-2013 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#include <common.h>
#include <malloc.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <idme.h>

#define DSN_PROP_SIZE		16
#define MAC_PROP_SIZE		12
#define MAC_SECRET_PROP_SIZE	20
#define BOARD_ID_PROP_SIZE	16
#define MIC_CALIB_SIZE		4	/* 4 bytes per mic * 7 mics*/
#define BOOT_MODE_PROP_SIZE 	16
#define POST_MODE_PROP_SIZE 	16
#define BOOT_MODE_PROP_SIZE	16
/// Device type property size.
#define DEVICE_TYPE_PROP_SIZE	16
#define RETRY_COUNT_PROP_SIZE	4
#define WIFI_MODULE_REV_SIZE	4

#define CONFIG_NAND_MFGDATA_BLOCK_NUM	16
#define CONFIG_NAND_MFGBACKUP_BLOCK_NUM	1

typedef struct nvram_s {
	const char *name;
	unsigned int offset;
	unsigned int size;
} nvram_t;

/* Back up data */
/* Block size - (checksum size + header + version)*/
#define IDME_DATA_SIZE	((128*1024) - (sizeof(u32)*3))
#define IDME_HEADER_CODE	0x49444d45
#define IDME_CHECKSUM_SZ	0x140 /* Offset of immutable data */
#define IDME_DATA_VERSION	0x2

/* Note: sizeof(idme_data_t) should NOT exceed a block size of 128K */
typedef struct idme_data_s {
	u8 data[IDME_DATA_SIZE];
	u32 header;
	u32 checksum;
	u32 version;
} idme_data_t;


static const nvram_t nvram_info[] = {
	{ // Device Serial number
	.name = "dsn",   .offset = 0x00, .size = DSN_PROP_SIZE, },
	{ // Wi-Fi MAC Address
	.name = "mac",   .offset = 0x30, .size = MAC_PROP_SIZE, },
	{ // BT MAC Address
	.name = "btmac", .offset = 0x40, .size = MAC_PROP_SIZE, },
	{ // MFG Secret
	.name = "mfg",   .offset = 0x50, .size = MAC_SECRET_PROP_SIZE, },
	{ // PCB Serial Number?
	.name = "pcbsn", .offset = 0x70, .size = BOARD_ID_PROP_SIZE, },
	{ // Mic Calibration Data
	.name = "mic0",  .offset = 0x80, .size = MIC_CALIB_SIZE, },
	{ // Mic Calibration Data
	.name = "mic1",  .offset = 0x84, .size = MIC_CALIB_SIZE, },
	{ // Mic Calibration Data
	.name = "mic2",  .offset = 0x88, .size = MIC_CALIB_SIZE, },
	{ // Mic Calibration Data
	.name = "mic3",  .offset = 0x8C, .size = MIC_CALIB_SIZE, },
	{ // Mic Calibration Data
	.name = "mic4",  .offset = 0x90, .size = MIC_CALIB_SIZE, },
	{ // Mic Calibration Data
	.name = "mic5",  .offset = 0x94, .size = MIC_CALIB_SIZE, },
	{ // Mic Calibration Data
	.name = "mic6",  .offset = 0x98, .size = MIC_CALIB_SIZE, },
	{ // AP Mode Mac address
	.name = "apmac", .offset = 0x100, .size = MAC_PROP_SIZE, },
	{ // Bottom board serial number
	.name = "bbsn",  .offset = 0x120, .size = BOARD_ID_PROP_SIZE, },
	{ // Top Board serial number
	.name = "tbsn",  .offset = 0x140, .size = BOARD_ID_PROP_SIZE, },
	{ // Bootmode
	.name = "bootmode", .offset = 0x150, .size = BOOT_MODE_PROP_SIZE, },
	{ // Bootmode
	.name = "postmode", .offset = 0x160, .size = POST_MODE_PROP_SIZE, },
	{ // Device type.
	.name = "devicetype", .offset = 0x170, .size = DEVICE_TYPE_PROP_SIZE, },
	{ // Last known good boot partition.
	.name = "knowngood", .offset = 0x180, .size = BOOT_MODE_PROP_SIZE, },
	{ // retry count for new image.
	.name = "otaretrycount", .offset = 0x190, .size = RETRY_COUNT_PROP_SIZE, },
	{ // WiFi Module Revision
	.name = "wifimodulerev", .offset = 0x194, .size = WIFI_MODULE_REV_SIZE, },
	{ // Terminate
	.name = NULL, },
};

static const nvram_t nvram_sig_info = {
	.name = "writesig", .offset = 0x1f0,.size = 16,
};

static u_char *varbuf = NULL;
static int varbuf_cached = 0;

extern int setup_board_info(void);

extern int nand_curr_device;
extern nand_info_t nand_info[];

/**
 * Read the contents of the IDME from NAND to RAM (global 'varbuf')
 * @return 0 on success
 */
static int read_varbuf_from_nand(void)
{
	int ret = -1;
	nand_info_t *nand = &nand_info[nand_curr_device];
	struct nand_chip *nand_chip = nand_info[0].priv;
	ulong off = CONFIG_NAND_MFGDATA_BLOCK_NUM << nand_chip->phys_erase_shift;
	size_t size = 1 << nand_chip->phys_erase_shift;

	if (NULL == varbuf) {
		/* Allocate enough space to hold idme data cache*/
		varbuf = (u_char *)malloc(size);
		if (NULL == varbuf) {
			printf("IDME Var buf malloc failed !!!\n");
			BUG();
		}
	}

	if (!varbuf_cached && varbuf) {
		ret = nand_read_skip_bad(nand, off, &size, (u_char *) varbuf);
		if(ret) {
			if(-EBADMSG == ret) {
				/* Version of uboot nand always returns this error, but works */
				ret = 0;
			} else {
				printf("%s: Couldn't read NV\n", __FUNCTION__);
				return ret;
			}
		}
		varbuf_cached = 1;
	} else {
		ret = 0;
	}

	return ret;
}

/**
 * Write the contents of the IDME from RAM to NAND
 * @return 0 on success
 */
static int write_varbuf_to_nand(void)
{
	int ret = -1;
	nand_info_t *nand = &nand_info[nand_curr_device];
	struct nand_chip *nand_chip = nand_info[0].priv;
	ulong off = CONFIG_NAND_MFGDATA_BLOCK_NUM << nand_chip->phys_erase_shift;
	size_t size = 1 << nand_chip->phys_erase_shift;;
	idme_data_t *idme_data = (idme_data_t*)varbuf;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE
			|| !nand_info[nand_curr_device].name) {
		printf("\n%s: no devices available\n", __FUNCTION__);
		return ret;
	}

	idme_data->header = IDME_HEADER_CODE;
	idme_data->version = IDME_DATA_VERSION;
	// Checksum is the checksum of the backed up data
	//  Update when data is backed up

	nand_erase(nand, off, size);
	ret = nand_write_skip_bad(nand, off, &size, (u_char *) varbuf, 0);
	if (ret) {
		printf("Error! Couldn't write NV variables. (%d)\n", ret);
		return ret;
	}

	return ret;
}

/**
 * Search for an IDME field
 * @param[in] name Name of IDME field to search for
 * @return nvram_t * Pointer field details - NULL on failure
 */
static const nvram_t *idme_find_var(const char *name) {
	int i = 0;
	int len;

	while (nvram_info[i].name != NULL) {

		len = strlen(nvram_info[i].name);
		if (strncmp(name, nvram_info[i].name, len) == 0) {
			return &nvram_info[i];
		}
		i++;
	}

	return NULL;
}

/**
 * Prints all IDME fields and values
 * @return 0 on success
 */
int idme_print_vars_nand(void) {
	int i = 0;
	char value[65];

	while (nvram_info[i].name != NULL) {

		idme_get_var(nvram_info[i].name, value, sizeof(value));
		value[nvram_info[i].size] = 0;

		printf("%s: %s\n", nvram_info[i].name, value);
		i++;
	}

	return 0;
}

/**
 * Retrieve an IDME field value
 * @param[in] name Name of IDME field
 * @param[out] buf Buffer into which to write field value
 * @param[in] buflen Size of buffer
 * @return 0 on success
 */
int idme_get_var_nand(const char *name, char *buf, int buflen) {
	int ret;
	const nvram_t *nv;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE
			|| !nand_info[nand_curr_device].name) {
		printf("\nno devices available\n");
		return 1;
	}

	buf[0] = 0;

	nv = idme_find_var(name);
	if (!nv) {
		printf("Error! Couldn't find NV variable %s\n", name);
		return -1;
	}

	ret = read_varbuf_from_nand();
	if(ret) {
		printf("Failed to read nand\n");
		return ret;
	}

	// buffer is too small, return needed size
	if (nv->size + 1 > buflen) {
		printf("Error! Buffer too small for variable %s. Need %d\n", name, nv->size + 1);
		return (nv->size + 1);
	}

	memcpy(buf, varbuf + nv->offset, nv->size);
	buf[nv->size] = 0;

	return 0;
}

/**
 * Store an IDME field value
 * @param[in] name Name of IDME field
 * @param[in] value Value to be updated with
 * @return 0 on success
 */
int idme_update_var_nand(const char *name, const char *value) {
	int ret = 0, sz;
	const nvram_t *nv;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE
			|| !nand_info[nand_curr_device].name) {
		printf("\nno devices available\n");
		return 1;
	}

	nv = idme_find_var(name);
	if (!nv) {
		printf("Error! Couldn't find NV variable %s\n", name);
		return -1;
	}

	ret = read_varbuf_from_nand();
	if(ret) {
		printf("Failed to read nand\n");
		return ret;
	}

	// clear out old value
	memset(varbuf + nv->offset, 0, nv->size);

	if (value != NULL) {
		// copy in new value
		sz = MIN(nv->size, strlen(value));
		memcpy(varbuf + nv->offset, value, sz);
	}

	ret = write_varbuf_to_nand();
	if(ret) {
		printf("Failed to write nand\n");
		return ret;
	}

	/* Make sure that the board info globals (DSN/pcbsn) are updated as well */
	setup_board_info();

	return ret;
}

/**
 * Read the back up data from nand
 * @param[in] buffer Buffer into which backup data should be written
 * @return 0 on success
 */
static int read_backup_data(idme_data_t *buffer) {

	nand_info_t *nand = &nand_info[nand_curr_device];
	struct nand_chip *nand_chip = nand_info[0].priv;
	ulong off = CONFIG_NAND_MFGBACKUP_BLOCK_NUM << nand_chip->phys_erase_shift;
	size_t size = 1 << nand_chip->phys_erase_shift;
	u_char *block_buffer = NULL;
	u_char *data_buffer = (u_char*)buffer;
	int ret = -1;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE
			|| !nand_info[nand_curr_device].name) {
		printf("\n%s: no devices available\n", __FUNCTION__);
		return ret;
	}

	// Check that we have a valid buffer
	if (!data_buffer) {
		printf("\n%s: Invalid buffer\n", __FUNCTION__);
		return ret;
	}

	if (sizeof(idme_data_t) > size) {
		printf("%s: Invalid idme data size - should be within a block size\n", __FUNCTION__);
		return ret;
	}else if (sizeof(idme_data_t) < size) {
		// Enforce block aligned operation
		block_buffer = (u_char*)malloc(size);
		if (!block_buffer) {
			printf("%s: Malloc failed !\n", __FUNCTION__);
			return ret;
		}
		data_buffer = block_buffer;
	}

	// Read the contents of the back up block from nand
	ret = nand_read_skip_bad(nand, off, &size, data_buffer);
	if(ret) {
		if(-EBADMSG == ret)
			/* Version of uboot nand always returns this error, but works */
			ret = 0;
		else {
			printf("%s: Couldn't read backup\n", __FUNCTION__);
			if (block_buffer)
				free(block_buffer);
			return ret;
		}
	}

	if (block_buffer) {
		memcpy(buffer, data_buffer, sizeof(idme_data_t));
		free(block_buffer);
	}

	return ret;
}

/**
 * Write the back up data to nand
 * @param[in] buffer Buffer from which backup data should be read
 * @return 0 on success
 */
static int write_backup_data(idme_data_t *buffer) {

	nand_info_t *nand = &nand_info[nand_curr_device];
	struct nand_chip *nand_chip = nand_info[0].priv;
	ulong off = CONFIG_NAND_MFGBACKUP_BLOCK_NUM << nand_chip->phys_erase_shift;
	size_t size = 1 << nand_chip->phys_erase_shift;
	u_char *block_buffer = NULL;
	u_char *data_buffer = (u_char*)buffer;
	int ret = -1;

	if (!buffer) {
		printf("\n%s: invalid buffer (size=%d\n", __FUNCTION__, size);
		return ret;
	}

	if (sizeof(idme_data_t) > size) {
		printf("%s: Invalid idme data size - should be within a block size\n", __FUNCTION__);
		return ret;
	} else if (sizeof(idme_data_t) < size) {
		// Enforce block aligned operation
		block_buffer = (u_char*)malloc(size);
		if (!block_buffer) {
			printf("%s: Malloc failed !\n", __FUNCTION__);
			return ret;
		}
		data_buffer = block_buffer;
		memcpy(data_buffer, buffer, sizeof(idme_data_t));
	}
	// Flush data back into nand
	nand_erase(nand, off, size);
	ret = nand_write_skip_bad(nand, off, &size, data_buffer, 0);
	if (ret) {
		printf("Error! Couldn't write backup data (%d)\n", ret);
		if (block_buffer)
			free(block_buffer);
		return ret;
	}

	if (block_buffer)
		free(block_buffer);

	return ret;
}

/**
 * Validate that the back up data is good
 * @param[in] buffer Buffer containing backup data
 * @param[out] crc Pointer to hold CRC value
 * @return 0 on success
 */
static int validate_backup_data(idme_data_t *backup_ptr, u32 *crc) {

	u32 checksum = 0;

	if(!backup_ptr || !crc) {
		printf("%s: Invalid argument\n", __FUNCTION__);
		return -1;
	}

	// Verify the header information
	if (backup_ptr->header != IDME_HEADER_CODE) {
		printf("%s: Invalid backup data (header)\n", __FUNCTION__);
		return -1;
	}

	// Verify checksum
	checksum = crc32(0, backup_ptr->data, IDME_CHECKSUM_SZ);
	if (backup_ptr->checksum != checksum) {
		*crc = 0;
		printf("%s: Invalid backup data (chksum)\n", __FUNCTION__);
		return -1;
	}

	*crc = checksum;

	return 0;
}

/**
 * Restore contents of idme var buf from backup data buffer
 * @param[in] backup_ptr Pointer to backup data
 * @return 0 on success
 */
static int idme_restore_from_buffer(idme_data_t *backup_ptr) {

	int ret = -1;
	u32 checksum = 0;

	if (!backup_ptr) {
		printf("%s: Invalid argument\n", __FUNCTION__);
		return ret;
	}

	ret = validate_backup_data(backup_ptr, &checksum);
	if (ret) {
		printf("%s: Invalid backup data (chksum)\n", __FUNCTION__);
		return -1;
	}

	// Copy the content of back-up data into idme
	memcpy(varbuf, backup_ptr, sizeof(idme_data_t));

	// Flush idme to nand
	ret = write_varbuf_to_nand();
	if(ret) {
		printf("Failed to write nand\n");
		return ret;
	}

	return ret;
}

/**
 * Restore contents of idme var buf from provided buffer
 * @param[in] backup_block Buffer containing backup data. If NULL, allocate buffer
 * @return 0 on success
 */
static int idme_restore_data(idme_data_t *buffer) {

	int ret = -1;

	if (!buffer) {
		idme_data_t *backup_ptr = NULL;

		// Read the current idme values from nand - should already be cached
		ret = read_varbuf_from_nand();
		if(ret) {
			printf("Failed to read idme from nand\n");
			return ret;
		}

		ret = -1;

		// Allocate a buffer for the backup data
		backup_ptr = (idme_data_t *)malloc(sizeof(idme_data_t));
		if (NULL == backup_ptr) {
			printf("IDME Backup block malloc failed !!!\n");
			return ret;
		}

		// Read the contents of the back up block from nand
		ret = read_backup_data(backup_ptr);
		if(ret) {
			printf("%s: Couldn't read NV\n", __FUNCTION__);
			return ret;
		}

		ret =  idme_restore_from_buffer(backup_ptr);

		free(backup_ptr);
	} else {
		ret = idme_restore_from_buffer(buffer);
	}

	return ret;
}

/**
 * Back up contents of idme var buf to special area of nand
 * @return 0 on success
 */
int idme_backup_nand(void) {

	int ret = -1;
	idme_data_t *backup_ptr = NULL;
	idme_data_t *idme_ptr = NULL;

	// Read the current idme values from nand
	ret = read_varbuf_from_nand();
	if (ret) {
		printf("%s: Failed to read idme from nand\n", __FUNCTION__);
		return ret;
	}

	// Allocate a buffer to hold a block
	backup_ptr = (idme_data_t  *)malloc(sizeof(idme_data_t));
	if (NULL == backup_ptr) {
		printf("%s: IDME Backup block malloc failed !!!\n", __FUNCTION__);
		return ret;
	}

	// Copy idme values into the back up area of nand
	memcpy(backup_ptr, varbuf, sizeof(idme_data_t));
	// Set the header
	backup_ptr->header = IDME_HEADER_CODE;
	// Calculate and set the crc
	backup_ptr->checksum =  crc32(0, backup_ptr->data, IDME_CHECKSUM_SZ);
	// Write the version
	backup_ptr->version = IDME_DATA_VERSION;

	// Flush data back into nand
	ret = write_backup_data(backup_ptr);
	if (ret) {
		printf("Error! Couldn't write backup data to nand (%d)\n", ret);
		free(backup_ptr);
		return ret;
	}

	// Update the checksum of the idme data to hold the backup checksum
	idme_ptr = (idme_data_t *)varbuf;
	idme_ptr->checksum = backup_ptr->checksum;

	free(backup_ptr);
	// Flush idme to nand
	ret = write_varbuf_to_nand();
	if(ret) {
		printf("Failed to write nand\n");
		return ret;
	}

	return ret;
}

/*
 * Restore the contents of IDME from backup storage
 * @return 0 on success
 */
int idme_restore_nand(void) {
	return idme_restore_data(NULL);
}

/*
 * Check and restore IDME data from backup if corruption is detected
 * @return 0 on success
 */
int idme_check_update_nand(void) {

	int ret = -1;
	idme_data_t *backup_ptr = NULL;
	idme_data_t *idme_ptr = NULL;
	u32 idme_checksum = 0;
	u32 backup_checksum = 0;

	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE
			|| !nand_info[nand_curr_device].name) {
		printf("\nno devices available\n");
		return ret;
	}

	ret = read_varbuf_from_nand();
	if(ret) {
		printf("Failed to read nand\n");
		return ret;
	}

	ret = -1;

	// Get the checksum of idme data
	idme_checksum = crc32(0, varbuf, IDME_CHECKSUM_SZ);

	idme_ptr = (idme_data_t *)varbuf;
	// Compare the idme checksum to the backup checksum
	if (idme_ptr && (idme_ptr->checksum == idme_checksum))
		return 0;

	// Allocate a buffer for the backup data
	backup_ptr = (idme_data_t *)malloc(sizeof(idme_data_t));
	if (NULL == backup_ptr) {
		printf("IDME Backup block malloc failed !!!\n");
		return ret;
	}

	// Read the contents of the back up block from nand
	ret = read_backup_data(backup_ptr);
	if(ret) {
		printf("%s: Couldn't read NV\n", __FUNCTION__);
		free(backup_ptr);
		return ret;
	}

	// Get the checksum of the backup data
	ret = validate_backup_data(backup_ptr, &backup_checksum);
	if (ret) {
		printf("%s: failed to validate backup data\n", __FUNCTION__);
		return ret;
	}

	if (backup_checksum != idme_checksum) {
		ret = idme_restore_data(backup_ptr);
		if (!ret)
			printf("%s: Restored IDME data\n", __FUNCTION__);
	} else if (backup_checksum == idme_checksum) {
		ret = 0;
	}

	free(backup_ptr);

	return ret;
}

/*
 * Get the entire IDME data blob
 * @return blob size on success
 */
int idme_get_blob_nand(char **buf)
{
	/* Blob support is not provided for nand based devices */
	return 0;
}

/**
 * Initialize - load contents of nand
 *
 */
int idme_init_nand(void) {
	return 0; // initialize on write
}
