include ../confbase.mk
include ../$(MODE).mk

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
LD := x86_64-fhtagn-gcc
AS := nasm

.PHONY: all clean 

all: $(OBJ)

clean:
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)

%.o : %.c 
	$(CC) -MM -MF $(patsubst %.o,%.d,$@) $(CFLAGS) $(CPPFLAGS) -I"../../kclib/usr/include" -I"../../../build/usr/include" -c $< 
	$(CC) $(CFLAGS) $(CPPFLAGS) -I"../../kclib/usr/include" -I"../../../build/usr/include" -c $< -o $@

%_o.o : %.s 
	$(AS) $(NASMFLAGS) $< -o $@

