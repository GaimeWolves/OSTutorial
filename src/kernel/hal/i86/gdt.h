#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define MAX_DESCRIPTORS 3

#define GDT_DESC_LO_ACCESS		0b00000001
#define GDT_DESC_LO_READWRITE	0b00000010
#define GDT_DESC_LO_EXPANSION	0b00000100
#define GDT_DESC_LO_EXEC_CODE	0b00001000
#define GDT_DESC_LO_CODEDATA	0b00010000
#define GDT_DESC_LO_DPL_RING1	0b00100000
#define GDT_DESC_LO_DPL_RING2	0b01000000
#define GDT_DESC_LO_DPL_RING3	0b01100000
#define GDT_DESC_LO_MEMORY		0b10000000

#define GDT_DESC_HI_LIMIT_MASK	0b00001111
#define GDT_DESC_HI_OS			0b00010000
#define GDT_DESC_HI_32BIT		0b01000000
#define GDT_DESC_HI_GRANULARITY	0b10000000

typedef struct 
{
	uint16_t	limit;
	uint16_t	base_lo;
	uint8_t		base_mid;
	uint8_t		flags_lo;
	uint8_t		flags_hi;
	uint8_t		base_hi;
} __attribute__((packed)) gdt_descriptor_t;

extern void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t flags_lo, uint8_t flags_hi);

extern gdt_descriptor_t* gdt_get_descriptor(int i);

extern int gdt_init();

#endif