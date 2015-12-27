TARGETPATH ?= ../../build/libpikodev/

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

CC := x86_64-piko-gcc
AR := x86_64-piko-ar
AS := nasm

.PHONY: all clean 

all: libpikodev.a

clean: 
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)
	-rm ${TARGETPATH}libpikodev.a
	-rm ${TARGETPATH}libpikod.h
	
%.o : %.c 
	$(CC) -MM -MF $(patsubst %.o,%.d,$@) $(CFLAGS) $(CPPFLAGS) -I"../kclib/usr/include" -c $<
	$(CC) $(CFLAGS) $(CPPFLAGS) -I"../kclib/usr/include" -c $< -o $@

%_o.o : %.s 
	$(AS) $(NASMFLAGS) $< -o $@
	
libpikodev.a: $(OBJ)
	cp libpikod.h ${TARGETPATH}libpikod.h -r 
	$(AR) -rcs ${TARGETPATH}$@ $(OBJ)