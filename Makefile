MODE ?= debug

.PHONY: clean kernel-clean kernel lib all 

all: lib kernel

clean: kernel-clean

kernel-clean:
	$(MAKE) clean -C src/kernel MODE=$(MODE)

lib:
	bash build_kclib.sh $(MODE)
	
kernel:
	$(MAKE) -C src/kernel MODE=$(MODE)
