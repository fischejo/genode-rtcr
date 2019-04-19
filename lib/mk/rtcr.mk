SRC_CC = target_child.cc target_state.cc module_factory.cc
vpath % $(REP_DIR)/src/rtcr

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

LIBS   = config rtcr_core rtcr_timer rtcr_ds rtcr_inc

#LIBS += rtcr_core_kcap rtcr_ds_cdma
