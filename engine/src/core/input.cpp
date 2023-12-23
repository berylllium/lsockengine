#include "core/input.hpp"

#include "core/event.hpp"

namespace lise
{

struct mouse_state
{
	vector2i pos;
	bool buttons[static_cast<int>(MouseButtons::MAX_BUTTONS)];
};

struct keyboard_state
{
	bool keys_pressed[static_cast<int>(Keys::MAX_KEYS)];
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

void input_process_keys(Keys key, bool down)
{
	// Fire event if the state has changed
	if (keyboard_current.keys_pressed[static_cast<int>(key)] != down)
	{
		keyboard_current.keys_pressed[static_cast<int>(key)] = down;

		event_context ctx = {};
		ctx.data.u32[0] = static_cast<uint32_t>(key);
		event_fire(down ? EventCodes::ON_KEY_DOWN : EventCodes::ON_KEY_UP, ctx);
	}
}

void input_process_button(MouseButtons button, bool down)
{
	// Fire event if the state has changed
	if (mouse_current.buttons[static_cast<int>(button)] != down)
	{
		mouse_current.buttons[static_cast<int>(button)] = down;

		// Fire event
		event_context ctx = {};
		ctx.data.u16[0] = static_cast<uint16_t>(button);
		event_fire(down ? EventCodes::ON_MOUSE_BUTTON_DOWN : EventCodes::ON_MOUSE_BUTTON_UP, ctx);
	}
}

void input_process_mouse_move(vector2i pos)
{
	if (mouse_current.pos != pos)
	{
		mouse_current.pos = pos;

		// Fire event
		event_context ctx = {};
		ctx.data.i32[0] = pos.x;
		ctx.data.i32[1] = pos.y;
		event_fire(EventCodes::ON_MOUSE_MOVE, ctx);
	}
}

void input_process_mouse_wheel(int8_t dz)
{
	// No internal state to handle.

	// Fire event
	event_context ctx = {};
	ctx.data.i8[0] = dz;
	event_fire(EventCodes::ON_MOUSE_WHEEL_MOVE, ctx);
}

// Exported getters

bool input_is_key_down(Keys key)
{
	return keyboard_current.keys_pressed[static_cast<uint32_t>(key)];
}

bool input_is_key_up(Keys key)
{
	return !keyboard_current.keys_pressed[static_cast<uint32_t>(key)];
}

bool input_was_key_down(Keys key)
{
	return keyboard_previous.keys_pressed[static_cast<uint32_t>(key)];
}

bool input_was_key_up(Keys key)
{
	return !keyboard_previous.keys_pressed[static_cast<uint32_t>(key)];
}

bool input_is_mouse_button_down(MouseButtons button)
{
	return mouse_current.buttons[static_cast<uint16_t>(button)];
}

bool input_is_mouse_button_up(MouseButtons button)
{
	return !mouse_current.buttons[static_cast<uint16_t>(button)];
}

bool input_was_mouse_button_down(MouseButtons button)
{
	return mouse_previous.buttons[static_cast<uint16_t>(button)];
}

bool input_was_mouse_button_up(MouseButtons button)
{
	return !mouse_previous.buttons[static_cast<uint16_t>(button)];
}

vector2i input_get_mouse_position()
{
	return mouse_current.pos;
}

vector2i input_get_previous_mouse_position()
{
	return mouse_previous.pos;
}

}
