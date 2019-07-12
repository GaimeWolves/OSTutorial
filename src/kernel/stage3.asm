[org 0x100000]		; Kernel starts at 1 MB
[bits 32]			; 32 bit code
 
jmp	stage3			; jump to stage 3
 
msg db  0x0A, 0x0A, "Welcome to OwOS v0.0.1.2.3.4.2.1.beta.2.3 666.8869", 0x0A, 0

%include "stdio32.asm"
%include "gdt.asm"
 
[bits 32]
stage3:
		cli

		mov	ax, DATASEG		; set data segments to data selector (0x10)
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax

		mov ebp, 0x90000        ;Erneure die Stack Position auf 0x90000.
		mov esp, ebp

		call	clearscreen

		mov	ebx, msg
		mov dh, TEXT_ATTRIB
		call	putstring
 
		cli
		hlt