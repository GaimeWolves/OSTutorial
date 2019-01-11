#include "timer.h"

#include "../kernel/drivers/terminal.h"
#include "../kernel/drivers/ports.h"
#include "../kernel/util/string.h"
#include "isr.h"

unsigned long tick = 0;

static void timer_callback(registers_t regs)
{
	tick++;
}

void init_timer(unsigned long freq) 
{
    register_interrupt_handler(IRQ0, timer_callback);

	unsigned long divisor = 1193180 / freq;
    unsigned char low  = (unsigned char)(divisor & 0xFF);
    unsigned char high = (unsigned char)((divisor >> 8) & 0xFF);

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}
