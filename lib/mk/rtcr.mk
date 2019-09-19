SRC_CC = child.cc module_factory.cc base_module.cc init_module.cc checkpointable.cc child_info.cc
SRC_CC += cpu_session.cc cpu_thread.cc
SRC_CC += pd_session.cc
SRC_CC += rm_session.cc region_map.cc
SRC_CC += rom_session.cc
SRC_CC += log_session.cc
SRC_CC += timer_session.cc
SRC_CC += capability_mapping.cc


vpath % $(REP_DIR)/src/rtcr

INC_DIR += $(BASE_DIR)/../base-focnados/src/include
LIBS += base syscall-foc

CC_OPT += -w
