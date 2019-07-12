#include <string.h>

size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[++len]);
	return len;
}

char* strcpy(char* dest, const char* src)
{
	char* ret = dest;
	while ((*dest++ = *src++));
	return ret;
}

void* memcpy(void* dest, void* src, size_t count)
{
	char* dp = (char*)dest;
	char* sp = (char*)src;
	for (; count > 0; count--)* dp++ = *sp++;
	return dest;
}

void* memset(void* dest, char val, size_t count)
{
	char* dp = (char*)dest;
	for (; count > 0; count--, dp[count] = val);
	return dest;
}
