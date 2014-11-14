        org 07c00h
; start address 0000:07c00h physical	
	
	jmp short skipdata
	nop
	db 'MajOS1.0'
	dw 512 		; Bytes per sector
	db 1		; Sectors per cluster
	dw 1		; Number of reserved sectors
	db 2  		; Number of FATs
	dw 0E0h		; Number of dirs in root
	dw 0B40h	; Number of sectors in volume
	db 0F0h		; Media descriptor
	dw 9		; Number of sectors per FAT
	dw 18		; Number of sectors per track
	dw 2 		; Number of read/write heads
	dw 0 		; Number of hidden sectors

skipdata:
bits 16
	mov ax,0x0600   ; BIOS-function clear/scroll window
	mov cx,0x00	; define window to be cleared from 0,0 
	mov dx,0x174F	; to 23,79
	mov bh,0	; fillcolor = 0 
	int 10h		; clear screen
	mov ax,0xB800
	mov gs,ax ; point gs to video memory
     	mov word [gs:0],0x641 ;display brown 'a'
	mov ah,00h 
	int 16h	;wait until keypress
	cli		; disable interupts
o32	lgdt [gdtr]	; load the global descriptor table
	mov  eax,cr0	
	or al,1
	mov  cr0,eax	; set PE bit to 1
	jmp codesel:go_pm
bits 32

go_pm:  ; address in rm 0000:16-bit offset go_pm
	; address in pm 0010:32-bit offset go_pm
	; address physical 0000:32-bit offset go_pm
	mov ax,datasel
	mov ds,ax 	; point gs to dataseg
	mov es,ax
	mov ax,videosel
	mov gs,ax 	; point gs to videomemory
	mov word [gs:0],0x741 ; display white 'a'
spin: 	jmp spin		; loop

[bits 16]

gdtr
	dw gdt_end-gdt-1; length of gdt 
	dd gdt	    ; linear, physical address of gdt	

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
	db 09ah
	db 0cfh
	db 0h
datasel equ $-gdt ;=10h  = 10000b 
data_gdt 	; data descriptor 4 GB flat segment starting 0000:0000h
	dw 0ffffh		; limit 4 GB
	dw 0h			; Base 0000:0000h
	db 0h			
	db 092h			
	db 0cfh
	db 0h
videosel	equ	$-gdt ;= 18h = 11000b
	dw 3999			; limit 80*25*2-1
	dw 0x8000		; base 0xB8000
	db 0x0B
	db 0x92			; present, ring 0, data, expand-up, writable
	db 0			; byte-granular, 16-bit
	db 0

gdt_end

;equates

lf equ 10
cr equ 13

times 510-($-$$)        db 0
                        dw 0xaa55
