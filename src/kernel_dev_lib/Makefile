TARGETPATH ?= ../../build/libnyarlathotep/

include confbase.mk
include $(MODE).mk

ASMSRC := $(wildcard *.s)
SRC    := $(wildcard *.c)
OBJ    := $(SRC:.c=.o) $(ASMSRC:.s=_o.o)
DEP    := $(SRC:.c=.d)
-include $(DEP)

CFLAGS    := $(BASE_CFLAGS)    $(MODE_CFLAGS) 
CPPFLAGS  := $(BASE_CPPFLAGS)  $(MODE_CPPFLAGS)
LDFLAGS   := $(BASE_LDFLAGS)   $(MODE_LDFLAGS)
NASMFLAGS := $(BASE_NASMFLAGS) $(MODE_NASMFLAGS)

CC := x86_64-fhtagn-gcc
AR := x86_64-fhtagn-ar
AS := nasm

.PHONY: all clean 

all: libnyarlathotep.a

clean: 
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)
	-rm ${TARGETPATH}libnyarlathotep.a
	-rm ${TARGETPATH}*.h
	
%.o : %.c 
	$(CC) -MM -MF $(patsubst %.o,%.d,$@) $(CFLAGS) $(CPPFLAGS) -I"../../osroot/usr/include" -c $<
	$(CC) $(CFLAGS) $(CPPFLAGS) -I"../../osroot/usr/include" -c $< -o $@

%_o.o : %.s 
	$(AS) $(NASMFLAGS) $< -o $@
	
libnyarlathotep.a: $(OBJ)
	cp *.h ${TARGETPATH} -r 
	$(AR) -rcs ${TARGETPATH}$@ $(OBJ)