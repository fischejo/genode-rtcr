SRC_CC = dataspace_module.cc

vpath % $(REP_DIR)/src/rtcr_ds

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

LIBS   += config

