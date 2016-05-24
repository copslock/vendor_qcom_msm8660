#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := com.qti.extendedoffscreen
LOCAL_CERTIFICATE := platform
LOCAL_STATIC_JAVA_LIBRARIES := DigitalPenSDK

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

