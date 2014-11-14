# Examples Makefile - BASICOS
#
# Example Classification:
#
# LOWEST: access to pm directly from assembly
# LOW: C implemented amain, basic hw access
# MID: libsys used, with only kernel 
# HIGH: libsys used, with user and kernel tasks
# HIGHEST: java :)
include ../../config.mk

all: pmprint pmc1 simplest

clean:
	echo "BASICOS Examples Cleaned"	
	rm *.o

.PHONY: pmprint pmc1 simplest

pmprint: pmprint.flp
pmc1: pmc1.flp
simplest: simplest.flp

# kb2: amain direct con accesso alla tastiera
# TYPE: LOW
# API: videocpy, inportb, halt
kb2.img : kb2.o
	$(LD) $< $(LIBDIR)/liblow.a $(LINKOPT_LOW) -lhc -o $@
	
pmc1.img: pmc1.o
	$(LD) $< $(LINKOPT) -o $@ 

# MID
simplest.img: simplest.o
	$(LD) $< $(LINKOPT_LOW) -o $@ 

# test per bug di alcune versioni del GCC+BINUTILS
twofilebug.img: twofilebug1.o twofilebug2.o
	$(LD) $^ $(LINKOPT_LOW) -o $@

umode1.img : umode1.o
	$(LD) $^ $(PMSETUP) $(LINKOPT) -o $@

# pmprint	
# HI
# User and System processes
pmprint.img: pmprint.o $(LIBDIR)/libhsys.a
	$(LD) $< $(PMSETUP) $(LINKOPT) -o $@  
