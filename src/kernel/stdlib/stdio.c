#include <stdio.h>

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

extern void int_to_str(int i, unsigned base, char* buf, bool prepend_zero);

int vsprintf(char* str, const char* fmt, va_list ap)
{
	if (!str || !fmt) return -1;

	size_t loc = 0;
	for (size_t i = 0; i < strlen(fmt); i++, loc++)
	{
		if (fmt[i] == '%')
		{
			switch (fmt[i + 1])
			{
			case 'c':
			{
				char c = va_arg(ap, char);
				str[loc] = c;
				break;
			}
			case 'd':
			case 'i':
			{
				int32_t c = va_arg(ap, int32_t);
				char s[32] = { 0 };
				itoa(c, 10, s, false);
				strcpy(&str[loc], s);
				loc += strlen(s) - 1;
				break;
			}
			case 'u':
			{
				uint32_t c = va_arg(ap, uint32_t);
				char s[32] = { 0 };
				uitoa(c, 10, s, false);
				strcpy(&str[loc], s);
				loc += strlen(s) - 1;
				break;
			}
			case 'x':
			case 'X':
			{
				uint32_t c = va_arg(ap, uint32_t);
				char s[32] = { 0 };
				uitoa(c, 16, s, true);
				strcpy(&str[loc], s);
				loc += strlen(s) - 1;
				break;
			}
			case 's':
			{
				char* s = va_arg(ap, char*);
				strcpy(&str[loc], s);
				loc += strlen(s) - 1;
				break;
			}
			case 'b':
			{
				uint32_t c = va_arg(ap, uint32_t);
				char s[32] = { 0 };
				uitoa(c, 2, s, false);
				strcpy(&str[loc], s);
				loc += strlen(s) - 1;
				break;
			}
			default:
				str[loc] = fmt[i];
				break;
			}
			i++;
		}
		else
		{
			str[loc] = fmt[i];
		}
	}
	return 0;
}