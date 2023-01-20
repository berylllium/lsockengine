#include "core/event.hpp"

#include <vector>

struct event_entry
{
	uint16_t event_code;
	std::vector<on_event_cb> listeners;
};

static std::vector<event_entry> REGISTERED_EVENTS;
static bool INITIALIZED = false;

bool event_init()
{
	for (int i = engine_event_codes::ON_WINDOW_CLOSE; i < engine_event_codes::ON_WINDOW_RESIZE; i++)
	{
		register_event(i);
	}

	return true;
}

void event_shutdown()
{

}

void register_event(uint16_t event_code)
{
	event_entry entry {};
	entry.event_code = event_code;

	REGISTERED_EVENTS.push_back(entry);
}

void fire_event(uint16_t event_code, event_context ctx)
{
	for (int entry = 0; entry < REGISTERED_EVENTS.size(); entry++)
	{
		if (REGISTERED_EVENTS[entry].event_code == event_code)
		{
			for (int listener = 0; listener < REGISTERED_EVENTS[entry].listeners.size(); listener++)
			{
				REGISTERED_EVENTS[entry].listeners[listener](event_code, ctx);
			}

			return;
		}
	}
}

void add_listener(uint16_t event_code, on_event_cb listener)
{
	for (int entry = 0; entry < REGISTERED_EVENTS.size(); entry++)
	{
		if (REGISTERED_EVENTS[entry].event_code == event_code)
		{
			REGISTERED_EVENTS[entry].listeners.push_back(listener);
		}
	}
}
