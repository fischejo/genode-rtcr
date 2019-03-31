SRC_CC = ram_session_handler.cc ram_session.cc
vpath % $(REP_DIR)/src/lib/rtcr_ram
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

