# Define target platform.
PLATFORM=dm3730

# The installation directory of the SDK.
# Keep old's SDK location for now till everyone switch to OE
# The new location, ie /usr/local/dvsdk can be set in OE environment
DVSDK_INSTALL_DIR?=$(HOME)/ti-dvsdk_dm3730-evm_04_03_00_06

# Where dsp software is installed
export MY_INSTALL_ROOT?=$(shell pwd)
# For backwards compatibility
DVEVM_INSTALL_DIR=$(DVSDK_INSTALL_DIR)

# Where DSP/BIOS is installed.
BIOS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dspbios_5_41_03_17

# Where the DSPBIOS Utils package is installed.
BIOSUTILS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/biosutils_1_02_02

# Where the Codec Engine package is installed.
CE_INSTALL_DIR?=$(DVSDK_INSTALL_DIR)/codec-engine_2_26_02_11

# Where the DSP Link package is installed.
LINK_INSTALL_DIR=${MY_INSTALL_ROOT}/dsplink

# Where the codecs are installed.
CODEC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/codecs-omap3530_4_02_00_00

# Where DMAI package is installed.
DMAI_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dmai_2_20_00_15

# Where the SDK demos are installed
DEMO_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dvsdk-demos_4_02_00_01

# Where the DVTB package is installed.
DVTB_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/dvtb_4_20_18

# Where the EDMA3 LLD package is installed.
EDMA3_LLD_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/edma3lld_01_11_02_05
EDMA3LLD_INSTALL_DIR=$(EDMA3_LLD_INSTALL_DIR)

# Where the Framework Components package is installed.
FC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/framework-components_2_26_00_01

# Where the MFC Linux Utils package is installed.
LINUXUTILS_INSTALL_DIR=${MY_INSTALL_ROOT}/linuxutils
CMEM_INSTALL_DIR=$(LINUXUTILS_INSTALL_DIR)

# Where the XDAIS package is installed.
XDAIS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/xdais_6_26_01_03

# Where the RTSC tools package is installed.
XDC_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/xdctools_3_16_03_36

# Where the Code Gen is installed.
CODEGEN_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/cgt6x_6_1_14

# Where the PSP is installed.
PSP_INSTALL_DIR?=$(DVSDK_INSTALL_DIR)/psp

# The directory that points to your kernel source directory.
LINUXKERNEL_INSTALL_DIR=$(MY_INSTALL_ROOT)/../linux
KERNEL_INSTALL_DIR=$(LINUXKERNEL_INSTALL_DIR)

# Where the local power manager is installed.
LPM_INSTALL_DIR=${MY_INSTALL_ROOT}/local-power-manager

# Where the development headers and libs are installed.
LINUXLIBS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/linux-devkit/arm-arago-linux-gnueabi/usr


# Where c6accel package is installed.
C6ACCEL_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/c6accel_1_01_00_07

# Where c6run package is installed.
C6RUN_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/c6run_0_98_03_03

# Where opengl graphics package is installed.
GRAPHICS_INSTALL_DIR=$(DVSDK_INSTALL_DIR)/graphics-sdk_4.03.00.02

# The prefix to be added before the GNU compiler tools (optionally including # path), i.e. "arm_v5t_le-" or "/opt/bin/arm_v5t_le-".
ifndef OECORE_SDK_VERSION
    export XCOMP_BINARIES=$(DVSDK_INSTALL_DIR)/linux-devkit/bin
    export CC=arm-arago-linux-gnueabi-gcc
    export LD=arm-arago-linux-gnueabi-gcc
    export AS=arm-arago-linux-gnueabi-as
    export AR=arm-arago-linux-gnueabi-ar
    export TARGET_PREFIX=arm-arago-linux-gnueabi-

    CSTOOL_DIR=$(DVSDK_INSTALL_DIR)/linux-devkit/bin
    CSTOOL_PREFIX=arm-arago-linux-gnueabi-
    CSTOOL_PATH=$(CSTOOL_DIR)
else
    CSTOOL_DIR=$(XCOMP_BINARIES)
    CSTOOL_PREFIX=$(TARGET_PREFIX)
    CSTOOL_PATH=$(CSTOOL_DIR)
endif

MVTOOL_DIR=$(CSTOOL_DIR)
MVTOOL_PREFIX=$(CSTOOL_PREFIX)

# Where to copy the resulting executables
EXEC_DIR?=$(HOME)/install/$(PLATFORM)

