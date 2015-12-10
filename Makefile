.PHONY: kclib clean

clean:
	$(MAKE) clean -C kclib

all: kclib

kclib:
	$(MAKE) -C $@
