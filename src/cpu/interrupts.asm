[extern isr_handler]
[extern irq_handler]

; Standard ISR Code
isr_common_stub:
    ; 1. Speicher den CPU Status
	pusha 			; Pusht edi, esi, ebp, esp, ebx, edx, ecx und eax
	mov ax, ds 		; Speichert ds (Datensegment Descriptor) in ax
	push eax 		; speichert den Datensegment Descriptor
	mov ax, 0x10  	; Setzt alles auf das Kernel Datensegment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
    ; 2. Callt den ISR Handler aus dem C Code.
	call isr_handler
	
    ; 3. Stellt den alten Zustand wieder her.
	pop eax 
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa
	add esp, 8 	; Räumt den gepushten Fehlercode und die ISR ID wieder auf.
	sti
	iret 		; Holt sich 5 Registerwerte vom Stack: CS, EIP, EFLAGS, SS, and ESP
	
;Standard IRQ Code
irq_common_stub:
    pusha 
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call irq_handler 	; Anders als ISR
    pop ebx  			; Anders als ISR
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    popa
    add esp, 8
    sti
    iret 
	
; Da wir keine Information darüber haben welcher Interrupt gecallt wird und ob dieser
; einen Fehlercode benötigt brauchen wir für jeden Interrupt einen seperaten Codeblock

; ISR Macro für mit und ohne Fehlercode.
%macro ISR_NOERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

; IRQ Macro
%macro IRQ 2
  global irq%1
  irq%1:
    cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

; Alle ISRs.
ISR_NOERRCODE 0		; 00: Divide By Zero Exception
ISR_NOERRCODE 1		; 01: Debug Exception
ISR_NOERRCODE 2		; 02: Non Maskable Interrupt Exception
ISR_NOERRCODE 3		; 03: Int 3 Exception
ISR_NOERRCODE 4		; 04: INTO Exception
ISR_NOERRCODE 5		; 05: Out of Bounds Exception
ISR_NOERRCODE 6		; 06: Invalid Opcode Exception
ISR_NOERRCODE 7		; 07: Coprocessor Not Available Exception
ISR_ERRCODE 8		; 08: Double Fault Exception (With Error Code!)
ISR_NOERRCODE 9		; 09: Coprocessor Segment Overrun Exception
ISR_ERRCODE 10		; 10: Bad TSS Exception (With Error Code!)
ISR_ERRCODE 11		; 11: Segment Not Present Exception (With Error Code!)
ISR_ERRCODE 12		; 12: Stack Fault Exception (With Error Code!)
ISR_ERRCODE 13		; 13: General Protection Fault Exception (With Error Code!)
ISR_ERRCODE 14		; 14: Page Fault Exception (With Error Code!)
ISR_NOERRCODE 15	; 15: Reserved Exception
ISR_NOERRCODE 16	; 16: Floating Point Exception
ISR_NOERRCODE 17	; 17: Alignment Check Exception
ISR_NOERRCODE 18	; 18: Machine Check Exception
ISR_NOERRCODE 19	; 19: Reserved
ISR_NOERRCODE 20	; 20: Reserved
ISR_NOERRCODE 21	; 21: Reserved
ISR_NOERRCODE 22	; 22: Reserved
ISR_NOERRCODE 23	; 23: Reserved
ISR_NOERRCODE 24	; 24: Reserved
ISR_NOERRCODE 25	; 25: Reserved
ISR_NOERRCODE 26	; 26: Reserved
ISR_NOERRCODE 27	; 27: Reserved
ISR_NOERRCODE 28	; 28: Reserved
ISR_NOERRCODE 29	; 29: Reserved
ISR_NOERRCODE 30	; 30: Reserved
ISR_NOERRCODE 31	; 31: Reserved

; Alle IRQs
IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ   10,    42
IRQ   11,    43
IRQ   12,    44
IRQ   13,    45
IRQ   14,    46
IRQ   15,    47
