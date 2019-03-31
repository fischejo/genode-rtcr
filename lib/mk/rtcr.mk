SRC_CC = target_child.cc session_handler_factory.cc
vpath % $(REP_DIR)/src/core
LIBS   = rtcr_pd rtcr_cpu rtcr_ram rtcr_rm rtcr_rom
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

