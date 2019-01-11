;Lädt eine bestimmte Anzahl Sektoren aus der angegebenen Floppy Disk
diskload:
    pusha
    push dx				;Speichere die Anzahl zu lesende Sektoren auf dem Stack

    mov ah, 0x02		;Interrupt 0x13 Funktion: 0x02 = Lese aus der Floppy Disk.
    mov al, dh			;Die Anzahl zu lesende Sektoren.
    mov cl, 0x02		;Anfangssektor (0x01 ist der Bootsektor und 0x02 der erste 'verfügbare').
    mov ch, 0x00		;Welcher Zylinder gelesen wird.
    mov dh, 0x00		;Welcher Kopf gelesen wird (HDDs haben 4 Platten also auch 4 Köpfe).

    int 0x13			;Starte die BIOS-Routine.

    jc diskerror		;Falls es einen Fehler gab (Carry Bit = 1) springe zu Fehlerausgabe.

    pop dx				;Lade die Anzahl zu lesende Sektoren vom Stack.
    cmp dh, al			;Falls diese nicht der Anzahl tatsächlich gelesener Sektoren entspricht. Springe zur Fehlerausgabe.
    jne diskerror
    popa
    ret

diskerror:
    mov bx, DISK_ERROR_MSG
    call printnl
    call print
    mov dh, ah
    call printhex
    call printnl
    jmp $

DISK_ERROR_MSG: db 'DISK READ ERROR: ', 0
