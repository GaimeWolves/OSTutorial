#include "keyboard.h"
#include "ports.h"
#include "../../cpu/isr.h"
#include "../util/string.h"
#include "../shell.h"
#include "terminal.h"

static void update_keyboard_state(unsigned char scancode, unsigned char special_sc);
static void keyboard_callback(registers_t regs);
static char handle_special_keys(unsigned char scancode);

static char KEY_PRESS = 0;
static char KEY_RELEASE = 0;

/* These arrays hold the ascii values of every keyboard scancode for every modifier. (Shift, AltGr, etc.) 
	0 is specified for scancodes that don't give ascii characters. */
const char keys_nomod[] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0xE1, '\'', 0, 0, 			//0x00 - 0x0F
	'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0x81, '+', 0, 0, 'a', 's', 		//0x10 - 0x1F
	'd', 'f', 'g', 'h', 'j', 'k', 'l', 0x94, 0x84, '^', 0, '#', 'y', 'x', 'c', 'v', 	//0x20 - 0x2F
	'b', 'n', 'm', ',', '.', '-', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,						//0x30 - 0x3F
	0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',					//0x40 - 0x4F
	'2', '3', '0', ',', 0, 0, '<'														//0x50 - 0x56
};

const char keys_shift[] = {
	0, 0, '!', '\"', 0, '$', '%', '&', '/', '(', ')', '=', '?', '`', 0, 0, 				//0x00 - 0x0F
	'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 0x9A, '*', 0, 0, 'A', 'S', 		//0x10 - 0x1F
	'D', 'F', 'G', 'H', 'J', 'K', 'L', 0x99, 0x8E, 0xF8, 0, '\'', 'Y', 'X', 'C', 'V', 	//0x20 - 0x2F
	'B', 'N', 'M', ';', ':', '_', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,						//0x30 - 0x3F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0,									//0x40 - 0x4F
	0, 0, 0, 0, 0, 0, '>'																//0x50 - 0x56
};

const char keys_ctrl_alt[] = {
	0, 0, 0, 0xFD, 0, 0xAC, 0xAB, 0xAA, '{', '[', ']', '}', '\\', 0, 0, 0, 				//0x00 - 0x0F
	'@', 0, 0, 0x9E, 0, 0, 0, 0, 0, 0, 0, '~', 0, 0, 0x91, 0, 							//0x10 - 0x1F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xAF, 0xAE, 0x9B, 0, 							//0x20 - 0x2F
	0, 0, 0xE6, 0xF9, 0, 0xC4, 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,						//0x30 - 0x3F
	0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',					//0x40 - 0x4F
	'2', '3', '0', ',', 0, 0, '|'														//0x50 - 0x56
};

key_event key_pressed[256];
key_event key_released[256];

unsigned char special_sc;
unsigned char skipcounter;

modifiers_t modifiers;
enum SPECIAL_KEYS sp_keys;
char ascii;

/*********************/
/* PRIVATE FUNCTIONS */
/*********************/

/* This function gets called by the keyboard interrupt and handles the given scancode.*/
static void keyboard_callback(registers_t regs)
{
	unsigned char scancode = port_byte_in(0x60);

	if (handle_special_keys(scancode) == 1)
		return;

	update_keyboard_state(scancode, special_sc);

	if (scancode == 0x0E || scancode == 0x0E + RELEASED) //Backspace
	{
		ascii = 0;
		sp_keys = Backspace;
	}
	else if (scancode == 0x1C || scancode == 0x1C + RELEASED) //Enter
	{
		ascii = 0;
		sp_keys = Enter;
	}
	else
	{
		char sub = 0;
		if (scancode > RELEASED)
		{
			scancode -= RELEASED;
			sub = 1;
		}

		if (keys_nomod[scancode] != '\0')
		{
			if (modifiers.shift | modifiers.shift_right | modifiers.capslock)
			{
				ascii = keys_shift[scancode];
				sp_keys = None;
			}
			else if (modifiers.altgr | (modifiers.alt & (modifiers.strg | modifiers.strg_right)))
			{
				ascii = keys_ctrl_alt[scancode];
				sp_keys = None;
			}
			else
			{
				ascii = keys_nomod[scancode];
				sp_keys = None;
			}
		}

		if (sub == 1)
			scancode += RELEASED;
	}

	if (scancode < RELEASED)
		KEY_PRESS = 1;
	else
		KEY_RELEASE = 1;
}

