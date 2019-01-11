#include "shell.h"
#include "drivers/terminal.h"
#include "drivers/keyboard.h"

#include "util/math.h"
#include "util/string.h"

static void redraw_buffer();
static void insert(unsigned char index, char value);

char buffer[BUFFER_SIZE];
unsigned char cursor;
unsigned char buffer_x, buffer_y;

int key_callback;

static void key_pressed(char ascii, enum SPECIAL_KEYS sp_keys, modifiers_t *modifier_keys)
{
	if (sp_keys == Backspace)
		shell_delete_key(0);
	else if (sp_keys == Enter)
		shell_execute();
	else
		shell_type_char(ascii);
}

void init_shell()
{
	terminal_writeline("OwO GmbH<R> OwOS ver. 0.1");
	terminal_writeline("    <C> OwO GmbH Sept. 2018 - Juli 2021\n");
	
	terminal_writestring("A:>");
	terminal_setcursorpos(-1, -1);
	
	cursor = 0;
	terminal_getcursorpos(&buffer_x, &buffer_y);
	terminal_setcursorpos((int)buffer_x, (int)buffer_y);
	
	key_callback = register_key_pressed_callback(key_pressed);
}

static void redraw_buffer()
{
	terminal_setinternalcursor(buffer_x, buffer_y);
	for (unsigned short i = 0; i < BUFFER_SIZE; i++)
	{
		terminal_putchar(buffer[i]);
	}
	terminal_setcursorpos((buffer_x + cursor) % VGA_WIDTH, buffer_y + ((cursor + buffer_x % VGA_WIDTH) / VGA_WIDTH));
}

static void insert(unsigned char index, char value)
{
	for (int i = BUFFER_SIZE - 1; i > index; i--)
		buffer[i] = buffer[i - 1];
	buffer[index] = value;
}

static void delete(unsigned char index)
{
	for (int i = index; i < BUFFER_SIZE - 1; i++)
	{
		buffer[i] = buffer[i + 1];
	}
	buffer[BUFFER_SIZE - 1] = '\0';
}

static void clear_buffer()
{
	for (int i = 0; i < BUFFER_SIZE - 1; i++)
		buffer[i] = '\0';
	cursor = 0;
}

static void print_filepath()
{
	terminal_writestring("A:>");
	terminal_getinternalcursor(&buffer_x, &buffer_y);
	terminal_setcursorpos((int)buffer_x, (int)buffer_y);
	clear_buffer();
}

void shell_execute()
{
	terminal_setinternalcursor((buffer_x + cursor) % VGA_WIDTH, buffer_y + ((cursor + buffer_x % VGA_WIDTH) / VGA_WIDTH));
	
	unsigned char argc = 0;
	char** argv = split(&buffer[0], ' ', 256, &argc);
	if (argv == 0)
		return;
		
	terminal_putchar('\n');
	
	if (strcmp(argv[0], "echo") == 0)
	{
		for (int i = 1; i < argc; i++)
		{
			terminal_writestring(argv[i]);
			terminal_putchar(' ');
		}
		terminal_putchar('\n');
	}
	else
	{
		terminal_writeline("Unknown Command");
	}
	
	print_filepath();
}

void shell_type_char(char c)
{
	insert(cursor, c);
	if (cursor < 255)
		cursor++;
	redraw_buffer();
}

void shell_delete_key(unsigned char front)
{
	if (front == 0 && cursor > 0)
		delete(--cursor);
	else if (front == 1)
		delete(cursor);
	redraw_buffer();
}

void shell_function_key(unsigned char index)
{
	
}

void shell_directional_key(unsigned char index)
{
	unsigned char limit = 255;
	while(limit > 0 && buffer[limit] == '\0')
		limit--;
		
	if (limit < 255) 
		limit++;
	
	switch(index)
	{
		case 0: //Up
			cursor = max((short)cursor - VGA_WIDTH, 0);
			break;
		case 1: //Left
			cursor = max((short)cursor - 1, 0);
			break;
		case 2: //Down
			cursor = min((short)cursor + VGA_WIDTH, limit);
			break;
		case 3: //Right
			cursor = min((short)cursor + 1, limit);
			break;
		default:
			break;
	}
	redraw_buffer();
}
