#include "drivers/terminal.h"
#include "../cpu/isr.h"
#include "../cpu/idt.h"
#include "drivers/keyboard.h"
#include "shell.h"
#include "drivers/floppy.h"
#include "../cpu/timer.h"
#include "fs.h"

void main() 
{
	isr_install();
	terminal_initialize();
	
	asm volatile("sti");
	init_timer(100);
	init_keyboard();
	init_floppy_driver();
	init_shell();
}
