#ifndef KEYBOARD
#define KEYBOARD

#define RELEASED 0x80

enum SPECIAL_KEYS
{
	None,
	ESC,
	Backspace,
	Delete,
	Enter,
	Up,
	Down,
	Left,
	Right,
	Next,
	Prior,
	End,
	Home,
	Insert,
	Apps,
	Windows_Right,
	Windows_Left,
	Pause_Break,
	Print
};

typedef struct 
{
	unsigned char alt : 1;
	unsigned char strg : 1;
	unsigned char shift : 1;
	unsigned char capslock : 1;
	unsigned char altgr : 1;
	unsigned char strg_right : 1;
	unsigned char shift_right : 1;
} modifiers_t;

typedef struct
{
	char ascii;
	enum SPECIAL_KEYS sp;
	modifiers_t mod;
} keypress_t;

typedef void (*key_event)(char, enum SPECIAL_KEYS, modifiers_t*);

void init_keyboard();

keypress_t read_key();

#endif
