/*
 * cmd_idme_emmc.c
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
#include <stdint.h>
#include <common.h>
#include <malloc.h>
#include <ext_common.h>
#include <fat.h>


// Error codes
#define ERR_NONE	0
#define ERR_UNKNOWN	0x1
#define ERR_INVALID_ARGUMENT	0x2
#define ERR_NOT_FOUND	0x4
#define ERR_INVALID_SIGN	0x8
#define ERR_INVALID_VERSION	0x10
#define ERR_INVALID_CRC	0x20
#define ERR_WRITE_FAILED	0x40
#define ERR_READ_FAILED	0x80

#define IDME_INTERFACE	"mmc"
#define IDME_PARTITION_NAME "idme"
#define IDME_MMC_DEVICE	1 /* eMMC */
#define IDME_MAX_FILE_SIZE	1024 /* 1024-bytes */
#define IDME_MFG_FILE_NAME	"/mfg.idme"
#define IDME_MFG_RECOVERY_FILE_NAME	"mfg.idme.rec"
#define IDME_BOOT_FILE_NAME	"/boot.idme"
#define IDME_BOOT_RECOVERY_FILE_NAME	"boot.idme.rec"

#define RECOVERY_PARTITION_NAME	"recovery"
#define EXT3_PARTITION_TYPE	"ext3"
#define FAT_PARTITION_TYPE	"fat"

#define IDME_MAX_NUM_LINES	128
#define IDME_BOOT_BACKUP_DATA \
		"bootmode=main-A\nknowngood=main-B\notaretrycount=2\npostmode=idmerecovered\n"

/* Partition helper functions see board/ti/lab126/partition.c */
extern int get_partition_num(const char* name);
extern const char * get_partition_type(const char* name);

// IDME FILE STRUCTURE
#define IDME_HEADER_SIGN	0x454D4449
typedef struct idme_file_s {
	/// IDME Signature
	uint32_t sign;
	/// IDME Version string
	uint32_t version;
	/// Size of payload
	uint32_t size;
	/// CRC checksum of payload
	uint32_t crc;
}__attribute__ ((packed)) IdmeDataHeaderT;

typedef struct line_data_s {
	// Data valid
	uint32_t valid;
	// Number of lines
	uint32_t size;
	// Line string
	char *line[IDME_MAX_NUM_LINES];
	/* Raw input buffer */
	char *raw;
	/* Raw input buffer size in bytes */
	uint32_t raw_size;
}LineDataT;

/// Global pointer to hold contents of the manufacturing idme file
static LineDataT * g_mfg_lines = NULL;
/// Global pointer to hold contents of the boot idme file
static LineDataT * g_boot_lines = NULL;
/// Global data buffer
static char * g_var_buf = NULL;

/**
 * Validate the IDME buffer headers. Ensure CRC, sign and version are correct
 * @param hdr IDME data header
 * @param buf Data buffer
 * @return error condition
 */
static int validate_idme(IdmeDataHeaderT* hdr, char * buf) {
	if (!hdr || !buf)
		return ERR_INVALID_ARGUMENT;

	// Check the header signature
	if (hdr->sign != IDME_HEADER_SIGN)
		return ERR_INVALID_SIGN;

	if (hdr->version != 0)
		return ERR_INVALID_VERSION;

	// Perform a CRC on the data
	if (hdr->crc != crc32(0, (const unsigned char*) buf, hdr->size)) {
		printf("CRC failed \n");
		return ERR_INVALID_CRC;
	}

	return ERR_NONE;
}

/**
 * Validate the IDME buffer headers. Ensure CRC, sign and version are correct
 * @param line_data Line data structure
 */
static void free_line_data(LineDataT* line_data) {
	int i = 0;

	if (!line_data)
		return;

	for (i = 0; i < line_data->size; ++i) {
		if (line_data->line[i])
			free(line_data->line[i]);
	}

	free(line_data);
}

/**
 * Register FAT partition on eMMC device
 * @param partition number
 */
