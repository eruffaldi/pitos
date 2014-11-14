	.file	"pmprint.cpp"
gcc2_compiled.:
__gnu_compiled_cplusplus:
.text
.LC0:
	.ascii "message to system logging\0"
	.balign 32
.LC1:
	.ascii "message to Application Viewport\12Two Lines\12Three\0"
	.balign 32
.LC2:
	.ascii "Welcome into Message Logging for OSSYS\0"
	.balign 32
.LC3:
	.ascii "Non posso Allocare un Processo\0"
	.balign 4
.globl umain__Fi
umain__Fi:
.LFB1:
	pushl %ebp
.LCFI0:
	movl %esp,%ebp
.LCFI1:
	subl $20,%esp
.LCFI2:
	pushl %ebx
.LCFI3:
	movl 8(%ebp),%ebx
	addl $-12,%esp
	pushl $10
.LCFI4:
	call __builtin_vec_new
	addl $-8,%esp
	pushl $.LC0
	pushl stderr
	call fprintf
	addl $32,%esp
	addl $-8,%esp
	pushl $.LC1
	pushl stdout
	call fprintf
	addl $-12,%esp
	pushl $13
	call putchar
	addl $32,%esp
	addl $-12,%esp
	pushl $42
	call putchar
	addl $16,%esp
	testl %ebx,%ebx
	jne .L85
/APP
	cli
/NO_APP
	jmp .L88
	.balign 4
.L85:
	addl $-12,%esp
	pushl $3
	call video_setpage
	.balign 4
.L88:
	jmp .L88
.LFE1:
	.balign 4
.globl cmain
cmain:
.LFB2:
	pushl %ebp
.LCFI5:
	movl %esp,%ebp
.LCFI6:
	subl $8,%esp
.LCFI7:
	call kernel_init__Fv
	addl $-12,%esp
	pushl $.LC2
.LCFI8:
	call log_message
	.balign 4
.L93:
	jmp .L93
.LFE2:

.data
	.balign 4
.globl _GLOBAL_$F$umain__Fi
_GLOBAL_$F$umain__Fi:
__FRAME_BEGIN__:
	.long .LECIE1-.LSCIE1

.LSCIE1:
	.long 0

		.byte	0x1
		.byte	0x0
		.byte	0x1
		.byte	0x7c
		.byte	0x8
		.byte	0xc
		.byte	0x5
		.byte	0x4
		.byte	0x88
		.byte	0x1
	.balign 4
.LECIE1:
	.long .LEFDE1-.LSFDE1

.LSFDE1:
	.long .LSFDE1-__FRAME_BEGIN__

	.long .LFB1

	.long .LFE1-.LFB1

		.byte	0x4
	.long .LCFI0-.LFB1

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long .LCFI1-.LCFI0

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long .LCFI3-.LCFI1

		.byte	0x83
		.byte	0x8
		.byte	0x4
	.long .LCFI4-.LCFI3

		.byte	0x2e
		.byte	0x10
	.balign 4
.LEFDE1:
	.long .LEFDE3-.LSFDE3

.LSFDE3:
	.long .LSFDE3-__FRAME_BEGIN__

	.long .LFB2

	.long .LFE2-.LFB2

		.byte	0x4
	.long .LCFI5-.LFB2

		.byte	0xe
		.byte	0x8
		.byte	0x84
		.byte	0x2
		.byte	0x4
	.long .LCFI6-.LCFI5

		.byte	0xd
		.byte	0x4
		.byte	0x4
	.long .LCFI8-.LCFI6

		.byte	0x2e
		.byte	0x10
	.balign 4
.LEFDE3:
	.long 0

