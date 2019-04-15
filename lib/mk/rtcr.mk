SRC_CC = target_child.cc target_state.cc module_factory.cc
vpath % $(REP_DIR)/src/rtcr
LIBS   = config rtcr_core rtcr_timer rtcr_ds
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

