#include "debugprint.h"

#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define VIDMEM 0xB8000

#define COLUMNS 80
#define ROWS 25

static unsigned char xPos = 0, yPos = 0;
static unsigned char color = 15;

static void set_dstring()
{
	for (int y = 2; y < ROWS - 1; y++)
		memcpy((void*)(VIDMEM + (y - 1) * COLUMNS * 2), (void*)(VIDMEM + y * COLUMNS * 2), COLUMNS * 2);
	memset((void*)(VIDMEM + (ROWS - 2) * COLUMNS * 2), 0, COLUMNS * 2);
	debug_set_pos(0, ROWS - 2);
	debug_set_color(0x0D);
	debug_print_string("DEBUG ");
	debug_set_color(0x0F);
}

void debug_putc(const unsigned char c)
{
	if (!c) return;

	if (c == '\n' || c == '\r' || xPos >= COLUMNS)
	{
		xPos = 0;
		yPos++;
		return;
	}

	unsigned char* ptr = (unsigned char*)VIDMEM + (xPos++) * 2 + yPos * 2 * COLUMNS;
	*ptr++ = c;
	*ptr = color;
}

void debug_set_color(const unsigned char c)
{
	color = c;
}

void debug_clear_screen(const unsigned char c)
{
	unsigned char* vidmem = (unsigned char*)VIDMEM;

	for (int i = 0; i < COLUMNS * ROWS; i++)
	{
		*vidmem++ = ' ';
		*vidmem++ = c;
	}
}

void debug_set_pos(unsigned char x, unsigned char y)
{
	xPos = x;
	yPos = y;
}

void debug_print_string(const char* str)
{
	for (size_t i = 0; i < strlen(str); i++)
		debug_putc(str[i]);
}

void debug_printf(const char* str, ...)
{
	if (!str) return;

	va_list args;
	va_start(args, str);

	for (size_t i = 0; i < strlen(str); i++)
	{
		if (str[i] == '%')
		{
			switch (str[i + 1])
			{
			case 'c':
			{
				char c = va_arg(args, char);
				debug_putc(c);
				break;
			}
			case 'd':
			case 'i':
			{
				int32_t c = va_arg(args, int32_t);
				char s[32] = { 0 };
				itoa(c, 10, s, false);
				debug_print_string(s);
				break;
			}
			case 'u':
			{
				uint32_t c = va_arg(args, uint32_t);
				char s[32] = { 0 };
				uitoa(c, 10, s, false);
				debug_print_string(s);
				break;
			}
			case 'x':
			case 'X':
			{
				uint32_t c = va_arg(args, uint32_t);
				char s[32] = { 0 };
				uitoa(c, 16, s, true);
				debug_print_string(s);
				break;
			}
			case 's':
			{
				char* s = va_arg(args, char*);
				debug_print_string(s);
				break;
			}
			case 'b':
			{
				uint32_t c = va_arg(args, uint32_t);
				char s[32] = { 0 };
				uitoa(c, 2, s, false);
				debug_print_string(s);
				break;
			}
			default:
				break;
			}
			i++;
		}
		else
		{
			debug_putc(str[i]);
		}
	}
	va_end(args);
}

void debug_print_dstring(const char* str)
{
	set_dstring();
	debug_print_string(str);
}

void debug_printf_dstring(const char* str, ...)
{	
	set_dstring();

	va_list ap;
	va_start(ap, str);

	char buf[1024] = {0};
	vsprintf(buf, str, ap);
	debug_print_string(buf);

	va_end(ap);
}