; Test PIC

; check pmload.asm

LOADSEG equ 0x1000
CODESEL equ 0x8
DATASEL equ 0x10
VIDEOSEL equ 0x18
org 0x10000

PICM equ 0x20
PICS equ 0xA
IRQ0M equ 0x20
IRQ0S equ 0x28

bits 32
; header:
dd 	end_pm		; end of all the fucked thing 
dd	data		; location of data 
dd  startup		; startup

; this code will be copied to location 0
startup:  
	push ebp
	mov ebp, esp
	mov ax,VIDEOSEL
	mov gs,ax 	
	mov word [gs:160*4],0x741 ; display white 'A'
	mov word [0xB8000+160*4+2],0x742 ; display white 'B'

	call init8259
	; scriviamo un messaggio sulla quinta riga
	mov ebx, 160*5
	mov edx, data
	mov ah, 0x6
topcp:
	mov al, byte [edx]
	and al,al
	jz spin
	mov word [gs:ebx],ax
	inc edx
	add ebx,2
	jmp topcp
	
spin:
	leave
	ret	
	
init8259:	
	mov al, 0x11
	out PICM, al		; ICW1
	mov al, IRQ0M
	out PICM+1, al		; ICW2
	mov al, 00000100b
	out PICM+1, al		; ICW3
	mov al, 1
	out PICM+1, al		; ICW4
	mov al, 11111111b
	out PICM+1, al		; OCW1
	mov al, 0x40
	out PICM, al		; OCW3
	
	mov al, 0x11
	out PICS, al		; ICW1
	mov al, IRQ0S
	out PICS+1, al	; ICW2
	mov al, 2
	out PICS+1, al		; ICW3
	mov al, 1
	out PICS+1, al		; ICW4
	mov al, 11111111b
	out PICS+1, al		; OCW1
	mov al, 0x40
	out PICS, al		; OCW3
	ret

data	
	db "Welcome into Protected Mode",0

end_pm:

times (1+(($-$$)/512))*512-($-$$)        db 0
                        
