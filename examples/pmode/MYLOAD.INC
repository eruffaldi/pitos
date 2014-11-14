rdSec 		dw	0		; 0..SECTRACK-1
rdHead	    dw	0		; 0/1
rdTrack		dw  0		; 0..TRACKS

; Meccanismo di Caricamento:
; 	carichiamo i settori del floppy usando: int 13h,AH=2,DL=drive, DH=head,
;		CH=track, CL[0-5] = settore (1-63), AL = numero settori al piu' 1-8
;		ES:BX = buffer
; 	architettura: testina traccia settore
;		cicliamo per ogni traccia, per ognuna delle due faccie, per ogni settore (SECTRACK)

; leggiSettori
; Input: rdSec,rdHead,rdTrack specify the start; 
;		 AL = length
;		 ES:BX = buffer
; Assume:
;		 DRIVE constant
; Output: 
; 		 AL = 0 no error, AL = 1 error 
leggiSettori:
	push dx
	push cx
	mov cx, word [rdSec]
	inc cx
	and cl, 111111b		; CL[0-5]
	mov dx, word [rdTrack]
	mov ch, dl			; CH
	mov dx, word [rdHead]
	mov dh,dl		
	and dh, 1			; DH head
	mov dl, DRIVE
	mov ah, 2
	int 13h	
	mov al,0
	jnc .fine
	inc al
.fine:
	pop  cx
	pop  dx 
	ret

kill_motor:
        push dx
        mov dx, 0x3f2
        mov al, 0
        out dx, al
        pop dx
        ret

