SRC_CC = target_child.cc target_state.cc module_factory.cc
vpath % $(REP_DIR)/src/rtcr
LIBS   = rtcr_core config
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

