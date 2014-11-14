	.file	"w_sqrtf.c"
gcc2_compiled.:
___gnu_compiled_c:
.text
	.align 4
.globl _sqrtf
	.def	_sqrtf;	.scl	2;	.type	32;	.endef
_sqrtf:
	pushl %ebp
	movl %esp,%ebp
	subl $40,%esp
	addl $-12,%esp
	pushl 8(%ebp)
	call ___ieee754_sqrtf
	fstps -20(%ebp)
	addl $16,%esp
	cmpl $-1,__fdlib_version
	je L4
	addl $-12,%esp
	pushl 8(%ebp)
	call _isnanf
	addl $16,%esp
	testl %eax,%eax
	je L3
L4:
	flds -20(%ebp)
	jmp L2
	.align 4
L3:
	fldz
	fcomps 8(%ebp)
	fnstsw %ax
	andb $69,%ah
	jne L5
	addl $-12,%esp
	pushl $126
	flds 8(%ebp)
	subl $8,%esp
	fstl (%esp)
	subl $8,%esp
	fstpl (%esp)
	call ___kernel_standard
	fstps -4(%ebp)
	flds -4(%ebp)
	jmp L9
	.align 4
L5:
	flds -20(%ebp)
L9:
L2:
	movl %ebp,%esp
	popl %ebp
	ret
	.def	___kernel_standard;	.scl	2;	.type	32;	.endef
	.def	_isnanf;	.scl	2;	.type	32;	.endef
	.def	___ieee754_sqrtf;	.scl	2;	.type	32;	.endef
