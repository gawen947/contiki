CONTIKI = ../../../..
APPS = powertrace collect-view
CONTIKI_PROJECT = udp-mon-sender
PROJECT_SOURCEFILES += collect-common.c

ifdef PERIOD
CFLAGS=-DPERIOD=$(PERIOD)
endif

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
