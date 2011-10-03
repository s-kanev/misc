CC=gcc
OEXT=o
EEXT=
AR=ar rcs

CFLAGS=-g -m32 -I.
LDFLAGS=-g -m32

#CFLAGS+=-DVERBOSE

LIBPFM=./libpfm.a

OUTLIB=libwrappedpfm.a

default: lib test

lib: $(OUTLIB)

$(OUTLIB): perf_util.$(OEXT) lib.$(OEXT) $(LIBPFM)
	cp -f $(LIBPFM) $(OUTLIB)
	$(AR) $(OUTLIB) $^

test: main.$(OEXT) $(OUTLIB)
	$(CC) $(LDFLAGS) -o main$(EEXT) $^

.$(OEXT): .c
	$(CC) $(CFLAGS) -c $*.c

.PHONY: clean lib

clean:
	rm -rf *.$(OEXT) main libwrappedpfm.a
