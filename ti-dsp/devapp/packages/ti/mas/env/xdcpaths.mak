#
#  ======== xdcpaths.mak ========
#  definition of XDC paths and commands
#
#  This makefile constructs the "search path" for the XDC tools where it finds
#  numerous components and packages needed to build Codec Engine examples and
#  programs.
#
#  USER NOTE:
#  1) you must specify various <component>_INSTALL_DIRs directores below to
#     reflect your installation, where <component> is CE for Codec Engine,
#     BIOS for DSP/BIOS, etc.
#  2) you must specify compiler path and name in CGTOOLS_* variables below
#  3) you can remove some of the devices from the "DEVICES" list and/or remove
#     some of the types of binaries from the "PROGRAMS" list to reduce
#     the build time (and possibly avoid checking for presence of a component
#     you don't need)

# Edit Rules.mak for your environment

export XDCCGROOT
export SDOARCH_INSTALL_DIR
export C64CODEGENTOOL
export XDCBUILDCFG

ifeq ("$(HOST_OS)","windows")
export WINCECGTOOL
export WINCEPROJROOT
export CGTOOLS_WINCE
else
ifneq (,$(findstring LEO, $(DEVICES)))
  export GCARMV6CGTOOL
  export CGTOOLS_GCARMV6
else
  ifneq (,$(findstring DM6446, $(DEVICES)))
    export MVARM9CGTOOL
    export CGTOOLS_MVARM9
  else
    ifneq (,$(findstring DM3730, $(DEVICES)))
      export GCARMV7ACGTOOL
      export CGTOOLS_GCARMV7
    endif
  endif
endif
endif

export CSL_INSTALL_DIR
export TI_DOXYGEN_TEMPLATES
export DOXYGEN_INSTALL_DIR
export GRAPHWIZ_INSTALL_DIR
export HHC_PATH
export COVPATH
export EDMA3LLD_INSTALL_DIR
export BIOSPSP_INSTALL_DIR
export FULL_SERVER

# -----------------------------------------------------------------------------

# Build the XDC path from the necessary components, verifying along the way
# that the required compoments are present
XDC_PATH :=

ERRMSG = which is invalid (could not find file "$(TEST_FILE)"). Set this in <codec engine examples>/xdcpaths.mak! See the build documentation to correct this error.

# CE_INSTALL_DIR is the location of your Codec Engine.
ifeq ($(REQUIRE_CE), 1)
    TEST_FILE := $(CE_INSTALL_DIR)/packages/ti/sdo/ce/package.xdc
    ifeq ($(wildcard $(TEST_FILE)),)
        $(error CE_INSTALL_DIR is set to "$(CE_INSTALL_DIR)", $(ERRMSG))
    endif
    XDC_PATH := $(CE_INSTALL_DIR)/packages
endif

# Add cetools to XDCPATH if
# 1) $(USE_CETOOLS_IF_EXISTS) is set, and
# 2) the CE distribution has "cetools/"
USING_CETOOLS := 0
ifeq ($(USE_CETOOLS_IF_EXISTS), 1)
    ifneq ($(wildcard $(CE_INSTALL_DIR)/cetools),)
        USING_CETOOLS := 1
        XDC_PATH := $(CE_INSTALL_DIR)/cetools/packages;$(XDC_PATH)
    endif
endif

# DSPLINK_INSTALL_DIR is the location of your DSPLINK distribution
ifeq ($(REQUIRE_LINK), 1)
    TEST_FILE := $(DSPLINK_INSTALL_DIR)/dsplink/gpp/package.xdc
    ifeq ($(wildcard $(TEST_FILE)),)
        $(error DSPLINK_INSTALL_DIR is set to "$(DSPLINK_INSTALL_DIR)", $(ERRMSG))
    endif
    XDC_PATH := $(XDC_PATH);$(DSPLINK_INSTALL_DIR)
endif

