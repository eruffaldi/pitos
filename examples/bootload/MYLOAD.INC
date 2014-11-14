; File: MYLOAD.INC
; Author: Emanuele Ruffaldi 2001
; Description: fx to load a sector from a floppy disk
;
; 	floppy loading using: int 13h/AH=2
;		DL=drive (0,1,...,80h..)
;		DH=head
;		CH=track
;		CL[0-5] = sector (1-63)
;		AL = number of sectors (1-8)
;		ES:BX = buffer
;
; Used by: pmload

; support variables
rdSec 		dw	0		; 0..SECTRACK-1
rdHead	    dw	0		; 0..1
rdTrack		dw  0		; 0..TRACKS

; leggiSettori
; Input: rdSec,rdHead,rdTrack specify the start; 
;		 AL = length
;		 ES:BX = buffer
; Assume:
;		 DRIVE constant
; Output: 
; 		 AL = 0 no error, AL = 1 error 
bios_read_sectors:
	push dx
	push cx
	mov cx, [rdSec]
	inc cx
	and cl, 111111b		; CL[0-5]
	mov dx, [rdTrack]
	mov ch, dl			; CH
	mov dx, [rdHead]
	mov dh,dl		
	and dh, 1			; DH head
	mov dl, DRIVE
	mov ah, 2
	int 13h	
	mov al,0
	jnc .end
	inc al
.end
	pop  cx
	pop  dx 
	ret

; stops floppy motor from running
; PORT=0x3f2 VALUE=0
kill_floppy_motor:
    push dx
    mov dx, 0x3f2
    mov al, 0
    out dx, al
    pop dx
    ret

