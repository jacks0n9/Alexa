# 
#  Copyright 2011 by Texas Instruments Incorporated.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; version 2 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
# 
#

#
# This makefile will build the ocvc_dm6446.ko driver.
#

# If KERNELRELEASE is undefined, then this makefile has been invoked
# directly from the command line. Invoke the kernel build system.
ifeq ($(KERNELRELEASE),)


# Update these macros to reflect you environment.
#
# LINUXKERNEL_INSTALL_DIR = the Linux kernel source directory
# MVTOOL_PREFIX = the toolchain directory and decorated name prefix
#
LINUXKERNEL_INSTALL_DIR = _your_kernel_installation_
MVTOOL_PREFIX = _your_codegen_installation_and_name_prefix_

MAKE_OPTS = ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX)
CFLAGS = -DOS_LINUX -DLINUX_KERNEL

# Invoke the kernel build system
.PHONY: default debug release clean help

default: release

debug: CFLAGS += -g -DDEBUG
debug: release

release:
	$(MAKE) -C $(LINUXKERNEL_INSTALL_DIR) M=$(CURDIR) $(MAKE_OPTS) \
        EXTRA_CFLAGS="$(CFLAGS)" modules

clean:
	$(MAKE) -C $(LINUXKERNEL_INSTALL_DIR) M=$(CURDIR) $(MAKE_OPTS) clean
	rm -f modules.order

help:
	$(MAKE) -C $(LINUXKERNEL_INSTALL_DIR) M=$(CURDIR) help


# If KERNELRELEASE is defined, then this makefile has been invoked
# from the kernel build system. Use native build language.
else 

obj-m := ocvc_dm6446.o
ocvc_dm6446-objs := ocvc_DaVinci.o

endif
#
#  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

#

