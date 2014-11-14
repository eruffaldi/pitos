; dumbload.asm									
; by Emanuele Ruffaldi 2001
; loads a string from the second track of disk
;											
; shows the basic use of myload.inc
;
   org 07C00h                      ;this tells nasm that the first byte will be positioned	
                                   ;at address 0000:07C00h so that all the jumps will
                                   ;be correct
		
   jmp short begin_bootroutine     ;jump to the start of our bootroutine skipping any data

strBad db "Cannot Load from Disk"
strBadL equ $-strBad
MessageLength equ 5

DRIVE 	equ 0
LOADSEG equ 0x6000

%include "myload.inc"
begin_bootroutine:                 
	
   ; prepare buffer,sector, and sector count
   mov ax, LOADSEG
   mov es, ax                      
   mov ds, ax
   mov bx, 0
   mov ax, 1
   mov word [rdSec], 1   
   call bios_read_sectors
   and al,al
   jnz error
   mov ax, LOADSEG
   mov es, ax
   mov bp, 0
   mov cx, MessageLength
 
say:
   ; INT 10h/AH=13h WRITE STRING
   ; AL = 01h (update cursor)
   ; BL = attribute
   ; BH = screen page
   ; ES:BX = string
   ; CX = length
   ; DH,DL = row,col starting
   mov ax,01301h                   ;Function 13h (ax=13h) Attribute in bl (al = 01h)
   mov bx,0007h                    ;screen page 0 (bh=0)  white on black (bl=07h)
   mov dx,0                        ;start in left corner
   int 10h                         ;display the string
	
   jmp $ 

 error:
   mov ax, 0
   mov es, ax
   mov bx, strBad
   mov cx, strBadL
   jmp say
   
   times 510-($-$$) db 0           
                                   
   dw 0xAA55                       ;write boot signature (actually goes in memory as 55h AAh)

