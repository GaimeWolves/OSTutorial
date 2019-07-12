#ifndef HAL_H
#define HAL_H

#include <stdint.h>

#ifdef I86
#define INT_INIT \
__asm__("add esp, 0x8"); \
__asm__("pop ebp"); \
__asm__("pusha"); \
__asm__("mov ax, ds"); \
__asm__("push eax"); \
__asm__("mov ax, 0x10"); \
__asm__("mov ds, ax"); \
__asm__("mov es, ax"); \
__asm__("mov fs, ax"); \
__asm__("mov gs, ax");

#define INT_EXIT \
__asm__("pop ebx"); \
__asm__("mov ds, bx"); \
__asm__("mov es, bx"); \
__asm__("mov fs, bx"); \
__asm__("mov gs, bx"); \
__asm__("popa"); \
__asm__("sti"); \
__asm__("iret");
#endif

extern int __attribute__((__cdecl__)) hal_init();
extern int __attribute__((__cdecl__)) hal_exit();

extern void __attribute__((__cdecl__)) int_done(unsigned int_no);
extern void __attribute__((__cdecl__)) INT(uint8_t n);

extern void __attribute__((__cdecl__)) sound(unsigned freq);

extern uint8_t __attribute__((__cdecl__)) inportb(uint16_t port);
extern void __attribute__((__cdecl__)) outportb(uint16_t port, uint8_t value);

extern void __attribute__((__cdecl__)) enable_int();
extern void __attribute__((__cdecl__)) disable_int();

extern void	__attribute__((__cdecl__)) set_int(int int_no, void (__attribute__((__cdecl__)) *irq) (void));
extern void	(__attribute__((__cdecl__)) *getvect(int int_no)) ( );

extern const char* get_cpu_vendor();

extern uint64_t get_tick_count();

#endif