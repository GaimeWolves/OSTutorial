[org 0x7C00]					;Setze den allgemeinen Offset auf 0x7C00 (Anfang des Bootsektors.
KERNEL_OFFSET equ 0x1000		;Speichere wo der Kernel geladen wird (Selbe Adresse wie im Linker Befehl der Makefile.

	mov [BOOTDRIVE], dl			;Spechere die ID der Floppy Disk in BOOTDRIVE.
	mov bp, 0x9000				;Setze den Stack auf 0x9000.
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

	mov bx, KERNEL_OFFSET		;Lade 31 Sektoren der Floppy Disk beim Kernel Offset.
    mov dh, 31
    mov dl, [BOOTDRIVE]
    call diskload
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
