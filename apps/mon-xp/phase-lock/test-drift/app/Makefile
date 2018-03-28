CONTIKI = ../../../../..
CONTIKI_PROJECT = drift

ifdef ENABLE_LPM
	CFLAGS += -DENABLE_SKY_LPM=1
endif

#CFLAGS += -DUIP_CONF_ND6_SEND_NA=1 -DUIP_CONF_ROUTER=1 -DNETSTACK_CONF_RDC=nullrdc_driver -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -DUART1_CONF_RX_WITH_DMA=0 -DNETSTACK_CONF_RDC=nullrdc_driver -DPROJECT_CONF_H=\"project-conf.h\"

all: $(CONTIKI_PROJECT)

ifdef PERIOD
	CFLAGS += -DPERIOD=$(PERIOD)
endif

all: $(CONTIKI_PROJECT)

CONTIKI_WITH_IPV6 = 0
CONTIKI_WITH_RPL  = 0
CONTIKI_WITH_RIME = 1
#CONTIKI_WITH_RIME = 1 # Contiki insists on having a network stack.
include $(CONTIKI)/Makefile.include
