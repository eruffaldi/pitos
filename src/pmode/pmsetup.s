# pmsetup.s
# Questo modulo contiene la funzione amain che viene chiamata dal modulo pmload
# poco dopo essere passato in modo protetto.
# Sara' compito di questo modulo inizializzare IDT, GDT e paginazione
# prima di passare al codice C
#
# Global Variables:
#	gdt
#	idt
.text
.globl amain
.globl _idt,_gdt, _ignore_int
.globl _testInt
.extern _cmain
amain:
	call gdtSetup
	call idtSetup
	# reload selectors
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %ss
	mov %ax, %gs
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
	.fill 253,8,0
	