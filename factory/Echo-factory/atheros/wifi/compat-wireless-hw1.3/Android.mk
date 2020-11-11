LOCAL_PATH   := $(call my-dir)
ifeq ($(call is-android-codename,JELLY_BEAN),true)
        DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
        DLKM_DIR := build/dlkm
endif

WLAN_ATH6KL_COMPAT_WIRELESS_DIR := $(call my-dir)

ifneq ($(WLAN_ATH6KL_DEBUG), true)
include $(CLEAR_VARS)
LOCAL_MODULE       := bdata_AR6004_1.1.bin
LOCAL_MODULE_STEM  := bdata.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.1
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.1/bdata.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := bdata_AR6004_1.2.bin
LOCAL_MODULE_STEM  := bdata.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.2/bdata.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := bdata_AR6004_1.3.bin
LOCAL_MODULE_STEM  := bdata.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.3/bdata.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := fw_AR6004_1.1.bin
LOCAL_MODULE_STEM  := fw.ram.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.1
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.1/fw.ram.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := fw_AR6004_1.2.bin
LOCAL_MODULE_STEM  := fw.ram.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.2/fw.ram.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := fw_AR6004_1.3.bin
LOCAL_MODULE_STEM  := fw.ram.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.3/fw.ram.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := utf_AR6004_1.1.bin
LOCAL_MODULE_STEM  := utf.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.1
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.1/utf.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := utf_AR6004_1.2.bin
LOCAL_MODULE_STEM  := utf.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.2/utf.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := utf_AR6004_1.3.bin
LOCAL_MODULE_STEM  := utf.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.3/utf.bin
include $(BUILD_PREBUILT)

else
include $(CLEAR_VARS)
LOCAL_MODULE       := fw_AR6004_epping_1.2.bin
LOCAL_MODULE_STEM  := epping.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.2/epping.bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := fw_AR6004_epping_1.3.bin
LOCAL_MODULE_STEM  := epping.bin
LOCAL_MODULE_TAGS  := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3
LOCAL_SRC_FILES    := etc/firmware/ath6k/AR6004/hw1.3/epping.bin
include $(BUILD_PREBUILT)

endif

include $(CLEAR_VARS)
export ATH6KL_ANDROID_BLD=1
export ATH_DRIVER_TOPDIR=$(realpath ${WLAN_ATH6KL_DIR})
export COMPAT_BASE_TREE_DEF="\"$(shell cat ${WLAN_ATH6KL_COMPAT_WIRELESS_DIR}/compat_base_tree)\""
export COMPAT_BASE_TREE_VERSION_DEF="\"$(shell cat ${WLAN_ATH6KL_COMPAT_WIRELESS_DIR}/compat_base_tree_version)\"" 
export COMPAT_PROJECT_DEF= "\"Compat-wireless\"" 
export COMPAT_VERSION_DEF="\"$(shell cat ${WLAN_ATH6KL_COMPAT_WIRELESS_DIR}/compat_version)\""

LOCAL_MODULE             := compat/compat.ko
LOCAL_MODULE_STEM        := compat.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules
include $(DLKM_DIR)/AndroidKernelModule.mk

include $(CLEAR_VARS)
LOCAL_MODULE             := net/wireless/ath6kl_cfg80211.ko
LOCAL_MODULE_STEM        := ath6kl_cfg80211.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules
include $(DLKM_DIR)/AndroidKernelModule.mk

ifneq ($(WLAN_ATH6KL_DEBUG), true)
include $(CLEAR_VARS)
LOCAL_MODULE             := drivers/net/wireless/ath/ath6kl/ath6kl_usb.ko
LOCAL_MODULE_STEM        := ath6kl_usb.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules
include $(DLKM_DIR)/AndroidKernelModule.mk

include $(CLEAR_VARS)
LOCAL_MODULE             := drivers/net/wireless/ath/ath6kl/ath6kl_sdio.ko
LOCAL_MODULE_STEM        := ath6kl_sdio.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules
include $(DLKM_DIR)/AndroidKernelModule.mk
endif

$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.1; \
        rm -f $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.1\*.bin)
$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2; \
	rm -f $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2\*.bin)
$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3; \
	rm -f $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3\*.bin)
ifeq ($(WLAN_ATH6KL_DEBUG), true)
$(shell rm -f $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2/fw.ram.bin; \
        ln -sf /data/ath6kl/fw.ram.bin \
	$(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2/fw.ram.bin)

$(shell rm -f $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2/bdata.bin; \
        ln -sf /data/ath6kl/bdata.bin \
	$(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.2/bdata.bin)

$(shell rm -f $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3/fw.ram.bin; \
        ln -sf /data/ath6kl/fw.ram.bin \
	$(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3/fw.ram.bin)

$(shell rm -f $(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3/bdata.bin; \
        ln -sf /data/ath6kl/bdata.bin \
	$(TARGET_OUT_ETC)/firmware/ath6k/AR6004/hw1.3/bdata.bin)

$(shell rm -f $(TARGET_OUT)/lib/modules/ath6kl_usb.ko; \
        ln -sf /data/ath6kl/ath6kl_usb.ko \
	$(TARGET_OUT)/lib/modules/ath6kl_usb.ko)
endif
