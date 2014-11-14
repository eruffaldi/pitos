; System Calls by user
; this file should be linked with user files 
; accessing primitives from "User Privilege Level"

[bits 32]
SECTION .text

SYSCALL_TIPO equ	0x30


	
%macro SYSCALL 3
	global _%1
	_%1:
		push dword %3
		int SYSCALL_TIPO
		add esp,4
		ret 		; after!!!
		
; Una SysCall puo' essere realizzata anche in stile Windows NT
; non essendo una interruzione, possiamo riciclare lo stato
; chiaramente se richiediamo un task switching dovremo salvare
; il contesto.
;		mov eax, %3
;		lea edx,[esp+4]			; edx points to the params...
;		int SYSCALL_TIPO
;		ret
%endmacro

%include "syscalls.h"

