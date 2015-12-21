MODE ?= debug

.PHONY: clean kernel-clean kernel lib all 

all: lib kernel

clean: kernel-clean

kernel-clean:
	$(MAKE) clean -C src/kernel MODE=$(MODE)

lib:
	bash build_kclib.sh debug
	
kernel:
	$(MAKE) -C src/kernel MODE=$(MODE)
