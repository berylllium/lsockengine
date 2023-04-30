/**
 * @file input.hpp
 * @brief This header file contains the definitions of structures and functions relating to user input.
 */
#pragma once

#include "definitions.hpp"

#include "math/vector2.hpp"

namespace lise
{

/**
 * @brief An enum of all the supported mouse buttons.
 */
enum class mouse_buttons
{
	LEFT, RIGHT, MIDDLE,

	MAX_BUTTONS
};

/**
 * @brief An enum of all the supported keys.
 */
enum class keys {
	BACKSPACE = 0x08,
	ENTER = 0x0D,
	TAB = 0x09,
	SHIFT = 0x10,
	CONTROL = 0x11,

	PAUSE = 0x13,
	CAPITAL = 0x14,

	ESCAPE = 0x1B,

	CONVERT = 0x1C,
	NONCONVERT = 0x1D,
	ACCEPT = 0x1E,
	MODECHANGE = 0x1F,

	SPACE = 0x20,
	PRIOR = 0x21,
	NEXT = 0x22,
	END = 0x23,
	HOME = 0x24,
	LEFT = 0x25,
	UP = 0x26,
	RIGHT = 0x27,
	DOWN = 0x28,
	SELECT = 0x29,
	PRINT = 0x2A,
	EXECUTE = 0x2B,
	SNAPSHOT = 0x2C,
	INSERT = 0x2D,
	DELETE = 0x2E,
	HELP = 0x2F,

	A = 0x41,
	B = 0x42,
	C = 0x43,
	D = 0x44,
	E = 0x45,
	F = 0x46,
	G = 0x47,
	H = 0x48,
	I = 0x49,
	J = 0x4A,
	K = 0x4B,
	L = 0x4C,
	M = 0x4D,
	N = 0x4E,
	O = 0x4F,
	P = 0x50,
	Q = 0x51,
	R = 0x52,
	S = 0x53,
	T = 0x54,
	U = 0x55,
	V = 0x56,
	W = 0x57,
	X = 0x58,
	Y = 0x59,
	Z = 0x5A,

	LWIN = 0x5B,
	RWIN = 0x5C,
	APPS = 0x5D,

	SLEEP = 0x5F,

	NUMPAD0 = 0x60,
	NUMPAD1 = 0x61,
	NUMPAD2 = 0x62,
	NUMPAD3 = 0x63,
	NUMPAD4 = 0x64,
	NUMPAD5 = 0x65,
	NUMPAD6 = 0x66,
	NUMPAD7 = 0x67,
	NUMPAD8 = 0x68,
	NUMPAD9 = 0x69,
	MULTIPLY = 0x6A,
	ADD = 0x6B,
	SEPARATOR = 0x6C,
	SUBTRACT = 0x6D,
	DECIMAL = 0x6E,
	DIVIDE = 0x6F,
	F1 = 0x70,
	F2 = 0x71,
	F3 = 0x72,
	F4 = 0x73,
	F5 = 0x74,
	F6 = 0x75,
	F7 = 0x76,
	F8 = 0x77,
	F9 = 0x78,
	F10 = 0x79,
	F11 = 0x7A,
	F12 = 0x7B,
	F13 = 0x7C,
	F14 = 0x7D,
	F15 = 0x7E,
	F16 = 0x7F,
	F17 = 0x80,
	F18 = 0x81,
	F19 = 0x82,
	F20 = 0x83,
	F21 = 0x84,
	F22 = 0x85,
	F23 = 0x86,
	F24 = 0x87,

	NUMLOCK = 0x90,
	SCROLL = 0x91,

	NUMPAD_EQUAL = 0x92,

	LSHIFT = 0xA0,
	RSHIFT = 0xA1,
	LCONTROL = 0xA2,
	RCONTROL = 0xA3,
	LMENU = 0xA4,
	RMENU = 0xA5,

	SEMICOLON = 0xBA,
	PLUS = 0xBB,
	COMMA = 0xBC,
	MINUS = 0xBD,
	PERIOD = 0xBE,
	SLASH = 0xBF,
	GRAVE = 0xC0,

	MAX_KEYS
};

/**
 * @brief Gets called by the engine every frame to update the input backend.
 */
void input_update();

// Keyboard input

/**
 * @brief Returns whether a key is down.
 * 
 * @param key The key to check.
 * @return true if the key is down.
 * @return false if the key is up
 */
LAPI bool input_is_key_down(keys key);

/**
 * @brief Returns whether a key is up.
 * 
 * @param key The key to check.
 * @return true if the key is up.
 * @return false if the key is down.
 */
LAPI bool input_is_key_up(keys key);

/**
 * @brief Returns whether a key was down the last frame.
 * 
 * @param key The key to check.
 * @return true if the key was down.
 * @return false if the key was up.
 */
LAPI bool input_was_key_down(keys key);

/**
 * @brief Returns whether a key was up the last frame.
 * 
 * @param key The key to check.
 * @return true if the key was up
 * @return false if the key was down.
 */
LAPI bool input_was_key_up(keys key);

/**
 * @brief Processes input from the platform backend.
 * 
 * Used only internally.
 * 
 * @param key The key to process.
 * @param down Whether the key is up or down.
 */
void input_process_keys(keys key, bool down);

// Mouse input

/**
 * @brief Checks whether the given mouse button is down.
 * 
 * @param button The mouse button to check.
 * @return true if the mouse button is down.
 * @return false if the mouse button is up.
 */
LAPI bool input_is_mouse_button_down(mouse_buttons button);

/**
 * @brief Checks whether the given mouse button is up.
 * 
 * @param button The mouse button to check.
 * @return true if the mouse button is up.
 * @return false if the mouse button is down.
 */
LAPI bool input_is_mouse_button_up(mouse_buttons button);

/**
 * @brief Checks whether the given mouse button was down in the last frame.
 * 
 * @param button The mouse button to check.
 * @return true the mouse button was down in the last frame.
 * @return false the mouse button was up in the last frame.
 */
LAPI bool input_was_mouse_button_down(mouse_buttons button);

/**
 * @brief Checks whether the given mouse button was up in the last frame.
 * 
 * @param button The mouse button to check.
 * @return true the mouse button was up in the last frame.
 * @return false the mouse button was down in the last frame.
 */
LAPI bool input_was_mouse_button_up(mouse_buttons button);

/**
 * @brief Returns the coordinates of the mouse on the screen in pixels.
 * 
 * @return vec2i The coordinates of the mouse in pixels.
 */
LAPI vector2i input_get_mouse_position();

/**
 * @brief Returns the coordinates of the mouse on the screen in the last frame in pixels.
 * 
 * @return vec2i The coordinates of the mouse in the last frame in pixels.
 */
LAPI vector2i input_get_previous_mouse_position();

void input_process_button(mouse_buttons button, bool down);
void input_process_mouse_move(vector2i pos);
void input_process_mouse_wheel(int8_t dz);

}
