TARGET = rtcr_trigger
SRC_CC = main.cc
LIBS   = base

CC_OPT += -w

# include serializer
INC_DIR += $(LIB_CACHE_DIR)
LIBS += rtcr_serializer stdcxx libc libprotobuf
