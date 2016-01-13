MODE ?= debug

.PHONY: clean kernel-clean kernel lib all nyarlathotep nyarlathotep-clean cthulhu cthulhu-clean

all: lib lds nyarlathotep kernel cthulhu init framebuffer

clean: lds-clean kernel-clean nyarlathotep-clean cthulhu-clean init-clean framebuffer-clean

lds-clean:
	$(MAKE) clean -C libds MODE=$(MODE)

kernel-clean:
	$(MAKE) clean -C src/kernel MODE=$(MODE)
	
nyarlathotep-clean:
	$(MAKE) clean -C src/kernel_dev_lib MODE=$(MODE)

cthulhu-clean:
	$(MAKE) clean -C src/cthulhu MODE=$(MODE)

init-clean:
	$(MAKE) clean -C src/init MODE=$(MODE)
	
framebuffer-clean:
	$(MAKE) clean -C src/services/framebuffer MODE=$(MODE)

lib:
	bash build_kclib.sh $(MODE)

lds:
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar CFLAGS_FOR_TARGET=-mcmodel=kernel SYSROOT=../osroot
	
kernel:
	$(MAKE) -C src/kernel MODE=$(MODE)

nyarlathotep:
	$(MAKE) -C src/kernel_dev_lib MODE=$(MODE)
	
cthulhu:
	$(MAKE) -C src/cthulhu MODE=$(MODE)
	
init:
	$(MAKE) -C src/init MODE=$(MODE)

framebuffer:
	$(MAKE) -C src/services/framebuffer MODE=$(MODE)