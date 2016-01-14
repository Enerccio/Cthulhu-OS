MODE ?= debug

.PHONY: clean kernel-clean kernel lib all nyarlathotep nyarlathotep-clean cthulhu cthulhu-clean lds lds-clean lds-u lds-u-clean framebuffer-install-headers ddm ddm-clean

all: lib kernel init framebuffer ddm

clean: kernel-clean framebuffer-clean init-clean ddm-clean

lib:
	bash build_kclib.sh $(MODE)

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

init-clean: lds-u-clean nyarlathotep-clean cthulhu-clean framebuffer-clean
	$(MAKE) clean -C src/init MODE=$(MODE)
	
framebuffer-clean: lds-u-clean nyarlathotep-clean cthulhu-clean
	$(MAKE) clean -C src/services/framebuffer MODE=$(MODE)

framebuffer-install-headers:
	$(MAKE) install-headers -C src/services/framebuffer MODE=$(MODE) SYSROOT=../../../osroot

lds:
	$(MAKE) clean -C libds MODE=$(MODE) SYSROOT=../osroot
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar CFLAGS_FOR_TARGET=-mcmodel=kernel SYSROOT=../build

lds-u: 
	$(MAKE) clean -C libds MODE=$(MODE) SYSROOT=../build
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar SYSROOT=../osroot
	
kernel: lib lds nyarlathotep cthulhu 
	$(MAKE) -C src/kernel MODE=$(MODE)

nyarlathotep: cthulhu-install-headers
	$(MAKE) -C src/kernel_dev_lib MODE=$(MODE)
	
cthulhu-install-headers:
	$(MAKE) install-headers -C src/cthulhu MODE=$(MODE)
	
cthulhu: cthulhu-install-headers
	$(MAKE) -C src/cthulhu MODE=$(MODE)
	
init: lds-u nyarlathotep cthulhu framebuffer-install-headers
	$(MAKE) -C src/init MODE=$(MODE)

framebuffer: lds-u nyarlathotep cthulhu 
	$(MAKE) -C src/services/framebuffer MODE=$(MODE)
	
ddm: lds-u nyarlathotep cthulhu 
	$(MAKE) -C src/ddm MODE=$(MODE)
	
ddm-clean: lds-u nyarlathotep cthulhu 
	$(MAKE) clean -C src/ddm MODE=$(MODE)