# makefile per libhsys.a

include ../config.mk

LIBSYSOBJECTS_LOW = mylib_low.o 8259.o
LIBSYSOBJECTS = sistema.o memory.o video.o utente.o userfx.o mylib.o 8259.o syscall2.o malloc.o except.o schedule.o $(LIBDIR)/pmsetup2.o ssignal.o systime.o 
OBJSUPPORT = support/_divdi3.o support/_udivdi3.o support/_moddi3.o


.PHONY: pmload
VPATH = klib support

all: $(LIBDIR)/pmsetup2.o $(LIBDIR)/libhsys.a $(PMLOAD) $(LIBDIR)/liblow.a


$(LIBDIR)/pmsetup2.o : $(PITOS)/src/pmode/pmsetup2.s
	$(AS32) $< -o $@	

syscall2.o: klib/syscall2.asm
	$(AS) klib\\syscall2.asm -iklib\\ -f coff -o syscall2.o
	
pmload: $(PMLOAD)


$(PMLOAD): boot/pmload.asm
	$(AS) -DDEBUG boot\\pmload.asm -iboot\\ -o ..\\lib\\pmload
		
$(LIBDIR)/libhsys.a: $(LIBSYSOBJECTS) $(OBJSUPPORT) 
	$(AR) rs $@ $^

$(LIBDIR)/liblow.a: $(LIBSYSOBJECTS_LOW) $(OBJSUPPORT) 
	$(AR) rs $@ $^ 
	