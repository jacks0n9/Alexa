/*
 * (c) Copyright 2011 by Tigris Elektronik GmbH
 *
 * Author:
 *  Maximilian Schwerin <mvs@tigris.de>
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

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <errno.h>
#include <fat.h>
#include <mmc.h>

/* Reference to names in env_common.c */
extern uchar default_environment[];

char *env_name_spec = "FAT";

env_t *env_ptr = NULL;
block_dev_desc_t *dev_desc = NULL;

DECLARE_GLOBAL_DATA_PTR;


uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV
int saveenv(void)
{
	int err;

	if(!env_ptr || !dev_desc)
		return 1;


	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
	err = file_fat_write(FAT_ENV_FILE, (void *)env_ptr, CONFIG_ENV_SIZE);
	if (err == -1) {
		printf("\n** Unable to write \"%s\" from %s%d:%d **\n",
			FAT_ENV_FILE, FAT_ENV_INTERFACE, FAT_ENV_DEVICE, FAT_ENV_PART);
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

void env_relocate_spec(void)
{
	int dev = FAT_ENV_DEVICE;
	int part = FAT_ENV_PART;
	int err;

#ifdef CONFIG_MMC
	if (strcmp(FAT_ENV_INTERFACE, "mmc") == 0) {
		struct mmc *mmc = find_mmc_device(dev);

		if (!mmc) {
			printf("no mmc device at slot %x\n", dev);
			return;
		}

		mmc->has_init = 0;
		mmc_init(mmc);
	}
#endif /* CONFIG_MMC */


	dev_desc = get_dev(FAT_ENV_INTERFACE, dev);
	if (dev_desc == NULL) {
		printf("Failed to find %s%d\n",
				FAT_ENV_INTERFACE, dev);
		set_default_env();
		return;
	}

	err = fat_register_device(dev_desc, part);
	if (err) {
		printf("Failed to register %s%d:%d\n",
			FAT_ENV_INTERFACE, dev, part);
		set_default_env();
		return;
	}

	err = fat_read_file(FAT_ENV_FILE, (uchar *)env_ptr, 0, CONFIG_ENV_SIZE);
	if (err == -1) {
		printf("\n** Unable to read \"%s\" from %s%d:%d **\n",
			FAT_ENV_FILE, FAT_ENV_INTERFACE, dev, part);
		set_default_env();
		return;
	}

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc) {
		printf("\n** CRC Failed \"%s\" from %s%d:%d **\n",
					FAT_ENV_FILE, FAT_ENV_INTERFACE, dev, part);
		set_default_env();
		return;
	}
}
