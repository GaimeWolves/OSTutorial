#include "exceptions.h"

extern void panic(const char* fmt, ...);

void __attribute__((__cdecl__)) divide_by_zero_fault() { panic("Divide by 0 fault"); for(;;); }
void __attribute__((__cdecl__)) single_step_trap() { panic("Single step trap"); for(;;); }
void __attribute__((__cdecl__)) nmi_trap() { panic("Non maskable interrupt trap"); for(;;); }
void __attribute__((__cdecl__)) breakpoint_trap() { panic("Breakpoint trap"); for(;;); }
void __attribute__((__cdecl__)) overflow_trap() { panic("Overflow trap"); for(;;); }
void __attribute__((__cdecl__)) bounds_check_fault() { panic("Bounds check fault"); for(;;); }
void __attribute__((__cdecl__)) invalid_opcode_fault() { panic("Invalid opcode fault"); for(;;); }
void __attribute__((__cdecl__)) no_device_fault() { panic("No device fault"); for(;;); }
void __attribute__((__cdecl__)) double_fault_abort() { panic("Double fault abort"); for(;;); }
void __attribute__((__cdecl__)) invalid_tss_fault() { panic("Invalid TSS fault"); for(;;); }
void __attribute__((__cdecl__)) no_segment_fault() { panic("No segment fault"); for(;;); }
void __attribute__((__cdecl__)) stack_fault() { panic("Stack fault"); for(;;); }
void __attribute__((__cdecl__)) general_protection_fault() { panic("General Protection Fault"); for(;;); }
void __attribute__((__cdecl__)) page_fault() { panic("Page fault"); for(;;); }
void __attribute__((__cdecl__)) fpu_fault() { panic("FPU fault"); for(;;); }
void __attribute__((__cdecl__)) alignment_check_fault() { panic("Alignment check fault"); for(;;); }
void __attribute__((__cdecl__)) machine_check_abort() { panic("Machine check abort"); for(;;); }
void __attribute__((__cdecl__)) simd_fpu_fault() { panic("SIMD FPU fault"); for(;;); }
void __attribute__((__cdecl__)) reserved() { panic("RESERVED"); for(;;); }