	.file	"pmload32.c"
gcc2_compiled.:
___gnu_compiled_c:
.text
LC0:
	.ascii "Hello from C in PM \0"
	.align 2
.globl _amain
_amain:
	pushl %ebp
	movl %esp,%ebp
	subl $12,%esp
	movl $LC0,-4(%ebp)
	movl $753664,-8(%ebp)
	movl -4(%ebp),%eax
	movl %eax,-12(%ebp)
L2:
	movl -12(%ebp),%eax
	cmpb $0,(%eax)
	jne L4
	jmp L3
	.align 2,0x90
L4:
	movl -8(%ebp),%eax
	movl -12(%ebp),%edx
	movb (%edx),%cl
	movb %cl,(%eax)
	incl -12(%ebp)
	incl -8(%ebp)
	movl -8(%ebp),%eax
	movb $7,(%eax)
	incl -8(%ebp)
	jmp L2
	.align 2,0x90
L3:
L1:
	leave
	ret
