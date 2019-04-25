SRC_CC = timer_module.cc timer_session.cc timer_state.cc

vpath % $(REP_DIR)/src/rtcr_timer

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

LIBS   += config

