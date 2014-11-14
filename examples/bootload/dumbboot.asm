;------------------------------------------------------------------------------------------	
; dumbboot.asm									
; demonstrates getting control after the compu has booted	
; does nothing but display "NO BOOT DISK"  and freeze		
;									
; compile with NASM to binary file (nasm is assumed to be in your path)		
;  nasm dumbboot.asm 					
;											
; written by emJay (c) 1998 last updated 29-08-98					
;											
;------------------------------------------------------------------------------------------	

   org 07C00h                      ;this tells nasm that the first byte will be positioned	
                                   ;at address 0000:07C00h so that all the jumps will
                                   ;be correct
			
   jmp short begin_bootroutine     ;jump to the start of our bootroutine skipping any data

BootMessage: db "NO BOOT DISK"     ;reserve space for the bootmessage and initialize it
MessageLength equ $-BootMessage    ;calculate the length of the boot message at compile time

begin_bootroutine:                 ;this is where the bootroutine starts
	
   mov ax, 0
   mov es, ax                      ;point es to the segment of the boot message
   mov cx, MessageLength
   
   mov ax,01301h                   ;Function 13h (ax=13h) Attribute in bl (al = 01h)
   mov bx,0007h                    ;screen page 0 (bh=0)  white on black (bl=07h)
   mov dx,0                        ;start in left corner
   mov bp, BootMessage             ;takes offset of BootMessage (no offset keyword, GREAT!)
   int 10h                         ;display the string

	mov ax, 0xb800
   mov es, ax
   mov word [es:0],0x741
   spin: jmp short  spin           ;go into an infinite loop
           
   times 510-($-$$) db 0           ;fill with zeroes until byte 510 of the boot sector
                                   ;See NASM doc for more info on $ and $$)
   dw 0xAA55                       ;write boot signature (actually goes in memory as 55h AAh)

