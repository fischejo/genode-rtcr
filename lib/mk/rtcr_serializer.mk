# serializer
# ==========

ifeq ($(filter-out $(SPECS),arm),)
vpath cpu_thread.cc $(REP_DIR)/src/rtcr_serializer/spec/arm/
vpath rtcr.proto $(REP_DIR)/proto/arm/
endif

ifeq ($(filter-out $(SPECS),arm_64),)
vpath cpu_thread.cc $(REP_DIR)/src/rtcr_serializer/spec/arm_64/
vpath rtcr.proto $(REP_DIR)/proto/arm_64/
endif


# generate protobuf objects from .proto
SRC_PROTO += rtcr.proto
include $(call select_from_repositories,lib/import/import-libprotobuf.mk)
INC_DIR += $(LIB_CACHE_DIR)
vpath rtcr.pb.cc $(LIB_CACHE_DIR)/rtcr_serializer

SRC_CC += serializer.cc cpu_thread.cc
vpath % $(REP_DIR)/src/rtcr_serializer

# minimal rtcr
LIBS += base rtcr

LIBS += zlib

# Protobuf Library
# ================

LIBS += libprotobuf protobuf_host_tools stdcxx libc

# include builder script for translating .proto to .cc/.h files
# Pay attention:
# 1. It is not possible to include multiple *.proto files
# 2. Your proto file name should not include the character `_`

CC_OPT += -w

