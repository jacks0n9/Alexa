/*
 * cmd_idme.c
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
#include <idme.h>

/// External definitions for device implementations
#define DEFINE_IDME_FUNCTIONS(dev) \
		extern int idme_init_##dev(void); \
		extern int idme_get_var_##dev(const char *name, char *buf, int buflen); \
		extern int idme_update_var_##dev(const char *name, const char *value); \
		extern int idme_backup_##dev(void); \
		extern int idme_restore_##dev(void); \
		extern int idme_check_update_##dev(void); \
		extern int idme_get_blob_##dev(char **buf); \
		extern int idme_get_data_store_##dev(const char *name, char *buf, int buflen); \
		extern int idme_print_vars_##dev(void);

// eMMC definitions
DEFINE_IDME_FUNCTIONS(emmc)

/// Idme function structure
struct idme_func_s {
	int (*init)(void);
	int (*get_var)(const char *name, char *buf, int buflen);
	int (*update_var)(const char *name, const char *value);
	int (*backup)(void);
	int (*restore)(void);
	int (*check)(void);
	int (*get_blob)(char **buf);
	int (*get_data_store)(const char *name, char *buf, int buflen);
	int (*print)(void);
};

/// Idme function lookup table
static struct idme_func_s func_lookup[] = {
		// EMMC_FLASH_DEVICE
		{
				.init = idme_init_emmc,
				.get_var = idme_get_var_emmc,
				.update_var = idme_update_var_emmc,
				.backup = idme_backup_emmc,
				.restore = idme_restore_emmc,
				.check = idme_check_update_emmc,
				.get_blob = idme_get_blob_emmc,
				.get_data_store = idme_get_data_store_emmc,
				.print = idme_print_vars_emmc
		}
};

/// Device identification - nand / emmc
static int device_index = -1;

/**
 * Print all idme vars
 * @return 0 on success
 */
static int idme_print_vars(void)
{
	return (-1 == device_index) ? -1 : func_lookup[device_index].print();
}
/**
 * Retrieve an IDME field value
 * @param[in] name Name of IDME field
 * @param[out] buf Buffer into which to write field value
 * @param[in] buflen Size of buffer
 * @return 0 on success
 */
int idme_get_var(const char *name, char *buf, int buflen)
{
	return (-1 == device_index) ? -1 : func_lookup[device_index].get_var(name, buf, buflen);
}

/**
 * Store an IDME field value
 * @param[in] name Name of IDME field
 * @param[in] value Value to be updated with
 * @return 0 on success
 */
int idme_update_var(const char *name, const char *value)
{

	return (-1 == device_index) ? -1 :
		func_lookup[device_index].update_var(name, value);
}

/**
 * Back up contents of idme var buf to special area of nand
 * @return 0 on success
 */
int idme_backup(void)
{

	return (-1 == device_index) ? -1 : func_lookup[device_index].backup();
}

/*
 * Restore the contents of IDME from backup storage
 * @return 0 on success
 */
int idme_restore()
{
	return (-1 == device_index) ? -1 : func_lookup[device_index].restore();
}

/*
 * Check and restore IDME data from backup if corruption is detected
 * @return 0 on success
 */
int idme_check_update(void)
{
	return (-1 == device_index) ? -1 : func_lookup[device_index].check();
}

/*
 * Get the entire IDME data blob
 * @return blob size on success
 */
int idme_get_blob(char **buf)
{
	return (-1 == device_index) ? -1 :
		func_lookup[device_index].get_blob(buf);
}

/**
 * Get data from storage
 * @param name Storage name
 * @param buf Raw buffer to write data into
 * @param buflen length of supplied buffer
 * @return 0 on success
 */
int idme_get_data_store(const char *name, char *buf, int buflen)
{
	return (-1 == device_index) ? -1 :
		func_lookup[device_index].get_data_store(name, buf, buflen);
}

/**
 * Initialize idme type
 * @param flash_type Type of flash device
 * @return 0 on success
 */
int idme_init(int flash_type)
{
	if (flash_type >= 0 && flash_type < sizeof(func_lookup)) {
		device_index = flash_type;
		return func_lookup[device_index].init();
	}

	return -1;
}

/*
 * Main command line method
 * @return 0 on success
 */
int do_idme(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc == 1 || strncmp(argv[1], "?", 1) == 0) {
		return idme_print_vars();
	} else if (argc <= 3) {
		char *value;

		if (argc == 3) {
			value = argv[2];
			printf("setting '%s' to '%s'\n", argv[1], argv[2]);
		} else {
			value = NULL;
			printf("clearing '%s'\n", argv[1]);
		}

		return idme_update_var(argv[1], value);
	}

	printf("Usage:\n%s", cmdtp->usage);
	return 1;
}

/***************************************************/

U_BOOT_CMD(
	idme, 3, 1, do_idme,
	"set/query IDME nv ram variables",
	"<var> <value>\n"
	"    - set <var> to <value>\n"
	"<var>\n"
	"    - clears <var>\n"
	"\"?\" or no args\n"
	"    - print all known variables\n"
);
