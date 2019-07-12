#ifndef STRING_H
#define STRING_H

#include <size_t.h>

size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);

void* memcpy(void* dest, void* src, size_t count);
void* memset(void* dest, char val, size_t count);

#endif