/* This handles special keys that consist of multiple scancodes. (eg. EO 48 for Up Arrow)*/
static char handle_special_keys(unsigned char scancode)
{
	if (skipcounter > 0)
	{
		skipcounter--;
		return 1;
	}

	if (scancode == 0xE0 || scancode == 0xE1)
	{
		if (scancode == 0xE1)
		{
			skipcounter = 5;
			ascii = 0;
			sp_keys = Pause_Break;
			return 1;
		}
		special_sc = scancode;
		return 1;
	}

	if (special_sc != 0)
	{
		int sub = 0;
		if (scancode > RELEASED)
		{
			scancode -= RELEASED;
			sub = 1;
		}

		switch (scancode)
		{
		case 0x2A:
			skipcounter = 2;
			ascii = 0;
			sp_keys = Print;
			break;
		case 0x37:
			ascii = 0;
			sp_keys = Print;
			break;
		case 0x46:
			skipcounter = 2;
			ascii = 0;
			sp_keys = Pause_Break;
			break;
		case 0x5B:
			ascii = 0;
			sp_keys = Windows_Left;
			break;
		case 0x5C:
			ascii = 0;
			sp_keys = Windows_Right;
			break;
		case 0x5D:
			ascii = 0;
			sp_keys = Backspace;
			break;
		case 0x52:
			ascii = 0;
			sp_keys = Insert;
			break;
		case 0x53:
			ascii = 0;
			sp_keys = Delete;
			break;
		case 0x47:
			ascii = 0;
			sp_keys = Home;
			break;
		case 0x4F:
			ascii = 0;
			sp_keys = End;
			break;
		case 0x49:
			ascii = 0;
			sp_keys = Prior;
			break;
		case 0x51:
			ascii = 0;
			sp_keys = Next;
			break;
		case 0x48:
			ascii = 0;
			sp_keys = Up;
			break;
		case 0x4B:
			ascii = 0;
			sp_keys = Left;
			break;
		case 0x50:
			ascii = 0;
			sp_keys = Down;
			break;
		case 0x4D:
			ascii = 0;
			sp_keys = Right;
			break;
		case 0x35:
			ascii = '/';
			sp_keys = None;
			break;
		}

		if (sub)
			scancode += RELEASED;

		special_sc = 0;
		update_keyboard_state(scancode, special_sc);
		return 1;
	}
	return 0;
}

/* This updates the modifiers (eg. Shift) that are currently pressed. */
static void update_keyboard_state(unsigned char scancode, unsigned char special_sc)
{
	if (special_sc == 0xE0)
	{
		switch (scancode)
		{
			//Strg Right
			case 0x1D:
				modifiers.strg_right = 1;
				break;
			case 0x1D + RELEASED:
				modifiers.strg_right = 0;
				break;

				//AltGr
			case 0x38:
				modifiers.altgr = 1;
				break;
			case 0x38 + RELEASED:
				modifiers.altgr = 0;
				break;
		}
	}
	else
	{
		switch (scancode)
		{
			//Shift
			case 0x2A:
				modifiers.shift = 1;
				break;
			case 0x2A + RELEASED:
				modifiers.shift = 0;
				break;

				//Shift right
			case 0x36:
				modifiers.shift_right = 1;
				break;
			case 0x36 + RELEASED:
				modifiers.shift_right = 0;
				break;

				//Strg
			case 0x1D:
				modifiers.strg = 1;
				break;
			case 0x1D + RELEASED:
				modifiers.strg = 0;
				break;

				//Alt
			case 0x38:
				modifiers.alt = 1;
				break;
			case 0x38 + RELEASED:
				modifiers.alt = 0;
				break;

				//Caps lock
			case 0x3A:
				modifiers.capslock = ~modifiers.capslock;
				break;
		}
	}
}

/********************/
/* PUBLIC FUNCTIONS */
/********************/

/* This initializes the keyboard handler */
void init_keyboard() 
{
	register_interrupt_handler(IRQ1, keyboard_callback);
	special_sc = 0;
	skipcounter = 0;
	modifiers = (modifiers_t) { 0, 0, 0, 0, 0, 0, 0 }; 
}

keypress_t read_key()
{
	while (!KEY_PRESS);
	KEY_PRESS = 0;
	return (keypress_t) { ascii, sp_keys, modifiers };
}
