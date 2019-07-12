#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define MAX_INTERRUPTS 256

#define IDT_DESC_BIT16		0x06	//00000110
#define IDT_DESC_BIT32		0x0E	//00001110
#define IDT_DESC_RING1		0x40	//01000000
#define IDT_DESC_RING2		0x20	//00100000
#define IDT_DESC_RING3		0x60	//01100000
#define IDT_DESC_PRESENT	0x80	//10000000

typedef void(__attribute__((__cdecl__)) *IRQ_HANDLER)(void);

typedef struct
{
	uint16_t base_lo;
	uint16_t selector;
	uint8_t reserved;
	uint8_t flags;
	uint16_t base_hi;
} __attribute__((packed)) idt_descriptor_t;

extern idt_descriptor_t* get_ir(uint32_t i);
extern int install_ir(uint32_t i, uint8_t flags, uint16_t selector, IRQ_HANDLER irq);
extern int idt_init(uint16_t selector); 

#endif