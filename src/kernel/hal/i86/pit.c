#include "pit.h"

#include "idt.h"
#include "pic.h"

#ifdef _DEBUG
#include "../../kernel/debug/debugprint.h"
#endif

#include <hal.h>

#define	PIT_REG_COUNTER0	0x40
#define	PIT_REG_COUNTER1	0x41
#define	PIT_REG_COUNTER2	0x42
#define	PIT_REG_COMMAND		0x43

static volatile uint64_t ticks = 0;
static bool is_init = false;

void __attribute__((__cdecl__)) pit_irq()
{
	int_init

	ticks++;

	int_done(0);
	int_exit
}

void pit_send_command(uint8_t cmd)
{
	outportb(PIT_REG_COMMAND, cmd);
}

void pit_send_data(uint8_t data, uint8_t counter)
{
	uint16_t port = (counter == PIT_OCW_COUNTER_0) ? PIT_REG_COUNTER0 : ((counter == PIT_OCW_COUNTER_1) ? PIT_REG_COUNTER1 : PIT_REG_COUNTER2);

	outportb(port, data);
}

uint8_t pit_read_data(uint8_t counter)
{
	uint16_t port = (counter == PIT_OCW_COUNTER_0) ? PIT_REG_COUNTER0 : ((counter == PIT_OCW_COUNTER_1) ? PIT_REG_COUNTER1 : PIT_REG_COUNTER2);

	return inportb(port);
}

uint64_t pit_set_tick_count(uint64_t i)
{
	uint64_t ret = ticks;
	ticks = i;
	return ret;
}

inline uint64_t pit_get_tick_count()
{
	return ticks;
}

void pit_start_counter(uint32_t freq, uint8_t counter, uint8_t mode)
{
	if (freq == 0) return;

	uint16_t divisor = (uint16_t)(1193181 / freq);

	uint8_t ocw = 0;
	ocw = (ocw & ~PIT_OCW_MASK_MODE) | mode;
	ocw = (ocw & ~PIT_OCW_MASK_RL) | PIT_OCW_RL_DATA;
	ocw = (ocw & ~PIT_OCW_MASK_COUNTER) | counter;
	pit_send_command(ocw);

	//! set frequency rate
	pit_send_data(divisor & 0xff, 0);
	pit_send_data((divisor >> 8) & 0xff, 0);

	//! reset tick count
	ticks = 0;
}

void pit_init()
{
	set_int(0x20, pit_irq);
	is_init = true;
}

inline bool pit_is_init()
{
	return is_init;
}