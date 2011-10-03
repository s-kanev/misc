CC=gcc
OEXT=o
EEXT=

CFLAGS=-g
LDFLAGS=-g -lpfm

default: test

test: main.$(OEXT) perf_util.$(OEXT) 
	$(CC) $(LDFLAGS) -o main$(EEXT) $^ 

.$(OEXT): .c
	$(CC) $(CFLAGS) -c $*.c

.PHONY: clean

clean:
	rm -rf *.$(OEXT) main
