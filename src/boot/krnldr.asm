[org 0x500]		; offset to 0, we will set segments later
[bits 16]		; we are still in real mode
 
jmp stage2	; jump to main
 
%include "stdio16.asm"
%include "gdt.asm"
%include "fat12.asm"
%include "floppy16.asm"

[bits 16]		; we are still in real mode

%define IMAGE_PMODE_BASE 0x100000
%define IMAGE_RMODE_BASE 0x3000

ImageName     db "KRNL    SYS"
ImageSize     db 0
 
;STAGE 2 Entry point
stage2:
		cli					; clear interrupts
		xor ax, ax
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax

		mov ax, 0x0
		mov ss, ax
		mov sp, 0xFFFF
		mov bp, 0xFFFF
		sti					; enable interrupts
		
		mov	si, Start
		call print16

		mov	si, LoadingMsg
		call	print16

		call	LoadRoot		; Load root directory table

		mov	ebx, 0			; BX:BP points to buffer to load to
		mov	bp, IMAGE_RMODE_BASE
		mov	si, ImageName		; our file to load
		call	LoadFile		; load our file
		mov	dword [ImageSize], ecx	; save size of kernel
		cmp	ax, 0			; Test for success
		je	EnterStage3		; yep--onto Stage 3!
		mov	si, msgFailure		; Nope--print error
		call	print16
		mov	ah, 0
		int     0x16                    ; await keypress
		int     0x19                    ; warm boot computer
		cli				; If we get here, something really went wong
		hlt
		
EnterStage3:
		call load_gdt

		cli					; clear interrupts
		mov	eax, cr0		; set bit 0 in cr0 and enter pmode
		or	eax, 1
		mov	cr0, eax
		
		jmp	CODESEG:stage3	; far jump to fix CS.
 
;Data section
Start:	db "Preparing to load operating system...", 0x0D, 0x0A, 0
LoadingMsg: db 0x0D, 0x0A, "Searching for Operating System...", 0x00
msgFailure: db 0x0D, 0x0A, "*** FATAL: MISSING OR CURRUPT KRNL.SYS. Press Any Key to Reboot", 0x0D, 0x0A, 0x0A, 0x00


;#########################################################################################################
;Stage 3

[bits 32]

%include "stdio32.asm"
%include "A20.asm"

[bits 32]

stage3:
		mov	eax, DATASEG		;set data segments to data selector (0x10)
		mov ds, ax               
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax

		mov ebp, 0x90000        ;Erneure die Stack Position auf 0x90000.
		mov esp, ebp

		call enable_a20		;Enables the A20 line (address bus goes to line 31 -> 4GB of memory)

		call clearscreen
		
		mov ebx, PModeMsg
		mov dh, TEXT_ATTRIB
		call putstring

CopyImage:
		mov	eax, dword [ImageSize]
		movzx	ebx, word [bpbBytesPerSector]
		mul	ebx
		mov	ebx, 4
		div	ebx
		cld
		mov esi, IMAGE_RMODE_BASE
		mov	edi, IMAGE_PMODE_BASE
		mov	ecx, eax
		rep	movsd                   ; copy image to its protected mode address
		
		jmp	CODESEG:IMAGE_PMODE_BASE; jump to our kernel! Note: This assumes Kernel's entry point is at 1 MB
		
		cli
		hlt

;Data section
PModeMsg: db "Entered PMode...", 0x0A, 0