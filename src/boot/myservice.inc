;
; NASM UTILS
;

	
; print_hex is for debugging purposes, and prints the word
; pointed to by dx in hexadecimal.
; Input: dx = hex digit, cx = length
; From bootsect.S of linux, great
print_hex:
	push ax
	push cx
	push edx	
print_digit:
	rol	edx,4				; rotate to use low 4 bits
	mov	  ax, 0e0fh			; ah = request
	and	al,dl   			; al = mask for nybble
	add	al,090h				; convert al to ascii hex
	daa							; in only four instructions!
	adc		al, 040h
	daa
	int		010h
	loop	print_digit
	pop edx
	pop cx
	pop ax
	ret

print_nl:
	push ax
	mov ax , 0E0Dh
	int 10h
	mov al, 0Ah
	int 10h
	pop ax
	ret
	
print_nl2:
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

	

; string pointed by ds:si
; output: writes the asciiz string using BIOS
; Loses the SI,AL
print_str:
	lodsb
	and		al,al
	jz	print_str__fin
	call	print_chr
	jmp	print_str
print_str__fin:	
	ret

; Space printing
print_sp2:	call	print_spc		; Print double space
print_spc:	mov		al, 020h		; Print single space (note: fall-thru)

; Part of above routine, this one just prints ascii al
print_chr:	
	push	ax
	push	cx
	xor		bh,bh
	mov		cx,1
	mov		ah, 0eh
	int		10h
	pop		cx
	pop		ax
	ret


