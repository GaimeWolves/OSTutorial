#include "keyboard.h"
#include "ports.h"
#include "../../cpu/isr.h"
#include "../util/string.h"
#include "../shell.h"

static void update_keyboard_state(unsigned char scancode, unsigned char special_sc);
static void keyboard_callback(registers_t regs);
static char handle_special_keys(unsigned char scancode);

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

unsigned char special_sc;
unsigned char skipcounter;

modifiers_t modifiers;

static void broadcast_key_event(int event, char ascii, enum SPECIAL_KEYS sp_keys)
{
	for (unsigned short i = 0; i < 256; i++)
	{
		if (key_pressed[i] != 0)
			key_pressed[i](ascii, sp_keys, &modifiers);
	}
}

static void keyboard_callback(registers_t regs) 
{
	unsigned char scancode = port_byte_in(0x60);
	
	if (handle_special_keys(scancode) == 1) 
		return;
    
    /*
    char* sc_ascii = "00";
    uint_to_hex(scancode, sc_ascii);
    terminal_writeline(sc_ascii);
    * */
    if (scancode < sizeof(keys_nomod) / sizeof(*keys_nomod) && keys_nomod[scancode] != 0)
    {
		if (modifiers.shift | modifiers.shift_right | modifiers.capslock)
			broadcast_key_event(0, keys_shift[scancode], None);
		else if (modifiers.altgr | (modifiers.alt & (modifiers.strg | modifiers.strg_right)))
			broadcast_key_event(0, keys_ctrl_alt[scancode], None);
		else
			broadcast_key_event(0, keys_nomod[scancode], None);
	}
	else if (scancode == 0x0E) //Backspace
	{
		for (unsigned short i = 0; i < 256; i++)
		{
			if (key_pressed[i] != 0)
				key_pressed[i](0, Backspace, &modifiers);
		}
	}
	else if (scancode == 0x1C) //Enter
	{
		for (unsigned short i = 0; i < 256; i++)
		{
			if (key_pressed[i] != 0)
				key_pressed[i](0, Enter, &modifiers);
		}
	}
    
    update_keyboard_state(scancode, special_sc);
    /*
    char* st_ascii = "0000";
    uint_to_bin(keyboard_state, st_ascii, 4);
    terminal_writestring("Keyboard State: ");
    terminal_writeline(st_ascii);
    * */
}

void init_keyboard() 
{
	keyboard_state = 0b00000000;
	register_interrupt_handler(IRQ1, keyboard_callback);
	special_sc = 0;
	skipcounter = 0;
	modifiers = (modifiers_t) { 0, 0, 0, 0, 0, 0, 0 }; 
}

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

void delete_key_pressed_callback(int index)
{
	key_pressed[index] = 0;
}

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
			//Pause/Break Key
			return 1;
		}
		special_sc = scancode;
		return 1;
	}
	
	if (special_sc != 0)
	{
		switch(scancode)
		{
			case 0x2A:
				skipcounter = 2;
				//Print
				break;
			case 0x37:
				//Print
				break;
			case 0x46:
				skipcounter = 2;
				//Pause/Break Key
				break;
			case 0x5B:
				//Left Windows Key
				break;
			case 0x5C:
				//Right Windows Key
				break;
			case 0x5D:
				//Apps Key
				break;
			case 0x52:
				//Insert Key
				break;
			case 0x53:
				//Delete Key
				shell_delete_key(1);
				break;
			case 0x47:
				//Home Key
				break;
			case 0x4F:
				//End Key
				break;
			case 0x49:
				//Prior Key
				break;
			case 0x51:
				//Next Key
				break;
			case 0x48:
				//Up Arrow Key
				shell_directional_key(0);
				break;
			case 0x4B:
				//Left Arrow Key
				shell_directional_key(1);
				break;
			case 0x50:
				//Down Arrow Key
				shell_directional_key(2);
				break;
			case 0x4D:
				//Right Arrow Key
				shell_directional_key(3);
				break;
			case 0x35:
				shell_type_char('/');
				break;
		}
		special_sc = 0;
		update_keyboard_state(scancode, special_sc);
		return 1;
	}
	return 0;
}

static void update_keyboard_state(unsigned char scancode, unsigned char special_sc)
{
	if (special_sc == 0xE0)
	{
		switch(scancode)
		{
			//Strg Right
			case 0x1D:
				keyboard_state |= 0b00000010;
				modifiers.strg_right = 1;
				break;
			case 0x1D + RELEASED:
				keyboard_state &= 0b11111101;
				modifiers.strg_right = 0;
				break;
				
			//AltGr
			case 0x38:
				keyboard_state |= 0b00010000;
				modifiers.altgr = 1;
				break;
			case 0x38 + RELEASED:
				keyboard_state &= 0b11101111;
				modifiers.altgr = 0;
				break;
		}
	}
	else
	{
		switch(scancode)
		{
			//Shift
			case 0x2A:
				keyboard_state |= 0b00000100;
				modifiers.shift = 1;
				break;
			case 0x2A + RELEASED:
				keyboard_state &= 0b11111011;
				modifiers.shift = 0;
				break;
				
			//Shift right
			case 0x36:
				keyboard_state |= 0b00000100;
				modifiers.shift_right = 1;
				break;
			case 0x36 + RELEASED:
				keyboard_state &= 0b11111011;
				modifiers.shift_right = 0;
				break;
				
			//Strg
			case 0x1D:
				keyboard_state |= 0b00000010;
				modifiers.strg = 1;
				break;
			case 0x1D + RELEASED:
				keyboard_state &= 0b11111101;
				modifiers.strg = 0;
				break;
				
			//Alt
			case 0x38:
				keyboard_state |= 0b00000001;
				modifiers.alt = 1;
				break;
			case 0x38 + RELEASED:
				keyboard_state &= 0b11111110;
				modifiers.alt = 0;
				break;
				
			//Caps lock
			case 0x3A:
				keyboard_state ^= 0b00001000;
				modifiers.capslock = ~modifiers.capslock;
				break;
		}
	}
}
