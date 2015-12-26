
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

ifeq ($(MODE),user)
	ADAPTER := $(MODE)/$(MACHINE)/$(OS)/adapter.o
else
	ADAPTER :=
endif

CFLAGS:= -ffreestanding -std=c99 -Wall -Wextra \
	-fmessage-length=0 -nostdlib \
	 $(DEBREL_CFLAGS) $(MODE_CFLAGS) $(MACHINEDEF_CFLAGS) $(SYSDEF_CFLAGS) $(CFLAGS_FOR_TARGET) 
CPPFLAGS = $(MODE_CPPFLAGS) $(MACHINEDEF_CPPFLAGS) \
	$(DEBREL_CPPLAGS) $(SYSDEF_CPPFLAGS) $(CPPFLAGS_FOR_TARGET)
LDFLAGS = -nostdlib -static \
	$(DEBREL_LDFLAGS) $(MODE_LDFLAGS) $(MACHINEDEF_LDFLAGS) $(SYSDEF_LDFLAGS) $(LDFLAGS_FOR_TARGET) 
ASFLAGS := $(DEBREL_ASFLAGS) $(MODE_ASFLAGS) $(MACHINEDEF_ASFLAGS) $(SYSDEF_ASFLAGS) $(ASFLAGS_FOR_TARGET) 
AS := nasm


SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP    := $(SRC:.c=.d)
-include $(DEP)

.PHONY: all clean crt0 kclib

all: libc.a

kclib: libc.a

clean:
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)
ifeq ($(MODE),user)
	-rm -f "$(ADAPTER)"
endif

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I"../include" $< -MM -MF $(patsubst %.o,%.d,$@)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -I"../include" $< -o $@

$(MODE)/$(MACHINE)/$(OS)/adapter.o: 
	-@$(CC) $(CFLAGS) $(CPPFLAGS) -I"../include" $< -o $@ $(MODE)/$(MACHINE)/$(OS)/adapter.c
	-@$(AS) $(MODE)/$(MACHINE)/$(OS)/adapter.s $(ASFLAGS) $< -o $@

libc.a: $(OBJ) $(ADAPTER)
	$(AR) -rcs $(LIBDIR)/$@ $(OBJ) $(ADAPTER)
	
crt0:
	$(MAKE) -C $(MODE)/$(MACHINE)/$(OS) $@ CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS)" \
	 ASFLAGS="$(ASFLAGS)" AS="$(AS)" CC="$(CC)" AR="$(AR)"