%ifndef MEMORY
%define MEMORY

[bits 16]

struc mem_entry
    .base       resq 1
    .length     resq 1
    .type       resd 1
    .acpi_null  resd 1
endstruc

get_mem_map:
    push ebx
	push ecx
	push edx
	push esi
	push edi
	push ebp
    xor ebx, ebx
    xor bp, bp
    mov edx, 'PAMS'
    mov eax, 0xe820
    mov ecx, 24
    int 0x15
    jc .get_mem_map.error
    cmp eax, 'PAMS'
    jne .get_mem_map.error
    test ebx, ebx
    je .get_mem_map.error
    jmp .get_mem_map.start
.get_mem_map.next_entry:
    mov edx, 'PAMS'
    mov ecx, 24
    mov eax, 0xe820
    int 0x15
.get_mem_map.start:
    jcxz .get_mem_map.skip_entry
.get_mem_map.add_entry:
    mov ecx, [es:di + mem_entry.length]
    test ecx, ecx
    jne short .get_mem_map.good_entry
    mov ecx, [es:di + mem_entry.length + 4]
    jecxz .get_mem_map.skip_entry
.get_mem_map.good_entry:
    inc bp
    add di, 24
.get_mem_map.skip_entry:
    cmp ebx, 0
    jne .get_mem_map.next_entry
    jmp .get_mem_map.done
.get_mem_map.error:
    stc
.get_mem_map.done:
	mov eax, ebp
	pop ebp
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
    ret


get_mem_size_64mb_32bit:
    push ecx
    push edx
    xor ecx, ecx
    xor edx, edx
    mov ax, 0xe881
    int 0x15
    jc .get_mem_size_64mb_32bit.error
    cmp ah, 0x86
    je .get_mem_size_64mb_32bit.error
    cmp ah, 0x80
    je .get_mem_size_64mb_32bit.error
    jcxz .get_mem_size_64mb_32bit.use_ax
    mov ax, cx
    mov bx, dx
.get_mem_size_64mb_32bit.use_ax:
    pop edx
    pop ecx
    ret
.get_mem_size_64mb_32bit.error:
    mov ax, -1
    mov bx, 0
    pop edx
    pop ecx
    ret


get_mem_size_64mb:
    push ecx
    push edx
    xor ecx, ecx
    xor edx, edx
    mov ax, 0xe801
    int 0x15
    jc .get_mem_size_64mb.error
    cmp ah, 0x86
    je .get_mem_size_64mb.error
    cmp ah, 0x80
    je .get_mem_size_64mb.error
    jcxz .get_mem_size_64mb.use_ax
    mov ax, cx
    mov bx, dx
.get_mem_size_64mb.use_ax:
    pop edx
    pop ecx
    ret
.get_mem_size_64mb.error:
    mov ax, -1
    mov bx, 0
    pop edx
    pop ecx
    ret


get_mem_size:
    int 0x12
    ret


get_ext_mem_size:
    mov ax, 0x88
    int 0x15
    jc .get_ext_mem_size.error
    test ax, ax
    je .get_ext_mem_size.error
    cmp ah, 0x86
    je .get_ext_mem_size.error
    cmp ah, 0x80
    je .get_ext_mem_size.error
    ret
.get_ext_mem_size.error:
    mov ax, -1
    ret

%endif