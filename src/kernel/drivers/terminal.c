#include "terminal.h"
#include "ports.h"
#include "../util/memory.h"
#include "../util/string.h"

static void terminal_putentryat(char c, char color, int x, int y);
static void terminal_write(const char* data, int size);

unsigned char terminal_row;
unsigned char terminal_column;
char terminal_color;

/********************************
 * 		Private Funktionen		*
 ********************************/

//Generiert das Attributbyte aus der Vorder- und Hintergrundfarbe.
static inline char vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

//Generiert einen Integer (2 Bytes) aus dem Buchstaben und der Farbe. 
static inline int vga_entry(unsigned char uc, unsigned char color) 
{
	return (unsigned int) uc | (unsigned int) color << 8;
}
 
//Schreibt einen Buchstaben an einer bestimmten Stelle mit eine bestimmten Farbe in den Video Speicher.
static void terminal_putentryat(char c, char color, int x, int y) 
{
	const int index = y * VGA_WIDTH + x;
	unsigned short* vidmem = (unsigned short*) VIDEO_ADDRESS;
	vidmem[index] = vga_entry(c, color);
}
 
//Loopt durch den String und schreibt jeden Buchstaben.
static void terminal_write(const char* data, int size) 
{
	for (int i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

/************************************
 *		Öffentliche Funktionen		*
 ************************************/

//Schreibt einen Buchstaben am derzeitigen internen Cursor, falls der Buchstaben Newline (\n) ist geht er in die nächste Zeile.
void terminal_putchar(char c) 
{
	//Schreiben eines Buchstaben und Zeilenumbruch.
	if (c == '\n')
	{
		terminal_row++;
		terminal_column = 0;
	}
	else
	{
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH)
		{ 
			terminal_column = 0;
			terminal_row++;
		}
	}
	
	//Scrollen des Bildschirms (jede Zeile um 1 hoch).
	if (terminal_row == VGA_HEIGHT)
	{
		terminal_row = VGA_HEIGHT - 1;
		unsigned char* vidmem = (unsigned char*) VIDEO_ADDRESS;
		for (int i = 1; i < VGA_HEIGHT; i++)
		{
			memcpy(&vidmem[(i - 1) * VGA_WIDTH * 2], &vidmem[i * VGA_WIDTH * 2], VGA_WIDTH * 2);
		}
		memset(&vidmem[VGA_WIDTH * terminal_row * 2], 0x00, VGA_WIDTH * 2);
	}
}

//Initialisiert alle Variablen und löscht den Bildschirm
void terminal_initialize(void) 
{
	terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	terminal_clearscreen();
}
 
//Ändert die Farbe des Terminals.
void terminal_setcolor(enum vga_color fg, enum vga_color bg) 
{
	terminal_color = vga_entry_color(fg, bg);
}

void terminal_setinternalcursor(unsigned char x, unsigned char y)
{
	terminal_row = y;
	terminal_column = x;
}

void terminal_getinternalcursor(unsigned char* x, unsigned char* y)
{
	*x = terminal_column;
	*y = terminal_row;
}

//Schreibt einen String auf dem Bildschirm
void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

//Schreibt einen String mit Zeilenumbruch am Ende auf dem Bildschirm
void terminal_writeline(const char* data)
{
	terminal_write(data, strlen(data));
	terminal_write("\n", 1);
}

//Löscht den ganzen Bildschirm und setzt den internen Cursor in die linke, oberste Ecke.
void terminal_clearscreen(void)
{
	terminal_row = 0;
	terminal_column = 0;
	unsigned short* vidmem = (unsigned short*) VIDEO_ADDRESS;
	for (int y = 0; y < VGA_HEIGHT; y++) 
	{
		for (int x = 0; x < VGA_WIDTH; x++) 
		{
			const int index = y * VGA_WIDTH + x;
			vidmem[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcursor(bool enabled)
{
	if (enabled)
	{
		port_byte_out(REG_SCREEN_CTRL, 0x0A);
		port_byte_out(REG_SCREEN_DATA, port_byte_in(REG_SCREEN_DATA) | 0b00100000);
	}
	else
	{
		port_byte_out(REG_SCREEN_CTRL, 0x0A);
		port_byte_out(REG_SCREEN_DATA, port_byte_in(REG_SCREEN_DATA) & 0b11011111);
	}
}

void terminal_setcursorpos(int x, int y)
{
	unsigned int pos = 0;
	if (x == -1 || y == -1)
		pos = terminal_row * VGA_WIDTH + terminal_column;
	else
		pos = y * VGA_WIDTH + x;
		
	port_byte_out(REG_SCREEN_CTRL, 0x0F);
	port_byte_out(REG_SCREEN_DATA, (unsigned char) (pos & 0xFF));
	port_byte_out(REG_SCREEN_CTRL, 0x0E);
	port_byte_out(REG_SCREEN_DATA, (unsigned char) ((pos >> 8) & 0xFF));
}

void terminal_getcursorpos(unsigned char* x, unsigned char* y)
{
	port_byte_out(REG_SCREEN_CTRL, 0x0F);
	unsigned char lower = port_byte_in(REG_SCREEN_DATA);
	port_byte_out(REG_SCREEN_CTRL, 0x0E);
	unsigned char higher = port_byte_in(REG_SCREEN_DATA);
	unsigned int position = lower | (((unsigned int)higher) << 8);
	
	*x = (unsigned char)(position % VGA_WIDTH);
	*y = (unsigned char)(((position - position % VGA_WIDTH) / VGA_WIDTH));
}
