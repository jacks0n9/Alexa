# Modify for your environment

include ../../../../../../Rules.mak


GCARMV7ACGTOOL=$(CSTOOL_DIR)
CODECS_INSTALL_DIR=$(MY_INSTALL_ROOT)/devapp/packages
CROSS_COMPILE=$(CROSS_COMPILE_HOST)-
ENV_DIR=$(MY_INSTALL_ROOT)/packages/ti/mas/env
IPPAFE_REPO=$(MY_INSTALL_ROOT)/devapp/packages
IPPDK_REPO=$(MY_INSTALL_ROOT)/devapp/packages
IPPVP_REPO=$(MY_INSTALL_ROOT)/devapp/packages
TI_include=$(IPPVP_REPO)
TOOLS_DIR=$(DVSDK_INSTALL_DIR)
C64CODEGENTOOL=$(CODEGEN_INSTALL_DIR)

DEVICES = DM3730

TOOLS_DIR=$(DVSDK_INSTALL_DIR)

XDCCGROOT           := $(TOOLS_DIR)/xdctools_3_16_03_36
SDOARCH_INSTALL_DIR := $(TOOLS_DIR)/sdoarch/sdoarch_standards_1_00_00/packages
XDCBUILDCFG         := $(IPPAFE_REPO)/ti/mas/swtools/config.bld
COVPATH             := /apps/cmdevtools/coverity/prevent-linux-3.8.0

# (Mandatory) Specify where various components are installed.
# What you need depends on what device(s) you're building for, what type(s) of
# programs you are building for, and whether your Codec Engine distribution
# is a "big" one that contains all the minor components in its "cetools"
# directory. The legend:
# CE      - Codec Engine (needed for ARM and for DSP)
# XDC     - XDC tools (ARM and DSP)
# BIOS    - DSP/BIOS (DSP only)
# XDAIS   - XDAIS header files (ARM and DSP)
# FC      - Framework Components, various resource managers (ARM and DSP)
# CMEM    - Contiguous memory manager (ARM only)
# DSPLINK - Arm<->DSP communication software (ARM + DSP)
#
# you can omit directory specifications for the components you think you don't
# need (will be warned if you do, based on your DEVICES + PROGRAMS selection
# above).

    ifneq (,$(findstring DM3730, $(DEVICES)))
      CGTOOLS_GCARMV6  := $(GCARMV7ACGTOOL)
      DSPLINK_INSTALL_DIR   := $(LINK_INSTALL_DIR)
      BIOSPSP_INSTALL_DIR  := $(MY_INSTALL_ROOT)/biospsp
    else
      $(error No known DEVICES defined)
    endif

XDC_INSTALL_DIR       := $(XDCCGROOT)

TI_DOXYGEN_TEMPLATES  := $(TOOLS_DIR)/doxygen/TI_Templates/10-01-2007
DOXYGEN_INSTALL_DIR   := $(TOOLS_DIR)/doxygen/current/bin
GRAPHWIZ_INSTALL_DIR  := $(TOOLS_DIR)/Doxygen/Graphviz/2.12/bin
HHC_PATH              := $(TOOLS_DIR)/Doxygen/HTML_Help_Workshop/10-01-2007

# No need to specify the installation directories below if your CE installation
# has cetools/ directory on top
USE_CETOOLS_IF_EXISTS := 1
ifeq ($(USE_CETOOLS_IF_EXISTS), 0)
  XDAIS_INSTALL_DIR     := # xdais install path
  CMEM_INSTALL_DIR      := # linuxutils_2_24_01
  FC_INSTALL_DIR        := # framework_components_2_24
  BIOSUTILS_INSTALL_DIR := # biosutils
endif

# (Mandatory) specify correct compiler paths and names for the architectures
# you'll be building for:



# compiler path and name for TI C64x toolchain. NOTE: make sure the
# directory you specify has a "bin" subdirectory
CGTOOLS_C64P := $(C64CODEGENTOOL)
CC_C64P      = bin/cl6x

# determine which components are necessary based on DEVICES and PROGRAMS
REQUIRE_CE    := 1
REQUIRE_XDC   := 1
REQUIRE_XDAIS := 1
REQUIRE_FC    := 1

REQUIRE_LINK  := 1
REQUIRE_CMEM  := 1
REQUIRE_BIOS  := 1


ifneq (,$(HHC_PATH))
PATH := $(HHC_PATH):$(DOXYGEN_INSTALL_DIR):$(GRAPHWIZ_INSTALL_DIR):$(XDC_INSTALL_DIR):$(shell echo -n $(PATH))
else
PATH := $(DOXYGEN_INSTALL_DIR):$(GRAPHWIZ_INSTALL_DIR):$(XDC_INSTALL_DIR):$(shell echo -n $(PATH))
endif
