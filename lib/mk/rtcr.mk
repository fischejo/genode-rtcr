SRC_CC = module_factory.cc base_module.cc init_module.cc checkpointable.cc child_info.cc
SRC_CC += cpu_session.cc cpu_thread.cc
SRC_CC += pd_session.cc
SRC_CC += rm_session.cc region_map.cc
SRC_CC += rom_session.cc
SRC_CC += log_session.cc
SRC_CC += timer_session.cc

vpath % $(REP_DIR)/src/rtcr

ifeq ($(filter-out $(SPECS),focnados),)
SRC_CC += capability_mapping_foc.cc
vpath capability_mapping_foc.cc $(REP_DIR)/src/rtcr/spec/focnados
INC_DIR += $(BASE_DIR)/../base-focnados/src/include
LIBS += syscall-foc
else
SRC_CC += capability_mapping.cc
endif


LIBS += base
CC_OPT += -w
