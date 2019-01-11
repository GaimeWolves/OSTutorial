[bits 16]
init_pm:
    cli                     ;Sperre alle Interrupts
    lgdt [gdt_descriptor]   ;Lade den GDT Descriptor

    mov eax, cr0			;Setze die 32-bit Protected Mode Flagge
    or eax, 0x1
    mov cr0, eax            

    jmp CODESEG:start_pm	;Mache einen 'weiten' Sprung zu 'start_pm' um den Chache der CPU zu leeren.

[bits 32]
start_pm:
    mov ax, DATASEG        	;Setze alle Segment Register zum Daten Segment.
    mov ds, ax              
    mov ss, ax 
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000        ;Erneure die Stack Position auf 0x90000.
    mov esp, ebp            

    call BEGIN_PM			;Starte den Protected Mode.
