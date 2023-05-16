#include "keyboard.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"
#include "../libc/function.h"
#include "../kernel/kernel.h"

#define ESCAPE 0x01
#define BACKSPACE 0x0E
#define TAB 0x0F
#define ENTER 0x1C /* For both enters. */
#define CTRL 0x1D /* For both controls. */
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LALT 0x38
#define CAPSLOCK 0x3A
#define F1 0x3A
#define F2 0x3C
#define F3 0x3D
#define F4 0x3E
#define F5 0x3F
#define F6 0x40
#define F7 0x41
#define F8 0x42
#define F9 0x43
#define F10 0x44
#define NUMLOCK 0x45
#define SCRLOCK 0x46
#define NUMPAD7 0x47 /* Some numpad keys and arrow keys are the same. */
#define UP 0x48
#define NUMPAD9 0x49
#define NUMPADDASH 0x4A
#define LEFT 0x4B
#define NUMPAD5 0x4C
#define RIGHT 0x4D
#define NUMPADPLUS 0x4E
#define NUMPAD1 0x4F
#define DIWN 0x50
#define NUMPAD3 0x51
#define INSERT 0x52
#define DELETE 0x53
#define F11 0x57
#define F12 0x58
#define PRTSCR 0x37
#define RALT 0x38
#define HOME 0x47
#define PGUP 0x49
#define END 0x4F
#define DOWN 0x50
#define PGDN 0x51
#define NUMPAD0 0x52
#define DEL 0x53
#define PAUSE 0x1D
#define NOCHAR 0x00

static char key_buffer[256];

uint8_t NUM_LOCK_ENABLED = 0;
uint8_t SCR_LOCK_ENABLED = 0;
uint8_t CAPS_LOCK_ENABLED = 0;

uint8_t keys_down[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define SC_MAX 119
const char *sc_name[] = { "ERROR", "Esc", "1", "2", "3", "4", "5", "6",
    "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E",
        "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "LCtrl",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
        "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
        "/", "RShift", "Keypad *", "LAlt", "Spacebar", "Caps", "F1",
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num Lock",
	"Scroll Lock", "Keypad 7", "Keypad 8", "Keypad 9",
	"Keypad -", "Keypad 4", "Keypad 5", "Keypad 6", "Keypad +",
	"Keypad 1", "Keypad 2", "Keypad 3", "Insert", "Delete", "", "", "",
	"F11", "F12", "", "", "", "", "", "", "", "Keypad Enter",
	 "RCtrl", "Keypad /", "PrtScr", "RAlt", "", "Home",
	"Up", "Page Up", "Left", "Right", "End", "Down", "PgDn", "Insert",
	"Del", "", "", "", "", "", "", "", "Pause"};

const char ascii_low[] = { '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
        'u', 'i', 'o', 'p', '[', ']', '?', '?', 'a', 's', 'd', 'f', 'g',
        'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', '?', '*', '?', ' ', '?', '?', '?', '?',
	'?', '?', '?', '?', '?', '?', '?', '?', '?', '7', '8', '9', '-', '4',
	'5', '6', '+', '1', '2', '3', '?', '?', '?', '?', '?', '?', '?', '?',
	'?', '?', '?', '?', '?', '?', '?', '?', '/'};

const char ascii_high[] = {'?', '?', '!', '@', '#', '$', '%', '^', '&', '*',
	'(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K',
	'L', ':', '\"', '~', '?', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
	'>', '?', '?', '*', '?', ' '};

static void keyboard_callback(registers_t *regs) {
	/* The PIC leaves us the scancode in port 0x60 */
	uint8_t scancode = port_byte_in(0x60);
	char letter = '?';
	char lowerletter = '?';
	char str[2] = {'?', '\0'};
	char lowerstr[2] = {'?', '\0'};
	if(scancode <= SC_MAX){
		letter = ascii_high[(int)scancode];
		lowerletter = ascii_low[(int)scancode];
		str[0] = letter;
		str[1] = '\0';
		lowerstr[0] = lowerletter;
		lowerstr[1] = '\0';
	}
	switch(scancode){
		case BACKSPACE:
			backspace(key_buffer);
			kprint_backspace();
			break;

		case TAB:
			kprint("     ");
			append(key_buffer, ' ');
			append(key_buffer, ' ');
			append(key_buffer, ' ');
			append(key_buffer, ' ');
			append(key_buffer, ' ');
			break;

		case ENTER:
			kprint("\n");
			user_input(key_buffer);
			key_buffer[0] = '\0';
			break;

		case LSHIFT:
			keys_down[(int)scancode] = 1;
			break;

		case RSHIFT:
			keys_down[(int)scancode] = 1;
			break;

		case CTRL:
			keys_down[(int)scancode] = 1;
			break;

		case LEFT:
			if(NUM_LOCK_ENABLED == 0) {
				set_cursor_offset(get_cursor_offset() - 2);
				break;
			}

		case RIGHT:
			if(NUM_LOCK_ENABLED == 0) {
				set_cursor_offset(get_cursor_offset() + 2);
				break;
			}

		case UP:
			break;

		case DOWN:
			break;

		case NUMLOCK:
			if(NUM_LOCK_ENABLED == 1) {
				NUM_LOCK_ENABLED == 0;
				break;
			 }
			NUM_LOCK_ENABLED = 1;
			break;

		case SCRLOCK:
			if(SCR_LOCK_ENABLED == 1) {
				SCR_LOCK_ENABLED == 0;
				break;
			}
			SCR_LOCK_ENABLED == 1;
			break;

		case CAPSLOCK:
			if(CAPS_LOCK_ENABLED == 1) {
				CAPS_LOCK_ENABLED == 0;
				break;
			}
			CAPS_LOCK_ENABLED == 1;
			break;

		default:
			if(scancode > SC_MAX){
				int val = (int)scancode - 0x80;
				if(SC_MAX <= 119){
					keys_down[val] = 0;
					break;
				} else {
					break;
				}
			}
			if(keys_down[(int)LSHIFT] || keys_down[(int)RSHIFT]){
				append(key_buffer, letter);
 				kprint(str);
			} else {
				append(key_buffer, lowerletter);
				kprint(lowerstr);
			}
			keys_down[(int)scancode] = 1;
	}
	UNUSED(regs);
}

void init_keyboard() {
   register_interrupt_handler(IRQ1, keyboard_callback);
}
