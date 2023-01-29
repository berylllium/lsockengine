#include "core/input.h"

#include "core/event.h"

typedef struct mouse_state
{
	lise_vector2i pos;
	bool buttons[LISE_MOUSE_MAX_MOUSE_BUTTONS];
} mouse_state;

typedef struct keyboard_state
{
	bool keys[LISE_KEY_MAX_KEYS];
} keyboard_state;

static mouse_state mouse_current;
static mouse_state mouse_previous;

static keyboard_state keyboard_current;
static keyboard_state keyboard_previous;

void lise_input_update()
{
	mouse_previous = mouse_current;
	keyboard_previous = keyboard_current;
}

void lise_input_process_keys(lise_keys key, bool down)
{
	// Fire event if the state has changed
	if (keyboard_current.keys[key] != down)
	{
		keyboard_current.keys[key] = down;

		lise_event_context ctx = {};
		ctx.data.u32[0] = key;
		lise_event_fire(down ? LISE_EVENT_ON_KEY_DOWN : LISE_EVENT_ON_KEY_UP, ctx);
	}
}

void lise_input_process_button(lise_mouse_buttons button, bool down)
{
	// Fire event if the state has changed
	if (mouse_current.buttons[button] != down)
	{
		mouse_current.buttons[button] = down;

		// Fire event
		lise_event_context ctx = {};
		ctx.data.u16[0] = button;
		lise_event_fire(down ? LISE_EVENT_ON_MOUSE_BUTTON_DOWN : LISE_EVENT_ON_MOUSE_BUTTON_UP, ctx);
	}
}

void lise_input_process_mouse_move(lise_vector2i pos)
{
	if (!lise_vector2i_equals(mouse_current.pos, pos))
	{
		mouse_current.pos = pos;

		// Fire event
		lise_event_context ctx = {};
		ctx.data.i32[0] = pos.x;
		ctx.data.i32[1] = pos.y;
		lise_event_fire(LISE_EVENT_ON_MOUSE_MOVE, ctx);
	}
}

void lise_input_process_mouse_wheel(int8_t dz)
{
	// No internal state to handle.

	// Fire event
	lise_event_context ctx = {};
	ctx.data.i8[0] = dz;
	lise_event_fire(LISE_EVENT_ON_MOUSE_WHEEL_MOVE, ctx);
}

// Exported getters

bool lise_input_is_key_down(lise_keys key)
{
	return keyboard_current.keys[key];
}

bool lise_input_is_key_up(lise_keys key)
{
	return !keyboard_current.keys[key];
}

bool lise_input_was_key_down(lise_keys key)
{
	return keyboard_previous.keys[key];
}

bool lise_input_was_key_up(lise_keys key)
{
	return !keyboard_previous.keys[key];
}

bool lise_input_is_mouse_button_down(lise_mouse_buttons button)
{
	return mouse_current.buttons[button];
}

bool lise_input_is_mouse_button_up(lise_mouse_buttons button)
{
	return !mouse_current.buttons[button];
}

bool lise_input_was_mouse_button_down(lise_mouse_buttons button)
{
	return mouse_previous.buttons[button];
}

bool lise_input_was_mouse_button_up(lise_mouse_buttons button)
{
	return !mouse_previous.buttons[button];
}

lise_vector2i lise_input_get_mouse_position()
{
	return mouse_current.pos;
}

lise_vector2i lise_input_get_previous_mouse_position()
{
	return mouse_previous.pos;
}
