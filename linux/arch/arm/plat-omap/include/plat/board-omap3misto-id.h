/*
 * board-omap3misto-id.h
 *
 * (C) Copyright 2012 Amazon Technologies, Inc.  All rights reserved.
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

#ifndef __PLAT_BOARDID_H__
#define __PLAT_BOARDID_H__

/* misto board id's, a u8 */
#define MISTO_BOARD_EVT_2_0     0x04
#define MISTO_BOARD_EVT_2_1     0x03
#define MISTO_BOARD_EVT_3_0     0x0A
#define MISTO_BOARD_EVT_3_1     0x12
#define MISTO_BOARD_EVT_3_2     0x1A
#define MISTO_BOARD_P_PROTO_1_0   0x09
#define MISTO_BOARD_P_PROTO_2_0   0x11
#define MISTO_BOARD_P_PROTO_3_0   0x21
#define MISTO_BOARD_P_EVT_1_0   0x29
#define MISTO_BOARD_P_DVT_1_0   0x31

/* Board is P_ Bit */
#define MISTO_BOARD_IS_P   0x1

/* board-omap3misto.c */
u8 get_omap3_misto_rev(void);

/* board-omap3misto-id.c */
void omap3_misto_init_boardid(void);
char omap3_pcbsn_x(void);
int bootmode_is_diags(void);

extern char omap3_serial_number[];
extern char omap3_wifi_mac_address[];
extern char omap3_bt_mac_address[];
extern char omap3_postmode[];

#endif // __PLAT_BOARDID_H__

