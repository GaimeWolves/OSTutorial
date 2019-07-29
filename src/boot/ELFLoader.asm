%ifndef ELF_LOADER
%define ELF_LOADER

%include "stdio32.asm"

struc elf_header_t
	.magic		resd 1
	.bits		resb 1
	.encoding	resb 1
	.version1	resb 1
	.reserved1	resq 1
	.reserved2	resb 1
	.type		resw 1
	.machine	resw 1
	.version2	resd 1
	.entry		resd 1
	.phys_off	resd 1
	.sect_off	resd 1
	.flags		resd 1
	.size		resw 1
	.phys_size	resw 1
	.phys_num	resw 1
	.sect_size	resw 1
	.sect_num	resw 1
	.string_tbl	resw 1
endstruc

struc phys_header_t
	.type		resd 1
	.offset		resd 1
	.v_address	resd 1
	.p_address	resd 1
	.file_size	resd 1
	.vmem_size	resd 1
	.flags		resd 1
	.align		resd 1
endstruc

;esi - source address
;edi - target address
copy_elf_header:
		pushad
		mov ecx, elf_header_t.string_tbl - elf_header_t.magic + 4
		rep movsb
		popad
		ret

;ebx address of elf_header
get_phys_headers:
		pushad
		xor edx, edx
		xor ecx, ecx
		xor eax, eax
		mov cx, WORD [ebx + elf_header_t.phys_num]
	.get_phys_headers.entry_loop:
		xor eax, eax
		mov ax, WORD [ebx + elf_header_t.phys_size]
		mul edx
		mov esi, [ebx + elf_header_t.phys_off]
		add esi, 0x100000
		add esi, eax
		mov edi, 0x10000
		add edi, eax
		push ecx
		mov cx, WORD [ebx + elf_header_t.phys_size]
		rep movsb
		pop ecx
		inc edx
		loop .get_phys_headers.entry_loop
	.get_phys_headers.done:
		popad
		ret

;ebx address of elf_header
copy_physical_segments:
		pushad
		xor edx, edx
		xor eax, eax
	.copy_physical_segments.loop:
		xor eax, eax
		mov ax, WORD [ebx + elf_header_t.phys_size]
		mul edx
		mov edi, DWORD [0x10000 + eax + phys_header_t.v_address]
		mov esi, 0x100000
		add esi, DWORD [0x10000 + eax + phys_header_t.offset]
		mov ecx, DWORD [0x10000 + eax + phys_header_t.file_size]
		rep movsb
		inc edx

		push ebx
		mov bh, 0x0F
		mov bl, '.'
		call putchar
		pop ebx

		xor eax, eax
		mov ax, WORD [ebx + elf_header_t.phys_num]
		test ax, dx
		jb .copy_physical_segments.loop
	.copy_physical_segments.done:
		popad
		ret


;ebx address of elf_header
load_elf_file:
		pushad

		push ebx
		mov ebx, LoadMsg
		mov dh, 0x0F
		call putstring
		pop ebx

		call get_phys_headers

		push ebx
		mov ebx, PhysHeaders
		mov dh, 0x0F
		call putstring
		pop ebx

		call copy_physical_segments
	.done:
		popad
		ret

LoadMsg: db "Loading File", 0x0A, 0
PhysHeaders: db "Got pyhsical sector headers. Loading sectors...", 0x0A, 0

%endif