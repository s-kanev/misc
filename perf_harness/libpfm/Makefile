CC=gcc
OEXT=o
EEXT=
AR=ar rcs

ifeq (${TARGET},ia32)
    CFLAGS=-m32
    LDFLAGS=-m32
    LIBPFM=./libpfm_32.a
else
    CFLAGS=
    LDFLAGS=
    LIBPFM=./libpfm_64.a
endif

CFLAGS+= -fPIC -g -I.
LDFLAGS+= -g 

#CFLAGS+=-DVERBOSE


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