ifeq ($(USING_CETOOLS),0)
    # XDAIS_INSTALL_DIR is the location of your XDAIS distribution
    ifeq ($(REQUIRE_XDAIS), 1)
        TEST_FILE := $(XDAIS_INSTALL_DIR)/packages/ti/xdais/package.xdc
        ifeq ($(wildcard $(TEST_FILE)),)
            $(error XDAIS_INSTALL_DIR is set to "$(XDAIS_INSTALL_DIR)", $(ERRMSG))
        endif
        XDC_PATH := $(XDC_PATH);$(XDAIS_INSTALL_DIR)/packages
    endif

    # CMEM_INSTALL_DIR is the location of your CMEM distribution
    ifeq ($(REQUIRE_CMEM), 1)
        TEST_FILE := $(CMEM_INSTALL_DIR)/packages/ti/sdo/linuxutils/cmem/package.xdc
        ifeq ($(wildcard $(TEST_FILE)),)
            $(error CMEM_INSTALL_DIR is set to "$(CMEM_INSTALL_DIR)", $(ERRMSG))
        endif
        XDC_PATH := $(XDC_PATH);$(CMEM_INSTALL_DIR)/packages
    endif

    # FC_INSTALL_DIR is the location of your Frameworks Components distribution
    ifeq ($(REQUIRE_FC), 1)
        TEST_FILE := $(FC_INSTALL_DIR)/packages/ti/sdo/fc/dskt2/package.xdc
        ifeq ($(wildcard $(TEST_FILE)),)
            $(error FC_INSTALL_DIR is set to "$(FC_INSTALL_DIR)", $(ERRMSG))
        endif
        XDC_PATH := $(XDC_PATH);$(FC_INSTALL_DIR)/packages
    endif

    # BIOSUTILS_INSTALL_DIR is the location of your BIOSUTILS distribution
    ifeq ($(REQUIRE_BIOS), 1)
        TEST_FILE := $(BIOSUTILS_INSTALL_DIR)/packages/ti/bios/utils/package.xdc
        ifeq ($(wildcard $(TEST_FILE)),)
            $(error BIOSUTILS_INSTALL_DIR is set to "$(BIOSUTILS_INSTALL_DIR)", $(ERRMSG))
        endif
        XDC_PATH := $(XDC_PATH);$(BIOSUTILS_INSTALL_DIR)/packages
    endif
endif

# BIOS_INSTALL_DIR is the location of your BIOS distribution
ifeq ($(REQUIRE_BIOS), 1)
    TEST_FILE := $(BIOS_INSTALL_DIR)/packages/ti/bios/package.xdc
    ifeq ($(wildcard $(TEST_FILE)),)
        $(error BIOS_INSTALL_DIR is set to "$(BIOS_INSTALL_DIR)", $(ERRMSG))
    endif
    XDC_PATH := $(XDC_PATH);$(BIOS_INSTALL_DIR)/packages
endif

# XDC_INSTALL_DIR is the location of your XDCTOOLS installation.
ifeq ($(REQUIRE_XDC), 1)
    TEST_FILE := $(XDC_INSTALL_DIR)/packages/xdc/package.xdc
    ifeq ($(wildcard $(TEST_FILE)),)
        $(error XDC_INSTALL_DIR is set to "$(XDC_INSTALL_DIR)", $(ERRMSG))
    endif
endif

# XDC_PATH is complete. Any other components you could add as
# XDC_PATH := <your component>/packages;$(XDC_PATH)
XDC_PATH := $(BIOS_INSTALL_DIR)/packages/ti/bios/include;$(XDC_PATH)
XDC_PATH := $(CSL_INSTALL_DIR)/inc;$(XDC_PATH)
XDC_PATH := $(SDOARCH_INSTALL_DIR);$(XDC_PATH)
XDC_PATH := $(DOXYGEN_INSTALL_DIR);$(XDC_PATH)
XDC_PATH := $(IPPDK_REPO);$(IPPAFE_REPO);$(XDC_PATH)
XDC_PATH := $(CE_INSTALL_DIR)/cetools/packages/ti/xdais;$(XDC_PATH)
XDC_PATH := $(CODEC_REPOS);$(XDC_PATH)
XDC_PATH := $(EDMA3LLD_INSTALL_DIR)/packages;$(XDC_PATH)
XDC_PATH := $(BIOSPSP_INSTALL_DIR)/packages;$(XDC_PATH)
XDC_PATH := $(XDC_INSTALL_DIR)/packages;$(XDC_PATH)
XDC_PATH := $(LPM_INSTALL_DIR)/packages;$(XDC_PATH)
XDC_PATH := $(TOOLS_DIR)/codecs-omap3530_4_02_00_00/packages;$(XDC_PATH)
XDC_PATH := $(LINK_INSTALL_DIR)/dsplink/gpp/inc/usr;$(XDC_PATH)
XDC_PATH := $(LINK_INSTALL_DIR)/dsplink/gpp/inc/sys/Linux;$(XDC_PATH)
ifeq ($(MSPCORE_BUILD_FROM_TAR),y)
XDC_PATH := $(XDC_PATH);$(MSPCORE_LIB_DIR)/flat
endif

# +++TODO: Uncomment this only when building devnodeServer outside
#          of a view setting.  Replace /tmp/ippdk/others/packages
#          by a path pointing to ti/sdo/codecs/msrtadec and msrtaenc/
#XDC_PATH := /tmp/ippdk/others/packages;$(XDC_PATH)


