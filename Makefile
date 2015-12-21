MODE ?= debug

.PHONY: clean kernel-clean kernel lib all 

all: lib kernel

clean: kernel-clean

kernel-clean:
	$(MAKE) clean -C src/kernel MODE=$(MODE)

lib:
	./build_kclib.sh
	
kernel:
	$(MAKE) -C src/kernel MODE=$(MODE)
