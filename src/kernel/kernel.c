#include "drivers/terminal.h"
#include "../cpu/isr.h"
#include "../cpu/idt.h"
#include "drivers/keyboard.h"
#include "shell.h"

void main() 
{
	isr_install();
	terminal_initialize();
	
	asm volatile("sti");
	init_keyboard();
	init_shell();
}
