/*
 * board-omap3misto-id.c
 *
 * Copyright (C) 2008,2010,2012 Amazon Technologies, Inc. All rights reserved.
 * Jon Mayo <jonmayo@amazon.com>
 * Arnaud Froment <afroment@amazon.com>
 *
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sysdev.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/proc_fs.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <plat/board-omap3misto-id.h>
#include <linux/list.h>

#define DRIVER_VER "1.0"
#define DRIVER_INFO "Board ID and Serial Number driver for Lab126 boards version " DRIVER_VER

#define BOARDID_USID_PROCNAME		"usid"
#define BOARDID_PROCNAME_BOARDID	"board_id"
#define BOARDID_PROCNAME_WIFIMACADDR	"wifi_mac_addr"
#define BOARDID_PROCNAME_BTMACADDR	"bt_mac_addr"
#define BOARDID_PROCNAME_MACSEC		"mac_sec"
#define BOARDID_PROCNAME_BOOTMODE	"bootmode"
#define BOARDID_PROCNAME_POSTMODE	"postmode"
#define BOARDID_PROCNAME_BOARDREV	"board_rev"

#define SERIAL_NUM_SIZE         16
#define BOARD_ID_SIZE           16

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

extern char* get_omap3_misto_rev_str(void);
extern void get_default_board_id(char*, int);

char omap3_serial_number[SERIAL_NUM_SIZE + 1];
EXPORT_SYMBOL(omap3_serial_number);

char omap3_board_id[BOARD_ID_SIZE + 1];
EXPORT_SYMBOL(omap3_board_id);

char omap3_wifi_mac_address[MAC_ADDR_SIZE + 1];
EXPORT_SYMBOL(omap3_wifi_mac_address);

char omap3_bt_mac_address[MAC_ADDR_SIZE + 1];
EXPORT_SYMBOL(omap3_bt_mac_address);

char omap3_mac_secret[MAC_SEC_SIZE + 1];
EXPORT_SYMBOL(omap3_mac_secret);

char omap3_bootmode[BOOTMODE_SIZE + 1];
char omap3_postmode[BOOTMODE_SIZE + 1];
EXPORT_SYMBOL(omap3_postmode);


/* Global for holding all idme data */
LIST_HEAD(g_idme_list);

#define PCBSN_X_INDEX 5
char omap3_pcbsn_x(void)
{
  return omap3_board_id[PCBSN_X_INDEX];
}
EXPORT_SYMBOL(omap3_pcbsn_x);


static int proc_id_read(char *page, char **start, off_t off, int count,
				int *eof, void *data, char *id)
{
	if (!id)
		return 0;

	strcpy(page, id);
	*eof = 1;

	return strlen(page);
}

#define PROC_ID_READ(id) proc_id_read(page, start, off, count, eof, data, id)

struct idme_entry {
	char *key; /* Idme key */
	char *value; /* Idme value */
	struct list_head list; /* list */
};

static char *get_idme_value(const char *key)
{
	struct list_head *pos;
	int ret = 0;

	list_for_each(pos, &g_idme_list) {
		struct idme_entry *tmp = list_entry(pos, struct idme_entry, list);
		if (!tmp)
			continue;
		ret = strcmp(tmp->key, key);
		if (!ret)
			return tmp->value;
	}

	return NULL;
}

static char *add_idme_entry(char *key, char *value)
{
	struct idme_entry *tmp;

	if (!key || !value)
		return NULL;

	tmp = (struct idme_entry *)kmalloc(sizeof(struct idme_entry), GFP_KERNEL);
	if (!tmp)
		return NULL;

	tmp->key = kmalloc(strlen(key), GFP_KERNEL);
	if (!tmp->key) {
		kfree(tmp);
		return NULL;
	}

	/* Copy the key */
	strcpy(tmp->key, key);

	tmp->value = kmalloc(strlen(value), GFP_KERNEL);
	if (!tmp->value) {
		kfree(tmp->key);
		kfree(tmp);
		return NULL;
	}

	/* Copy the value */
	strcpy(tmp->value, value);

	list_add(&(tmp->list), &g_idme_list);

	return tmp->key;
}

