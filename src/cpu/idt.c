#include "idt.h"

void set_idt_gate(int index, unsigned long handler) 
{
    idt[index].low_offset = (unsigned short) (handler & 0x0000FFFF);
    idt[index].sel = KERNEL_CS;
    idt[index].always0 = 0;
    idt[index].flags = 0x8E; 
    idt[index].high_offset = (unsigned short) ((handler >> 16) & 0x0000FFFF);
}

void set_idt() 
{
    idt_des.base = (unsigned long) &idt;
    idt_des.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;

    asm volatile("lidt [%0]" : : "r" (&idt_des));
}
