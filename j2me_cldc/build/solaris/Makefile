BUILDDIR=$(shell pwd)
TOP=../..
PLATFORM=solaris

SUBDIRS = \
  $(TOP)/tools/preverifier/build/solaris \
  $(TOP)/api \
  $(TOP)/samples \
  $(TOP)/samples/jam

ifneq ($(ROMIZING), false) 
   SUBDIRS += $(TOP)/tools/jcc
endif

SUBDIRS += $(TOP)/kvm/VmUnix/build

all clean : FORCE
	@for i in $(SUBDIRS) ; do \
	    echo ">>>Recursively making "$$i" "$@"..."; \
	    cd $$i; $(MAKE) $@ \
	    || exit 1; cd $(BUILDDIR); \
	    echo "<<<Finished Recursively making "$$i" "$@"." ; \
	done


kvm.zip : FORCE
	@echo ... $@
	@rm -rf $@
	@(cd $(TOP);  \
	   find . -type f -print | grep SCCS/s | grep -v deleted  \
	                         | grep -v Codemgr_wsdata/backup  \
	                         | sed -e 's/SCCS\/s.//'          \
                                 | zip build/solaris/kvm.zip -@   )

FORCE: 
