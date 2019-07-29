#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

extern void debug_set_color(const unsigned char c);
extern void debug_clear_screen(const unsigned char c);
extern void debug_set_pos(unsigned char x, unsigned char y);
extern void debug_print_string(const char* str);
extern void debug_printf(const char* str, ...);

extern void debug_print_dstring(const char* str);
extern void debug_printf_dstring(const char* str, ...);

#endif