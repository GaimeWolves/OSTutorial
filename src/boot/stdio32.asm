%ifndef STDIO32
%define STDIO32

[bits 32]

;##### <VGA printing> #####;
 
%define VGAMEM 0xB8000
%define COLUMS 80
%define LINES 25
%define TEXT_ATTRIB 00001111b
%define ERR_ATTRIB 00001100b

_CursorX db 0
_CursorY db 0

clearscreen:
		pusha
		cld
		mov	edi, VGAMEM
		mov	cx, 2000
		mov	ah, TEXT_ATTRIB
		mov	al, ' '	
		rep	stosw
		mov	byte [_CursorX], 0
		mov	byte [_CursorY], 0
		popa
		ret

;BX -> Char to print with attribute
putchar:
		pusha

		cmp bl, 0x0A
		je .putchar_row

		xor eax, eax
		mov al, byte [_CursorX]
		mov ecx, 2
		mul ecx
		push eax

		xor eax, eax
		mov al, byte [_CursorY]
		mov ecx, COLUMS * 2
		mul ecx
		
		pop ecx
		add eax, ecx
		add eax, VGAMEM

		mov word [eax], bx
		inc byte [_CursorX]
		cmp byte [_CursorX], COLUMS
		je .putchar_row
		jmp .putchar_done

	.putchar_row:
		mov byte [_CursorX], 0
		inc byte [_CursorY]

	.putchar_done:
		popa
		ret

;EBX -> String address
;DH -> Attribute
putstring:
		pusha
		push ebx
		pop edi
		xor ebx, ebx
		mov bh, dh

	.putstring_loop:
		mov bl, byte [edi]
		cmp bl, 0
		je .putstring_done

		call putchar
		
		inc edi
		jmp .putstring_loop

	.putstring_done:
		mov bh, byte [_CursorY]
		mov bl, byte [_CursorX]

		call setcursor

		popa
		ret

;BH -> y-pos
;BL -> x-pos
setcursor:
		pusha

		xor eax, eax
		mov ecx, COLUMS
		mov al, bh
		mul ecx
		add al, bl
		mov ebx, eax

		mov	al, 0x0f		;Cursor location low byte index
		mov	dx, 0x03D4		;Write it to the CRT index register
		out	dx, al
 
		mov	al, bl
		mov	dx, 0x03D5		;Write it to the data register
		out	dx, al			;low byte of position
 
		xor	eax, eax
		mov	al, 0x0e		;Cursor location high byte index
		mov	dx, 0x03D4		;Write to the CRT index register
		out	dx, al
 
		mov	al, bh
		mov	dx, 0x03D5		;Write it to the data register
		out	dx, al			;High byte of position

		popa
		ret

;##### </VGA printing> #####;

%endif ;STDIO32