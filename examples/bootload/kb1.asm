;------------------------------------------------------------------------------------------	
; kb1.asm									
; Booting program for Keyboard...
; 
; Interfaccia Tastiera:
;	0x60	RBR
;	0x64 	STR	bit 0=FI, bit 1=FO
;
; by Emanuele Ruffaldi 09022001
;						
; quando FI=1 posso prelevare un "codice di sistema da RBR", se e' maggiore di 128
; si tratta di un break code+128, altrimenti un make code (key down)
;
; Tasti Notevoli:
;	1 = ESC
; 	
;------------------------------------------------------------------------------------------	


   org 07C00h                      ;this tells nasm that the first byte will be positioned	
                                   ;at address 0000:07C00h so that all the jumps will
                                   ;be correct
   jmp begin_bootroutine     ;jump to the start of our bootroutine skipping any data

BootMessage: db "KEYBOARD TEST",10,"Line",0     ;reserve space for the bootmessage and initialize it

moveCur:
	push ax
	push bx
	push dx
	mov ah,3
	mov bh,0
	int 10h
	cmp dl, 79
	jg .nextRow
	inc dl
	jmp .done
.nextRow:
	mov dl, 0
	cmp dh, 24
	jg .nextPage
	inc dh
	jmp .done
.nextPage:
	mov dh,0
.done:
	mov ah,2
	mov bh,0
	int 10h
	pop dx
	pop bx
	pop ax
	ret

nextRow:
	push ax
	push bx
	push dx
	mov ah,3
	mov bh,0
	int 10h
	mov dl, 0
	cmp dh, 24
	jg .nextPage
	inc dh
	jmp .done
.nextPage:
	mov dh,0
.done:
	mov ah,2
	mov bh,0
	int 10h
	pop dx
	pop bx
	pop ax
	ret

videochr:
	cmp al, 0
	je .esci
	cmp al,10
	je .nextRow
	call video
	call moveCur
	jmp .esci
.nextRow
	call nextRow
.esci:
	ret
	
; ax = address
; return ax = length
strlen:
	push bp
	mov bp, ax
	xor ax,ax
.top:	
	cmp byte [bp], 0
	je .esci
	inc ax
	inc bp
	jmp .top
.esci:
	pop bp
	ret

; ax = string ASCIIZ
videostr:
	push bp
	push ax
	mov bp, ax
.top:
	mov al, [bp]
	inc bp
	cmp al,0
	je .esci
	call videochr
	jmp .top
.esci:	
	pop ax
	pop bp
   	ret

; al = code
; uses:
;	int 10h al=ascii ah=09 bx=color cx=count
video:
	push ax
	push bx
	push cx
	mov ah, 09h
	mov bx, 0007h
	mov cx, 1
	int 10h
	pop cx
	pop bx
	pop ax
	ret


begin_bootroutine:                 ;this is where the bootroutine starts

   mov ax,0
   mov es,ax
   mov ds,ax
   
   mov ax, BootMessage             ;takes offset of BootMessage (no offset keyword, GREAT!)
   call videostr
   mov al, 'a'
   call videochr
	
   spin: jmp short spin           ;go into an infinite loop
           
   times 510-($-$$) db 0           ;fill with zeroes until byte 510 of the boot sector
                                   ;See NASM doc for more info on $ and $$)
   dw 0xAA55                       ;write boot signature (actually goes in memory as 55h AAh)

