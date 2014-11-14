; SYSCALL2!!!!
; History:
; 4/3/2001 added signals support...
;
; implementazione delle chiamate di sistema attraverso la
; carica stato e la salva stato
;
; Note: la salva_stato e la cambia_stato effettuano le operazioni a livello di sistema
; pertanto abbiamo la garanzia che gli stack sui quali andiamo ad operare sono quelli di sistema
; Dunque non e' necessario salvare SS.
; Inoltre, viene salvato ESP in modo tale da puntare correttamente.
; Grafico Stack:
; UTENTE:
;	^
;	| -> first param [last in fx]
;	| ...
;	| -> last param [first in fx]
;	| indirizzo caller primitiva
;	| syscall index
;   ^
; SISTEMA:
;	^
;	| SS utente
;	| ESP utente -> syscallindex
;	| EFLAGS
;	| CS utente
;	| indirizzo after int_XX [utente]
;	| -> first copied param
;   | ...
;	| -> last copied param [first]
;
; La carica/salva stato tengono conto della posizione corrente
; La copia dei parametri utilizza SS:ESP utente per copiare i parametri
; sullo stack di sistema. il numero di parametri e' calcolato 
; COME IMPEDISCO CRISI IN QUESTO PASSAGGIO? 
[bits 32]
SECTION .text

extern esecuzione,desproc,traceSyscall,badSysCall
extern _TSS
extern handlesignals
global carica_stato,syscall_table,syscall_params
global ki__signalrespin
global syscalle
extern cdriver_t
global acdriver_t

; Offset dalla des_proc!!!
P_OFFCONTESTO equ 0
P_OFFSYSSTACK equ 12*4
P_OFFSIGMASK  equ P_OFFSYSSTACK+3*4

;SYSCALL_NUM equ 7

; 
SYSDATASEL  		equ 0x10

_EAX equ 0
_EBX equ 4
_ECX equ 8
_EDX equ 12
_ESI equ 16
_EDI equ 20
_EBP equ 24
_ESP equ 28		; see sistema.h
_ES equ 32
_FS equ 36
_GS equ 40
_DS equ 44

; Input:
;	eax = valid syscall index 
;   edx = user stack pointer
%macro copy_params 0
	mov ecx, dword [cs:syscall_params+eax*4]	
	and ecx,ecx
	jz .finecopy
.topcopy:
	push dword [edx+ecx*4+4]
	loop .topcopy
.finecopy:
%endmacro

%macro godwait 0
	sti
	.here: jmp .here
%endmacro

; output: DS = SYSDATASEL
%macro __salva_stato 0
	push  eax
	push  ds
	mov ax, SYSDATASEL
	mov ds,ax
	mov eax,[esecuzione]
	add eax,P_OFFCONTESTO
	mov [eax+_EBX], ebx
	mov [eax+_ECX], ecx
	mov [eax+_EDX], edx
	mov [eax+_ESI], esi
	mov [eax+_EDI], edi
	mov [eax+_EBP], ebp
	; 32 = esp
	mov [eax+_ES], es
	mov [eax+_FS], fs
	mov [eax+_GS], gs
	; 48 = ds
	; 52 = ss
	pop ebx
	mov [eax+_DS], bx		; ds
	pop ebx
	mov [eax+_EAX], ebx		; eax
	mov [eax+_ESP], esp
%endm
	
%macro __carica_stato 0
	mov ax, SYSDATASEL
	mov ds, ax
	; metti lo stack di sistema del task dentro la TSS
	; ovvero lo stack base
	mov eax, [esecuzione]
	mov ebx, [eax+P_OFFSYSSTACK]
	mov [_TSS+4], ebx
	add eax, P_OFFCONTESTO

	mov ecx, [eax+_ECX]
	mov edx, [eax+_EDX]
	mov esi, [eax+_ESI]
	mov edi, [eax+_EDI]
	mov ebp, [eax+_EBP]
	mov es, [eax+_ES]		; essendo little endian posso leggere e
	mov fs, [eax+_FS]		; scrivere in un long come una word
	mov gs, [eax+_GS]
	
	; sistema lo stack del destinatario per
	; avere ds pronto
	mov esp, [eax+_ESP]
	push dword [eax+_DS]
	; sistema EBX, EAX
	mov ebx, [eax+_EBX]
	mov eax, [eax+_EAX]
	pop ds
	iret
%endm 

carica_stato:
	__carica_stato

; chiamata di sistema con salvataggio completo del contesto:
; 	salva_stato
;	stabilisci la classe 
syscalle:
	__salva_stato
	; System Stack if CPL changed
	; |  address after INT
	; |  cs
	; |  eflags
	; |  esp
	; |  ss
	; User Stack
	; |	 CALLID
	; |  address after CALL primitiva
	; |  parametri della primitiva
	; System Stack if same CPL
	; |  address after INT
	; |  cs
	; |  eflags
	; |  CALLID
	; |  return after primitive
	; |  parameters..
	;
	; This code will make
	;  EAX = CALLID
	;  EDX = point to CALLID 
	mov edx, [esp+3*4]			; maybe a user call 
	mov eax, [esp+4]			; check CPL of caller
	and eax, 3
	jnz .goahead
	lea edx, [esp+3*4]			; fix stack for system 
.goahead:	
	mov eax, [edx]				; = CALLID
	
	; Facciamo un po' di tracing...
	push eax
	push edx
	push esp
	; void traceSyscall(int* esp,int*pastack)
	call traceSyscall
	pop edx
	pop edx
	pop eax

	; Stabiliamo se l'ID e' corretto
	cmp dword [ds:syscall_table+eax*4], 0
	je badsyscall
	copy_params
	; sistema gli ultimi registri
	mov bx, ds
	mov es, bx
	
	; chiama la funzione opportuna usando l'indice
	; rispetto alla tabella	
	call dword [ds:syscall_table+eax*4]
	
	; occorre preservare eax!!!
;	mov ebx, [_esecuzione]
;	mov [ebx+_EAX],eax
	
	; recupera lo stato e anche lo stack
	; con iret automatica
	
	; FALL!  controlla se ho una signal...
signals_check:	
	mov edx, [esecuzione]
	mov ebx, [edx+P_OFFSIGMASK]
	and ebx,ebx
	jnz .next
	jmp carica_stato
.next	
	call handlesignals
	jmp carica_stato			; fall !!

badsyscall:
	push eax
	call badSysCall
	jmp carica_stato

acdriver_t:	
	__salva_stato
	call cdriver_t
	jmp carica_stato
	

	
	
; dichiarazione della tabella, ovvero prendiamo solamente il nome della
; funzione, prefisso c e dichiarazione extern
%macro SYSCALL 3
	extern ki_%1
	dd ki_%1
%endmacro

syscall_table:
%include "syscalls.h"
	times(100) dd 0
	
; tabella parametri
; numero bytes
%macro SYSCALL 3
	dd %2
%endmacro

syscall_params:
%include "syscalls.h"
	times(100) dd 0
