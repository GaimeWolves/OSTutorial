#include "debugprint.h"

#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#define VIDMEM 0xB8000

#define COLUMNS 80
#define ROWS 25

static unsigned char xPos = 0, yPos = 0;
static unsigned char color = 15;

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

char basechars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
char temp[32];

void int_to_str(int i, unsigned base, char* buf, bool prepend_zero)
{
	if (base > 16 || base < 2) return;

	if (i < 0)
	{
		*buf++ = '-';
		i *= -1;
	}

	int pos = 0;

	do
	{
		temp[pos] = basechars[i % base];
		pos++;
		i /= base;
	} while ((prepend_zero && ((base == 16 && pos < 8) || (base == 2 && pos < 32))) || i);

	int len = pos--;

	for (int bpos = 0; bpos < len; pos--, bpos++)
		buf[bpos] = temp[pos];

	buf[len] = 0;
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
				int c = va_arg(args, int);
				char s[32] = { 0 };
				int_to_str(c, 10, s, false);
				debug_print_string(s);
				break;
			}
			case 'x':
			case 'X':
			{
				int c = va_arg(args, int);
				char s[32] = { 0 };
				int_to_str(c, 16, s, true);
				debug_putc('0');
				debug_putc('x');
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
				int c = va_arg(args, int);
				char s[32] = { 0 };
				int_to_str(c, 2, s, false);
				debug_putc('0');
				debug_putc('b');
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
	for (int y = 2; y < ROWS - 1; y++)
		memcpy((void*)(VIDMEM + (y - 1) * COLUMNS * 2), (void*)(VIDMEM + y * COLUMNS * 2), COLUMNS * 2);
	memset((void*)(VIDMEM + (ROWS - 2) * COLUMNS * 2), 0, COLUMNS * 2);
	debug_set_pos(0, ROWS - 2);
	debug_set_color(0x0D);
	debug_print_string("DEBUG ");
	debug_set_color(0x0F);
	debug_print_string(str);
}
