#include "shell.h"
#include "drivers/terminal.h"
#include "drivers/keyboard.h"
#include "drivers/floppy.h"

#include "util/math.h"
#include "util/string.h"

static void redraw_buffer();
static void insert(unsigned char, char);
static void key_pressed(char, enum SPECIAL_KEYS, modifiers_t*);
static void clear_buffer(void);
static void print_filepath();
static void shell_execute();
static void shell_type_char(char);
static void shell_delete_key(unsigned char);
static void shell_directional_key(unsigned char);
static unsigned char buffer_size();

char buffer[BUFFER_SIZE];
unsigned char cursor;
unsigned char buffer_x, buffer_y;
unsigned char last_redraw;

int key_callback;

/*********************/
/* PRIVATE FUNCTIONS */
/*********************/

/* Zeichnet den char buffer erneut auf dem terminal. */
static void redraw_buffer()
{
	terminal_setinternalcursor(buffer_x, buffer_y);
	for (unsigned short i = 0; i < BUFFER_SIZE; i++)
	{
		terminal_putchar(buffer[i]);
		if (i >= last_redraw && buffer[i] == '\0')
		{
			last_redraw = i;
			break;
		}
	}

	terminal_setcursorpos((buffer_x + cursor) % VGA_WIDTH, buffer_y + ((cursor + buffer_x % VGA_WIDTH) / VGA_WIDTH));
}

/* Fügt ein Zeichen am index ein.*/
static void insert(unsigned char index, char value)
{
	for (int i = BUFFER_SIZE - 1; i > index; i--)
		buffer[i] = buffer[i - 1];
	buffer[index] = value;
}

/* Löscht ein Zeichen am index. */
static void delete(unsigned char index)
{
	for (int i = index; i < BUFFER_SIZE - 1; i++)
	{
		buffer[i] = buffer[i + 1];
	}
	buffer[BUFFER_SIZE - 1] = '\0';
}

/* Setzt den buffer zurück.*/
static void clear_buffer(void)
{
	for (int i = 0; i < BUFFER_SIZE - 1; i++)
		buffer[i] = '\0';
	cursor = 0;
}

/* Gibt den derzeitig gefüllten platz des buffers an. */
static unsigned char buffer_size()
{
	unsigned char limit = 255;
	while (limit > 0 && buffer[limit] == '\0')
		limit--;
	return limit;
}

/* Schreibt den derzeitigen Dateipfad auf dem terminal. */
static void print_filepath()
{
	terminal_writestring("A:>");
	terminal_writestring("\n\n\n\n");
	terminal_getinternalcursor(&buffer_x, &buffer_y);
	buffer_y -= 4;
	buffer_x += 3;
	terminal_setcursorpos((int)buffer_x, (int)(buffer_y));
	clear_buffer();
	last_redraw = 0;
}

static void shell_execute()
{
	terminal_setinternalcursor((buffer_x + buffer_size()) % VGA_WIDTH, buffer_y + ((buffer_size() + buffer_x % VGA_WIDTH) / VGA_WIDTH));

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
	else if (strcmp(argv[0], "hexdmp") == 0)
	{
		unsigned char* address = (unsigned char*) parse_hex(argv[1]);

		char* addr = "00000000";

		for (int j = 0; j < 20; j++)
		{
			ulong_to_hex((unsigned long)address + j * 16, addr, 8);
			terminal_writestring(addr);
			terminal_writestring(" |");

			for (int i = 0; i < 16; i++)
			{
				if (i == 8)
					terminal_putchar(' ');
				ulong_to_hex(address[i + j * 16], addr, 2);
				terminal_putchar(' ');
				terminal_writestring(addr);
			}

			terminal_writestring(" | ");

			for (int i = 0; i < 16; i++)
			{
				if (addr[i + j * 16] == '\n')
					terminal_putchar(' ');
				else
					terminal_putchar(address[i + j * 16]);
			}

			terminal_putchar('\n');
		}
	}
	else if (strcmp(argv[0], "floppy") == 0)
	{
		if (argc != 4)
			terminal_writeline("Command Usage: floppy -r/-w sector[0 - floppy length] drive[0, 1]");
		else
		{
			int sector = parse_int(argv[2]);
			int drive = parse_int(argv[3]);

			if (sector < 0)
				terminal_writeline("Command Usage: floppy -r/-w sector[0 - floppy size] drive[0, 1]");
			else if (drive != 0 && drive != 1)
				terminal_writeline("Command Usage: floppy -r/-w sector[0 - floppy size] drive[0, 1]");
			else if (strcmp(argv[1], "-r") != 0 && strcmp(argv[1], "-w") != 0)
				terminal_writeline("Command Usage: floppy -r/-w sector[0 - floppy size] drive[0, 1]");
			else
				flpydsk_read_write_sector(strcmp(argv[1], "-r") == 0 ? 0 : 1, sector, (char)drive);
		}
	}
	else
	{
		terminal_writeline("Unknown Command");
	}

	print_filepath();
}

/* Schreibt einen Buchstaben an der Cursorstelle. */
static void shell_type_char(char c)
{
	insert(cursor, c);
	if (cursor < 255)
		cursor++;
	redraw_buffer();
}

/* Löscht einen Buchstaben an der Cursorstelle. */
static void shell_delete_key(unsigned char front)
{
	if (front == 0 && cursor > 0)
		delete(--cursor);
	else if (front == 1)
		delete(cursor);
	redraw_buffer();
}

/* Bewegt den Cursor. */
static void shell_directional_key(unsigned char index)
{
	unsigned char limit = buffer_size();
	if (limit < 255)
		limit++;

	switch (index)
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

/* Event receiver für das key_pressed Event. */
static void key_pressed(char ascii, enum SPECIAL_KEYS sp_keys, modifiers_t *modifier_keys)
{
	switch (sp_keys)
	{
		case Backspace:
			shell_delete_key(0);
			break;
		case Delete:
			shell_delete_key(1);
			break;
		case Enter:
			shell_execute();
			break;
		case Up:
			shell_directional_key(0);
			break;
		case Down:
			shell_directional_key(1);
			break;
		case Left:
			shell_directional_key(2);
			break;
		case Right:
			shell_directional_key(3);
			break;
		default:
			if (ascii != '\0')
				shell_type_char(ascii);
			break;
	}
}

/********************/
/* PUBLIC FUNCTIONS */
/********************/

/* Initialisiert die Shell. */
void init_shell()
{
	terminal_writeline("OwO GmbH<R> OwOS ver. 0.1");
	terminal_writeline("    <C> OwO GmbH Sept. 2018 - Juli 2021\n");
	
	terminal_writestring("A:>");
	terminal_setcursorpos(-1, -1);
	
	cursor = 0;
	terminal_getcursorpos(&buffer_x, &buffer_y);
	terminal_setcursorpos((int)buffer_x, (int)buffer_y);

	for (;;)
	{
		keypress_t key = read_key();
		key_pressed(key.ascii, key.sp, &key.mod);
	}
}