static int register_fat_partition(int part_num)
{
	// Register the device
	block_dev_desc_t * dev_desc = get_dev(IDME_INTERFACE, IDME_MMC_DEVICE);
	if (dev_desc == NULL)
		return ERR_UNKNOWN;

	// Register partition as FAT
	if (fat_register_device(dev_desc, part_num))
		return ERR_UNKNOWN;

	return ERR_NONE;
}

/**
 * Validate the IDME buffer headers. Ensure CRC, sign and version are correct
 * @param buffer Data buffer to parse
 * @param bufsz Size of data buffer
 * @return NULL if error occurs
 */
static LineDataT * get_line_data(char *buffer, unsigned int bufsz) {
	char c;
	unsigned int line_count = 0;
	unsigned int char_count = 0;
	unsigned int line_char_count = 0;
	char * line_buffer = NULL;
	char tmp_line_buffer[bufsz];

	if (!buffer)
		return NULL;

	// Create a temp buffer
	LineDataT * line_data = (LineDataT*) malloc(sizeof(LineDataT));
	if (!line_data)
		return NULL;

	memset(line_data, 0, sizeof(LineDataT));
	memset(tmp_line_buffer, 0, sizeof(tmp_line_buffer));

	do {

		c = tmp_line_buffer[line_char_count++] = buffer[char_count];
		if ('\n' == c) {
			line_buffer = (char*) malloc(bufsz);
			if (!line_buffer)
				goto CLEANUP;
			// Clear the buffer
			memset(line_buffer, 0, bufsz);
			// Update the number of lines found
			line_data->size++;
			// Set the null char in place of the new line
			tmp_line_buffer[line_char_count - 1] = '\0';
			// copy data from the temp buffer into the line buffer
			memcpy(line_buffer, tmp_line_buffer, line_char_count - 1);
			// Add the line buffer to the list of lines
			line_data->line[line_count++] = line_buffer;
			line_char_count = 0;
			// Clear all the data just written
			memset(tmp_line_buffer, 0, line_char_count);
		}

		++char_count;
	} while (char_count < bufsz && line_count < IDME_MAX_NUM_LINES);

	return line_data;

CLEANUP:
	free_line_data(line_data);
	return NULL;
}

/**
 * Get the value offset for a key
 * @param str Key=Value string
 * @param name Key string
 * @return Offset of Value in the Key=Value string
 */
static unsigned int get_value_offset(char* str, const char* name) {
	char * value = NULL;
	unsigned long key_length = 0;

	value = strchr((const char*) str, (int) '=');
	if (value) {
		// Get the offset of
		key_length = (long)value - (long)str;
		if (strlen(name) != key_length || strncmp(name, str, key_length)) {
			/* Not match for key */
			return 0;
		}

		return ++key_length; // factor in the '=' char
	}

	return 0;
}

/**
 * Load idme files from flash
 * @param buffer Buffer into which to load
 * @param file idme filename
 * @param part_name Name of partition to load from
 * @return Ptr to start of actual data (offsetting header). NULL on failure
 */
