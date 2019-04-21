SRC_CC = target_child.cc target_state.cc module_factory.cc
vpath % $(REP_DIR)/src/rtcr

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

# minimum rtcr
LIBS   = config rtcr_core rtcr_ds

# optional
LIBS += rtcr_timer rtcr_log

# optional incremental extension
#LIBS += rtcr_inc

# optional fpga extension
LIBS += rtcr_fpga
