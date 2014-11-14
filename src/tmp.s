	.file	"systime.c"
gcc2_compiled.:
___gnu_compiled_c:
	.def	___udivdi3;	.scl	2;	.type	32;	.endef
.text
	.align 4
LC1:
	.long 0xaaaaaaab
	.align 4
.globl _get_cmos_time
	.def	_get_cmos_time;	.scl	2;	.type	32;	.endef
_get_cmos_time:
	pushl %ebp
	movl %esp,%ebp
	subl $108,%esp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl 8(%ebp),%edi
	movl $0,-88(%ebp)
	movl $112,%esi
	movb $10,%bl
	movl $113,%ecx
	.align 4
L108:
	movl %esi,%edx
	movb %bl,%al
/APP
	outb %al, %dx
/NO_APP
	movl %ecx,%edx
/APP
	inb %dx, %al
/NO_APP
	testb %al,%al
	jl L106
	incl -88(%ebp)
	cmpl $999999,-88(%ebp)
	jle L108
L106:
	movl $0,-88(%ebp)
	movl $112,%esi
	movb $10,%bl
	movl $113,%ecx
	.align 4
L116:
	movl %esi,%edx
	movb %bl,%al
/APP
	outb %al, %dx
/NO_APP
	movl %ecx,%edx
/APP
	inb %dx, %al
/NO_APP
	testb %al,%al
	jge L114
	incl -88(%ebp)
	cmpl $999999,-88(%ebp)
	jle L116
L114:
	movl $112,%ebx
	movl $113,%esi
	.align 4
L138:
	movl %ebx,%edx
	movb $0,%al
/APP
	outb %al, %dx
/NO_APP
	movl %esi,%edx
/APP
	inb %dx, %al
/NO_APP
	movzbl %al,%eax
	movl %eax,-68(%ebp)
	movb $2,%al
	movl %ebx,%edx
/APP
	outb %al, %dx
/NO_APP
	movl %esi,%edx
/APP
	inb %dx, %al
/NO_APP
	movzbl %al,%eax
	movl %eax,-64(%ebp)
	movb $4,%al
	movl %ebx,%edx
/APP
	outb %al, %dx
/NO_APP
	movl %esi,%edx
/APP
	inb %dx, %al
/NO_APP
	movzbl %al,%eax
	movl %eax,-60(%ebp)
	movb $7,%al
	movl %ebx,%edx
/APP
	outb %al, %dx
/NO_APP
	movl %esi,%edx
/APP
	inb %dx, %al
/NO_APP
	movzbl %al,%eax
	movl %eax,-56(%ebp)
	movb $8,%al
	movl %ebx,%edx
/APP
	outb %al, %dx
/NO_APP
	movl %esi,%edx
/APP
	inb %dx, %al
/NO_APP
	movzbl %al,%eax
	movl %eax,-52(%ebp)
	movb $9,%al
	movl %ebx,%edx
/APP
	outb %al, %dx
/NO_APP
	movl %esi,%edx
/APP
	inb %dx, %al
/NO_APP
	movzbl %al,%eax
	movl %eax,-84(%ebp)
	movl %ebx,%edx
	movb $0,%al
/APP
	outb %al, %dx
/NO_APP
	movl %esi,%edx
/APP
	inb %dx, %al
/NO_APP
	movzbl %al,%eax
	cmpl %eax,-68(%ebp)
	jne L138
	movl $112,%edx
	movb $11,%al
/APP
	outb %al, %dx
/NO_APP
	movl $113,%edx
/APP
	inb %dx, %al
/NO_APP
	movl -68(%ebp),%edx
	andl $15,%edx
	movl -68(%ebp),%eax
	shrl $4,%eax
	leal (%eax,%eax,4),%eax
	leal (%edx,%eax,2),%eax
	movl %eax,-68(%ebp)
	movl -64(%ebp),%edx
	andl $15,%edx
	movl -64(%ebp),%eax
	shrl $4,%eax
	leal (%eax,%eax,4),%eax
	leal (%edx,%eax,2),%eax
	movl %eax,-64(%ebp)
	movl -60(%ebp),%edx
	andl $15,%edx
	movl -60(%ebp),%eax
	shrl $4,%eax
	leal (%eax,%eax,4),%eax
	leal (%edx,%eax,2),%eax
	movl %eax,-60(%ebp)
	movl -56(%ebp),%edx
	andl $15,%edx
	movl -56(%ebp),%eax
	shrl $4,%eax
	leal (%eax,%eax,4),%eax
	leal (%edx,%eax,2),%eax
	movl %eax,-56(%ebp)
	movl -52(%ebp),%edx
	andl $15,%edx
	movl -52(%ebp),%eax
	shrl $4,%eax
	leal (%eax,%eax,4),%eax
	leal (%edx,%eax,2),%eax
	movl %eax,-52(%ebp)
	movl -84(%ebp),%edx
	andl $15,%edx
	movl -84(%ebp),%eax
	shrl $4,%eax
	leal (%eax,%eax,4),%eax
	leal (%edx,%eax,2),%eax
	movl %eax,-84(%ebp)
	cmpl $69,%eax
	ja L142
	addl $100,%eax
	movl %eax,-84(%ebp)