static int proc_usid_read(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
        return PROC_ID_READ(omap3_serial_number);
}

static int proc_board_id_read(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
        return PROC_ID_READ(omap3_board_id);
}


static int proc_wifi_mac_address_read(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
        return PROC_ID_READ(omap3_wifi_mac_address);
}

static int proc_bt_mac_address_read(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
        return PROC_ID_READ(omap3_bt_mac_address);
}

static int proc_mac_secret_read(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
        return PROC_ID_READ(omap3_mac_secret);
}

static int proc_bootmode_read(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
        return PROC_ID_READ(omap3_bootmode);
}

static int proc_postmode_read(char *page, char **start, off_t off, int count,
				int *eof, void *data)
{
        return PROC_ID_READ(omap3_postmode);
}

static int proc_boardrev_read(char *page, char **start, off_t off, int count,
		int *eof, void *data)
{
	return PROC_ID_READ(get_omap3_misto_rev_str());
}

static int idme_read(char *page, char **start, off_t off, int count,
		int *eof, void *data)
{
	return PROC_ID_READ(get_idme_value((char *)data));
}

int bootmode_is_diags(void)
{
	return (strncmp(system_bootmode, "diags", 5) == 0);
}
EXPORT_SYMBOL(bootmode_is_diags);

/**
 * Create a proc file entry for a key
 * @param key Proc fs file name (must be a valid string pointer)
 * @param key_len
 */
static int idme_create_proc(const char *key)
{
	struct proc_dir_entry *proc = create_proc_entry(key, S_IRUGO, NULL);

	if (!key)
		return -1;

	proc->data = (void *)key;
	proc->read_proc = idme_read;
	proc->write_proc = NULL;

	return 0;

}

/**
 * Initialize idme variables.
 *  This parses the idme tag field in the kernel and populates
 *  list of key-value pairs
 */
static void omap3_idme_init(void)
{
	/* Copy idme data out of the system tags */
	unsigned int i = 0;
	unsigned int write_value = 0;
	unsigned int key_len = 0;
	unsigned int value_len = 0;
	char ch;
	char key[64];
	char value[64];
	char *entry_key;

	do {
		ch = system_idme[i++];
		if (ch != '\n') {
			if (ch == '=') {
				write_value = 1;
				continue;
			}

			if (!write_value)
				key[key_len++] = ch;
			else
				value[value_len++] = ch;
		} else {
			/* Add NULL character to the end of both key and value */
			key[key_len] = '\0';
			value[value_len] = '\0';
			/* Add the entry to the list */
			entry_key = add_idme_entry(key, value);
			if (entry_key)
				idme_create_proc(entry_key);
			/* Reset all */
			write_value = 0;
			key_len = 0;
			value_len = 0;
		}
	} while (i < system_idme_size);
}

