TARGET = rtcr_app
SRC_CC += main.cc
LIBS += base rtcr

# include serializer
INC_DIR += $(LIB_CACHE_DIR)
LIBS += rtcr_serializer stdcxx libc libprotobuf


# enable debugging log
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

CC_OPT += -w

