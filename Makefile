MODE ?= debug

.PHONY: clean kernel-clean kernel lib all nyarlathotep nyarlathotep-clean cthulhu cthulhu-clean lds lds-clean lds-u lds-u-clean

all: lib lds nyarlathotep kernel cthulhu lds-u init framebuffer

clean: lds-clean kernel-clean nyarlathotep-clean cthulhu-clean lds-u init-clean framebuffer-clean

lds-clean:
	$(MAKE) clean -C libds MODE=$(MODE) SYSROOT=../osroot

lds-u-clean:
	$(MAKE) clean -C libds MODE=$(MODE) SYSROOT=../build

kernel-clean: lds-clean nyarlathotep-clean cthulhu-clean 
	$(MAKE) clean -C src/kernel MODE=$(MODE)
	
nyarlathotep-clean:
	$(MAKE) clean -C src/kernel_dev_lib MODE=$(MODE)

cthulhu-clean:
	$(MAKE) clean -C src/cthulhu MODE=$(MODE)

init-clean: lds-u-clean nyarlathotep-clean cthulhu-clean
	$(MAKE) clean -C src/init MODE=$(MODE)
	
framebuffer-clean: lds-u-clean nyarlathotep-clean cthulhu-clean
	$(MAKE) clean -C src/services/framebuffer MODE=$(MODE)

lib:
	bash build_kclib.sh $(MODE)

lds:
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar CFLAGS_FOR_TARGET=-mcmodel=kernel SYSROOT=../build

lds-u:
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar SYSROOT=../osroot
	
kernel: lib lds nyarlathotep cthulhu 
	$(MAKE) -C src/kernel MODE=$(MODE)

nyarlathotep:
	$(MAKE) -C src/kernel_dev_lib MODE=$(MODE)
	
cthulhu:
	$(MAKE) -C src/cthulhu MODE=$(MODE)
	
init: lds-u nyarlathotep cthulhu 
	$(MAKE) -C src/init MODE=$(MODE)

framebuffer: lds-u nyarlathotep cthulhu 
	$(MAKE) -C src/services/framebuffer MODE=$(MODE)