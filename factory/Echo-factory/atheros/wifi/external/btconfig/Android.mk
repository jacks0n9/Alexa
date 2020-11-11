ifeq ($(BOARD_HAVE_BLUETOOTH),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES:= \
	$(call include-path-for,bluez)/lib/ \

LOCAL_SRC_FILES:= \
	compat/getline.c \
	btconfig.c

LOCAL_SHARED_LIBRARIES := \
	libbluetooth \

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE:=btconfig

include $(BUILD_EXECUTABLE)
endif
