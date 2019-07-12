#include "idt.h"

#include <string.h>
#include <hal.h>

#ifdef _DEBUG
#include "../../kernel/debug/debugprint.h"
#endif

typedef struct
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idtr_t;

static idt_descriptor_t idt[MAX_INTERRUPTS];
static idtr_t idtr;

static void idt_install()
{
	__asm__ __volatile__("lidt [%0]" : : "r" (&idtr));
}

static void __attribute__((__cdecl__)) default_handler()
{
#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: Unhandled Exception! Halting system...");
#endif

	__asm__("cli\nhlt");
}

idt_descriptor_t* get_ir(uint32_t i)
{
	if (i > MAX_INTERRUPTS)
		return 0;

	return &idt[i];
}

int install_ir(uint32_t i, uint8_t flags, uint16_t selector, IRQ_HANDLER irq)
{
	if (i > MAX_INTERRUPTS || !irq) return -1;

	idt[i].base_lo = (uint16_t)((uint32_t)&(*irq) & 0xFFFF);
	idt[i].selector = selector;
	idt[i].reserved = 0;
	idt[i].flags = flags;
	idt[i].base_hi = (uint16_t)(((uint32_t) &(*irq) >> 16) & 0xFFFF);

	return 0;
}

int idt_init(uint16_t selector)
{
	idtr.limit = sizeof(idt_descriptor_t) * MAX_INTERRUPTS - 1;
	idtr.base = (uint32_t) &idt[0];

	memset((void*)& idt[0], 0, sizeof(idt_descriptor_t) * MAX_INTERRUPTS - 1);

	for (uint32_t i = 0; i < MAX_INTERRUPTS; i++)
		install_ir(i, IDT_DESC_PRESENT | IDT_DESC_BIT32, selector, (IRQ_HANDLER)default_handler);

	idt_install();

#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: Loaded IDT descriptors.\n");
#endif

	return 0;
}