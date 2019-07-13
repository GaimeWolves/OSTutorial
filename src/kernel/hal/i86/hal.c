#include <hal.h>

#include "cpu.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"

#ifdef _DEBUG
#include "../../kernel/debug/debugprint.h"
#endif

int __attribute__((__cdecl__)) hal_init()
{
#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: HAL initialization started.\n");
#endif

	cpu_init();
	pic_init(0x20, 0x28);
	pit_init();
	pit_start_counter(10, PIT_OCW_COUNTER_0, PIT_OCW_MODE_SQUAREWAVEGEN);

#ifdef _DEBUG
	debug_print_dstring("[i86 HAL]: HAL initialization finished.\n");
#endif
	return 0;
}

int __attribute__((__cdecl__)) hal_exit()
{
	cpu_exit();
	return 0;
}

inline void __attribute__((__cdecl__)) INT(uint8_t n)
{
	__asm__ __volatile__(
		"movb point+1, %0\n"
		"point:\n"
		"int 0\n" 
		: : "r"(n)
	);
}

void __attribute__((__cdecl__)) int_done(unsigned int_no)
{
	if (int_no > 15) return;

	pic_send_command(PIC_OCW2_MASK_EOI, int_no >= 8);
}

inline void __attribute__((__cdecl__)) sound(unsigned freq)
{
	outportb(0x61, 3 | UINT8_C(freq << 2));
}

inline uint8_t __attribute__((__cdecl__)) inportb(uint16_t port)
{
	uint8_t result = 0;
	__asm__ __volatile__("in al, dx" : "=a" (result) : "d" (port));
	return result;
}

inline void __attribute__((__cdecl__)) outportb(uint16_t port, uint8_t value)
{
	__asm__ __volatile__("out dx, al" : : "d"(port), "a"(value));
}

inline void __attribute__((__cdecl__)) enable_int()
{
	__asm__("sti");
}

inline void __attribute__((__cdecl__)) disable_int()
{
	__asm__("cli");
}

void __attribute__((__cdecl__)) set_int(int int_no, void (__attribute__((__cdecl__)) *irq) (void))
{
	install_ir(int_no, IDT_DESC_PRESENT | IDT_DESC_BIT32, 0x08, irq);
}

void	(__attribute__((__cdecl__)) *getvect(int int_no)) ( )
{
	idt_descriptor_t* desc = get_ir(int_no);

	if (!desc) return 0;

	uint32_t addr = desc->base_lo | (desc->base_hi << 16);
	return (IRQ_HANDLER)addr;
}

const char* get_cpu_vendor()
{
	return cpu_id();
}

inline uint64_t get_tick_count()
{
	return pit_get_tick_count();
}