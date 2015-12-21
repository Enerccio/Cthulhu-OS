
PREFIX?=
LIBDIR?=$(PREFIX)/lib
TARGET?=x86_64-piko
MODE?=kernel
DEBREL?=debug

MACHINE != expr $(TARGET) : '\([^-]*\)-.*'
OS != expr $(TARGET) : '.*-\([^-]*\)'

include  $(DEBREL).mk
include  $(MODE).mk
include  machine/$(MACHINE).mk
-include $(MODE)/$(MACHINE)/$(OS)/defaults.mk
-include $(MODE)/$(MACHINE)/defaults.mk
-include $(MODE)/defaults.mk

CFLAGS:= -ffreestanding -std=c99 -Wall -Wextra \
	-fmessage-length=0 -nostdlib \
	 $(DEBREL_CFLAGS) $(MODE_CFLAGS) $(MACHINEDEF_CFLAGS) $(SYSDEF_CFLAGS) $(CFLAGS_FOR_TARGET) 
CPPFLAGS = $(MODE_CPPFLAGS) $(MACHINEDEF_CPPFLAGS) \
	$(DEBREL_CPPLAGS) $(SYSDEF_CPPFLAGS) $(CPPFLAGS_FOR_TARGET)
LDFLAGS = -nostdlib -static \
	$(DEBREL_LDLAGS) $(MODE_LDFLAGS) $(MACHINEDEF_LDFLAGS) $(SYSDEF_LDFLAGS) $(LDFLAGS_FOR_TARGET) 


SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP    := $(SRC:.c=.d)
-include $(DEP)

.PHONY: all clean

all: libc.a

clean:
	rm -f $(OBJ)
	rm -f $(wildcard *.d)

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I"../include" $< -MM -MF $(patsubst %.o,%.d,$@)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I"../include" $< -o $@

libc.a: $(OBJ)
	$(AR) -rcs $(LIBDIR)/$@ $(OBJ)

kclib:
	$(MAKE) -C $(MODE)/$(MACHINE)/$(OS)