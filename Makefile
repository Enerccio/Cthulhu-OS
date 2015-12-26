.PHONY: kclib clean crt0

clean:
	$(MAKE) clean -C kclib

all: kclib

kclib:
	$(MAKE) -C $@
	
crt0:
	$(MAKE) -C kclib $@ 
