# pmsetvm.s
# Questo modulo contiene la funzione _amain che viene chiamata dal modulo pmload
# poco dopo essere passato in modo protetto.
# Sara' compito di questo modulo inizializzare IDT, GDT e paginazione
# prima di passare al codice C
#
# Global Variables:
#	gdt
#	idt
# Task Structure Segment
#	carichiamo ltr
# Aggiungiamo anche la memoria virtuale:
#	cr0[31] = 1
#	cr3     = 0
#
# Supporto Per la gestione Blocchi di Memoria:
#	* marchiamo le aree di memoria in base all'utilizzo estendendo l'idea
#	  dell'ACPI, rendendo piu' facile la vita al O.S
#	* le zone disponibili solo quelle definite...
#	* aggiungiamo: paging memory = 4096*(MEMORIAINMB/4+1)
#	* aggiungiamo: memoria di caricamento LOADSEG..LOADSEG+size
#	* aggiungiamo: memoria per lo stack base (accodata al caricamento)
#
# Dunque occorre che questo file sappia la quantita' di memoria installata...
# Le aree di memoria fisica restanti sono lasciate al sistema operativo
#
# es: con 16 MB arriviamo fino a 4096*5 = 20K 
# Selettori modalita segmentata flat:
#	0 null
#	8 system code segment
#  10 system data code segment & stack
#  18 user code segement
#  20 user data segment & stack
#  28 tss descriptor 

#DES_LIMIT		0
#DES_BASE1500	2
#DES_BASE2316    4
#DES_BASE3124	7

.text
.globl _amain
.globl _idt,_gdt, _ignore_int
.globl _testInt
.extern _cmain
.extern __TSS
_amain:
	call gdtSetup
	call idtSetup
	# reload selectors
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %ss
	mov %ax, %gs
	
	# call getInstalledMemory
	movl $16, %eax
	call vmSetup
	
	# FIX TSS ADDRESS INTO TSS-descriptor
	leal _dtss,%ebx
	leal  __TSS,%eax
	movw %ax, 2(%ebx)			# low base
	shrl $16, %eax
	movb %al, 4(%ebx)			# mid base
	movb %ah, 7(%ebx)			# hi base	
	# LOAD TSS 
	mov $0x28, %al
	ltr %ax
	
	# FIX STACK ADDRESS
	# lss _stack_start , %esp
	mov $0x90000, %esp
	jmp _cmain

gdtSetup:
	lgdt gdt_descr
	ret
	
# impostiamo la tabella di interruzione 
# inserendovi puntatori...
# IDT Descriptor For Interrupt Gate
# 0] selector offsetL
# 4] offsetH  | P DPL 0 | D 11 0 | 000 X*5
idtSetup:
	# edx will be high dword, eax low
	leal _ignore_int, %edx
	movw $8, %ax
	shl $16, %eax			# code selector RPL = 0
	movw %dx, %ax			# offsetL
	movw $0x8E00, %dx		# P=1, DPL=0, D=32bit 
	
	leal _idt, %edi
	movl $256, %ecx
idtSetup_t:
	movl %eax, (%edi)
	movl %edx, 4(%edi)
	addl $8,%edi
	loop idtSetup_t
	lidt idt_descr
	ret
	
_testInt:
	int $10
	ret

# mapping of real memory directly into VM memory
#	MEMORYSIZEMB
# Abbiamo bisogno di 
# Memorizziamo la page Directory in 0, ogni entry:
#	page address 20, Avail 3, G, PS, 0, A, PCD, PWT, U/S, R/W, P
#   page table 20, Avail 3, G, PAT, D, A, PCD, PWT, U/S, R/W, P
# note: A/B vuol dire A se 0 , B se 1
# ogni pagina sono 4K
# ogni tabella di pagine indirizza: 1024 descrittori di tabella ovvero 
# Per la memoria Alias, accessibile ad ogni livello:
#	ADDRESS 0 0 0 0 0 0 0 0 0 0 1 1 
#   ADDRESS 0 0 0 0 0 0 0 0 0 0 1 1 
# 0.. 1023*4 = direttorio
# 1024*4 .. 1024*4 + N*4*1024 = N tabelle di pagine
#
# occupazione totale (N+1)*4096 ovvero per 16 MB pari
# a 20K di memoria

# INPUT = EAX = MEMORY IN MB
vmSetup:
# clean all
	shrl $2, %eax
	movl %eax, %ebx
	
	# clear all the directory
	movl %ebx, %ecx
	shll $10, %ecx
	addl $1024, %ecx
	xorl %eax, %eax
	xorl %edi,%edi
	cld
	rep
	stosl
	
	movl %ebx,%ecx
	xorl %edi,%edi
	movl $0x1000+7, %eax
	cld
topDir:
	stosl
	addl $0x1000, %eax	
	loop topDir
		
	# edi = points to the last page
	# eax = points to the last block
	movl %ebx, %edi
	shll $12, %edi
	addl $4092, %edi
	movl %ebx, %eax		
	shll $22, %eax			# N * 4MB
	decl %eax
	andl $0xFFFFF000, %eax
	addl $7, %eax			# fix privilege...
	std
a:	stosl
	subl $0x1000, %eax
	jge a
	
	# activate 
	xorl %eax,%eax
	movl %eax, %cr3
	movl %cr0, %eax
	orl $0x80000000 , %eax
	movl %eax, %cr0
	ret

.align 2
# show something
_ignore_int:
	# movw $0x742, 0xb8000+160*2	# fix notify
	incb 0xb8000+160			# absolute address write
	movb $2, 0xb8000+161
	iret
	
.align 2
.word 0
gdt_descr:
	.word 256*8-1		# 256 entries limit
	.long _gdt 			# linear address

.align 2
.word 0
idt_descr:
	.word 256*8-1
	.long _idt
	
.align 3
_idt: .fill 256,8,0

# per il descrittore gdt vedi anche pmload.asm
_gdt:	
	.quad 0
	.quad 0x00cf9a000000FFFF		# 4GB  code limit r
	.quad 0x00cf92000000FFFF		# 4GB  data limit rw
	.quad 0x00cfFa000000FFFF		# 4GB  code limit r		user 
	.quad 0x00cfF2000000FFFF		# 4GB  data limit rw	user
	# TSS
_dtss:
	.word	105						# limit
	.word   0							# address low
	.word 	0xE900					# lo byte is address 23:16
	.word 	0x0000					# hi byte is address 31:24
									# limit is always small
	.fill 253,8,0
	
	
	