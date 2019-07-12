#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef _DEBUG
#include "../../kernel/debug/debugprint.h"
#endif

#define	PIT_OCW_MASK_BINCOUNT	0x01
#define	PIT_OCW_MASK_MODE		0x0E	
#define	PIT_OCW_MASK_RL			0x30
#define	PIT_OCW_MASK_COUNTER	0xC0

#define	PIT_OCW_BINCOUNT_BINARY	0x00
#define	PIT_OCW_BINCOUNT_BCD 	0x01

#define	PIT_OCW_MODE_TERMINALCOUNT 	0x00
#define	PIT_OCW_MODE_ONESHOT 		0x02
#define	PIT_OCW_MODE_RATEGEN 		0x04	
#define	PIT_OCW_MODE_SQUAREWAVEGEN 	0x06	
#define	PIT_OCW_MODE_SOFTWARETRIG 	0x08	
#define	PIT_OCW_MODE_HARDWARETRIG 	0x0A	

#define	PIT_OCW_RL_LATCH 	0x00
#define	PIT_OCW_RL_LSBONLY 	0x10
#define	PIT_OCW_RL_MSBONLY 	0x20
#define	PIT_OCW_RL_DATA	 	0x30

#define	PIT_OCW_COUNTER_0 	0x00
#define	PIT_OCW_COUNTER_1 	0x40
#define	PIT_OCW_COUNTER_2 	0x80

extern void pit_send_command(uint8_t cmd);

extern void pit_send_data(uint8_t data, uint8_t counter);
extern uint8_t pit_read_data(uint8_t counter);

extern uint64_t pit_set_tick_count(uint64_t i);
extern uint64_t pit_get_tick_count();

extern void pit_start_counter(uint32_t freq, uint8_t counter, uint8_t mode);

extern void pit_init();
extern bool pit_is_init();

#endif