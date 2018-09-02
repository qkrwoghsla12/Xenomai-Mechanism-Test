SKIN = native

XENO ?= /mxq-rt/xenomai
XENOCONFIG=$(shell PATH=$(XENO):$(XENO)/bin:$(PATH) which xeno-config 2>/dev/null)

CFLAGS  := $(shell $(XENOCONFIG) --skin=$(SKIN) --cflags)
LDFLAGS := $(shell $(XENOCONFIG) --skin=$(SKIN) --ldflags) 

$(TARGET) : $(TARGET).c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

t2 : t2.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

t1 : t1.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

mechanism : mechanism.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

sem3 : sem3.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

evf : eventflag.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -lrtdm
	

clean :
	@rm $(TARGET)
