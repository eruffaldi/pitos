# Examples Makefile - JKVM

include ../../config.mk

all: kvmain

clean:
	echo "VM Examples Cleaned"	
	rm *.o

.PHONY: kmain

kvmain: $(BINDIR)/kvm1.flp

$(BINDIR)/kvm1.flp: kvm1.img $(PMLOAD)
	cat $(PMLOAD) kvm1.img > $@

kvm1.img: kmain.o $(BINDIR)/libkvm.a $(LIBDIR)/libhsys.a $(LIBDIR)/libhc.a $(LIBDIR)/libhm.a
	$(LD) kmain.o $(LIBDIR)/libhsys.a $(BINDIR)/libkvm.a $(LINKOPT) -o $@  
	
kmain.o: kmain.cpp 
	$(GCC) $(CFLAGS) -I. -c kmain.cpp -o $@
	

	
	
	

