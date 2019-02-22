#ifndef STRING_UTILS
#define STRING_UTILS

#define HEX "0123456789ABCDEF"

int strlen(const char* str);
void int_to_str(int num, char* str);
void ulong_to_hex(unsigned long num, char* str, int len);
void uint_to_bin(unsigned int num, char* str, int len);
void reverse(char* str);
char** split(char* str, char splitter, unsigned int maxlen, unsigned char* argc);
int strcmp(const char* a, const char* b);
int parse_int(const char* str);
unsigned long parse_hex(const char* str);

#endif

