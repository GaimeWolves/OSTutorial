#include <hal.h>
#include <stdarg.h>
#include "debug/debugprint.h"

void __attribute__((__cdecl__)) panic(const char* fmt, ...) 
{
	disable_int();

	va_list		args;

	va_start (args, fmt);

    //TODO

	va_end (args);

	char* disclamer = "\nOwOS v0.1 DEBUG BUILD\n\nOwOS has encountered a problem. The system will be halted to\nprevent damage to the computer.\n\n";

	debug_clear_screen(0x1f);
	debug_set_pos(0,0);
	debug_set_color(0x1f);
	debug_print_string(disclamer);

	debug_printf("*** HALTED - ERR CODE: %s ***", fmt);

	for (;;);
}