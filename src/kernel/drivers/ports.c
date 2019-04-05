#include "ports.h"

// Liest ein Byte aus dem angegebenen Port des I/O Registers.
unsigned char port_byte_in(unsigned short port) 
{
    unsigned char result;
    asm("in al, dx" : "=a" (result) : "d" (port));
    return result;
}

// Schreibt ein Byte in den angegebenen Port des I/O Registers.
void port_byte_out(unsigned short port, unsigned char data) 
{
    asm("out dx, al" : "=d" (port) : "a" (data));
}

// Liest ein Wort (2 Bytes) aus dem angegebenen Port des I/O Registers.
unsigned short port_word_in(unsigned short port) 
{
    unsigned short result;
    asm("in ax, dx" : "=a" (result) : "d" (port));
    return result;
}

// Schreibt ein Wort (2 Bytes) in den angegebenen Port des I/O Registers.
void port_word_out(unsigned short port, unsigned short data) 
{
    asm("out dx, ax" : "=d" (port) : "a" (data));
}
