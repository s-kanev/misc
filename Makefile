CC=gcc
OEXT=o
EEXT=

CFLAGS=-g -m32 -I. -DVERBOSE
LDFLAGS=-g -m32

default: test

test: main.$(OEXT) perf_util.$(OEXT) 
	$(CC) $(LDFLAGS) -o main$(EEXT) $^ libpfm.a

.$(OEXT): .c
	$(CC) $(CFLAGS) -c $*.c

.PHONY: clean

clean:
	rm -rf *.$(OEXT) main
