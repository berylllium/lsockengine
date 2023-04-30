#include "core/event.hpp"

#include <vector>

#include "core/logger.hpp"

namespace lise
{

typedef struct event_entry
{
	uint16_t event_code;
	std::vector<on_event_cb> listeners;
} event_entry;

static std::vector<event_entry> registered_events;
static bool initialized = false;

bool event_init()
{
	for (int i = event_codes::ON_WINDOW_CLOSE; i < event_codes::MAX_ENUM; i++)
	{
		event_register(i);
	}

	LINFO("Successfully initialized the event subsystem.");

	initialized = true;

	return true;
}

void event_shutdown()
{
	LINFO("Successfully shut down the event system.");
}

void event_register(uint16_t event_code)
{
	event_entry entry = {};
	entry.event_code = event_code;

	registered_events.push_back(entry);
}

void event_fire(uint16_t event_code, event_context ctx)
{
	for (event_entry entry : registered_events)
	{
		if (entry.event_code == event_code)
		{
			for (on_event_cb event_cb : entry.listeners)
			{
				(*event_cb)(event_code, ctx);
			}

			return;
		}
	}
}

void event_add_listener(uint16_t event_code, on_event_cb listener)
{
	for (event_entry& entry : registered_events)
	{
		if (entry.event_code == event_code)
		{
			entry.listeners.push_back(listener);

			return;
		}
	}
}

}
