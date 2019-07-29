#include "pmm.h"

#include <string.h>

#define PMM_BLOCKS_PER_BYTE 8
#define PMM_BLOCK_SIZE 4096
#define PMM_BLOCK_ALIGN PMM_BLOCK_SIZE

static uint32_t pmm_memory_size = 0;
static uint32_t pmm_used_blocks = 0;
static uint32_t pmm_max_blocks = 0;
static uint32_t *pmm_memory_map = 0;

static inline void pmm_set(int bit);
static inline void pmm_unset(int bit);
static inline bool pmm_test(int bit);
static int pmm_first_free();
static int pmm_first_free_continuous(size_t size);

static inline void pmm_set(int bit)
{
	pmm_memory_map[bit / 32] |= (1 << (bit % 32));
}

static inline void pmm_unset(int bit)
{
	pmm_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

static inline bool pmm_test(int bit)
{
	return pmm_memory_map[bit / 32] & (1 << (bit % 32));
}

static int pmm_first_free()
{
	for (uint32_t i = 0; i < pmm_get_block_count() / 32; i++)
	{
		if (pmm_memory_map[i] != 0xffffffff)
		{
			for (int j = 0; j < 32; j++)
			{
				int bit = 1 << j;
				if (!(pmm_memory_map[i] & bit))
					return i * 32 + j;
			}
		}
	}

	return -1;
}

static int pmm_first_free_continuous(size_t size)
{
	if (size == 0)
		return -1;

	if (size == 1)
		return pmm_first_free();

	for (uint32_t i = 0; i < pmm_get_block_count() / 32; i++)
		if (pmm_memory_map[i] != 0xffffffff)
			for (int j = 0; j < 32; j++)
			{
				int bit = 1 << j;
				if (!(pmm_memory_map[i] & bit))
				{
					int startingBit = i * 32;
					startingBit += bit;
					
					uint32_t free = 0;
					for (uint32_t count = 0; count <= size; count++)
					{
						if (!pmm_test(startingBit + count))
							free++;

						if (free == size)
							return i * 32 + j;
					}
				}
			}

	return -1;
}

void pmm_init(size_t memSize, uintptr_t bitmap)
{

	pmm_memory_size = memSize;
	pmm_memory_map = (uint32_t*)bitmap;
	pmm_max_blocks = (pmm_get_memory_size() * 1024) / PMM_BLOCK_SIZE;
	pmm_used_blocks = pmm_max_blocks;

	memset(pmm_memory_map, 0xf, pmm_get_block_count() / PMM_BLOCKS_PER_BYTE);
}

void pmm_init_region(uintptr_t base, size_t size)
{
	int align = base / PMM_BLOCK_SIZE;
	int blocks = size / PMM_BLOCK_SIZE;

	for (; blocks >= 0; blocks--)
	{
		pmm_unset(align++);
		pmm_used_blocks--;
	}

	pmm_set(0);
}

void pmm_deinit_region(uintptr_t base, size_t size)
{
	int align = base / PMM_BLOCK_SIZE;
	int blocks = size / PMM_BLOCK_SIZE;

	for (; blocks >= 0; blocks--)
	{
		pmm_set(align++);
		pmm_used_blocks++;
	}
}

void *pmm_alloc()
{
	if (pmm_get_free_block_count() <= 0)
		return 0;

	int frame = pmm_first_free();

	if (frame == -1)
		return 0;

	pmm_set(frame);

	uintptr_t addr = frame * PMM_BLOCK_SIZE;
	pmm_used_blocks++;

	return (void *)addr;
}

void pmm_free(void *p)
{
	uintptr_t addr = (uintptr_t)p;
	int frame = addr / PMM_BLOCK_SIZE;

	pmm_unset(frame);

	pmm_used_blocks--;
}

void* pmm_alloc_continuous(size_t size)
{
	if (pmm_get_free_block_count() <= size)
		return 0;

	int frame = pmm_first_free_continuous(size);

	if (frame == -1)
		return 0;

	for (uint32_t i = 0; i < size; i++)
		pmm_set(frame + i);

	uintptr_t addr = frame * PMM_BLOCK_SIZE;
	pmm_used_blocks += size;

	return (void *)addr;
}

void pmm_free_continuous(void *p, size_t size)
{
	uintptr_t addr = (uintptr_t)p;
	int frame = addr / PMM_BLOCK_SIZE;

	for (uint32_t i = 0; i < size; i++)
		pmm_unset(frame + i);

	pmm_used_blocks -= size;
}

size_t pmm_get_memory_size()
{
	return pmm_memory_size;
}

uint32_t pmm_get_block_count()
{
	return pmm_max_blocks;
}

uint32_t pmm_get_used_block_count()
{
	return pmm_used_blocks;
}

uint32_t pmm_get_free_block_count()
{
	return pmm_max_blocks - pmm_used_blocks;
}

uint32_t pmm_get_block_size()
{
	return PMM_BLOCK_SIZE;
}

void pmm_set_paging(bool b)
{

#ifdef I86
	__asm__(
		"pusha\n"
		"mov eax, cr0\n"
		"cmp %0, 1\n"
		"je	enable\n"
		"jmp disable\n"
	"enable:\n"
		"or eax, 0x80000000\n"
		"mov cr0, eax\n"
		"jmp done\n"
	"disable:\n"
		"and eax, 0x7FFFFFFF\n"
		"mov cr0, eax\n"
	"done:\n"
		"popa\n"
	:
	: "r" (b)
	);
#endif
}

bool pmm_is_paging()
{
	uint32_t res = 0;

#ifdef I86
	__asm__(
		"mov eax, cr0\n"
		"mov %0, eax"
		: "=r" (res)
	);
#endif

	return (res & 0x80000000) ? false : true;
}

void pmm_load_PDBR(uintptr_t addr)
{
#ifdef I86
	__asm__(
		"mov eax, [%0]\n"
		"mov cr3, eax"
		:
		: "m" (addr)
	);
#endif
}

uintptr_t pmm_get_PDBR()
{
	uintptr_t pdbr = 0;
#ifdef I86
	__asm__(
		"mov %0, cr3"
		: "=r" (pdbr)
	);
#endif
	return pdbr;
}