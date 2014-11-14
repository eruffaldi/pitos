	.file	"pmprint.cpp"
gcc2_compiled.:
___gnu_compiled_cplusplus:
.text
LC0:
	.ascii "message to system logging\0"
	.align 32
LC1:
	.ascii "message to Application Viewport\12Two Lines\12Three\0"
	.align 32
LC2:
	.ascii "Welcome into Message Logging for OSSYS\0"
	.align 32
LC3:
	.ascii "Non posso Allocare un Processo\0"
	.align 4
.globl _umain__Fi
	.def	_umain__Fi;	.scl	2;	.type	32;	.endef
_umain__Fi:
	pushl %ebp
	movl %esp,%ebp
	subl $20,%esp
	pushl %ebx
	movl 8(%ebp),%ebx
	addl $-8,%esp
	pushl $LC0
	movl _stderr,%eax
	pushl %eax
	call _fprintf
	addl $-8,%esp
	pushl $LC1
	movl _stdout,%eax
	pushl %eax
	call _fprintf
	addl $32,%esp
	addl $-12,%esp
	pushl $13
	call _putchar
	addl $-12,%esp
	pushl $42
	call _putchar
	addl $32,%esp
	testl %ebx,%ebx
	jne L85
/APP
	cli
/NO_APP
	jmp L88
	.align 4
L85:
	addl $-12,%esp
	pushl $3
	call _video_setpage
	.align 4
L88:
	jmp L88
	.align 4
.globl _cmain
	.def	_cmain;	.scl	2;	.type	32;	.endef
_cmain:
	pushl %ebp
	movl %esp,%ebp
	subl $24,%esp
	call _kernel_init__Fv
	addl $-12,%esp
	pushl $LC2
	call _log_message
	addl $-12,%esp
	leal -1(%ebp),%eax
	pushl %eax
	leal -8(%ebp),%eax
	pushl %eax
	pushl $0
	pushl $0
	pushl $_umain__Fi
	call _ki_activate_p
	addl $48,%esp
	addl $-12,%esp
	leal -1(%ebp),%eax
	pushl %eax
	leal -8(%ebp),%eax
	pushl %eax
	pushl $0
	pushl $100
	pushl $_umain__Fi
	call _ki_activate_p
	addl $32,%esp
	cmpb $0,-1(%ebp)
	jne L93
	addl $-12,%esp
	pushl $LC3
	call _log_message
	addl $16,%esp
L93:
	call _kernel_go__Fv
	movl %ebp,%esp
	popl %ebp
	ret
	.def	_kernel_go__Fv;	.scl	2;	.type	32;	.endef
	.def	_ki_activate_p;	.scl	3;	.type	32;	.endef
	.def	_umain__Fi;	.scl	2;	.type	32;	.endef
	.def	_log_message;	.scl	3;	.type	32;	.endef
	.def	_kernel_init__Fv;	.scl	2;	.type	32;	.endef
	.def	_video_setpage;	.scl	3;	.type	32;	.endef
	.def	_putchar;	.scl	3;	.type	32;	.endef
	.def	_fprintf;	.scl	3;	.type	32;	.endef
