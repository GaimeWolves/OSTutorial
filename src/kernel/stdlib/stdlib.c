#include <stdlib.h>

char basechars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
char temp[32];

void uitoa(uint32_t i, size_t base, char* buf, bool prepend_zero)
{
	if (base > 16 || base < 2) return;

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

void itoa(int32_t i, size_t base, char* buf, bool prepend_zero)
{
	if (base > 16 || base < 2) return;

	if (i < 0)
	{
		*buf++ = '-';
		i *= -1;
	}

	uitoa((uint32_t)i, base, buf, prepend_zero);
}