%ifndef GDT
%define GDT

[bits 16]

load_gdt:
		cli							; clear interrupts
		pusha						; save registers
		lgdt 	[gdt_descriptor]	; load GDT into GDTR
		sti							; enable interrupts
		popa						; restore registers
		ret							; All done!

;Dieser Bereich bereitet den Global Descriptor Table (GDT) vor f�r den Protected Mode.
gdt_null:			;Erstes 'Start Segment' besteht nur aus 0.
    dd 0x0
    dd 0x0

gdt_code:                       ;CODE Segment
        dw 0xffff               ;Limit (Maxiumum)
        dw 0x0                  ;Base (No offset) 0-15
        db 0x0                  ;Base (No offset) 16-23
        ;; -- Other Flags & Type Flags --
        ;; Present: 1
        ;; Privilege: 00 - ring 0
        ;; - Type Flags -
        ;; Descriptor Type: 1 - Code oder Data segment
        ;; Executable: 1 - Ausf�hrbar sein
        ;; Conforming: 0 - Segmente mit niedrigerem privilege d�rfen dieses Segment nicht ausf�hren
        ;; Readable: 1 - Les- und Schreibbar
        ;; Accessed: 0 - Wird von CPU w�hrend Zugriff auf 1 gesetzt
        ;; = 10011010b
        db 10011010b            ;Andere Flags & Type flags

        ;; -- Limit Flags --
        ;; Granularity: 1 - Bitshift um 3, 4GB verf�gbar
        ;; Size: 1 - 32-bit anstatt 16-bit
        db 11001111b            ;Limit flags
        db 0x0                  ;Segment base

gdt_data:
        dw 0xffff
        dw 0x0
        db 0x0
        ;; -- Other Flags & Type Flags --
        ;; Siehe oben, au�er:
        ;; Code: 0 - Nur Daten
        ;; Bedeutungs�nderung, da Code-Flag 0
        ;; (Conforming) Expand down: 0
        ;; (Readable) Writable: 1
        ;; = 10010010b
        db 10010010b
        ;; Siehe oben
        db 11001111b
        db 0x0
gdt_end:

gdt_descriptor:						;Der GDT Descriptor
    dw gdt_end - gdt_null - 1		;Die Gr��e des GDTs
    dd gdt_null					;Die Adresse des GDTs

CODESEG equ gdt_code - gdt_null		;Offset des Code Segments
DATASEG equ gdt_data - gdt_null		;Offset des Daten Segments

%endif ;GDT