static char * load_idme_file(char* buffer, const char * file, const char * part_name) {
	char * buf = NULL;
	char buf_addr[16];
	char part[16];
	int part_num = 0;
	const char * part_type = get_partition_type(part_name);
	IdmeDataHeaderT *hdr = (IdmeDataHeaderT *) buffer;

	memset(buf_addr, 0x0, sizeof(buf_addr));
	memset(part, 0x0, sizeof(part));

	if (sprintf(buf_addr, "0x%lx", (long unsigned int) buffer) < 0)
		return NULL;

	part_num = get_partition_num(part_name);
	if (part <= 0)
		return NULL;

	// Get the idme partition num as "<emmc>:<part_num>"
	if (sprintf(part, "%d:%d", IDME_MMC_DEVICE, part_num) < 0)
		return NULL;
	// Load the file
	if (buffer) {
		char* const argv[] = { NULL, IDME_INTERFACE, part, buf_addr, file };
		// Load the file from the filesystem
		if (strcmp(part_type, EXT3_PARTITION_TYPE) == 0) {
			if (do_ext4_load(NULL, 0, 5, argv) != 0) {
				printf("IDME: failed to load %s\n", file);
				return NULL;
			}
		}
		else if (strcmp(part_type, FAT_PARTITION_TYPE) == 0) {
			if (register_fat_partition(part_num) != ERR_NONE) {
				printf("IDME: failed to register FAT partition\n");
				return NULL;
			}

			// Load the file to fat filesystem
			if (fat_read_file(file, (void *)buffer, 0, 0) == -1) {
				printf("IDME: failed to read %s\n", file);
				return NULL;
			}
		}
	}

	buf = &buffer[sizeof(IdmeDataHeaderT)];
	// Check that the data is not corrupt before parsing
	if (ERR_NONE == validate_idme(hdr, buf)) {
		return buf;
	}

	return NULL;
}

static LineDataT * load_idme_line_data(char* buffer, const char* file, const char* part_name) {

	IdmeDataHeaderT *hdr = (IdmeDataHeaderT *) buffer;
	char * buf = load_idme_file(buffer, file, part_name);
	if (buf) {
		LineDataT *line_data = get_line_data(buf, hdr->size);
		if (line_data != NULL) {
			/* Set the pointer to the raw buffer
			(input with no header) */
			line_data->raw = buf;
			line_data->raw_size = hdr->size;
			/* Get the line contents */
			return line_data;
		}
	}

	return NULL;
}

/**
 * Flush idme data to file
 * @param Pointer to line contents
 * @param file idme filename
 * @param part_name partition name
 * @return error condition
 */
static int flush_to_file(const LineDataT * const line_data, const char * file, const char * part_name) {
	int ret = ERR_UNKNOWN;
	unsigned int i = 0;
	unsigned long buf_sz = 0;
	char buf_addr[16];
	char size_string[16];
	char part[16];
	char * buffer = NULL;
	unsigned int size_to_write = sizeof(IdmeDataHeaderT);
	char * data_buffer = NULL;
	int part_num = 0;
	const char * part_type = get_partition_type(part_name);

	if (!line_data || !file)
		return ERR_INVALID_ARGUMENT;

	buffer = malloc(IDME_MAX_FILE_SIZE);
	if (!buffer)
		return ERR_UNKNOWN;

	data_buffer = (char*) ((long) buffer + sizeof(IdmeDataHeaderT));

	for (i = 0; i < line_data->size; ++i) {
		strcpy(&data_buffer[buf_sz], line_data->line[i]);
		buf_sz += strlen(line_data->line[i]);
		// Add a new line
		data_buffer[buf_sz++] = '\n';
	}

	if (sprintf((char*) buf_addr, "0x%lx", (long unsigned int) buffer) < 0) {
		ret = ERR_UNKNOWN;
		goto CLEANUP;
	}

	part_num = get_partition_num(part_name);
	if (part < 0) {
		ret = ERR_UNKNOWN;
		goto CLEANUP;
	}

	memset((char*) part, 0, sizeof(part));
	// Get the idme partition num as "<emmc>:<part_num>"
	if (sprintf(part, "%d:%d", IDME_MMC_DEVICE, part_num) < 0) {
		ret = ERR_UNKNOWN;
		goto CLEANUP;
	}

	size_to_write += buf_sz;
	if (sprintf((char*) size_string, "0x%lx", (long unsigned int) size_to_write)
			< 0) {
		ret = ERR_UNKNOWN;
		goto CLEANUP;
	}

	if (buf_sz) {
		char* const argv[] = { NULL, IDME_INTERFACE, part, buf_addr, file,
				size_string };
		IdmeDataHeaderT *hdr = (IdmeDataHeaderT *) buffer;
		hdr->sign = IDME_HEADER_SIGN;
		hdr->version = 0;
		hdr->size = buf_sz;
		hdr->crc = crc32(0,
				(const unsigned char *) &buffer[sizeof(IdmeDataHeaderT)],
				buf_sz);

		if (strcmp(part_type, EXT3_PARTITION_TYPE) == 0) {
			// Load the file to ext filesystem
			if (do_ext4_write(NULL, 0, 6, argv) != 0) {
				printf("IDME: failed to write %s\n", file);
				ret = ERR_WRITE_FAILED;
				goto CLEANUP;
			}
		} else if (strcmp(part_type, FAT_PARTITION_TYPE) == 0) {
			// Register the partition as FAT and write to it
			if (register_fat_partition(part_num) != ERR_NONE) {
				printf("IDME: failed to register FAT partition\n");
				ret = ERR_WRITE_FAILED;
				goto CLEANUP;
			}

			// Load the file to fat filesystem
			if (file_fat_write(file, (void *)buffer, size_to_write) == -1) {
				printf("IDME: failed to write %s\n", file);
				ret = ERR_WRITE_FAILED;
				goto CLEANUP;
			}
		}
		else {
			ret = ERR_UNKNOWN;
			goto CLEANUP;
		}

		ret = ERR_NONE;
	}

CLEANUP:
	free(buffer);

	return ret;
}

