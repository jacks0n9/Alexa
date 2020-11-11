#
#  ======== xdcrules.mak ========
#  definition of XDC rules
#

# If not set in the environment, set XDCOPTIONS to verbose (the 'v' option)
XDCOPTIONS ?= v

# This command actually performs the build.
# Note that the "-PD ." says "build the package in this directory ("-P ."),
# and all of the packages it depends on ("-D").
all:
%::
	$(XDC_INSTALL_DIR)/xdc XDCARGS="all obj" XDCPATH="$(XDC_PATH)" \
		XDCOPTIONS=$(XDCOPTIONS) $@ -PD .
