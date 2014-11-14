; PMLOAD3: memory information, BE CAREFUL! 512 bytes max
;
; Type: Bootsector Program
; Description: loads the program stored starting from the second sector of the disk, goes into protected
; mode and let the user program do the rest. Be Careful with Dimensions (se after)
;
; User Programs is loaded at address LOADSEG, with stack at STACK32 (linear) and
; with interrupts disabled.
;
; Second Sector Structure:
; 0 DWORD	#SIZE of bytes of program (512 bytes aligned)
; 1 DWORD   reserved
; 2 DWORD   32 bit entry point
;
; Memory Structure:
; 07C00: boot sector loaded by the BIOS (512 bytes)
; 10000: moved boot sector (512 bytes) 
;		 and 32bit stack (65K)
; 30000: first byte of the user program
;  ...
; 30000+#SIZE: last byte of user program
;

; Some Macros

BOOTSEG equ 0x7c0
LOADSEG equ 0x3000		; user program uses this!!!
STACK32 equ 0x10000
INITSEG equ 0x1000
MEMORYINFO equ 0x20000

; Disk Parameters
FIRSTSEC equ 1		
SECTRACK equ 18
DRIVE    equ 0


begin:
bits 16

	; move bootsector from 7c0 to another place
	; let us play with other loaders (if sometime...)
	cld	
	mov ax, BOOTSEG
	mov ds, ax
	mov ax, INITSEG
	mov es,ax
	mov cx,256
	xor si,si
	xor di,di
	rep movsw

	; segment change
	jmp INITSEG:go
go:

	mov ax, cs
	mov ss, ax
	mov sp, 0x400	

	; get memory info changes ds es, so don't waste bytes to setup them
	push ax
	call get_memory_info
	pop ax
	mov ds, ax
	mov es, ax
	
	call load_program

	; disable interupts
	cli		

	; load Descriptor Tables
	; look at the linear address we've specified
	o32 lgdt [gdtr]		
		
	; enable line A20
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

; Fx: load_program
; This routine load the program stored in the floppy starting at the second sector. It
; determines the number of sectors to read from the first dword of the second sector.
; It check for big reads of 8 sectors, and handles the segment overlapping 
rdTotal
	dd 0	
load_program:

	; first try to load one track at time...
	; we load 512 at time so 32 segments at time
	mov ax, LOADSEG
	mov es, ax
	xor bx, bx
	mov word [rdSec], FIRSTSEC
	mov word [rdHead],bx
	mov word [rdTrack],bx
	mov ax, 1		
	call bios_read_sectors
	; TODO check for error al != 0
	
	; get the number of sectors to be read and adjust segment
	mov eax, dword [es:0]
	shr eax, 9
	dec ax
	mov word [rdTotal], ax
	inc word [rdSec]
	mov ax, LOADSEG+32
	mov es, ax
	
	; ASSUME BX = 0	
.top
	cmp word [rdTotal], bx
	jne .continue
	jmp .end
	
	; check floppy limits: Sectors Heads Tracks
.continue
	cmp word [rdSec], SECTRACK
	je .endSurface
	jmp .goread
.endSurface
	mov word [rdSec], bx		; bx = 0 always
	cmp word [rdHead], 1
	je .endTrack
	inc word [rdHead]
	jmp .goread
.endTrack
	mov word [rdHead], bx
	inc word [rdTrack]

.goread
	; sectors to be read = min(8, min(rdTotal, 18-rdSec)))
	mov ax, SECTRACK
	sub ax, [rdSec]
	cmp ax, [rdTotal]
	jle .a_min
	mov ax, [rdTotal]
.a_min
	cmp ax, 8
	jle .a_min2
	mov ax, 8
.a_min2	
	mov dx, ax

	
	call bios_read_sectors	
	; TODO: error check al != 0
	
	; adjust counters and es segment to avoid overflow
	; (each sector is 32*16 byte)
	sub [rdTotal], dx
	add [rdSec],dx
	shl dx, 5		
	mov cx, es
	add cx, dx
	mov es, cx
	jmp .top
.end

	; finish with floppy and return
    call kill_floppy_motor   
	ret

%include "myload.inc"	

; this little 32bit zone is the entry point called after real to protected mode transition.
; it initialize the selectors and jump to the user defined program
; The loaded program has the entry stored inside the 3rd dword
bits 32
here_pm:
	mov ax,datasel
	mov ds,ax
	mov ss,ax
	mov es,ax
	mov gs,ax
	mov fs,ax
	mov esp, STACK32
	
	; load the address of entry point from the 3rd dword of LOADSEG
	mov eax, dword [ds:8+(LOADSEG<<4)]
	call eax
	jmp $


[bits 16]

gdtr
	dw gdt_end-gdt-1; length of gdt 
	dw gdt
	dw INITSEG>>12		; linear, physical address of gdt	
	
gdt
nullsel equ $-gdt ; 0h = 000b


; Descriptor Structure
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

; store memory info into MEMORYINFO :
;	# of items
;	memory size (valid if 0 items)
;	MEMORYINFO 1
;	MEMORYINFO 2
; MEMORYINFO structure:
;	low 32, hi 32 base
;	low 32, hi 32 size
; 	type: 1 is available,2 reserved , #...
; Assume: es ds setted up
SMAP equ 0534d4150h

get_memory_info:
   mov ax, (MEMORYINFO >> 4)
   mov ds, ax
   mov es, ax  
   xor ebx,ebx
   mov di, bx
   mov [di],ebx
   add di, 8
   
; Start playing with ACPI:
; INT 15/EAX=E820
; EBX = continuation (0 first time)
; ECX = buffer size (20 bytes)
; ES:DI = buffer
; EDX = signature (SMAP)
.topACPI
	mov eax, 0E820h
	mov ecx, 20
	mov edx, SMAP	
	int 15h
	jc .try88
	cmp eax, edx ; SMAP
	jne .try88
	cmp ecx, 20
	jne .try88		
	inc dword [0]
	add di, cx
	and ebx,ebx
	jne .next
	ret
.next
	jmp .topACPI	
.try88
	; standard mode 
	mov	ah,088h
	int	015h
	jc .tryE801
	mov dx, ax
	and edx, 0FFFFh
	jmp .setfreemem
	
.tryE801
	mov  ax, 0E801h
	int 15h
	jnc .next2
	ret
.next2
	; dx = above 16M in 64K
	; cx = under 1M to 16M in K
	and edx, 0FFFFh
	shl edx, 6
	and ecx, 0FFFFh
	add edx, ecx
.setfreemem
	shl edx,10
	mov dword [4], edx
	ret
	

; exactly 512 bytes
times 510-($-$$)        db 0
                        dw 0xaa55

