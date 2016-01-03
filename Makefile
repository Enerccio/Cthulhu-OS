MODE ?= debug

.PHONY: clean kernel-clean kernel lib all nyarlathotep nyarlathotep-clean

all: lib kernel nyarlathotep init

clean: kernel-clean nyarlathotep-clean init-clean

kernel-clean:
	$(MAKE) clean -C src/kernel MODE=$(MODE)
	
nyarlathotep-clean:
	$(MAKE) clean -C src/kernel_dev_lib MODE=$(MODE)

init-clean:
	$(MAKE) clean -C src/init MODE=$(MODE)

lib:
	bash build_kclib.sh $(MODE)
	
kernel:
	$(MAKE) -C src/kernel MODE=$(MODE)

nyarlathotep:
	$(MAKE) -C src/kernel_dev_lib MODE=$(MODE)
	
init:
	$(MAKE) -C src/init MODE=$(MODE)