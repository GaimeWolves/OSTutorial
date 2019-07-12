#include "cpu.h"

#include "gdt.h"
#include "idt.h"

#ifdef _DEBUG
#include "../../kernel/debug/debugprint.h"
#endif

int cpu_init()
{
#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: CPU hardware initialization started.\n");
#endif

	gdt_init();
	idt_init(0x08);

#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: CPU hardware initialization finished.\n");
#endif

	return 0;
}

int cpu_exit()
{
	return 0;
}

char* cpu_id()
{
	static char id[32] = { 0 };

	__asm__(
		"mov eax, 0\n"
		"cpuid\n"
		"mov dword ptr %0, ebx\n"
		"mov dword ptr %1, edx\n"
		"mov dword ptr %2, ecx"
		: "=m" (id[0]), "=m" (id[4]), "=m" (id[8])
	);

	return id;
}