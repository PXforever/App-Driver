LOCAL_PATH := $(call my-dir)
  
include $(CLEAR_VARS)
LOCAL_MODULE := px_debug_test
LOCAL_SRC_FILES := px_debug_test.c
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)