/**
 * Update an idme variable value
 * @param Pointer to line contents
 * @param name Key of variable
 * @param value Value to set variable
 * @param flush if to write to file
 * @return error condition
 */
static int update_var(LineDataT * line_data, const char *name,
		const char *value, int flush) {
	int ret = ERR_UNKNOWN;
	unsigned int i = 0;
	unsigned int offset = 0;
	const char * new_value = (value == NULL) ? " " : value;

	if (!line_data || !name)
		return ERR_INVALID_ARGUMENT;

	for (i = 0; line_data && i < line_data->size; ++i) {
		offset = get_value_offset(line_data->line[i], name);
		if (offset && offset < IDME_MAX_NUM_LINES) {
			char * val_ptr = (char*) (line_data->line[i] + offset);
			// Update the value
			strncpy(val_ptr, new_value, IDME_MAX_NUM_LINES - offset);
			ret = ERR_NONE;
			if (flush)
				ret = flush_to_file(line_data, IDME_BOOT_FILE_NAME, IDME_PARTITION_NAME);
			break;
		}
	}

	return ret;
}

/**
 * Retrieve an idme variable value
 * @param Pointer to line contents
 * @param name Key of variable
 * @param buf Buffer into which to write the value
 * @param buflen Length of the buffer
 * @return error condition
 */
static int get_var(LineDataT * line_data, const char * name, char *buf,
		int buflen) {
	unsigned int i = 0;
	unsigned int offset = 0;
	int ret = ERR_UNKNOWN;

	if (!line_data || !name || !buf)
		return ERR_INVALID_ARGUMENT;

	for (i = 0; i < line_data->size; ++i) {
		offset = get_value_offset(line_data->line[i], name);
		if (offset) {
			strncpy(buf, line_data->line[i] + offset, buflen);
			ret = ERR_NONE;
			break;
		}
	}

	return ret;
}

/**
 * Get the data buffer
 * @param new_buffer Allocate a new buffer
 */
static char * get_var_buf(int new_buffer) {

	if (new_buffer)
		return malloc(IDME_MAX_FILE_SIZE);
	else if (g_var_buf)
		return g_var_buf;

	g_var_buf = malloc(IDME_MAX_FILE_SIZE);

	return g_var_buf;
}

/**
 * Free the data buffer
 */
static void free_var_buf(void) {

	if (g_var_buf)
		free(g_var_buf);

	g_var_buf = NULL;
}

/**
 * Restore idme data from back up partition
 * @param line_data Pointer to line data structure
 * @param backup Backup file name
 * @param restore Restore file name
 * @param tmp_buffer Do not keep the buffer around
 * @return error condition
 */

