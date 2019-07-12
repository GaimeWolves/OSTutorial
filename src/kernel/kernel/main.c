#include "debug/debugprint.h"
#include <hal.h>
#include "exceptions.h"

void __attribute__((__cdecl__)) main()
{
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

	debug_set_color(0x70);
	debug_set_pos(0, 0);
	debug_print_string("                                                                                 ");
	debug_set_pos(33, 0);
	debug_print_string("DEBUG TERMINAL");
	enable_int();
	debug_set_pos(1, 0);
	debug_print_string(get_cpu_vendor());

	enable_int();

	for (;;)
	{
		debug_set_color(0x70);
		debug_set_pos(70, 0);
		debug_printf("%d", (int)(get_tick_count()));
	}
}