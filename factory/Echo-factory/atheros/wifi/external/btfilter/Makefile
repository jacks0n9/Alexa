# Copyright (c) 2008-2011 Atheros Communications Inc.
# Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#USE_GLIB=1
#USE_HCIUTILS=1

ifdef V210
V210_DIR=/tftpboot/ubuntu-taiwan
endif
PREFIX ?= /usr
SBINDIR ?= $(PREFIX)/sbin
MANDIR ?= $(PREFIX)/share/man

PKG_CONFIG ?= pkg-config
ifdef V210
CC=		$(ATH_CROSS_COMPILE_TYPE)gcc
LD=		$(ATH_CROSS_COMPILE_TYPE)ld
else
CC=		$(ATH_CROSSS_COMPILE_TYPE)gcc
LD=		$(ATH_CROSSS_COMPILE_TYPE)ld
endif
SOURCES=abtfilt_main.c \
		abtfilt_wlan.c \
		abtfilt_core.c \
		abtfilt_utils.c \
		btfilter_action.c\
		nl80211_utils.c\
		btfilter_core.c

INCLUDES=	-Iinclude \
		-Ios/linux/include \
		-Icommon/include \
		-I$(V210_DIR)/usr/include/dbus-1.0/ \
		-I$(V210_DIR)/usr/lib/dbus-1.0/include \
		-I$(V210_DIR)/usr/local/include/dbus-1.0 \
		-I$(V210_DIR)/usr/local/lib/dbus-1.0/include \
		-I$(V210_DIR)/usr/include/bluetooth \
		-I$(V210_DIR)/usr/local/include/bluetooth \

CFLAGS=		-Wall -g -DABF_DEBUG
#LIBS=		-ldbus-1 -lpthread -lbtfilt -lrt -lbluetooth
LIBS=		-ldbus-1 -lpthread -lrt -lbluetooth

NL1FOUND := $(shell $(PKG_CONFIG) --atleast-version=1 libnl-1 && echo Y)
NL2FOUND := $(shell $(PKG_CONFIG) --atleast-version=2 libnl-2.0 && echo Y)

ifeq ($(NL1FOUND),Y)
NLLIBNAME = libnl-1
endif

ifeq ($(NL2FOUND),Y)
CFLAGS += -DCONFIG_LIBNL20
LIBS += -lnl-genl
NLLIBNAME = libnl-2.0
endif

#ifeq ($(NLLIBNAME),)
#$(error Cannot find development files for any supported version of libnl)
#endif

LIBS += $(shell $(PKG_CONFIG) --libs $(NLLIBNAME))
CFLAGS += $(shell $(PKG_CONFIG) --cflags $(NLLIBNAME))


ifdef USE_HCIUTILS
SOURCES += abtfilt_bluez_hciutils.c
LIBS += -ldl
else
CFLAGS += -DCONFIG_NO_HCILIBS
endif

ifdef USE_GLIB
SOURCES += abtfilt_bluez_dbus_glib.c

LIBS += -lgobject-2.0 -lglib-2.0 -ldbus-glib-1

INCLUDES += -I/usr/include/glib-2.0     \
			-I/usr/lib/glib-2.0/include \

OBJECTS=	$(SOURCES:.c=.o)

else
SOURCES += abtfilt_bluez_dbus.c

OBJECTS=	$(SOURCES:.c=.o)
endif



#LDFLAGS=	-L$(WORKAREA)/host/btfilter
#copy libbluetooth.so from /tftpboot/ubuntu-taiwan/usr/lib to /tftpboot/ubuntu-taiwan/lib [for V210]
ifdef V210
LDFLAGS=	-L/tftpboot/ubuntu-taiwan/lib
endif
FILTERAPP=	abtfilt

all: $(OBJECTS)
	$(CC) -o $(FILTERAPP) $(OBJECTS) $(LDFLAGS) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f $(FILTERAPP) $(OBJECTS)
