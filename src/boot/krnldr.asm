[org 0x500]		; offset to 0, we will set segments later
[bits 16]		; we are still in real mode
 
jmp stage2	; jump to main
 
%include "stdio16.asm"
%include "gdt.asm"
%include "fat12.asm"
%include "floppy16.asm"
%include "memory.asm"
%include "multiboot.asm"
%include "A20.asm"

[bits 16]		; we are still in real mode

%define IMAGE_PMODE_BASE 0x100000
%define IMAGE_RMODE_BASE 0x3000

ImageName     db "KRNL    SYS"
ImageSize     db 0

boot_info:
istruc multiboot_info
	at multiboot_info.flags, 			dd 0
	at multiboot_info.memory_lo, 		dd 0
	at multiboot_info.memory_hi, 		dd 0
	at multiboot_info.boot_device, 		dd 0
	at multiboot_info.cmd_ln, 			dd 0
	at multiboot_info.mods_cnt,			dd 0
	at multiboot_info.mods_addr, 		dd 0
	at multiboot_info.syms0, 			dd 0
	at multiboot_info.syms1, 			dd 0
	at multiboot_info.syms2, 			dd 0
	at multiboot_info.mmap_len, 		dd 0
	at multiboot_info.mmap_addr, 		dd 0
	at multiboot_info.drives_len, 		dd 0
	at multiboot_info.drives_addr, 		dd 0
	at multiboot_info.conf_table, 		dd 0
	at multiboot_info.boot_name, 		dd 0
	at multiboot_info.apm_table, 		dd 0
	at multiboot_info.vbe_ctrl_info, 	dd 0
	at multiboot_info.vbe_mode_info, 	dw 0
	at multiboot_info.vbe_io_seg, 		dw 0
	at multiboot_info.vbe_io_off, 		dw 0
	at multiboot_info.vbe_io_len, 		dw 0
iend
 
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

		mov [boot_info + multiboot_info.boot_device], dl
		
		mov	si, Start
		call print16

		mov	si, LoadingMsg
		call	print16

		call enable_a20		;Enables the A20 line (address bus goes to line 31 -> 4GB of memory)
		call load_gdt
		sti

		xor eax, eax
		xor ebx, ebx
		call get_mem_size_64mb

		mov DWORD [boot_info + multiboot_info.memory_hi], ebx
		mov DWORD [boot_info + multiboot_info.memory_lo], eax

		mov eax, 0
		mov ds, ax
		mov di, 0x1000
		call get_mem_map

		mov DWORD [boot_info + multiboot_info.mmap_addr], 0x1000
		mov DWORD [boot_info + multiboot_info.mmap_len], eax

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
%include "ELFLoader.asm"

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

		call clearscreen
		
		mov ebx, PModeMsg
		mov dh, TEXT_ATTRIB
		call putstring

copy_image:
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

test_image:
		mov ebx, IMAGE_PMODE_BASE
		mov esi, ebx
		mov edi, ImageSignature
		cmpsw
		je EXECUTE_KERNEL
		mov ebx, BadImage
		mov dh, TEXT_ATTRIB
		call putstring
		cli
		hlt

ImageSignature db 0x7F, "ELF" ;.ELF magic number

elf_header:
istruc elf_header_t
	at elf_header_t.magic,		dd 0
	at elf_header_t.bits,		db 0
	at elf_header_t.encoding,	db 0
	at elf_header_t.version1,	db 0
	at elf_header_t.reserved1,	dq 0
	at elf_header_t.reserved2,	db 0
	at elf_header_t.type,		dw 0
	at elf_header_t.machine,	dw 0
	at elf_header_t.version2,	dd 0
	at elf_header_t.entry,		dd 0
	at elf_header_t.phys_off,	dd 0
	at elf_header_t.sect_off,	dd 0
	at elf_header_t.flags,		dd 0
	at elf_header_t.size,		dw 0
	at elf_header_t.phys_size,	dw 0
	at elf_header_t.phys_num,	dw 0
	at elf_header_t.sect_size,	dw 0
	at elf_header_t.sect_num,	dw 0
	at elf_header_t.string_tbl,	dw 0
iend

EXECUTE_KERNEL:
		mov esi, IMAGE_PMODE_BASE
		mov edi, elf_header
		call copy_elf_header

		mov ebx, elf_header
		call load_elf_file

		mov ebp, [elf_header + elf_header_t.entry]

		mov eax, 0x2badb002		;multiboot spec
		mov ebx, boot_info
		mov edx, [ImageSize]

		call ebp				;execute kernel with multiboot parameter

		cli
		hlt

;Data section
PModeMsg: db "Entered PMode...", 0x0A, 0
BadImage: db " *** FATAL ERROR: Bad kernel image. Halting...", 0