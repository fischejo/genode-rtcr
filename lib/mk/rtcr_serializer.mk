# serializer
# ==========

include $(call select_from_repositories,lib/import/import-libprotobuf.mk)
SRC_PROTO += $(REP_DIR)/proto/rtcr.proto

SRC_CC = serializer.cc
vpath % $(REP_DIR)/src/rtcr_serializer

# enable/disable debugging and verbosity
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG


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

