include commands.mk

OPTS    := -O2
CFLAGS  := -std=c99 $(OPTS) -fPIC -Wall
LDFLAGS :=

SRC = $(wildcard *.c)
OBJ = $(foreach obj, $(SRC:.c=.o), $(notdir $(obj)))
DEP = $(SRC:.c=.d)

TARGETS = trace

TRACE_OBJ = mon-ids.o mon-names.o htable.o hash.o utils.o sys-endian.o dump.o help.o iobuf.o mode.o parser.o print-mode.o stat-mode.o graph-mode.o trace.o typed.o version.o

PREFIX ?= /usr/local
BIN    ?= /bin

CFLAGS += -D_BSD_SOURCE=1 -DVERSION="\"$(shell cat VERSION)\""

commit = $(shell ./hash.sh)
ifneq ($(commit), UNKNOWN)
	CFLAGS += -DCOMMIT="\"$(commit)\""
	CFLAGS += -DPARTIAL_COMMIT="\"$(shell echo $(commit) | cut -c1-8)\""
endif

ifndef DISABLE_DEBUG
	CFLAGS += -ggdb
else
	CFLAGS += -DNDEBUG=1
endif

.PHONY: all clean

all: $(TARGETS)

trace: $(TRACE_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -Wp,-MMD,$*.d -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(DEP)
	$(RM) $(OBJ)
	$(RM) $(CATALOGS)
	$(RM) $(TARGETS)

install:
	$(MKDIR) -p $(DESTDIR)/$(PREFIX)/$(BIN)
	$(INSTALL_BIN) trace $(DESTDIR)/$(PREFIX)/$(BIN)

uninstall:
	$(RM) $(DESTDIR)/$(PREFIX)/$(BIN)/trace

-include $(DEP)
