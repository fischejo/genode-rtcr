TARGET = rtcr_app
SRC_CC += main.cc
LIBS += base rtcr rtcr_cdma



# include serializer
INC_DIR += $(LIB_CACHE_DIR)/rtcr_serializer
LIBS += rtcr_serializer stdcxx libc libprotobuf

# enable debugging log
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG


