#############################################################################
#
#	Makefile for building the gpio-event kernel module
#
#############################################################################

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= gpio-event-drv.o

else

PWD       	= $(shell pwd)

OVEROTOP	?= /home/a0271467
#CROSS_COMPILE	?= $(OVEROTOP)/CodeSourcery/Sourcery_G++_Lite/bin
CROSS_COMPILE	?= $(OVEROTOP)/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-
KERNEL_PATH     ?= $(OVEROTOP)/Lab126/SitaraAM37x_sdk/psp/linux-2.6.37-psp04.02.00.07.sdk
ARCH		?= arm

export ARCH CROSS_COMPILE

KBUILD_FLAGS 	= -C $(KERNEL_PATH) M=$(PWD)

default: modules

modules:
	$(MAKE) $(KBUILD_FLAGS) modules

clean:
	rm -rf *.o *~ *.ko *.mod.c .*.cmd .tmp* modules.order Module.symvers
endif

