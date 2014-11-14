; Test di Caricamento del componente a 32bit dalla seconda traccia
;
; Organizzazione Memoria: spostiamo le routine in modo protetto in modo tale
; che siano posizionate all'inizio della memoria (0). E' giusto uno sfizio che
; mi tolgo dato che non sarebbe necessario.
;
; Osservazione: la linea A20 non e' strettamente necessaria
;
; TODO: DEBUG THE CODE LOADING FOR CROSSING OF TRACKS,HEAD... AND SEGMENT CROSSING!!!
BOOTSEG equ 0x7c0
INITSEG equ 0x9000
LOADSEG equ 0x1000		; see pmload32.asm

; Parametri Disco
FIRSTSEC equ 1		; first sector it's this code 
SECTRACK equ 18
DRIVE    equ 0

begin:
bits 16
	
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
	
	call leggiCodice

	; disable interupts
	cli		

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
	
	; finally jump to the local entry 32
	jmp dword 0x8:(here_pm+(INITSEG<<4))


empty_8042:
	db 0xeb
	db 0xeb
	in al,0x64
	test al,2
	jnz empty_8042
	ret

; questa routine legge il codice a 32bit da disco partendo
; dal settore FIRSTSEC e mettendolo in PMSEG. Problemi di cui tenere conto:
; 1) ottimizzare al massimo il caricamento ovvero 8 settori alla volta (512*8)
; 2) attenzione ai limiti dei segmenti nella copia
; Il primo settore letto e' molto importante poiche' vi e' posta la dimensione 
; totale dei dati da leggere. Per ora ci limiteremo  

rdTotal
	dd 0
leggiCodice:
	; first try to load one track at time...
	mov ax, LOADSEG
	mov es, ax
	mov bx, 0
	mov word [rdSec], FIRSTSEC
	mov word [rdHead],0
	mov word [rdTrack],0
	mov ax, 1		
	call leggiSettori
	add bx, 512
	
	; read the number of sectors to be read
	mov eax, dword [es:0]
	shr eax, 9
	dec ax
	mov word [rdTotal], ax
	
.top:
	cmp word [rdTotal], 0
	je .fine
	cmp word [rdSec], SECTRACK-1
	je .fineSec
	inc word [rdSec]	
	jmp .lettura
.fineSec
	mov word [rdSec], 0		
	cmp word [rdHead], 1
	je .fineHead
	inc word [rdHead]
	jmp .lettura
.fineHead
	mov word [rdHead],0
	inc word [rdTrack]
	; fall 
.lettura
	mov ax, 1		
	call leggiSettori	
	cmp bx, 0x10000-512
	jne .ok
	mov bx, es
	add bx, (0x10000-512)/16		; numero di segmenti
	mov es, bx
	jmp .next
.ok	
	add bx, 512			; fix segment bound	
.next	
	dec word [rdTotal]
	jmp .top
.fine:	
	; finish with floppy
    call kill_motor
	ret
leggiCodiceE:

%include "myload.inc"	

; Questa piccola zona a 32 bit e' l'entry dopo aver inizializzato il modo protetto
; consiste nell'impostare i selettori dati, (eventualmente lo stack) ed infine
; passare alla routine definita dall'utente e ricavata dalla terza dword del
; blocco appena caricato da disco.
bits 32
here_pm:
	mov ax,datasel
	mov ds,ax
	mov ss,ax
	mov es,ax
	mov gs,ax
	mov fs,ax
	
	; yet we don't have a stack, so just a jump
	; load the address of entry point from byte 8 of LOADSEG
	mov eax, dword [ds:8+(LOADSEG<<4)]
	; arbitrary
	mov esp, 0x90000
	call eax
spin:	jmp spin	


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

times 510-($-$$)        db 0
                        dw 0xaa55
