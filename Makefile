COMPILER_PREFIX =
CC = $(COMPILER_PREFIX)gcc
CPPFLAGS = -I. -MMD
CFLAGS = -g3 -Wall
ifneq ($(DEBUG),)
 CFLAGS += -O0
else
 CFLAGS += -O2
endif
LDFLAGS =

TARGETS = MeasureTsw MeasureGettime RwIo


all: $(TARGETS)

MeasureTsw: measureTaskSwitch.o calcDiffs.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

MeasureGettime: measureGettime.o calcDiffs.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

RwIo: rwIO.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGETS) *.o *~ .*~

realclean:
	rm -rf $(TARGETS) *.o *~ *.d .*~

.PHONY: clean realclean all

-include *.d
