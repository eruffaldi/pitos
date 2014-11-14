	.file	"strtoul.c"
gcc2_compiled.:
___gnu_compiled_c:
.text
	.align 4
.globl _strtoul
	.def	_strtoul;	.scl	2;	.type	32;	.endef
_strtoul:
	pushl %ebp
	movl %esp,%ebp
	subl $44,%esp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl 8(%ebp),%edi
	movl $0,-12(%ebp)
	.align 4
L6:
	movb (%edi),%bl
	incl %edi
	addl $-12,%esp
	movsbl %bl,%eax
	pushl %eax
	call _isspace
	addl $16,%esp
	testl %eax,%eax
	jne L6
	cmpb $45,%bl
	jne L7
	movl $1,-12(%ebp)
	jmp L37
	.align 4
L7:
	cmpb $43,%bl
	jne L8
L37:
	movb (%edi),%bl
	incl %edi
L8:
	cmpl $0,16(%ebp)
	je L11
	cmpl $16,16(%ebp)
	jne L10
L11:
	cmpb $48,%bl
	jne L10
	cmpb $120,(%edi)
	je L12
	cmpb $88,(%edi)
	jne L10
L12:
	movb 1(%edi),%bl
	addl $2,%edi
	movl $16,16(%ebp)
L10:
	cmpl $0,16(%ebp)
	jne L13
	movl $10,16(%ebp)
	cmpb $48,%bl
	jne L13
	movl $8,16(%ebp)
L13:
	movl $-1,%eax
	xorl %edx,%edx
	divl 16(%ebp)
	movl %eax,-8(%ebp)
	movl %edx,-20(%ebp)
	movl $0,-4(%ebp)
	movl $0,-16(%ebp)
	jmp L16
	.align 4
L19:
	addl $-12,%esp
	movsbl %bl,%eax
	pushl %eax
	call _isdigit
	addl $16,%esp
	testl %eax,%eax
	je L20
	addb $-48,%bl
	jmp L21
	.align 4
L20:
	addl $-12,%esp
	movsbl %bl,%esi
	pushl %esi
	call _isalpha
	addl $16,%esp
	testl %eax,%eax
	je L17
	movzbl %bl,%ebx
	addl $-12,%esp
	pushl %esi
	call _isupper
	addl $16,%esp
	testl %eax,%eax
	je L23
	leal -55(%ebx),%eax
	jmp L24
	.align 4
L23:
	leal -87(%ebx),%eax
L24:
	movb %al,%bl
L21:
	movzbl %bl,%eax
	cmpl 16(%ebp),%eax
	jge L17
	cmpl $0,-16(%ebp)
	jl L28
	movl -8(%ebp),%edx
	cmpl %edx,-4(%ebp)
	ja L28
	jne L27
	cmpl -20(%ebp),%eax
	jle L27
L28:
	movl $-1,-16(%ebp)
	jmp L18
	.align 4
L27:
	movl $1,-16(%ebp)
	movl -4(%ebp),%eax
	imull 16(%ebp),%eax
	movl %eax,-4(%ebp)
	movzbl %bl,%eax
	addl %eax,-4(%ebp)
L18:
	movb (%edi),%bl
	incl %edi
L16:
	addl $-12,%esp
	movsbl %bl,%eax
	pushl %eax
	call _isalnum
	addl $16,%esp
	testl %eax,%eax
	jne L19
L17:
	cmpl $0,-16(%ebp)
	jge L31
	movl $-1,-4(%ebp)
	call ___errnumber1
	movl $34,(%eax)
	jmp L32
	.align 4
L31:
	cmpl $0,-12(%ebp)
	je L32
	negl -4(%ebp)
L32:
	cmpl $0,12(%ebp)
	je L34
	movl 8(%ebp),%eax
	cmpl $0,-16(%ebp)
	je L35
	leal -1(%edi),%eax
L35:
	movl 12(%ebp),%edx
	movl %eax,(%edx)
L34:
	movl -4(%ebp),%eax
	leal -56(%ebp),%esp
	popl %ebx
	popl %esi
	popl %edi
	movl %ebp,%esp
	popl %ebp
	ret
	.def	___errnumber1;	.scl	2;	.type	32;	.endef
	.def	_isupper;	.scl	2;	.type	32;	.endef
	.def	_isalpha;	.scl	2;	.type	32;	.endef
	.def	_isdigit;	.scl	2;	.type	32;	.endef
	.def	_isalnum;	.scl	2;	.type	32;	.endef
	.def	_isspace;	.scl	2;	.type	32;	.endef
