[bits 32]

start:
; testing trampoline
push ds
push es
push fs
push gs
pusha
mov ax, 0x10
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
; ? new stack..
cld 
mov eax, 10000
fix:
call eax
popa
pop gs
pop fs
pop es
pop ds
iret
end:
