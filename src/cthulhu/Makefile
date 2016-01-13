SYSROOT ?= ../../osroot
PREFIX ?= /usr
LIBPATH ?= ${SYSROOT}${PREFIX}/lib
INCPATH ?= ${SYSROOT}${PREFIX}/include

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

.PHONY: all clean 

all: libcthulhu.a

clean: 
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)
	-rm ${LIBPATH}/libcthulhu.a
	
%.o : %.c 
	$(CC) -MM -MF $(patsubst %.o,%.d,$@) $(CFLAGS) $(CPPFLAGS) -I"../../osroot/usr/include" -c $<
	$(CC) $(CFLAGS) $(CPPFLAGS) -I"../../osroot/usr/include" -c $< -o $@

%_o.o : %.s 
	$(AS) $(NASMFLAGS) $< -o $@
	
libcthulhu.a: $(OBJ)
	-mkdir ${INCPATH}/cthulhu
	cp *.h ${INCPATH}/cthulhu -r 
	$(AR) ${ARFLAGS} -rcs ${LIBPATH}/$@ $(OBJ)