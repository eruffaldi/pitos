; NON FUNZIONA
; Test di Accoppiamento Codice 32bit
; il programam e' fatto di un boot sector che carica il resto del file e passa
; in modo protetto saltando alla routine specificata
;
; Sfortunatamente non funziona dato che non sembra considerare la direttiva org
; quindi lo costruiamo separatamente come pmload32

LOADSEG equ 0x1000
org 0x10000

; Here Starts the Second sector loaded by Leggi codice with
; its 32 bit space, we can use as much as space we want!

; PMLOAD Test by Emanuele Ruffaldi
; Lato a 32bit del test di caricamento
;
; Questa routine viene caricata all'indirizzo 0 della memoria lineare,
; il caricamento avviene usando le funzioni del BIOS

%include "pmload.asm"
[bits 32]

userstart equ (0x10000-0x200)

; header:
dd  end_pm+userstart		; end of all the fucked thing 
dd	data+userstart			; location of data 
dd  startup+userstart		; startup

; this code will be copied to location 0
startup:  
	jmp esci
	push ebp
	mov ebp, esp
	mov word [0xB8000+160*4+2],0x742 ; display white 'B'

	; scriviamo un messaggio sulla quinta riga
	mov ebx, 160*5+0xB8000
	mov edx, data+userstart
	mov ah, 0x6
topcp:
	mov al, byte [ds:edx]
	and al,al
	jz esci
	mov word [ds:ebx],ax
	inc edx
	add ebx,2
	jmp topcp
		
esci: jmp esci
	leave
	ret	

data	
	db "Welcome into Protected Mode",0
	
end_pm:
                       
; adjust to multiple of sector size 512                        
times (1+(($-$$)/512))*512-($-$$)        db 0
                        