L142:
	testl %edi,%edi
	je L143
	movl -68(%ebp),%ecx
	movl %ecx,-48(%ebp)
	movl -64(%ebp),%esi
	movl %esi,-44(%ebp)
	movl -60(%ebp),%eax
	addl $9,%eax
	movl %eax,-40(%ebp)
	movl -52(%ebp),%edx
	movl %edx,-32(%ebp)
	movl -56(%ebp),%ecx
	movl %ecx,-36(%ebp)
	movl -84(%ebp),%esi
	movl %esi,-28(%ebp)
	leal -48(%ebp),%esi
	cld
	movl $11,%ecx
	rep
	movsl
L143:
	movl -84(%ebp),%esi
	movl -52(%ebp),%ebx
	addl $-2,%ebx
	testl %ebx,%ebx
	jg L144
	addl $12,%ebx
	decl %esi
L144:
	movl %esi,%ecx
	shrl $2,%ecx
	movl $1374389535,%edx
	movl %edx,%eax
	mull %esi
	movl %edx,%eax
	shrl $5,%eax
	subl %eax,%ecx
	movl %edx,%eax
	shrl $7,%eax
	addl %eax,%ecx
	imull $367,%ebx,%ebx
	movl %ebx,%eax
	mull LC1
	movl %edx,%eax
	shrl $3,%eax
	addl %eax,%ecx
	addl -56(%ebp),%ecx
	imull $365,%esi,%eax
	addl $-719499,%eax
	xorl %edx,%edx
	movl %eax,%esi
	movl %edx,%edi
	addl %ecx,%esi
	adcl $0,%edi
	movl %esi,%ecx
	movl %edi,%ebx
	movl %ecx,%eax
	movl %ebx,%edx
	shldl $1,%eax,%edx
	sall $1,%eax
	addl %ecx,%eax
	adcl %ebx,%edx
	shldl $3,%eax,%edx
	sall $3,%eax
	movl %eax,%ecx
	movl %edx,%ebx
	addl -60(%ebp),%ecx
	adcl $0,%ebx
	movl %ecx,%eax
	movl %ebx,%edx
	shldl $4,%eax,%edx
	sall $4,%eax
	subl %ecx,%eax
	sbbl %ebx,%edx
	shldl $2,%eax,%edx
	sall $2,%eax
	movl %eax,%ecx
	movl %edx,%ebx
	addl -64(%ebp),%ecx
	adcl $0,%ebx
	movl %ecx,%eax
	movl %ebx,%edx
	shldl $4,%eax,%edx
	sall $4,%eax
	subl %ecx,%eax
	sbbl %ebx,%edx
	shldl $2,%eax,%edx
	sall $2,%eax
	movl %eax,%ecx
	movl %edx,%ebx
	addl -68(%ebp),%ecx
	adcl $0,%ebx
	movl %ecx,%eax
	movl %ebx,%edx
	popl %ebx
	popl %esi
	popl %edi
	movl %ebp,%esp
	popl %ebp
	ret
	.align 4
.globl _init_boot_time
	.def	_init_boot_time;	.scl	2;	.type	32;	.endef
_init_boot_time:
	pushl %ebp
	movl %esp,%ebp
	subl $16,%esp
	pushl %esi
	pushl %ebx
	movl $1000,%ebx
	movl $0,%esi
	movl %ebx,%esi
	xorl %ebx,%ebx
	call _get_cpu_clock
	xorl %edx,%edx
	pushl %edx
	pushl %eax
	pushl %esi
	pushl %ebx
	call ___udivdi3
	addl $16,%esp
	movl %eax,_STAMP_TO_MILLIS
	addl $-12,%esp
	pushl $0
	call _get_cmos_time
	movl %eax,%ecx
	movl %edx,%ebx
	movl %ecx,_boot_time
	movl %ebx,_boot_time+4
