SRC_CC = target_child.cc target_state.cc module_factory.cc module_thread.cc
vpath % $(REP_DIR)/src/rtcr

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

# minimum rtcr
LIBS   = config rtcr_core rtcr_ds

# optional
LIBS += rtcr_timer rtcr_log

# optional incremental extension
LIBS += rtcr_inc

# optional dma memory copying extension
LIBS += rtcr_cdma

# optional fpga capability parsing extension
LIBS += rtcr_kcap

# optional redundant memory extension
LIBS += rtcr_red

# optional parallized memory copying
LIBS += rtcr_para