static int restore_backup_file(LineDataT **line_data, const char * backup,
		const char * restore, int tmp_buffer) {
	int ret = ERR_UNKNOWN;
	/* Allocate some memory to hold the manufacturing data */
	char *var_buf = get_var_buf(tmp_buffer);
	if (!var_buf)
		return ret;

	if (*line_data) {
		free_line_data(*line_data);
		*line_data = NULL;
	}

	/* Read idme data from back up fat partition */
	*line_data = load_idme_line_data(var_buf, backup, RECOVERY_PARTITION_NAME);
	if (!*line_data && !tmp_buffer) {
		free_var_buf();
		return ret;
	}

	/* Write the data into the idme partition */
	ret = flush_to_file(*line_data, restore, IDME_PARTITION_NAME);

	/* if tmp_buffer != 1, var_buf is allocated memory,
	 * that contains the raw data for idme mfg
	 * This must not be freed, so that u-boot can access this raw data easily
	 * see idme_get_blob_emmc()
	 */
	if (tmp_buffer)
		free(var_buf);

	return ret;
}

/**
 * Retrieve an idme variable value
 * @param name Key of variable
 * @param buf Buffer into which to write the value
 * @param buflen Length of the buffer
 * @return error condition
 */
int idme_get_var_emmc(const char *name, char *buf, int buflen) {
	int ret = ERR_UNKNOWN;

	ret = get_var(g_mfg_lines, name, buf, buflen);
	if (ERR_NONE == ret)
		/* Found */
		return ret;

	return get_var(g_boot_lines, name, buf, buflen);
}

/**
 * Update an idme variable value
 * @param name Key of variable
 * @param value New value
 * @return error condition
 */
int idme_update_var_emmc(const char *name, const char *value) {
	int ret = ERR_UNKNOWN;

	ret = update_var(g_mfg_lines, name, value, 0/* No flush */);
	if (ERR_NONE == ret)
		// Found and updated
		return ret;

	return update_var(g_boot_lines, name, value, 1/*flush to file*/);
}

/**
 * Backup idme data
 * @return error condition
 */
int idme_backup_emmc(void) {
	int ret = ERR_NONE;
	// Write the data into the idme partition
	ret = flush_to_file(g_mfg_lines,
			IDME_MFG_RECOVERY_FILE_NAME, RECOVERY_PARTITION_NAME);
	ret |= flush_to_file(g_boot_lines,
			IDME_BOOT_RECOVERY_FILE_NAME, RECOVERY_PARTITION_NAME);

	return ret;
}

/**
 * Restore idme data
 * @return error condition
 */
int idme_restore_emmc(void) {
	int ret = ERR_NONE;

	ret = restore_backup_file(&g_boot_lines, IDME_BOOT_RECOVERY_FILE_NAME,
			IDME_BOOT_FILE_NAME, 1);
	ret |= restore_backup_file(&g_mfg_lines, IDME_MFG_RECOVERY_FILE_NAME,
			IDME_MFG_FILE_NAME, 0);

	return ret;
}

/**
 * Print all idme variables Key=Value
 * @return error condition
 */
int idme_print_vars_emmc(void) {
	unsigned int i = 0;

	for (i = 0; g_mfg_lines && i < g_mfg_lines->size; ++i)
		printf("%s\n", g_mfg_lines->line[i]);

	for (i = 0; g_boot_lines && i < g_boot_lines->size; ++i)
		printf("%s\n", g_boot_lines->line[i]);

	return ERR_NONE;
}

/**
 * Determine if idme data needs to be recovered and restore recovered data
 * @return error condition
 */
