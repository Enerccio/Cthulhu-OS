SYSROOT ?=
PREFIX  ?=
LIBDIR  ?= $(SYSROOT)/$(PREFIX)/lib
INCLDIR ?= $(SYSROOT)/include
AS 		:= nasm

.PHONY: crt0 all

all: crt0

crt0: crt0.o crti.o crtn.o

crt0.o: crt0.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I"$(INCLDIR)" $< -o $@
	cp $@ $(LIBDIR)/
	
crti.o: crti.s
	$(AS) $(ASFLAGS) $< -o $@
	cp $@ $(LIBDIR)/
	
crtn.o: crtn.s
	$(AS) $(ASFLAGS) $< -o $@
	cp $@ $(LIBDIR)/