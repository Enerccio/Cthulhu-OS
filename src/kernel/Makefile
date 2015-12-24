TARGETPATH ?= ../../build/

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
LD := x86_64-piko-gcc
AS := nasm

.PHONY: all clean kclib memory utils kclib-clean memory-clean utils-clean ports ports-clean tasks tasks-clean \
		structures structures-clean \
		utils-collections utils-collections-clean

all: piko-kernel.img

clean: kclib-clean memory-clean utils-clean ports-clean tasks-clean structures-clean utils-collections-clean
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)
	-rm ${TARGETPATH}piko-kernel.img

%.o : %.c 
	$(CC) -MM -MF $(patsubst %.o,%.d,$@) $(CFLAGS) $(CPPFLAGS) -I"../kclib/usr/include" -c $<
	$(CC) $(CFLAGS) $(CPPFLAGS) -I"../kclib/usr/include" -c $< -o $@

%_o.o : %.s 
	$(AS) $(NASMFLAGS) $< -o $@
	
kclib:
	$(MAKE) -C $@ MODE=$(MODE)
	
kclib-clean:
	$(MAKE) clean -C kclib MODE=$(MODE)
	
memory:
	$(MAKE) -C $@ MODE=$(MODE)
	
memory-clean:
	$(MAKE) clean -C memory MODE=$(MODE)
	
utils:
	$(MAKE) -C $@ MODE=$(MODE)

utils-clean:
	$(MAKE) clean -C utils MODE=$(MODE)
	
ports:
	$(MAKE) -C $@ MODE=$(MODE)

ports-clean:
	$(MAKE) clean -C ports MODE=$(MODE)
	
tasks:
	$(MAKE) -C $@ MODE=$(MODE)

tasks-clean:
	$(MAKE) clean -C tasks MODE=$(MODE)
	
structures:
	$(MAKE) -C $@ MODE=$(MODE)

structures-clean:
	$(MAKE) clean -C structures MODE=$(MODE)
	
utils-collections:
	$(MAKE) -C utils/collections MODE=$(MODE)

utils-collections-clean:
	$(MAKE) clean -C utils/collections MODE=$(MODE)
	
piko-kernel.img: $(OBJ) kclib memory utils ports tasks structures utils-collections
	$(LD) -Tlinker.ld -o ${TARGETPATH}$@ $(OBJ) \
		$(wildcard kclib/*.o) $(wildcard utils/*.o) $(wildcard memory/*.o) \
		$(wildcard ports/*.o) $(wildcard tasks/*.o) $(wildcard structures/*.o) \
		$(wildcard utils/collections/*.o) \
		$(LDFLAGS) -L../kclib/usr/lib
