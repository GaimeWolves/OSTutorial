#include "keyboard.h"
#include "ports.h"
#include "../../cpu/isr.h"
#include "../util/string.h"
#include "../shell.h"
#include "terminal.h"

static void update_keyboard_state(unsigned char scancode, unsigned char special_sc);
static void keyboard_callback(registers_t regs);
static char handle_special_keys(unsigned char scancode);

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

/*********************/
/* PRIVATE FUNCTIONS */
/*********************/

/* This function broadcasts the key_pressed and key_released events to the event handlers specified
	in the key_event arrays.*/
static void broadcast_key_event(int event, char ascii, enum SPECIAL_KEYS sp_keys)
{
	if (event == 0)
	{
		for (unsigned short i = 0; i < 256; i++)
		{
			if (key_pressed[i] != 0)
				key_pressed[i](ascii, sp_keys, &modifiers);
		}
	}
	else
	{
		for (unsigned short i = 0; i < 256; i++)
		{
			if (key_released[i] != 0)
				key_released[i](ascii, sp_keys, &modifiers);
		}
	}
}

/* This function gets called by the keyboard interrupt and handles the given scancode.*/
static void keyboard_callback(registers_t regs) 
{
	unsigned char scancode = port_byte_in(0x60);
	
	if (handle_special_keys(scancode) == 1) 
		return;

    if (scancode < sizeof(keys_nomod) / sizeof(*keys_nomod) && keys_nomod[scancode] != 0)
    {
		if (modifiers.shift | modifiers.shift_right | modifiers.capslock)
			broadcast_key_event(0, keys_shift[scancode], None);
		else if (modifiers.altgr | (modifiers.alt & (modifiers.strg | modifiers.strg_right)))
			broadcast_key_event(0, keys_ctrl_alt[scancode], None);
		else
			broadcast_key_event(0, keys_nomod[scancode], None);
	}
	else if (scancode > RELEASED && keys_nomod[scancode - RELEASED] != 0)
	{
		if (modifiers.shift | modifiers.shift_right | modifiers.capslock)
			broadcast_key_event(1, keys_shift[scancode], None);
		else if (modifiers.altgr | (modifiers.alt & (modifiers.strg | modifiers.strg_right)))
			broadcast_key_event(1, keys_ctrl_alt[scancode], None);
		else
			broadcast_key_event(1, keys_nomod[scancode], None);
	}
	else if (scancode == 0x0E || scancode == 0x0E + RELEASED) //Backspace
	{
		if (scancode == 0x0E)
			broadcast_key_event(0, 0, Backspace);
		else
			broadcast_key_event(1, 0, Backspace);
	}
	else if (scancode == 0x1C || scancode == 0x1C + RELEASED) //Enter
	{
		if (scancode == 0x1C)
			broadcast_key_event(0, 0, Enter);
		else
			broadcast_key_event(1, 0, Enter);
	}
    
    update_keyboard_state(scancode, special_sc);
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
			broadcast_key_event(0, 0, Pause_Break);
			return 1;
		}
		special_sc = scancode;
		return 1;
	}

	if (special_sc != 0)
	{
		switch (scancode)
		{
		case 0x2A:
			skipcounter = 2;
			broadcast_key_event(0, 0, Print);
			break;
		case 0x37:
			broadcast_key_event(0, 0, Print);
			break;
		case 0x46:
			skipcounter = 2;
			broadcast_key_event(0, 0, Pause_Break);
			break;
		case 0x5B:
			broadcast_key_event(0, 0, Windows_Left);
			break;
		case 0x5C:
			broadcast_key_event(0, 0, Windows_Right);
			break;
		case 0x5D:
			broadcast_key_event(0, 0, Apps);
			break;
		case 0x52:
			broadcast_key_event(0, 0, Insert);
			break;
		case 0x53:
			broadcast_key_event(0, 0, Delete);
			break;
		case 0x47:
			broadcast_key_event(0, 0, Home);
			break;
		case 0x4F:
			broadcast_key_event(0, 0, End);
			break;
		case 0x49:
			broadcast_key_event(0, 0, Prior);
			break;
		case 0x51:
			broadcast_key_event(0, 0, Next);
			break;
		case 0x48:
			broadcast_key_event(0, 0, Up);
			break;
		case 0x4B:
			broadcast_key_event(0, 0, Left);
			break;
		case 0x50:
			broadcast_key_event(0, 0, Down);
			break;
		case 0x4D:
			broadcast_key_event(0, 0, Right);
			break;
		case 0x35:
			broadcast_key_event(0, '/', None);
			break;
		}
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

/* This registers a pointer to the key_event function and returnes the 
	specific index at which it's stored. */
int register_key_pressed_callback(key_event event)
{
	for (unsigned short i = 0; i < 256; i++)
	{
		if (key_pressed[i] == 0)
		{
			key_pressed[i] = event;
			return i;
		}
	}
	return -1;
}

/* This frees the index of the key_event. */
void delete_key_pressed_callback(int index)
{
	key_pressed[index] = 0;
}
