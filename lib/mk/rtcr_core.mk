SRC_CC = core_module.cc core_state.cc
SRC_CC += core_module_cpu.cc cpu_session.cc cpu_thread_component.cc
SRC_CC += core_module_pd.cc pd_session.cc
SRC_CC += core_module_rm.cc rm_session.cc region_map_component.cc
SRC_CC += core_module_rom.cc rom_session.cc
SRC_CC += core_module_ram.cc ram_session.cc


vpath % $(REP_DIR)/src/rtcr_core
vpath % $(REP_DIR)/src/rtcr_core/cpu
vpath % $(REP_DIR)/src/rtcr_core/pd
vpath % $(REP_DIR)/src/rtcr_core/rm
vpath % $(REP_DIR)/src/rtcr_core/rom
vpath % $(REP_DIR)/src/rtcr_core/ram


CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

LIBS   += base config
INC_DIR += $(BASE_DIR)/../base-foc/src/include
