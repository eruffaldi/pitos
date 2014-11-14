# Main Makefile
# 
# Targets: all libs examples 
MAKE=make
BUILDDIR=$(shell pwd)
SUBDIRS = examples src

examples : libs 
	cd examples
	$(MAKE) 

libs : 
	cd src
	$(MAKE) 

all : libs examples 
	
	

clean : FORCE
	@for i in $(SUBDIRS) ; do \
	    echo ">>>Recursively making "$$i" "$@"..."; \
	    cd $$i; $(MAKE) $@ \
	    || exit 1; cd $(BUILDDIR); \
	    echo "<<<Finished Recursively making "$$i" "$@"." ; \
	done
	rm ./bin/*s

FORCE: ;
