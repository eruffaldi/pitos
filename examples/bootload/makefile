include ../../config.mk

# dumbboot: first example of boot
dumbboot.flp:  dumbboot.asm
	$(AS) $^ -o $@	

# kb1: just say hello
kb1.flp: kb1.asm
	$(AS) $^ -o $@ 	

# dumbload: loading example, in real mode, uses two file of 512 bytes 
$(BUILDPATH)/dumbload dumbload:   dumbload.asm dumbload2.asm
	$(AS) dumbload.asm
	$(AS) dumbload2.asm 
	cat -B dumbload dumbload2 | cp $(BUILDPATH)/dumbload
