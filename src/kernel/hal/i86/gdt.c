#include "gdt.h"
#include <string.h>

#ifdef _DEBUG
#include "../../kernel/debug/debugprint.h"
#endif

typedef struct
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) gdtr_t;

static gdt_descriptor_t gdt[MAX_DESCRIPTORS];
static gdtr_t gdtr;

static void gdt_install() 
{
	__asm__("lgdt [%0]" : : "r" (&gdtr));
}

void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t flags_lo, uint8_t flags_hi)
{
	if (i > MAX_DESCRIPTORS) return;

	memset((void*)& gdt[i], 0, sizeof(gdt_descriptor_t));

	gdt[i].base_lo = (uint8_t)(base & 0xFFFF);
	gdt[i].base_mid = (uint8_t)((base >> 16) & 0xFF);
	gdt[i].base_hi = (uint8_t)((base >> 24) & 0xFF);
	gdt[i].limit = (uint8_t)(limit & 0xFFFF);
	gdt[i].flags_lo = flags_lo;
	gdt[i].flags_hi = (uint8_t)((limit >> 16) & 0x0F) | flags_hi;
}

gdt_descriptor_t* gdt_get_descriptor(int i)
{
	if (i > MAX_DESCRIPTORS)
		return 0;
	return &gdt[i];
}

int gdt_init()
{
	gdtr.limit = (sizeof(gdt_descriptor_t) * MAX_DESCRIPTORS) - 1;
	gdtr.base = (uint32_t)&gdt[0];

	gdt_set_descriptor(0, 0, 0, 0, 0);
	gdt_set_descriptor(1, 0, 0xFFFFFFFF,
		GDT_DESC_LO_READWRITE | GDT_DESC_LO_CODEDATA | GDT_DESC_LO_EXEC_CODE | GDT_DESC_LO_MEMORY,
		GDT_DESC_HI_32BIT | GDT_DESC_HI_GRANULARITY | GDT_DESC_HI_LIMIT_MASK);
	gdt_set_descriptor(2, 0, 0xFFFFFFFF,
		GDT_DESC_LO_READWRITE | GDT_DESC_LO_CODEDATA | GDT_DESC_LO_MEMORY,
		GDT_DESC_HI_32BIT | GDT_DESC_HI_GRANULARITY | GDT_DESC_HI_LIMIT_MASK);

	gdt_install();

#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: Loaded GDT descriptors.\n");
#endif

	return 0;
}
