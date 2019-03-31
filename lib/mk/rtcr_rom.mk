SRC_CC = rom_session.cc rom_session_handler.cc
vpath % $(REP_DIR)/src/lib/rtcr_rom
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

