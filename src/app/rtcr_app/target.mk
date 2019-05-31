TARGET = rtcr_app
SRC_CC += main.cc

LIBS   = base rtcr

# enable debugging log
CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG


