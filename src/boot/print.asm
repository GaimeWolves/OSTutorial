print:				;Startet das Schreiben eines Strings ohne automatischen Zeilenumbruch.
    pusha
    call print_loop
    popa
    ret

printline:			;Startet das Schreiben eines Strings mit automatischen Zeilenumbruch.
    pusha
    call print_loop
    call printnl
    popa
    ret

print_loop:			;Schreibt einen Buchstaben und wiederholt sich so lange bis der Buchstabe null ist.
    mov al, [bx]	;Speichert den derzeitigen Buchstaben
    cmp al, 0		;Testet ob dieser null ist und springt in dem Fall zu 'done'.
    je done
    mov ah, 0x0e	;Sagt dem BIOS, dass er einen Buchstaben schreiben muss.
    int 0x10
    add bx, 1		;Erhöht die Adresse von bx um 1.
    jmp print_loop	;Wiederholt das Ganze.

done:
    ret

printnl:			;Schreibt '\n\c' für einen Zeilenumbruch.
    mov ah, 0x0e	;Sagt dem BIOS, dass er einen Buchstaben schreiben muss.
    mov al, 0x0a 	; '\n'
    int 0x10		;Schreibt
    mov al, 0x0d 	; '\c'
    int 0x10		;Schreibt
    ret

printhex: 			;Schreibt eine Zahl als Hexadezimalzahl (16 bit = 0x0000)
    pusha
    mov cx, 0		;Initialisiert den Zähler mit 0
    call decodehex	;Startet das Schreiben
    popa
    ret

decodehex:			;Wandelt die derzeitige Zahlstelle zu Ascii um.
    cmp cx, 4		;Wenn viermal durchloffen wurde, gehe zu 'donehex'.
    je donehex

    mov ax, dx		;Speichere die derzeitige Zahl ab
    and ax, 0x000f	;Nehme nur die ersten 4 Bit
    add al, 0x30	;Addiere 0x30 dazu (0x30 = '0').
    cmp al, 0x39	;Wenn die Zahlenstelle größer als 9 ist addiere 7 dazu (0x41 = 'A'). Falls nicht, speichere den Buchstaben.
    jle puthexchar
    add al, 7

puthexchar:
    mov bx, HEXOUT + 5	;Gehe ans Ende vom HEXOUT Platzhalter-String.
    sub bx, cx			;Gehe die Anzahl des Zählers zurück.
    mov [bx], al		;Speichere den Buchstaben an dieser Stelle.
    shr dx, 4			;Verschiebe alle Bits vier nach rechts (0xABCD -> 0xDABC).
    add cx, 1			;Erhöhe den Zähler.
    jmp decodehex		;Wiederhole das Ganze.

donehex:
    mov bx, HEXOUT		;Lade den Platzhalter-String
    call print			;Schreibe diesen.
    ret

HEXOUT: db '0x0000 ', 0
