[org 0x7C00]					;Setze den allgemeinen Offset auf 0x7C00 (Anfang des Bootsektors).
KERNEL_OFFSET equ 0x9000		;Speichere wo der Kernel geladen wird (Selbe Adresse wie im Linker Befehl der Makefile).

initfat12:  ;All in little-endian format
	jmp short 0x3C
	nop
	dd 0x293A637E
	dd 0x2D494843
	dw 0x0004		;Bytes per sector (1024)
	db 0x01			;Sectors per cluster (1)
	dw 0x4800		;Reserved sectors (72)
	db 0x02			;no. of FATs (2)
	dw 0x0002		;dir entries (512) -> 32 * 16 sectors
	dw 0x8016		;no. of sectors (5760)
	db 0xF0			;media type descriptor (1.44Mb or 2.88Mb Floppy Disk)
	dw 0x0900		;no. of sectors per FAT (9)
	dw 0x2400		;no. of sectors per track (36)
	dw 0x0200		;no. of heads / sides of media (2)
	dd 0x00000000	;no. of hidden sectors (0)
	dd 0x00000000	;Large sector count (0) not used because sector count <= 65535

	;Extended Boot record
	db 0x00			;Drive number (0x00 = Floppy   0x80 = Hard disk)
	db 0x00			;Reserved
	db 0x28			;Signature (must be 0x28 or 0x29)
	dd 0x00000000	;VolumeID (can be ignored)
	db "yppolF 88.2";Volume Name (11 Bytes)
	db "   21TAF"	;System Name (8 Bytes)


boot:
	cli
	mov [BOOTDRIVE], dl			;Spechere die ID der Floppy Disk in BOOTDRIVE.
	mov bp, 0x8000				;Setze den Stack auf 0x9000.
	mov sp, bp

	mov bx, MSG_WELCOME			;Schreibe die Wilkommensnachricht.
	call printline

	call load_kernel			;Lade den Kernel.
	call init_pm				;Starte den Kernel.

	jmp $						;Falls der Kernel doch returned (was er nicht sollte) hänge für immer.

;Inkludiere alle Hilfsroutinen
%include "print.asm"
%include "diskload.asm"
%include "gdt.asm"
%include "print32.asm"
%include "start32.asm"

[bits 16]
load_kernel:					;Lade den Kernel.
	mov bx, MSG_LOAD_KERNEL		;Schreibe das der Kernel geladen wird.
    call printline

	xor cx, cx
	mov bx, KERNEL_OFFSET		;Lade 35 Sektoren der Floppy Disk beim Kernel Offset.
	mov ch, 0x00
	mov cl, 0x02
    mov dh, 35
    mov dl, [BOOTDRIVE]
    call diskload

	xor cx, cx
	mov bx, KERNEL_OFFSET + 35 * 512		;Lade 36 Sektoren der Floppy Disk beim Kernel Offset.
	mov ch, 0x01
	mov cl, 0x01
    mov dh, 20
    mov dl, [BOOTDRIVE]
    call diskload

	;mov dx, 0x1000
	;mov es, dx
	;xor cx, cx
	;mov bx, 0x1E00		;Lade 32 Sektoren der Floppy Disk beim Kernel Offset.
	;mov ch, 0x02
	;mov cl, 0x00
    ;mov dh, 36
    ;mov dl, [BOOTDRIVE]
    ;call diskload

    ret

[bits 32]
BEGIN_PM:						;Starte den Kernel.
    mov ebx, MSG_PM				;Schreibe dass der Computer den Protected Mode erreicht hat.
    call print_pm
    call KERNEL_OFFSET			;Starte die externe Funktion main() des Kernels beim KERNEL_OFFSET durch kernel_entry.asm welches am Anfang steht.
    jmp $						;Hänge für immer falls der Kernel returned

BOOTDRIVE db 0					;Variable für die ID der Floppy Disk
MSG_WELCOME db "Booting OwOS in 16-Bit Real Mode...", 0			;Nachrichten zum ausgeben.
MSG_PM db "Booted 32-bit Protected Mode!", 0
MSG_LOAD_KERNEL db "Loading Kernel...", 0

times 510 -( $ - $$ ) db 0		;Fülle den Rest des Bootsektors mit 0 und schreibe am Ende die Kennzeichnungsbytes 0xAA55
dw 0xAA55
