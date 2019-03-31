SRC_CC = rm_session.cc region_map_component.cc rm_session_handler.cc
vpath % $(REP_DIR)/src/lib/rtcr_rm
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

