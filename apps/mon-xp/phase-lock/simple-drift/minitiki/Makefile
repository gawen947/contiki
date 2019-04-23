CC       = msp430-gcc
LD       = msp430-gcc
AS       = msp430-as
AR       = msp430-ar
NM       = msp430-nm
OBJCOPY  = msp430-objcopy
STRIP    = msp430-strip
BSL      = msp430-bsl

TARGET=minitiki

PLATFORM   = sky
MCU        = msp430f1611

# Contiki uses --vla -Ohz --multiplier=16s --core=430 --double=32
MSP_CFLAGS  = -mmcu=$(MCU)
MSP_LDFLAGS =

CFLAGS  := -std=c99 -O2 -Wall $(MSP_CFLAGS) -DF_CPU=3900000
LDFLAGS :=

SRC  = $(wildcard *.c)
OBJ  = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP  = $(SRC:.c=.d)

ifdef DISABLE_SLEEP
CFLAGS += -DDISABLE_SLEEP=1
endif

ifdef TIMER
CFLAGS += -DTIMER=$(TIMER)
endif

ifdef DEBUG
CFLAGS += -ggdb
endif

.PHONY: all clean

all: $(TARGET).sky

$(TARGET).$(PLATFORM): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -Wp,-MMD,$*.d -c $(CFLAGS) -o $@ $< 

clean:
	$(RM) $(DEP)
	$(RM) $(OBJ)
	$(RM) $(TARGET).$(PLATFORM)

-include $(DEP)