/* initialize the proc accessors */
static int __init omap3_board_proc_init(void)
{
	struct proc_dir_entry *proc_usid = create_proc_entry(BOARDID_USID_PROCNAME, S_IRUGO, NULL);
	struct proc_dir_entry *proc_board_id = create_proc_entry(BOARDID_PROCNAME_BOARDID, S_IRUGO, NULL);
	struct proc_dir_entry *proc_wifi_mac_address = create_proc_entry(BOARDID_PROCNAME_WIFIMACADDR, S_IRUGO, NULL);
	struct proc_dir_entry *proc_bt_mac_address = create_proc_entry(BOARDID_PROCNAME_BTMACADDR, S_IRUGO, NULL);
	struct proc_dir_entry *proc_mac_secret = create_proc_entry(BOARDID_PROCNAME_MACSEC, S_IRUGO, NULL);
	struct proc_dir_entry *proc_bootmode = create_proc_entry(BOARDID_PROCNAME_BOOTMODE, S_IRUGO, NULL);
	struct proc_dir_entry *proc_postmode = create_proc_entry(BOARDID_PROCNAME_POSTMODE, S_IRUGO, NULL);
	struct proc_dir_entry *proc_boardrev = create_proc_entry(BOARDID_PROCNAME_BOARDREV, S_IRUGO, NULL);

	if (proc_usid != NULL) {
		proc_usid->data = NULL;
		proc_usid->read_proc = proc_usid_read;
		proc_usid->write_proc = NULL;
	}

	if (proc_board_id != NULL) {
		proc_board_id->data = NULL;
		proc_board_id->read_proc = proc_board_id_read;
		proc_board_id->write_proc = NULL;
	}

	if (proc_wifi_mac_address != NULL) {
		proc_wifi_mac_address->data = NULL;
		proc_wifi_mac_address->read_proc = proc_wifi_mac_address_read;
		proc_wifi_mac_address->write_proc = NULL;
	}

	if (proc_bt_mac_address != NULL) {
		proc_bt_mac_address->data = NULL;
		proc_bt_mac_address->read_proc = proc_bt_mac_address_read;
		proc_bt_mac_address->write_proc = NULL;
	}

	if (proc_mac_secret != NULL) {
		proc_mac_secret->data = NULL;
		proc_mac_secret->read_proc = proc_mac_secret_read;
		proc_mac_secret->write_proc = NULL;
	}

	if (proc_bootmode != NULL) {
		proc_bootmode->data = NULL;
		proc_bootmode->read_proc = proc_bootmode_read;
		proc_bootmode->write_proc = NULL;
	}

	if (proc_postmode != NULL) {
		proc_postmode->data = NULL;
		proc_postmode->read_proc = proc_postmode_read;
		proc_postmode->write_proc = NULL;
	}

	if (proc_boardrev != NULL) {
		proc_boardrev->data = NULL;
		proc_boardrev->read_proc = proc_boardrev_read;
		proc_boardrev->write_proc = NULL;
	}

	omap3_idme_init();

	return 0;
}
module_init(omap3_board_proc_init);

/* copy the serial numbers from the special area of memory into the kernel */
static void omap3_serial_board_numbers(void)
{
	memcpy(omap3_serial_number, system_serial16, MIN(SERIAL_NUM_SIZE, sizeof(system_serial16)));
	omap3_serial_number[SERIAL_NUM_SIZE] = '\0';

	memcpy(omap3_board_id, system_rev16, MIN(BOARD_ID_SIZE, sizeof(system_rev16)));
	omap3_board_id[BOARD_ID_SIZE] = '\0';

	memcpy(omap3_wifi_mac_address, system_wifi_mac_addr, MIN(sizeof(omap3_wifi_mac_address)-1, sizeof(system_wifi_mac_addr)));
	omap3_wifi_mac_address[sizeof(omap3_wifi_mac_address)-1] = '\0';

	memcpy(omap3_bt_mac_address, system_bt_mac_addr, MIN(sizeof(omap3_bt_mac_address)-1, sizeof(system_bt_mac_addr)));
	omap3_bt_mac_address[sizeof(omap3_bt_mac_address)-1] = '\0';

	memcpy(omap3_mac_secret, system_mac_sec, MIN(sizeof(omap3_mac_secret)-1, sizeof(system_mac_sec)));
	omap3_mac_secret[sizeof(omap3_mac_secret)-1] = '\0';

	memcpy(omap3_bootmode, system_bootmode, MIN(sizeof(omap3_bootmode)-1, sizeof(system_bootmode)));
	omap3_bootmode[sizeof(omap3_bootmode)-1] = '\0';

	memcpy(omap3_postmode, system_postmode, MIN(sizeof(omap3_postmode)-1, sizeof(system_postmode)));
	omap3_postmode[sizeof(omap3_postmode)-1] = '\0';

	pr_debug ("misto Board id - %s\n", omap3_board_id);
	pr_debug ("misto Serial # - %s\n", omap3_serial_number);
}

void omap3_misto_init_boardid(void)
{
	pr_debug(DRIVER_INFO "\n");

	omap3_serial_board_numbers();
}
