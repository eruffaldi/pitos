# Compiler configuration:
# GNU LD:
#   the linker is not working with binary output if target is Windows PE (like CYGWIN), so use a ld
#   retargeted one to linux coff. 
# CYGWIN Feb 2001:
#	as is bad, makes mistakes on relocations, check latest binutils, so does the gcc of CYGWIN
PITOS = /cygdrive/c/dev/asm/pitos
GCC = /usr/local/i386-linux-coff/bin/gcc
LD = $(PITOS)/tools/ld-new
AS32 = $(PITOS)/tools/as-new
AR = $(PITOS)/tools/ar

# 16 bit part: adjust the specified version of nasm, e.g. for windows is nasmw
AS = nasmw

LINKOPT_LOW = -T $(PITOS)/tools/pmloadc.lnk -L$(PITOS)/lib --oformat binary
LINKOPT = --oformat binary -T $(PITOS)/tools/pmloadc.lnk -L$(PITOS)/lib -lhsys -lhm -lhc   
INCLUDES = -I$(PITOS)/h   -I$(PITOS)/h/klib -I$(PITOS)/h/ll
BUILDPATH = $(PITOS)/bin
BINDIR = $(PITOS)/bin
LIBDIR = $(PITOS)/lib
C_OPT = -Wall -O -finline-functions -fno-builtin -nostdinc $(INCLUDES)
ASM_OPT = -ad $(INCLUDES)	
#ASM_OPT = -x assembler-with-cpp -ad $(INCLUDES)	

PMLOAD = $(PITOS)/lib/pmload
PMSETUP = $(LIBDIR)/pmsetup2.o

.SUFFIXES: .s .cpp .c .asm .img

.cpp.o:
	$(GCC) $(C_OPT) -c $< -o $@
	
.c.o:
	$(GCC) $(C_OPT) -c $< -o $@
		
%.flp : %.img
	cat $(PMLOAD) $< > $@
	
.asm.o:
	$(AS) -f coff $< -o $@

.s.o:
	$(AS32) $(ASM_OPT) $< -o $@ 
	
	
