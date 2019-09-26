TARGET   = rtcr_init
SRC_CC   = main.cc child.cc server.cc
LIBS     = base  rtcr
INC_DIR += $(PRG_DIR)

# include serializer
INC_DIR += $(LIB_CACHE_DIR)
LIBS += rtcr_serializer stdcxx libc libprotobuf

CC_OPT += -w
#CONFIG_XSD = config.xsd
