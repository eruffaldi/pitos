# Examples Makefile
# memory	memory information
# jkvm		k-virtual machine
# bootload	
# pmode		basic pmode startup
# vm		virtual memory testing
# basicos 	basic os test 
#
# Each directory contains some projects that creates a .flp image into the bin directory
# test with your favorite machine emulator.
include ../config.mk

SUBDIRS = memory jkvm bootload pmode vm basicos

all clean : FORCE
	@for i in $(SUBDIRS) ; do \
	    echo ">>>Recursively making "$$i" "$@"..."; \
	    cd $$i; $(MAKE) $@ \
	    || exit 1; cd $(BUILDDIR); \
	    echo "<<<Finished Recursively making "$$i" "$@"." ; \
	done

FORCE: ;


