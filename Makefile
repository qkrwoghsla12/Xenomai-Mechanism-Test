SKIN = native

XENO ?= /zynq-rt/xenomai
XENOCONFIG=$(shell PATH=$(XENO):$(XENO)/bin:$(PATH) which xeno-config 2>/dev/null)

CFLAGS  := $(shell $(XENOCONFIG) --skin=$(SKIN) --cflags)
LDFLAGS := $(shell $(XENOCONFIG) --skin=$(SKIN) --ldflags) 

all: mechanism

$(TARGET) : $(TARGET).c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

mechanism : mechanism.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -D T2

sem : mechanism.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -D SEM

evf : mechanism.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -D EVF

msgq : mechanism.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -D MSGQ
	
clean :
	rm mechanism
