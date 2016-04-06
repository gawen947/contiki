all: balanced
CONTIKI = ../../../..

CONTIKI_WITH_RIME=1

ifndef CYCLES
	${error CYCLES not defined! You must specify a working time duration}
endif

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -DCYCLES=$(CYCLES)
SMALL = 1

include $(CONTIKI)/Makefile.include
