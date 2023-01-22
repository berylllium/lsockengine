#include "core/input.hpp"

#include "core/event.hpp"

struct mouse_state
{
	lise::vector2i pos;
	bool buttons[mouse_buttons::MOUSE_MAX_MOUSE_BUTTONS];
};

struct keyboard_state
{
	bool keys[keys::KEY_MAX_KEYS];
};

static mouse_state mouse_current;
static mouse_state mouse_previous;

static keyboard_state keyboard_current;
static keyboard_state keyboard_previous;

void input_update()
{
	mouse_previous = mouse_current;
	keyboard_previous = keyboard_current;
}

void input_process_keys(keys key, bool down)
{
	// Fire event if the state has changed
	if (keyboard_current.keys[key] != down)
	{
		keyboard_current.keys[key] = down;

		event_context ctx {};
		ctx.data.u32[0] = key;
		event_fire(down ? engine_event_codes::ON_KEY_DOWN : engine_event_codes::ON_KEY_UP, ctx);
	}
}

void input_process_button(mouse_buttons button, bool down)
{
	// Fire event if the state has changed
	if (mouse_current.buttons[button] != down)
	{
		mouse_current.buttons[button] = down;

		// Fire event
		event_context ctx {};
		ctx.data.u16[0] = button;
		event_fire(down ? engine_event_codes::ON_MOUSE_BUTTON_DOWN : engine_event_codes::ON_MOUSE_BUTTON_UP, ctx);
	}
}

void input_process_mouse_move(lise::vector2i pos)
{
	if (mouse_current.pos != pos)
	{
		mouse_current.pos = pos;

		// Fire event
		event_context ctx {};
		ctx.data.i32[0] = pos.x;
		ctx.data.i32[1] = pos.y;
		event_fire(engine_event_codes::ON_MOUSE_MOVE, ctx);
	}
}

void input_process_mouse_wheel(int8_t dz)
{
	// No internal state to handle.

	// Fire event
	event_context ctx {};
	ctx.data.i8[0] = dz;
	event_fire(engine_event_codes::ON_MOUSE_WHEEL_MOVE, ctx);
}

// Exported getters

bool input_is_key_down(keys key)
{
	return keyboard_current.keys[key];
}

bool input_is_key_up(keys key)
{
	return !keyboard_current.keys[key];
}

bool input_was_key_down(keys key)
{
	return keyboard_previous.keys[key];
}

bool input_was_key_up(keys key)
{
	return !keyboard_previous.keys[key];
}

bool input_is_mouse_button_down(mouse_buttons button)
{
	return mouse_current.buttons[button];
}

bool input_is_mouse_button_up(mouse_buttons button)
{
	return !mouse_current.buttons[button];
}

bool input_was_mouse_button_down(mouse_buttons button)
{
	return mouse_previous.buttons[button];
}

bool input_was_mouse_button_up(mouse_buttons button)
{
	return !mouse_previous.buttons[button];
}

lise::vector2i input_get_mouse_position()
{
	return mouse_current.pos;
}

lise::vector2i input_get_previous_mouse_position()
{
	return mouse_previous.pos;
}
