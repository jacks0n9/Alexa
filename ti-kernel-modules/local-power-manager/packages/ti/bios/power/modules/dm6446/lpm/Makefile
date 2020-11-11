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

# If KERNELRELEASE is undefined, then this makefile has been invoked
# directly from the command line. Invoke the kernel build system.
ifeq ($(KERNELRELEASE),)


# Update these macros to reflect you environment.
#
# LINUXKERNEL_INSTALL_DIR = the Linux kernel source directory
# MVTOOL_PREFIX  = the toolchain directory and decorated name prefix
# DSPLINK_REPO = the repository which contains DSP/BIOS LINK
#
LINUXKERNEL_INSTALL_DIR = _your_kernel_installation_
MVTOOL_PREFIX = _your_codegen_installation_and_name_prefix_
DSPLINK_REPO = _your_dsplink_repository_

# Process DSPLINK flags
LINK_DIR = $(DSPLINK_REPO)/dsplink
LINK_PATH = $(LINK_DIR)/gpp/export/BIN/Linux/DAVINCI/RELEASE
LINK_ORIG := $(shell cat $(LINK_PATH)/DSPLINK.txt)
LINK_DEFS := $(shell cat $(LINK_PATH)/PMGR_defines.txt)
LINK_INCS := $(shell cat $(LINK_PATH)/PMGR_includes.txt)

# remove KBUILD_MODNAME from the list
LINK_DEFS := $(filter-out -DKBUILD_MODNAME=%,$(LINK_DEFS))

# extract only dsplink include paths
LINK_INCS := $(filter -I$(LINK_ORIG)/%,$(LINK_INCS))

# replace original build path with current package path
LINK_INCS := $(patsubst -I$(LINK_ORIG)/%,-I$(LINK_DIR)/%,$(LINK_INCS))

# add internal header files
LINK_INCS_INTERNAL = -I$(LINK_DIR)/gpp/export/INCLUDE/Linux/OMAP3530/internal

ifneq ($(wildcard $(LINK_PATH)/Module.symvers),)
EXTRA_SYMBOLS = $(LINK_PATH)/Module.symvers
endif

MAKE_OPTS = ARCH=arm CROSS_COMPILE=$(MVTOOL_PREFIX)
CFLAGS = -DUSE_UDEV -DOS_LINUX -DLINUX_KERNEL \
         $(LINK_DEFS) $(LINK_INCS) $(LINK_INCS_INTERNAL)

# Invoke the kernel build system
.PHONY: default debug release clean help

default: release

debug: CFLAGS += -g -DDEBUG
debug: release

release:
ifneq ($(EXTRA_SYMBOLS),)
	rm -f Module.symvers
	cat $(foreach file, $(EXTRA_SYMBOLS), $(file)) >> Module.symvers
endif
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

obj-m := lpm_dm6446.o
lpm_dm6446-objs := lpm_driver.o lpm_DaVinci.o tal_dsplink_gpp.o

endif
#
#  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

#

