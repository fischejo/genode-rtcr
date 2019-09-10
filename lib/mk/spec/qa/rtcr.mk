LIBS += profiler

include $(REP_DIR)/lib/mk/rtcr.mk

# disable any debugging log to improve logging performance
CC_OPT += -UVERBOSE
CC_OPT += -UDEBUG
CC_OPT += -UPROFILE
