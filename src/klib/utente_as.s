/** System Calls by user
//this file should be linked with user files 
// accessing primitives from "User Privilege Level"
*/
.text
	.align 4

.equ SYSCALL_TIPO ,	0x30


.macro SYSCALL nome x codice
	.globl \nome
	.def	\nome;	.scl	2;	.type	32;	.endef	
	\nome:
		pushl \codice
		int $SYSCALL_TIPO
		addl %esp,4
		ret 				
/*
// Una SysCall puo' essere realizzata anche in stile Windows NT
// non essendo una interruzione, possiamo riciclare lo stato
// chiaramente se richiediamo un task switching dovremo salvare
// il contesto.
//		mov eax, %3
//		lea edx,[esp+4]			// edx points to the params...
//		int SYSCALL_TIPO
//		ret
*/
.endm

.include "syscall.inc"

