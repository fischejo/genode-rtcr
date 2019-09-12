LIBS += profiler

include $(REP_DIR)/lib/mk/rtcr_serializer.mk

# disable any debugging log to improve logging performance
CC_OPT += -UVERBOSE
CC_OPT += -UDEBUG
CC_OPT += -DPROFILE
