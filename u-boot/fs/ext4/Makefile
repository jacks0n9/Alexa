#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2003
# Pavel Bartusek, Sysgo Real-Time Solutions AG, pba@sysgo.de
#
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libext4fs.a

AOBJS	=
COBJS-$(CONFIG_FS_EXT4) := ext4fs.o ext4_common.o dev.o ../fs.o
COBJS-$(CONFIG_EXT4_WRITE) += ext4_write.o ext4_journal.o crc16.o

SRCS	:= $(AOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS-y))


all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
