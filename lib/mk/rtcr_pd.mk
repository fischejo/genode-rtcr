SRC_CC = pd_session_handler.cc pd_session.cc
vpath % $(REP_DIR)/src/lib/rtcr_pd
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

