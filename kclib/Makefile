
PREFIX?=
LIBDIR?=$(PREFIX)/lib
TARGET?=x86_64-piko
MODE?=kernel
DEBUG?= -g -O0

MACHINE != expr $(TARGET) : '\([^-]*\)-.*'
OS != expr $(TARGET) : '.*-\([^-]*\)'

include  $(MODE).mk
include  machine/$(MACHINE).mk
-include $(MODE)/$(MACHINE)/$(OS)/defaults.mk
-include $(MODE)/$(MACHINE)/defaults.mk
-include $(MODE)/defaults.mk

CFLAGS:= -ffreestanding -std=c99 -Wall -Wextra \
	-fmessage-length=0 -nostdlib \
	 $(DEBUG) $(MODE_CFLAGS) $(MACHINEDEF_CFLAGS) $(SYSDEF_CFLAGS) $(CFLAGS_FOR_TARGET) 
CPPFLAGS = $(MODE_CPPFLAGS) $(MACHINEDEF_CPPFLAGS) \
	$(SYSDEF_CPPFLAGS) $(CPPFLAGS_FOR_TARGET)
LDFLAGS = -nostdlib -static \
	$(MODE_LDFLAGS) $(MACHINEDEF_LDFLAGS) $(SYSDEF_LDFLAGS) $(LDFLAGS_FOR_TARGET) 


SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: libc.a

clean:
	rm -f $(OBJ)

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I"../includes" $< -o $@

libc.a: $(OBJ)
	$(AR) -rcs $(LIBDIR)/$@ $(OBJ)

kclib:
	$(MAKE) -C $(MODE)/$(MACHINE)/$(OS)