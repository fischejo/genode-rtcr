# serializer
# ==========

SRC_PROTO += $(REP_DIR)/proto/rtcr.proto
include $(call select_from_repositories,lib/import/import-libprotobuf.mk)
INC_DIR += $(LIB_CACHE_DIR)
vpath rtcr.pb.cc $(LIB_CACHE_DIR)/rtcr_serializer


SRC_CC += serializer.cc 
vpath % $(REP_DIR)/src/rtcr_serializer

# minimal rtcr
LIBS += base rtcr

LIBS += zlib

# Protobuf
# ========

LIBS += libprotobuf protobuf_host_tools stdcxx libc

# include builder script for translating .proto to .cc/.h files
# Pay attention:
# 1. It is not possible to include multiple *.proto files
# 2. Your proto file name should not include the character `_`

CC_OPT += -w
