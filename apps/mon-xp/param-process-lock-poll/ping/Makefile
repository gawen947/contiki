all: ping
CONTIKI = ../../../..

CONTIKI_WITH_RIME=1

ifndef SEND_COUNT
	${error SEND_COUNT not defined! You must specify the number of messages to send}
endif

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DSEND_COUNT=$(SEND_COUNT)
SMALL = 1

include $(CONTIKI)/Makefile.include
