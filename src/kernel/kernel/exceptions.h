#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

extern void __attribute__((__cdecl__)) divide_by_zero_fault();
extern void __attribute__((__cdecl__)) single_step_trap();
extern void __attribute__((__cdecl__)) nmi_trap();
extern void __attribute__((__cdecl__)) breakpoint_trap();
extern void __attribute__((__cdecl__)) overflow_trap();
extern void __attribute__((__cdecl__)) bounds_check_fault();
extern void __attribute__((__cdecl__)) invalid_opcode_fault();
extern void __attribute__((__cdecl__)) no_device_fault();
extern void __attribute__((__cdecl__)) double_fault_abort();
extern void __attribute__((__cdecl__)) invalid_tss_fault();
extern void __attribute__((__cdecl__)) no_segment_fault();
extern void __attribute__((__cdecl__)) stack_fault();
extern void __attribute__((__cdecl__)) general_protection_fault();
extern void __attribute__((__cdecl__)) page_fault();
extern void __attribute__((__cdecl__)) fpu_fault();
extern void __attribute__((__cdecl__)) alignment_check_fault();
extern void __attribute__((__cdecl__)) machine_check_abort();
extern void __attribute__((__cdecl__)) simd_fpu_fault();
extern void __attribute__((__cdecl__)) reserved();

#endif