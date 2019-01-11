#include "memory.h"

unsigned char* current = 0;

//Kopiert den Speicher Byteweise von einer Adresse in die andere
void memcpy(unsigned char* dest, unsigned char* src, int len)
{
	for (int i = 0; i < len; i++)
		dest[i] = src[i];
}

void memset(unsigned char* adr, unsigned char fill, int len)
{
	for (int i = 0; i < len; i++)
	adr[i] = fill;
}

void* malloc(unsigned int size)
{
	void* ret = &current;
	current += size;
	return ret;
}
