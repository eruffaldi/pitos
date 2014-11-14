# Makefile FOR PITOS
MAKE=make
TOP=${ROOT}/j2me_cldc
BUILDDIR=${TOP}/build
PLATFORM=win32
ROMIZING=true
INCLUDE_ALL_CLASSES=false
export PLATFORM

SUBDIRS = \
  $(TOP)/tools/preverifier/build/win32 \
  $(TOP)/api 
#  $(TOP)\samples \
#  $(TOP)\samples/jam

ifneq ($(ROMIZING), false) 
   SUBDIRS += $(TOP)/tools/jcc
endif

SUBDIRS += $(TOP)/kvm/VmPit/build


all clean : FORCE
	@for i in $(SUBDIRS) ; do \
	    echo ">>>Recursively making "$$i" "$@"..."; \
	    cd $$i; $(MAKE) $@ \
	    || exit 1; cd $(BUILDDIR); \
	    echo "<<<Finished Recursively making "$$i" "$@"." ; \
	done

FORCE: ;
