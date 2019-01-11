#ifndef IDT
#define IDT

#define KERNEL_CS 0x08 //Segment Selektor

typedef struct 
{
    unsigned short low_offset; 		/* Untere 16 Bits der Funktionsadresse */
    unsigned short sel; 			/* Kernel segment Selektor */
    unsigned char always0;
    
    /* First byte
     * Bit 7: 		Ist das Interrupt present?
     * Bits 6-5: 	Privileg Level des Callers (0 = kernel - 3 = user)
     * Bit 4: 		Muss immer 0 sein für Interrupt Gates.
     * Bits 3-0: 	1100 = 14: Ein 32 bit Interrupt Gate. */
    unsigned char flags; 
    unsigned short high_offset; 	/* Obere 16 Bits der Funktionsadresse */
} __attribute__((packed)) idt_gate_t ;

//Gleich wie beim GDT Descriptor
typedef struct 
{
    unsigned short limit;
    unsigned long base;
} __attribute__((packed)) idt_descriptor_t;

#define IDT_ENTRIES 256 //Maximal 256 Interrupts
idt_gate_t idt[IDT_ENTRIES];
idt_descriptor_t idt_des;

void set_idt_gate(int index, unsigned long handler);
void set_idt();

#endif
