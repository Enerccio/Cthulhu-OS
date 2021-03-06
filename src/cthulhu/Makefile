SYSROOT ?= ../../osroot
PREFIX ?= /sys
LIBPATH ?= ${SYSROOT}${PREFIX}/dev/lib
INCPATH ?= ${SYSROOT}${PREFIX}/dev/include

include confbase.mk
include $(MODE).mk

HSRC   := $(wildcard *.h)
ASMSRC := $(wildcard *.s)
SRC    := $(wildcard *.c)
OBJ    := $(SRC:.c=.o) $(ASMSRC:.s=_o.o)
DEP    := $(SRC:.c=.d)
-include $(DEP)

CFLAGS    := $(BASE_CFLAGS)    $(MODE_CFLAGS) 
CPPFLAGS  := $(BASE_CPPFLAGS)  $(MODE_CPPFLAGS)
ARFLAGS   := $(BASE_ARFLAGS)   $(MODE_ARFLAGS)
NASMFLAGS := $(BASE_NASMFLAGS) $(MODE_NASMFLAGS)

CC := x86_64-fhtagn-gcc
AR := x86_64-fhtagn-ar
AS := nasm

.PHONY: all clean install-headers  

all: install-headers libcthulhu.a

install-headers:
	-mkdir ${INCPATH}/cthulhu
	cp *.h ${INCPATH}/cthulhu -r

clean: 
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)
	-rm ${LIBPATH}/libcthulhu.a
	
%.o : %.c install-headers
	$(CC) -MM -MF $(patsubst %.o,%.d,$@) $(CFLAGS) $(CPPFLAGS) -I"../../osroot/sys/dev/include" -c $<
	$(CC) $(CFLAGS) $(CPPFLAGS) -I"../../osroot/sys/dev/include" -c $< -o $@

%_o.o : %.s 
	$(AS) $(NASMFLAGS) $< -o $@
	
libcthulhu.a: $(OBJ)  
	$(AR) ${ARFLAGS} -rcs ${LIBPATH}/$@ $(OBJ)