; Prima Procedura di Attivazione del Boot in modo
; protetto. 
;
; Lasciamo perdere tutti gli automatismi dovuti alla
; direttiva org, andiamo, invece a lavorare direttamente
; con i segmenti.
;

INITSEG equ 0x9000	
BOOTSEG equ 0x7c0

lf equ 10
cr equ 13


begin:
bits 16

	mov ax, BOOTSEG
	mov ds, ax
	mov es, ax

	; affermiamo la nostra esistenza al mondo con un primo
	; messaggio
	mov cx, msglBefore
	mov bx, 7
	mov bp, msgBefore
	mov ax, 0x1301
	mov dl, 0
	mov dh, 0
	int 0x10
	
	; spostiamoci dal boot segment 7c0 alla locazione di 
	; inizializzazione del segmento (grazie Linus!)
	cld	
	mov ax, BOOTSEG
	mov ds, ax
	mov ax, INITSEG
	mov es,ax
	mov cx,256
	xor si,si
	xor di,di
	rep movsw

	; cambio di segmento
	jmp INITSEG:go
go:

	; nuovo segmento nuovo tutto!!!
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x400	
	
	; diciamo ancora una volta al mondo della nostra esistenza
	mov cx, msglAfter
	mov bx, 7
	mov bp, msgAfter
	mov ax, 0x1301
	mov dl, 0
	mov dh, 1
	int 0x10
		
;	mov ax,0x0600   ; BIOS-function clear/scroll window
;	mov cx,0x00	; define window to be cleared from 0,0 
;	mov dx,0x174F	; to 23,79
;	mov bh,0	; fillcolor = 0 
;	int 10h		; clear screen

	; proviamo a scrivere a mano sullo schermo 
	mov ax,0xB800
	mov gs,ax 
   	mov word [gs:160*4],0x641 ;display brown 'a'
   	
   	; aspettiamo un tasto
	mov ah,00h 
	int 16h	
	
	; disable interupts
	cli		
	
	; copy from go_pm to end_pm 
	; making good things for protected mode
	cld
	mov ax, 0
	mov es, ax
	mov si, go_pm
	mov cx, end_pm - go_pm + 1
	mov di, 0
	rep movsb
	
	; load Descriptor Tables
	; look at the linear address we specified
	o32 lgdt [gdtr]		
	
	; enable A20
	call empty_8042
	mov al, 0xD1
	out  0x64,al
	call empty_8042
	mov al, 0xD
	out 0x60,al
	call empty_8042


	; enable protected mode using cr0
	mov ebx,cr0
	or ebx,1
	mov cr0, ebx
	
	; finally jump to the startup code we specified...
	jmp 0x8:0
	
empty_8042:
	db 0xeb
	db 0xeb
	in al,0x64
	test al,2
	jnz empty_8042
	ret
	
bits 32

; this code will be copied to location 0

go_pm:  
	; address in rm 0000:0000 
	; address in pm 0010:0000
	; address physical 0000:0000
	; usiamo videosel per accedere allo schermo
	; protremmo anche scrivere direttamente usando
	; la locazione lineare...
	mov ax,datasel
	mov ds,ax 	
	mov es,ax
	mov ax,videosel
	mov gs,ax 	
	mov word [gs:160*4],0x741 ; display white 'A'
	mov word [0xB8000+160*4+2],0x742 ; display white 'B'

	; scriviamo un messaggio sulla quinta riga
	mov ebx, 160*5
	mov edx, data-go_pm
	mov ah, 0x6
topcp:
	mov al, byte [edx]
	and al,al
	jz spin
	mov word [gs:ebx],ax
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
	dw gdt
	dw INITSEG>>12		; linear, physical address of gdt	

gdt
nullsel equ $-gdt ; 0h = 000b


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
videosel	equ	$-gdt ;= 18h = 11000b
	dw 3999			; limit 80*25*2-1
	dw 0x8000		; base 0xB8000
	db 0x0B			; 
	db 0x92			; present, ring 0, data, expand-up, writable
	db 0			; byte-granular, 16-bit
	db 0

gdt_end

msgBefore
	db "Before Moving to 9000:0000",cr,lf
	msglBefore equ $-msgBefore   
msgAfter
	db "After Moving to 9000:0000",cr,lf,"Press any key to PM",cr,lf
	msglAfter equ $-msgAfter 

times 510-($-$$)        db 0
                        dw 0xaa55
