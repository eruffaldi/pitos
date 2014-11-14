# Examples Makefile - PMODE
# 
# Some low level examples in historical order, my passage to protected mode...
# first in raw assembly, then with loadable assembly from floppy, finally from
# C
# Next there is the system library

include ../../config.mk

all: 

clean:
	echo "PMODE Examples Cleaned"	
	rm *.o

.PHONY: 

# impostazione della linea A20
seta20.flp: seta20.asm
	$(AS) $^ -o $@        
	
# primo passaggio in modo protetto
pmboot.flp: pmboot.asm
	$(AS) $^ -o $@    

# passaggio in modo protetto minimale
pmmin.flp: pmmin.asm
	$(AS) $^ -o $@

pmload.bin: pmload.asm myload.inc
	$(AS) $< -o $@

pmload32.bin: pmload32.asm
	$(AS) $< -o $@

# primo esempio di caricamento da disco floppy del modulo e passaggio a pmode
pmload1.flp:  pmload.bin pmload32.bin
	cat pmload.bin pmload32.bin > pmload1.flp        

# primo esempio di C
# sfrutta il nostro formato per il caricamento...
pmload32.o: pmload32.c
	gcc $(C_OPT) -c pmload32.c
	
pmloadc.flp: pmload32.o pmload.bin
	$(LD) -T pmloadc.lnk pmload32.o --oformat binary -o pmloadc.bin
	cat pmload.bin pmloadc.bin > $@

# test del pic
# BUGS:
# OK on VMWARE2 
# NOTIMPLEMENTED on VMWARE3 1455
pmint1.flp:  pmint1.asm pmload.bin
	$(AS) $< -o pmint1.bin
	cat pmload.bin pmint1.bin > $@

#

