SRC_CC = log_module.cc log_session.cc log_state.cc

vpath % $(REP_DIR)/src/rtcr_log

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

LIBS   += config

