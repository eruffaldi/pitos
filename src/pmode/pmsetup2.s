# pmsetup2.s
# Questo modulo contiene la funzione amain che viene chiamata dal modulo pmload
# poco dopo essere passato in modo protetto.
# Sara' compito di questo modulo inizializzare IDT, GDT e paginazione
# prima di passare al codice C
#
# Global Variables:
#	gdt
#	idt
# Task Structure Segment
#	carichiamo ltr
# Partiamo in modalita' sistema...
#
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
.globl amain
.globl idt,gdt, ignore_int
.globl testInt
.extern cmain
.extern _TSS
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
	
	# FIX TSS ADDRESS INTO TSS-descriptor
	leal dtss,%ebx
	leal  _TSS,%eax
	movw %ax, 2(%ebx)			# low base
	shrl $16, %eax
	movb %al, 4(%ebx)			# mid base
	movb %ah, 7(%ebx)			# hi base	
	# LOAD TSS 
	mov $0x28, %al
	ltr %ax
	# lss _stack_start , %esp
	mov $0x70000, %esp
	jmp cmain

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
	leal ignore_int, %edx
	movw $8, %ax
	shl $16, %eax			# code selector RPL = 0
	movw %dx, %ax			# offsetL
	movw $0x8E00, %dx		# P=1, DPL=0, D=32bit 
	
	leal idt, %edi
	movl $256, %ecx
idtSetup_t:
	movl %eax, (%edi)
	movl %edx, 4(%edi)
	addl $8,%edi
	loop idtSetup_t
	lidt idt_descr
	ret
	
testInt:
	int $10
	ret

.align 2
# show something
ignore_int:
	# movw $0x742, 0xb8000+160*2	# fix notify
	incb 0xb8000+160			# absolute address write
	movb $2, 0xb8000+161
	iret
	
.align 2
.word 0
gdt_descr:
	.word 256*8-1		# 256 entries limit
	.long gdt 			# linear address

.align 2
.word 0
idt_descr:
	.word 256*8-1
	.long idt
	
.balign 4
idt: .fill 256,8,0

# per il descrittore gdt vedi anche pmload.asm
gdt:	
	.quad 0
	.quad 0x00cf9a000000FFFF		# 4GB  code limit r
	.quad 0x00cf92000000FFFF		# 4GB  data limit rw
	.quad 0x00cfFa000000FFFF		# 4GB  code limit r		user 
	.quad 0x00cfF2000000FFFF		# 4GB  data limit rw	user
	
	# TSS
dtss:
	.word	105						# limit
	.word   0							# address low
	.word 	0xE900					# lo byte is address 23:16
	.word 	0x0000					# hi byte is address 31:24
									# limit is always small
	.fill 253,8,0
	
	
	