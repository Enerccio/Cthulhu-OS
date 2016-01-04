MODE ?= debug

.PHONY: clean kernel-clean kernel lib all nyarlathotep nyarlathotep-clean

all: lib lds kernel nyarlathotep init

clean: lds-clean kernel-clean nyarlathotep-clean init-clean

lds-clean:
	$(MAKE) clean -C libds MODE=$(MODE)

kernel-clean:
	$(MAKE) clean -C src/kernel MODE=$(MODE)
	
nyarlathotep-clean:
	$(MAKE) clean -C src/kernel_dev_lib MODE=$(MODE)

init-clean:
	$(MAKE) clean -C src/init MODE=$(MODE)

lib:
	bash build_kclib.sh $(MODE)

lds:
	$(MAKE) -C libds MODE=$(MODE) CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar CFLAGS_FOR_TARGET=-mcmodel=kernel SYSROOT=../osroot
	
kernel:
	$(MAKE) -C src/kernel MODE=$(MODE)

nyarlathotep:
	$(MAKE) -C src/kernel_dev_lib MODE=$(MODE)
	
init:
	$(MAKE) -C src/init MODE=$(MODE)