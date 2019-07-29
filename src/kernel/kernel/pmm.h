#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <size_t.h>
#include <stdbool.h>

typedef struct 
{
	uint32_t start_lo;
	uint32_t start_hi;
	uint32_t size_lo;
	uint32_t size_hi;
	uint32_t type;
	uint32_t acpi_3_0;
} __attribute__((packed)) memory_region_t;

extern void pmm_init(size_t, uintptr_t);

extern void pmm_init_region(uintptr_t, size_t);
extern void pmm_deinit_region(uintptr_t, size_t);

extern void* pmm_alloc();
extern void pmm_free(void*);

extern void* pmm_alloc_continuous(size_t);
extern void pmm_free_continuous(void*, size_t);

extern size_t pmm_get_memory_size();
extern uint32_t pmm_get_used_block_count();
extern uint32_t pmm_get_free_block_count();
extern uint32_t pmm_get_block_count();
extern uint32_t pmm_get_block_size();

extern void pmm_set_paging(bool);
extern bool pmm_is_paging();

extern void pmm_load_PDBR(uintptr_t);
extern uintptr_t pmm_get_PDBR();

#endif