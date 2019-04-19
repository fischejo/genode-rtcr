SRC_CC = log_module.cc log_session.cc

vpath % $(REP_DIR)/src/rtcr_log

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

LIBS   += config

