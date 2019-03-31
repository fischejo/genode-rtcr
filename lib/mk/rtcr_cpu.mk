SRC_CC = cpu_session_handler.cc cpu_session.cc cpu_thread_component.cc
vpath % $(REP_DIR)/src/lib/rtcr_cpu
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG
LIBS += base
