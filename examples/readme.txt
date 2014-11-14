Passo Passo verso un Sistema Operativo ...

* Simple Boot Sector
	dumbboot.asm
	dosboot.asm
* Keyboard Test
	kb2.c pmload.asm (protected mode, using libc and libm)
* A20 line Test
	seta20.asm
* Memoria installata e regioni di memoria (ACPI)
	mem1.asm
* Entrata in modo protetto
	pmboot.asm		versione ok
	pmmin.asm		versione minimale
* Caricamento di un settore da disco, prepara per pmload.asm
	dumbload.asm (dumbload2.asm)
* Caricamento di un programma destinato al pmode, prima in Asm e poi in C (richiede stack)
	pmload.asm (pmload32.asm) => pmload
	pmload.asm (pmload32.c) => pmloadc (attenzione a pmloadc.lnk)
* Giochini con la IDT, prima inizializzazione e wrapper in C 
  per ricevere notifiche di interruzione
	pmsetup.s pmc1.c => pmc1	prima idt setup
	pmsetup.s 8259.c pmc2.c => pmc2 	8259 setting up and showing timer
	
	
	
Operating System Work:

1) make a logging system
2) check components Separately

Testing:

1) salva_stato, carica_stato without side effects