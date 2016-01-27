MODE ?= debug

.PHONY: clean kernel-clean kernel lib all nyarlathotep nyarlathotep-clean cthulhu cthulhu-clean lds lds-clean lds-u lds-u-clean framebuffer-install-headers ddm ddm-clean

all: lib kernel init framebuffer ddm sata

clean: kernel-clean framebuffer-clean init-clean ddm-clean sata-clean

lib:
	bash build_kclib.sh $(MODE)

lds-clean:
	$(MAKE) clean -C libds MODE=$(MODE) LIBPATH=../osroot/dev/sys/lib INCPATH=../osroot/dev/sys/include

lds-u-clean:
	$(MAKE) clean -C libds MODE=$(MODE) SYSROOT=../build PREFIX=/usr

kernel-clean: lds-clean nyarlathotep-clean cthulhu-clean 
	$(MAKE) clean -C src/kernel MODE=$(MODE)
	
nyarlathotep-clean:
	$(MAKE) clean -C src/kernel_dev_lib MODE=$(MODE)

cthulhu-clean:
	$(MAKE) clean -C src/cthulhu MODE=$(MODE)

init-clean: lds-u-clean nyarlathotep-clean cthulhu-clean framebuffer-clean
	$(MAKE) clean -C src/init MODE=$(MODE)
	
sata-clean: lds-u-clean nyarlathotep-clean cthulhu-clean
	$(MAKE) clean -C src/drivers/sata MODE=$(MODE)
	
framebuffer-clean: lds-u-clean nyarlathotep-clean cthulhu-clean
	$(MAKE) clean -C src/services/framebuffer MODE=$(MODE)

framebuffer-install-headers:
	$(MAKE) install-headers -C src/services/framebuffer MODE=$(MODE) SYSROOT=../../../osroot

lds:
	$(MAKE) clean -C libds MODE=$(MODE) SYSROOT=../osroot PREFIX=/sys/dev
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar CFLAGS_FOR_TARGET=-mcmodel=kernel SYSROOT=../build PREFIX=/usr

lds-u: 
	$(MAKE) clean -C libds MODE=$(MODE) SYSROOT=../build PREFIX=/usr
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar SYSROOT=../osroot PREFIX=/sys/dev
	
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
	
sata: lds-u nyarlathotep cthulhu 
	$(MAKE) -C src/drivers/sata MODE=$(MODE)

framebuffer: lds-u nyarlathotep cthulhu 
	$(MAKE) -C src/services/framebuffer MODE=$(MODE)
	
ddm: lds-u nyarlathotep cthulhu 
	$(MAKE) -C src/ddm MODE=$(MODE)
	
ddm-clean: lds-u nyarlathotep cthulhu 
	$(MAKE) clean -C src/ddm MODE=$(MODE)