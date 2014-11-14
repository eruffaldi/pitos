;------------------------------------------------------------------------------------------	
; mem1.asm									
;
; by Emanuele Ruffaldi 13022001
;						
; Ritorna le dimensioni della memoria
; 1) usando INT 15/E820 ovvero ACPI che fornisce aree di memoria, con possibilita' 
;	 di buchi, il tipo e': 1 RAM, 2 Reserved, 3 ACPI Reserved, 4 NVS sleep
;	http://www.teleport.com/~acpi/acpihtml/topic245.htm
; 2) usando INT 15/E801 ritorna memoria fino a 32bit
; 3) usando INT 15/88 ritorna memoria fino a 64MB
;------------------------------------------------------------------------------------------	


   org 07C00h                      ;this tells nasm that the first byte will be positioned	
                                   ;at address 0000:07C00h so that all the jumps will
                                   ;be correct
   jmp begin_bootroutine     ;jump to the start of our bootroutine skipping any data


%INCLUDE "myservice.inc"

CR equ 0D0h
LF equ 0A0h
SMAP equ 0534d4150h

szIntro: db "Extended Memory Sizes in KB...",CR,LF,0
szBase: db "ACPI Base:",0
szLimit: db " Limit:",0
szType: db " Type:",0
szKB: db "KB",0
szE801: db "INT 15/AX=E801 ",0
szMem88: db "INT 15/AH=88 ",0

bufsMAP:
	dd	0	; low 32	base
	dd	0	; hi 32
	dd	0	; low 32	length
	dd	0	; hi 32
	dd	0	; type
	
begin_bootroutine:                 ;this is where the bootroutine starts

   mov ax,0
   mov es,ax
   mov ds,ax
; Start playing with ACPI 
	xor ebx,ebx
topACPI:
	xor ax,ax
	mov es, ax
	mov eax,0E820h
	mov ecx, 20
	mov edx,SMAP	; 'SMAP'
	mov di, bufsMAP
	int 15h
	jc try88
	cmp eax, edx ; SMAP
	jne try88
	; the size should be at least 20 and not more than the input
	; buffer
	cmp ecx, 20
	jne try88	
	push ebx
	mov si, szBase
	call print_str
	mov cx,8
	mov ebp, bufsMAP
	mov edx, DWORD [ebp+4]
	call print_hex
	mov edx, DWORD [ebp]
	call print_hex
	mov si, szLimit
	call print_str
	mov edx, DWORD [ebp+12]
	call print_hex
	mov edx, DWORD [ebp+8]
	call print_hex
	mov si, szType
	call print_str
	mov edx, DWORD [ebp+16]
	call print_hex
	call print_nl
	pop ebx
	cmp ebx,0
	je try88
	jmp topACPI
	
try88:
	; standard mode 
	mov	ah,088h
	int	015h
	jc tryE801
	mov dx, ax
	and edx, 0FFFFh
	;shl edx, 10
	mov cx, 8
	mov si, szMem88
	call print_str
	call print_hex
	mov si, szKB
	call print_str
	call print_nl
	
tryE801:
	mov  ax, 0E801h
	int 15h
	jc spin
	; dx = above 16M in 64K
	; cx = under 1M to 16M in K
	and edx, 0FFFFh
	shl edx, 6
	and ecx, 0FFFFh
	add edx, ecx
	mov si, szE801
	call print_str
	mov cx, 8
	call print_hex
	mov si,szKB
	call print_str
	call print_nl
	

   spin: jmp short spin           ;go into an infinite loop
           
   times 510-($-$$) db 0           ;fill with zeroes until byte 510 of the boot sector
                                   ;See NASM doc for more info on $ and $$)
   dw 0xAA55                       ;write boot signature (actually goes in memory as 55h AAh)

	