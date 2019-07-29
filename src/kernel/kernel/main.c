#include <hal.h>
#include <multiboot.h>
#include <stdio.h>
#include "exceptions.h"
#include "pmm.h"

#ifdef _DEBUG
#include "debug/debugprint.h"

char* mem_type_str[] = {
	"Available",
	"Reserved",
	"ACPI Reclaim",
	"ACPI NVS Memory"
};
#endif

void __attribute__((__cdecl__)) main(multiboot_info_t* boot_info)
{
	uint32_t kernel_size = 0;
#ifdef _DEBUG
	__asm__("mov %0, edx" : "=rm" (kernel_size));
#endif

	debug_clear_screen(0x0F);

	hal_init();

	set_int(0, divide_by_zero_fault);
	set_int(1, single_step_trap);
	set_int(2, nmi_trap);
	set_int(3, breakpoint_trap);
	set_int(4, overflow_trap);
	set_int(5, bounds_check_fault);
	set_int(6, invalid_opcode_fault);
	set_int(7, no_device_fault);
	set_int(8, double_fault_abort);
	set_int(10, invalid_tss_fault);
	set_int(11, no_segment_fault);
	set_int(12, stack_fault);
	set_int(13, general_protection_fault);
	set_int(14, page_fault);
	set_int(16, fpu_fault);
	set_int(17, alignment_check_fault);
	set_int(18, machine_check_abort);
	set_int(19, simd_fpu_fault);
	set_int(20, reserved);
	set_int(21, reserved);
	set_int(22, reserved);
	set_int(23, reserved);
	set_int(24, reserved);
	set_int(25, reserved);
	set_int(26, reserved);
	set_int(27, reserved);
	set_int(28, reserved);
	set_int(29, reserved);
	set_int(30, reserved);
	set_int(31, reserved);
	set_int(9, reserved);

	enable_int();

	uint32_t mem_size = 1024 + boot_info->memory_lo + boot_info->memory_hi * 64;

	pmm_init(mem_size, 0x100000 + kernel_size * 512);

#ifdef _DEBUG
	debug_printf_dstring("[PMM]: System has %iKB physical memory",mem_size);
	debug_print_dstring("[PMM] Physical Memory Map:");
#endif

	memory_region_t* region = (memory_region_t*)(boot_info->mmap_addr);

	for (uint32_t i = 0; i < boot_info->mmap_len; i++) 
	{
		if (region[i].type > 4)
			region[i].type = 1;

#ifdef _DEBUG
		debug_printf_dstring("[PMM]: %i: base: 0x%x%x len: 0x%x%x (%s)%i", i, 
			region[i].start_hi, region[i].start_lo,
			region[i].size_hi,region[i].size_lo,
			mem_type_str[region[i].type-1], region[i].type);
#endif

		if (region[i].type==1)
			pmm_init_region(region[i].start_lo, region[i].size_lo);
	}

	pmm_deinit_region(0x100000, kernel_size * 512 + 4096);

#ifdef _DEBUG
	debug_printf_dstring("[PMM]: initialized: %i blocks; used/reserved: %i free: %i ",
		pmm_get_block_count(), pmm_get_used_block_count(), pmm_get_free_block_count ());
#endif

	debug_set_color(0x70);
	debug_set_pos(0, 0);
	debug_print_string("                                                                                 ");
	debug_set_pos(33, 0);
	debug_print_string("DEBUG TERMINAL");
	enable_int();
	debug_set_pos(1, 0);
	debug_print_string(get_cpu_vendor());

	for (;;)
	{
		debug_set_color(0x70);
		debug_set_pos(70, 0);
		debug_printf("%d", (int)(get_tick_count()));
	}
}