#!/bin/bash

# Note: much of this was done by observing how bitbake does it.

set -x
set -e

function die() {
    echo "FATAL ERROR: $@" 1>&2
    exit 1
}

[ ! -z "$OECORE_TARGET_SYSROOT" ] || die "OE not set up... did you source the OE environment?"
[ ! -z "$OECORE_NATIVE_SYSROOT" ] || die "OE not set up... did you source the OE environment?"
[ -z "$BBPATH" ] || die "never Never NEVER use this in a bitbake script. Shame on you!"

if [ -e aclocal-copy ] ; then
    rm -rf aclocal-copy
fi

mkdir -p aclocal-copy
mkdir -p m4
cp -r ${OECORE_TARGET_SYSROOT}/usr/share/aclocal/. aclocal-copy/
cp ${OECORE_TARGET_SYSROOT}/usr/share/gettext/config.rpath .

glib-gettextize --force --copy

intltoolize --copy --force --automake

autoreconf --verbose --install --force --exclude=autopoint \
    -Im4/ \
    -I${OECORE_NATIVE_SYSROOT}/usr/share/aclocal-1.11 \
    -Iaclocal-copy/

./configure \
    --build=x86_64-linux \
    --host=arm-decibel-linux-gnueabi \
    --target=arm-decibel-linux-gnueabi \
    --prefix=/usr \
    --exec_prefix=/usr \
    --bindir=/usr/bin \
    --sbindir=/usr/sbin \
    --libexecdir=/usr/libexec \
    --datadir=/usr/share \
    --sysconfdir=/etc \
    --sharedstatedir=/com \
    --localstatedir=/var/local \
    --libdir=/usr/lib \
    --includedir=/usr/include \
    --oldincludedir=/usr/include \
    --infodir=/usr/share/info \
    --mandir=/usr/share/man \
    --disable-silent-rules \
    --disable-dependency-tracking \
    --with-libtool-sysroot=${OECORE_TARGET_SYSROOT} \
    --disable-gstreamer \
    --enable-usb \
    --enable-tools \
    --enable-bccmd \
    --enable-hid2hci \
    --enable-dfutool \
    --disable-hidd \
    --disable-pand \
    --disable-dund \
    --disable-cups \
    --enable-test \
    --enable-datafiles \
    "$@"

