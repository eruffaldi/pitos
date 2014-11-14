; Prima Procedura di Attivazione del Boot in modo
; protetto. 
; VERSIONE MINIMALE!!!!
;
;
BOOTSEG equ 0x7c0
VIDEOSEG equ 0xB800

begin:
bits 16

	mov ax, BOOTSEG
	mov ds, ax
	   		
	; disable interupts
	cli		
	
	; load Descriptor Tables
	; look at the linear address we specified
	o32 lgdt [gdtr]		
	
	; enable protected mode using cr0
	mov ebx,cr0
	or ebx,1
	mov cr0, ebx
	
	; finally jump to the startup code we specified...
	jmp 0x8:(go_pm+(BOOTSEG<<4))
	
bits 32

; this code will be copied to location 0

go_pm:  
	mov ax,datasel
	mov ds,ax 	

	; scriviamo un messaggio sulla seconda riga
	mov ebx, 160*1 + (VIDEOSEG<<4)
	mov edx, data  + (BOOTSEG<<4)
	mov ah, 0x6
topcp:
	mov al, byte [edx]
	and al,al
	jz spin
	mov word [ebx],ax
	inc edx
	add ebx,2
	jmp topcp
	
spin: 	jmp spin		; loop

data	
	db "Welcome into Protected Mode",0
	
end_pm:

[bits 16]
	
gdtr
	dw gdt_end-gdt-1; length of gdt 
	dw (BOOTSEG << 4)+gdt			; linear, physical address of gdt	
	dw 0			


; Struttura di un Descrittore:
; lim  0:15
; base 0:15
; base 16:23
; access: P DPL S Type 
;	data 0 0 E W A, con E = expandable
;	code 0 1 C R A, con C = conforme
; granularity: G D/B/E 0 AVL lim 19:16
;	D = default length if Code, 1 = 32bit
;	B = stack size if Not Expandable, 1 = 32bit
;	E = expand down segment limit, 1 = 4GB, 0 = 64K
; base 24:31
gdt
nullsel equ $-gdt ; 0h = 000b
gdt0    ; null descriptor
	dd 0 		
	dd 0
codesel equ  $-gdt ; = 8h = 1000b 
code_gdt    ; code descriptor 4 GB flat segment starting 0000:0000h 
	dw 0ffffh
	dw 0h 
	db 0h
	db 09ah			; 1 - 0 0 - 1 - 1 0 1 0
	db 0cfh			; granular,FF limit,32bit
	db 0h
datasel equ $-gdt ;=10h  = 10000b 
data_gdt 	; data descriptor 4 GB flat segment starting 0000:0000h
	dw 0ffffh		; limit 4 GB
	dw 0h			; Base 0000:0000h
	db 0h			
	db 092h			; 1 - 0 0 - 1 - 0 0 1 0		
	db 0cfh			; 1 - 1 - 0 0 - FF
	db 0h

gdt_end

times 510-($-$$)        db 0
                        dw 0xaa55
