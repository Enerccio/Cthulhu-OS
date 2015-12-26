SYSROOT ?=
PREFIX  ?=
LIBDIR  ?= $(SYSROOT)/$(PREFIX)/lib
INCLDIR ?= $(SYSROOT)/include

.PHONY: crt0 all

all: crt0

crt0: crt0.o

crt0.o: crt0.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I"$(INCLDIR)" $< -o $@
	cp $@ $(LIBDIR)/