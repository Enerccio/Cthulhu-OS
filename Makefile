MODE ?= debug

.PHONY: clean kernel-clean kernel lib all pikodev pikodev-clean libelrond libelrond-clean 

all: lib kernel pikodev

clean: kernel-clean pikodev-clean

kernel-clean:
	$(MAKE) clean -C src/kernel MODE=$(MODE)
	
pikodev-clean:
	$(MAKE) clean -C src/kernel_dev_lib MODE=$(MODE)

lib:
	bash build_kclib.sh $(MODE)
	
kernel: libelrond-clean libelrond
	$(MAKE) -C src/kernel MODE=$(MODE)

pikodev:
	$(MAKE) -C src/kernel_dev_lib MODE=$(MODE)
	
libelrond:
	$(MAKE) -C $@ MODE=$(MODE) PREFIX="$(shell pwd)/osroot/usr" CC=x86_64-piko-gcc AR=x86_64-piko-ar
	
libelrond-clean:
	$(MAKE) clean -C libelrond MODE=$(MODE)