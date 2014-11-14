; PMLOAD Test by Emanuele Ruffaldi
; Lato a 32bit del test di caricamento
;
; Questa routine viene caricata all'indirizzo 0 della memoria lineare,
; il caricamento avviene usando le funzioni del BIOS

LOADSEG equ 0x1000

org 0x10000

; check pmload.asm
CODESEL equ 0x8
DATASEL equ 0x10
VIDEOSEL equ 0x18


bits 32
header:
dd 	end_pm-header		; end of all the fucked thing 
dd	data		; location of data 
dd  go_pm		; startup


; this code will be copied to location 0
go_pm:  
	push ebp
	mov ebp, esp
	mov ax,VIDEOSEL
	mov gs,ax 	
	mov word [gs:160*4],0x741 ; display white 'A'
	mov word [0xB8000+160*4+2],0x742 ; display white 'B'

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
	;jmp spin		; loop

	
; this time fit it into a full sector 
times 512-($-$$)        db 0

data	
	db "Welcome into Protected Mode",0

times 1024-($-$$)        db 0
                        
end_pm: