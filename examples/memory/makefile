# Examples Makefile - Memory 

include ../../config.mk

all: mem1 pmmem1

clean:
	echo "Memory Examples Cleaned"	
	rm *.o *.flp

# examples ...
.PHONY: mem1 pmmem1

mem1: $(BINDIR)/mem1.flp 

pmmem1: $(BINDIR)/pmmem1.flp 

# visualizza informazioni sulla memoria
$(BINDIR)/mem1.flp: mem1.asm
	$(AS) mem1.asm -o mem1.flp
	cp mem1.flp  $(BINDIR)/mem1.flp
	
# PM visualizza informazioni sulla memoria 
$(BINDIR)/pmmem1.flp: pmmem1.o ../../lib/libsys.a $(PMLOAD)
	$(LD)  pmmem1.o  -o $@ $(LINKOPT) 
	cat $(PMLOAD) $@ > $(BINDIR)/pmmem1.flp    
    
    

	
	
	

