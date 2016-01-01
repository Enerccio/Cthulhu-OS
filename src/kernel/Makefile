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

CC := x86_64-fhtagn-gcc
LD := x86_64-fhtagn-gcc
AS := nasm

.PHONY: all clean kclib memory utils kclib-clean memory-clean utils-clean ports ports-clean processes processes-clean \
		structures structures-clean syscalls syscalls-clean cpus cpus-clean interrupts interrupts-clean \
		grx grx-clean rlyeh rlyeh-clean \
		utils-collections utils-collections-clean

all: azathoth.img

clean: kclib-clean memory-clean utils-clean ports-clean processes-clean structures-clean \
		cpus-clean utils-collections-clean interrupts-clean syscalls-clean grx-clean \
		rlyeh-clean 
	-rm -f $(OBJ)
	-rm -f $(wildcard *.d)
	-rm ${TARGETPATH}azathoth.img

%.o : %.c 
	$(CC) -MM -MF $(patsubst %.o,%.d,$@) $(CFLAGS) $(CPPFLAGS) -I"../kclib/usr/include"-c $<
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
	
processes:
	$(MAKE) -C $@ MODE=$(MODE)

processes-clean:
	$(MAKE) clean -C processes MODE=$(MODE)
	
interrupts:
	$(MAKE) -C $@ MODE=$(MODE)

interrupts-clean:
	$(MAKE) clean -C interrupts MODE=$(MODE)
	
structures:
	$(MAKE) -C $@ MODE=$(MODE)

structures-clean:
	$(MAKE) clean -C structures MODE=$(MODE)

syscalls:
	$(MAKE) -C $@ MODE=$(MODE)

syscalls-clean:
	$(MAKE) clean -C syscalls MODE=$(MODE)
	
cpus:
	$(MAKE) -C $@ MODE=$(MODE)

cpus-clean:
	$(MAKE) clean -C cpus MODE=$(MODE)

grx:
	$(MAKE) -C $@ MODE=$(MODE)

grx-clean:
	$(MAKE) clean -C grx MODE=$(MODE)
	
rlyeh:
	$(MAKE) -C $@ MODE=$(MODE)

rlyeh-clean:
	$(MAKE) clean -C rlyeh MODE=$(MODE)
	
utils-collections:
	$(MAKE) -C utils/collections MODE=$(MODE)

utils-collections-clean:
	$(MAKE) clean -C utils/collections MODE=$(MODE)
	
azathoth.img: $(OBJ) kclib memory utils ports processes structures syscalls cpus interrupts utils-collections \
					 grx rlyeh 
	$(LD) -Tlinker.ld -o ${TARGETPATH}$@ $(OBJ) \
		$(wildcard kclib/*.o) $(wildcard utils/*.o) $(wildcard memory/*.o) \
		$(wildcard ports/*.o) $(wildcard processes/*.o) $(wildcard structures/*.o) \
		$(wildcard syscalls/*.o) $(wildcard cpus/*.o) $(wildcard interrupts/*.o)\
		$(wildcard grx/*.o) $(wildcard rlyeh/*.o) \
		$(wildcard utils/collections/*.o) \
		$(LDFLAGS) -L../kclib/usr/lib