int idme_check_update_emmc(void) {
	int ret = ERR_NONE;
	int use_tmp_buffer = 1;

	if (NULL == g_boot_lines) {
		ret = restore_backup_file(&g_boot_lines, IDME_BOOT_RECOVERY_FILE_NAME,
				IDME_BOOT_FILE_NAME, use_tmp_buffer);
		/* Special case when no boot.idme backup exists */
		if (NULL == g_boot_lines) {
			/* Create and backup the boot entries, if we don't already have a backup */
			g_boot_lines = get_line_data(IDME_BOOT_BACKUP_DATA,
					strlen(IDME_BOOT_BACKUP_DATA));
			if (g_boot_lines) {
				/* Save the data into the recovery partition */
				flush_to_file(g_boot_lines, IDME_BOOT_RECOVERY_FILE_NAME,
						RECOVERY_PARTITION_NAME);
				/* Save the data into the idme partition as well*/
				flush_to_file(g_boot_lines, IDME_BOOT_FILE_NAME,
						IDME_PARTITION_NAME);
			}
		}
	}

	if (NULL == g_mfg_lines) {
		use_tmp_buffer = 0;
		// mfg or boot failed to load, restore it
		ret |= restore_backup_file(&g_mfg_lines, IDME_MFG_RECOVERY_FILE_NAME,
				IDME_MFG_FILE_NAME, use_tmp_buffer);
	}

	return ret;
}

/*
 * Get the entire IDME data blob
 * @return blob size on success
 */
int idme_get_blob_emmc(char **buf)
{
	/* Return the entire chunk of idme raw data */
	if (buf && g_mfg_lines) {
		*buf = g_mfg_lines->raw;
		return g_mfg_lines->raw_size;
	}

	return 0;
}

/**
 * Get data from storage
 * @param name Storage name
 * @param buf Raw buffer to write data into
 * @param buflen length of supplied buffer - or 0 for 'unlimited'
 * @return 0 on success
 */
int idme_get_data_store_emmc(const char *name, char *buf, int buflen)
{
	/* Return the entire chunk of idme raw data */
	unsigned long addr;
	char * data_buf;
	unsigned long file_size;
	const char *addr_str;
	const char *size_str;

	addr_str = getenv("loadaddr");
	if (addr_str != NULL)
		addr = simple_strtoul(addr_str, NULL, 16);
	else
		addr = CONFIG_SYS_LOAD_ADDR;

	data_buf = load_idme_file((char*)addr, name, IDME_PARTITION_NAME);
	if (data_buf) {
		// Get the file size
		size_str = getenv("filesize");
		if (size_str != NULL) {
			file_size = simple_strtoul(size_str, NULL, 16);
			if (buflen > 0 && buflen < file_size)
				return ERR_INVALID_ARGUMENT;

			memcpy(buf, data_buf, file_size);
			return ERR_NONE;
		}
	}

	return ERR_UNKNOWN;
}

/**
 * Initialize - Read relevant idme variables from the filesystem
 *
 * @return 0 on success
 */
int idme_init_emmc(void) {
	char * var_buf = NULL;

	// Allocate some memory to hold the manufacturing data
	var_buf = get_var_buf(0);
	if (!var_buf)
		return -1;

	// Get the boot.idme data
	g_boot_lines = load_idme_line_data(var_buf, IDME_BOOT_FILE_NAME,
			IDME_PARTITION_NAME);
	if (!g_boot_lines)
		printf("idme: boot file get contents failed\n");
	else
		/* Nullify the raw buffer for boot data,
		 * since we re-use the same buffer below
		 */
		g_boot_lines->raw = NULL;

	/* Clear the buffer and re-use for mfg data */
	memset(var_buf, 0, IDME_MAX_FILE_SIZE);
	// Get the mfg.idme data
	g_mfg_lines = load_idme_line_data(var_buf, IDME_MFG_FILE_NAME,
			IDME_PARTITION_NAME);
	if (!g_mfg_lines)
		goto CLEANUP_ALL;

	/* var_buf, is the raw buffer that should be kept alive.
	 Part of the data stored in this buffer is provided via
	 the _get_blob_ method */

	return ERR_NONE;

CLEANUP_ALL:
    free_var_buf();

	return ERR_UNKNOWN;
}
