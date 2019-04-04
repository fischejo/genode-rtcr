SRC_CC = target_child.cc
vpath % $(REP_DIR)/src/rtcr
LIBS   = rtcr_core
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