/APP
	rdtsc
/NO_APP
	movl %eax,_boot_stamp
	movl %edx,_boot_stamp+4
	movl %ecx,%eax
	movl %ebx,%edx
	shldl $5,%eax,%edx
	sall $5,%eax
	subl %ecx,%eax
	sbbl %ebx,%edx
	shldl $2,%eax,%edx
	sall $2,%eax
	addl %ecx,%eax
	adcl %ebx,%edx
	shldl $3,%eax,%edx
	sall $3,%eax
	movl %eax,_boot_millis
	movl %edx,_boot_millis+4
	leal -24(%ebp),%esp
	popl %ebx
	popl %esi
	movl %ebp,%esp
	popl %ebp
	ret
	.align 4
.globl _get_current_millis
	.def	_get_current_millis;	.scl	2;	.type	32;	.endef
_get_current_millis:
	pushl %ebp
	movl %esp,%ebp
	subl $48,%esp
	pushl %esi
	pushl %ebx
	call _get_tick_count
	movl %eax,%ecx
	movl %edx,%ebx
	subl _boot_stamp,%ecx
	sbbl _boot_stamp+4,%ebx
	movl _STAMP_TO_MILLIS,%eax
	xorl %edx,%edx
	movl %eax,-24(%ebp)
	movl %edx,-20(%ebp)
	mull %ecx
	movl %eax,-8(%ebp)
	movl %edx,-4(%ebp)
	movl -20(%ebp),%esi
	imull %ecx,%esi
	addl %esi,%edx
	movl %edx,-4(%ebp)
	movl -24(%ebp),%eax
	imull %ebx,%eax
	addl %eax,-4(%ebp)
	movl -8(%ebp),%eax
	movl -4(%ebp),%edx
	movl %edx,-8(%ebp)
	movl $0,-4(%ebp)
	movl _boot_millis,%edx
	addl %edx,-8(%ebp)
	movl _boot_millis+4,%edx
	adcl %edx,-4(%ebp)
	movl -8(%ebp),%eax
	movl -4(%ebp),%edx
	popl %ebx
	popl %esi
	movl %ebp,%esp
	popl %ebp
	ret
	.align 4
.globl _get_boot_millis
	.def	_get_boot_millis;	.scl	2;	.type	32;	.endef
_get_boot_millis:
	pushl %ebp
	movl %esp,%ebp
	movl _boot_millis,%eax
	movl _boot_millis+4,%edx
	movl %ebp,%esp
	popl %ebp
	ret
	.align 4
.globl _get_tick_count
	.def	_get_tick_count;	.scl	2;	.type	32;	.endef
_get_tick_count:
	pushl %ebp
	movl %esp,%ebp
/APP
	rdtsc
/NO_APP
	movl %ebp,%esp
	popl %ebp
	ret
.lcomm _boot_time,16
.lcomm _boot_stamp,16
.lcomm _boot_millis,16
.lcomm _STAMP_TO_MILLIS,16
	.align 4
.globl _sinc_cmos_second
	.def	_sinc_cmos_second;	.scl	2;	.type	32;	.endef
_sinc_cmos_second:
	pushl %ebp
	movl %esp,%ebp
	pushl %edi
	pushl %esi
	pushl %ebx
	xorl %edi,%edi
	movl $112,%esi
	movb $10,%bl
	movl $113,%ecx
	.align 4
L91:
	movl %esi,%edx
	movb %bl,%al
/APP
	outb %al, %dx
/NO_APP
	movl %ecx,%edx
/APP
	inb %dx, %al
/NO_APP
	testb %al,%al
	jl L89
	incl %edi
	cmpl $999999,%edi
	jle L91
L89:
	xorl %edi,%edi
	movl $112,%esi
	movb $10,%bl
	movl $113,%ecx
	.align 4
L99:
	movl %esi,%edx
	movb %bl,%al
/APP
	outb %al, %dx
/NO_APP
	movl %ecx,%edx
/APP
	inb %dx, %al
/NO_APP
	testb %al,%al
	jge L97
	incl %edi
	cmpl $999999,%edi
	jle L99
L97:
	popl %ebx
	popl %esi
	popl %edi
	movl %ebp,%esp
	popl %ebp
	ret
	.def	_get_cmos_time;	.scl	2;	.type	32;	.endef
	.def	_get_cpu_clock;	.scl	2;	.type	32;	.endef
	.def	_get_tick_count;	.scl	2;	.type	32;	.endef
