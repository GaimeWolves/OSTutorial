[bits 32]
VIDMEM equ 0xB8000 + 160 * 19		;Die Video Speicher Adresse in der Zeile unter dem vorangegangenen Textes.
WhiteOnBlack equ 0x0F				;Der Attributwert für Weiß-auf-Schwarzen Text.

print_pm:					;Started das Schreiben für den Protected Mode
    pusha
    mov edx, VIDMEM			;Lade die Addresse vom Video Speicher in dx.

print_loop_pm:
    mov al, [ebx]			;Lade den Buchstaben
    mov ah, WhiteOnBlack	;Lade den Attributwert

    cmp al, 0				;Falls der Buchstabe null ist, springe zu 'print_done_pm'.
    je print_done_pm

    mov [edx], ax			;Speichere den Buchstaben und das Attribut an die Video Speicher Stelle.
    add ebx, 1				;Gehe zum nächsten Buchstaben.
    add edx, 2				;Gehe zur nächsten Zelle im Video Speicher.
    jmp print_loop_pm		;Wiederhole das Ganze.

print_done_pm:
    popa
    ret
