#ifndef TERMINAL
#define TERMINAL

#include <stdbool.h>

//Adresse des Video Speichers
#define VIDEO_ADDRESS (char*) 0xB8000

//Anzahl Spalten und Zeilen im Video Speicher
#define VGA_HEIGHT 25
#define VGA_WIDTH 80

//Die I/O Register Ports zur Kontrolle des Cursors
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

//Mögliche Farben des VGA Standards
enum vga_color 
{
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

void terminal_initialize(void);
void terminal_setcolor(enum vga_color fg, enum vga_color bg);
void terminal_setinternalcursor(unsigned char x, unsigned char y);
void terminal_getinternalcursor(unsigned char* x, unsigned char* y);
void terminal_writestring(const char* data);
void terminal_writeline(const char* data);
void terminal_putchar(char c);
void terminal_clearscreen(void);
void terminal_setcursor(bool enabled);
void terminal_setcursorpos(int x, int y);
void terminal_getcursorpos(unsigned char* x, unsigned char* y);   

#endif
