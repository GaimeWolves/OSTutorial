%ifndef STDIO16
%define STDIO16

[bits 16]

;Prints the string (null-terminated) in DS:SI
print16:
		pusha

	.print16_loop:
		lodsb
		or al, al
		jz .print16_done
		mov ah, 0Eh
		int 10h
		jmp .print16_loop

	.print16_done:
		popa
		ret

%endif ;STDIO16