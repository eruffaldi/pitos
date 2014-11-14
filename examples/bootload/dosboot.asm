;-------------------------------------------------------------------------------------------------------- 
; dosboot.asm 
; demonstrates getting control after the compu has booted 
; does nothing but display "NO BOOT DISK" and freeze 
; while DOS is still able to read/write the disk 
; 
; compile with NASM to binary file (nasm is assumed to be in your path) 
; nasm dosboot.asm 
; 
; written by emJay (c) 1998 last updated 31-08-98 
; 
;------------------------------------------------------------------------------------------------------- 
 
org 07C00h  ;this tells nasm that the first byte will be positioned  
 ;at address 0000:07C00h so that all the jumps will 
 ;be correct  
jmp short begin_bootroutine ;jump to the start of our bootroutine skipping any data 
nop  ;first field must be 3 bytes long jmp short is 2 bytes  
db 'MajOS1.0'  ; OEM identification  
dw 512  ; Bytes per sector  
db 1 ; Sectors per cluster 
dw 1  ; Number of reserved sectors 
db 2 ; Number of FATs  
dw 0E0h  ; Number of dirs in root  
dw 0B40h  ; Number of sectors in volume 
db 0F0h  ; Media descriptor  
dw 9  ; Number of sectors per FAT  
dw 18  ; Number of sectors per track  
dw 2  ; Number of read/write heads  
dw 0  ; Number of hidden sectors  
BootMessage: db "NO BOOT DISK"  ;reserve space for the bootmessage and initialize it 
MessageLength equ $-BootMessage ;calculate the length of the boot message at compile time 
begin_bootroutine: ;this is where the bootroutine starts  

mov ax, 0  
mov es, ax ;point es to the segment of the boot message 
mov cx, MessageLength  
mov ax,01301h ;Function 13h (ax=13h) Attribute in bl (al = 01h) 
mov bx,0007h ;screen page 0 (bh=0) white on black (bl=07h) 
mov dx,0 ;start in left corner  
mov bp, BootMessage ;takes offset of BootMessage (no offset keyword needed GREAT!) 
int 10h ;display the string 
spin: jmp short spin ; go into an infinite loop 
times 510-($-$$) db 0 ;fill with zeroes until byte 510 of the boot sector 
 ;See NASM doc for more info on $ and $$)  
dw 0xAA55 ;write boot signature (actually goes in memory as 55h AAh) 
