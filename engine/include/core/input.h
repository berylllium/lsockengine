#pragma once

#include "definitions.h"

#include "container/vector2.h"

typedef enum lise_mouse_buttons
{
	LISE_MOUSE_LEFT, LISE_MOUSE_RIGHT, LISE_MOUSE_MIDDLE,

	LISE_MOUSE_MAX_MOUSE_BUTTONS
} lise_mouse_buttons;

typedef enum lise_keys {
	LISE_KEY_BACKSPACE = 0x08,
	LISE_KEY_ENTER = 0x0D,
	LISE_KEY_TAB = 0x09,
	LISE_KEY_SHIFT = 0x10,
	LISE_KEY_CONTROL = 0x11,

	LISE_KEY_PAUSE = 0x13,
	LISE_KEY_CAPITAL = 0x14,

	LISE_KEY_ESCAPE = 0x1B,

	LISE_KEY_CONVERT = 0x1C,
	LISE_KEY_NONCONVERT = 0x1D,
	LISE_KEY_ACCEPT = 0x1E,
	LISE_KEY_MODECHANGE = 0x1F,

	LISE_KEY_SPACE = 0x20,
	LISE_KEY_PRIOR = 0x21,
	LISE_KEY_NEXT = 0x22,
	LISE_KEY_END = 0x23,
	LISE_KEY_HOME = 0x24,
	LISE_KEY_LEFT = 0x25,
	LISE_KEY_UP = 0x26,
	LISE_KEY_RIGHT = 0x27,
	LISE_KEY_DOWN = 0x28,
	LISE_KEY_SELECT = 0x29,
	LISE_KEY_PRINT = 0x2A,
	LISE_KEY_EXECUTE = 0x2B,
	LISE_KEY_SNAPSHOT = 0x2C,
	LISE_KEY_INSERT = 0x2D,
	LISE_KEY_DELETE = 0x2E,
	LISE_KEY_HELP = 0x2F,

	LISE_KEY_A = 0x41,
	LISE_KEY_B = 0x42,
	LISE_KEY_C = 0x43,
	LISE_KEY_D = 0x44,
	LISE_KEY_E = 0x45,
	LISE_KEY_F = 0x46,
	LISE_KEY_G = 0x47,
	LISE_KEY_H = 0x48,
	LISE_KEY_I = 0x49,
	LISE_KEY_J = 0x4A,
	LISE_KEY_K = 0x4B,
	LISE_KEY_L = 0x4C,
	LISE_KEY_M = 0x4D,
	LISE_KEY_N = 0x4E,
	LISE_KEY_O = 0x4F,
	LISE_KEY_P = 0x50,
	LISE_KEY_Q = 0x51,
	LISE_KEY_R = 0x52,
	LISE_KEY_S = 0x53,
	LISE_KEY_T = 0x54,
	LISE_KEY_U = 0x55,
	LISE_KEY_V = 0x56,
	LISE_KEY_W = 0x57,
	LISE_KEY_X = 0x58,
	LISE_KEY_Y = 0x59,
	LISE_KEY_Z = 0x5A,

	LISE_KEY_LWIN = 0x5B,
	LISE_KEY_RWIN = 0x5C,
	LISE_KEY_APPS = 0x5D,

	LISE_KEY_SLEEP = 0x5F,

	LISE_KEY_NUMPAD0 = 0x60,
	LISE_KEY_NUMPAD1 = 0x61,
	LISE_KEY_NUMPAD2 = 0x62,
	LISE_KEY_NUMPAD3 = 0x63,
	LISE_KEY_NUMPAD4 = 0x64,
	LISE_KEY_NUMPAD5 = 0x65,
	LISE_KEY_NUMPAD6 = 0x66,
	LISE_KEY_NUMPAD7 = 0x67,
	LISE_KEY_NUMPAD8 = 0x68,
	LISE_KEY_NUMPAD9 = 0x69,
	LISE_KEY_MULTIPLY = 0x6A,
	LISE_KEY_ADD = 0x6B,
	LISE_KEY_SEPARATOR = 0x6C,
	LISE_KEY_SUBTRACT = 0x6D,
	LISE_KEY_DECIMAL = 0x6E,
	LISE_KEY_DIVIDE = 0x6F,
	LISE_KEY_F1 = 0x70,
	LISE_KEY_F2 = 0x71,
	LISE_KEY_F3 = 0x72,
	LISE_KEY_F4 = 0x73,
	LISE_KEY_F5 = 0x74,
	LISE_KEY_F6 = 0x75,
	LISE_KEY_F7 = 0x76,
	LISE_KEY_F8 = 0x77,
	LISE_KEY_F9 = 0x78,
	LISE_KEY_F10 = 0x79,
	LISE_KEY_F11 = 0x7A,
	LISE_KEY_F12 = 0x7B,
	LISE_KEY_F13 = 0x7C,
	LISE_KEY_F14 = 0x7D,
	LISE_KEY_F15 = 0x7E,
	LISE_KEY_F16 = 0x7F,
	LISE_KEY_F17 = 0x80,
	LISE_KEY_F18 = 0x81,
	LISE_KEY_F19 = 0x82,
	LISE_KEY_F20 = 0x83,
	LISE_KEY_F21 = 0x84,
	LISE_KEY_F22 = 0x85,
	LISE_KEY_F23 = 0x86,
	LISE_KEY_F24 = 0x87,

	LISE_KEY_NUMLOCK = 0x90,
	LISE_KEY_SCROLL = 0x91,

	LISE_KEY_NUMPAD_EQUAL = 0x92,

	LISE_KEY_LSHIFT = 0xA0,
	LISE_KEY_RSHIFT = 0xA1,
	LISE_KEY_LCONTROL = 0xA2,
	LISE_KEY_RCONTROL = 0xA3,
	LISE_KEY_LMENU = 0xA4,
	LISE_KEY_RMENU = 0xA5,

	LISE_KEY_SEMICOLON = 0xBA,
	LISE_KEY_PLUS = 0xBB,
	LISE_KEY_COMMA = 0xBC,
	LISE_KEY_MINUS = 0xBD,
	LISE_KEY_PERIOD = 0xBE,
	LISE_KEY_SLASH = 0xBF,
	LISE_KEY_GRAVE = 0xC0,

	LISE_KEY_MAX_KEYS
} lise_keys;

void lise_input_update();

// Keyboard input

LAPI bool lise_input_is_key_down(lise_keys key);
LAPI bool lise_input_is_key_up(lise_keys key);
LAPI bool lise_input_was_key_down(lise_keys key);
LAPI bool lise_input_was_key_up(lise_keys key);

void lise_input_process_keys(lise_keys key, bool down);

// Mouse input

LAPI bool lise_input_is_mouse_button_down(lise_mouse_buttons button);
LAPI bool lise_input_is_mouse_button_up(lise_mouse_buttons button);
LAPI bool lise_input_was_mouse_button_down(lise_mouse_buttons button);
LAPI bool lise_input_was_mouse_button_up(lise_mouse_buttons button);
LAPI lise_vector2i lise_input_get_mouse_position();
LAPI lise_vector2i lise_input_get_previous_mouse_position();

void lise_input_process_button(lise_mouse_buttons button, bool down);
void lise_input_process_mouse_move(lise_vector2i pos);
void lise_input_process_mouse_wheel(int8_t dz);
