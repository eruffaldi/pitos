   org 07C00h                      ;this tells nasm that the first byte will be positioned	
                                   ;at address 0000:07C00h so that all the jumps will
                                   ;be correct
			
   jmp short begin_bootroutine     ;jump to the start of our bootroutine skipping any data

BootMessage: db "TESTING..."     ;reserve space for the bootmessage and initialize it
MessageLength equ $-BootMessage    ;calculate the length of the boot message at compile time
A20Enabled: db "A20 is Enabled"
A20EnabledLL equ $-A20Enabled
A20NotEnabled: db "A20 is NOT Enabled"
A20NotEnabledLL equ $-A20NotEnabled

begin_bootroutine:                 ;this is where the bootroutine starts
	
   mov ax, 0
   mov es, ax                      ;point es to the segment of the boot message
   mov cx, MessageLength
   
   mov ax,01301h                   ;Function 13h (ax=13h) Attribute in bl (al = 01h)
   mov bx,0007h                    ;screen page 0 (bh=0)  white on black (bl=07h)
   mov dx,0                        ;start in left corner
   mov bp, BootMessage             ;takes offset of BootMessage (no offset keyword, GREAT!)
   int 10h                         ;display the string

   call EnableA20
   mov bp, A20Enabled
   mov cx, A20EnabledLL
   jnc doA20Msg
   mov bp, A20NotEnabled
   mov cx, A20NotEnabledLL   
doA20Msg:
	mov ax,01301h
	mov bx, 0003h
	mov dl,0
	mov dh,1
	int 10h
   spin: jmp short  spin           ;go into an infinite loop


enablea20kbwait:                      ;wait for safe to write to 8042
   xor cx,cx                          ;loop a maximum of FFFFh times
enablea20kbwaitl0:
   jmp short $+2                      ;these three jumps are inserted to wait some clockcycles
   jmp short $+2                      ;for the port to settle down
   jmp short $+2
   in al,64h                          ;read 8042 status
   test al,2                          ;buffer full? zero-flag is set if bit 2 of 64h is not set
   loopnz enablea20kbwaitl0           ;if yes (bit 2 of 64h is set), loop until cx=0
  ret

;while the above loop is executing keyboard interrupts will occur which will empty the buffer
;so be sure to have interrupts still enabled when you execute this code

enablea20test:                        ;test for enabled A20
   mov al,byte [fs:0]                 ;get byte from 0:0
   mov ah,al                          ;preserve old byte
   not al                             ;modify byte
   xchg al,byte [gs:10h]              ;put modified byte to 0ffffh:10h
                                      ;which is either 0h or 100000h depending on the a20 state
   cmp ah,byte [fs:0]                 ;set zero if byte at 0:0 equals preserved value
                                      ;which means a20 is enabled
   mov [gs:10h],al                    ;put back old byte at 0ffffh:10h
  ret                                 ;return, zeroflag is set if A20
enabled

EnableA20:                            ;hardware enable gate A20 (entry point of routine

   xor ax,ax                          ;set A20 test segments 0 and 0ffffh
   mov fs,ax                          ;fs=0000h
   dec ax
   mov gs,ax                          ;gs=0ffffh

   call enablea20test                 ;is A20 already enabled?
   jz short enablea20done             ;if yes (zf is set), done

;if the system is PS/2 then bit 2 of port 92h (Programmable Option Select)
;controls the state of the a20 gate

   in al,92h                          ;PS/2 A20 enable
   or al,2                            ;set bit 2 without changing the rest of al
   jmp short $+2                      ;Allow port to settle down
   jmp short $+2
   jmp short $+2
   out 92h,al                         ;enable bit 2 of the POS
   call enablea20test                 ;is A20 enabled?
   jz short enablea20done             ;if yes, done

   call enablea20kbwait               ;AT A20 enable using the 8042 keyboard controller
                                      ;wait for buffer empty (giving zf set)
   jnz short enablea20f0              ;if failed to clear buffer jump

   mov al,0d1h                        ;keyboard controller command 01dh (next byte written to
   out 64h,al                         ;60h will go to the 8042 output port

   call enablea20kbwait               ;clear buffer and let line settle down
   jnz short enablea20f0              ;if failed to clear buffer jump

   mov al,0dfh                        ;write 11011111b to the 8042 output port 
                                      ;(bit 2 is anded with A20 so we should set that one) 
   out 60h,al
   call enablea20kbwait               ;clear buffer and let line settle down

enablea20f0:                          ;wait for A20 to enable
   mov cx,800h                        ;do 800h tries

enablea20l0:
   call enablea20test                 ;is A20 enabled?
   jz enablea20done                   ;if yes, done

   in al,40h                          ;get current tick counter (high byte)
   jmp short $+2
   jmp short $+2
   jmp short $+2
   in al,40h                          ;get current tick counter (low byte)
   mov ah,al                          ;save low byte of clock in ah

enablea20l1:                          ;wait a single tick
   in al,40h                          ;get current tick counter (high byte)
   jmp short $+2
   jmp short $+2
   jmp short $+2
   in al,40h                          ;get current tick counter (low byte)
   cmp al,ah                          ;compare clocktick to one saved in ah
   je enablea20l1                     ;if equal wait a bit longer

   loop enablea20l0                   ;wait a bit longer to give a20 a chance to get enabled
   stc                                ;a20 hasn't been enabled so set carry to indicate failure
  ret                                 ;return to caller
enablea20done:
   clc                                ;a20 has been enabled succesfully so clear carry
  ret                                 ;return to caller

           
   times 510-($-$$) db 0           ;fill with zeroes until byte 510 of the boot sector
                                   ;See NASM doc for more info on $ and $$)
   dw 0xAA55                       ;write boot signature (actually goes in memory as 55h AAh)

