#include "core/event.hpp"

#include <vector>

#include <simple-logger.hpp>

namespace lise
{

struct EventEntry
{
	uint16_t event_code;
	std::vector<on_event_cb> listeners;
};

static std::vector<EventEntry> registered_events;
static bool initialized = false;

bool event_init()
{
	for (int i = EventCodes::ON_WINDOW_CLOSE; i < EventCodes::MAX_ENUM; i++)
	{
		event_register(i);
	}

	sl::log_info("Successfully initialized the event subsystem.");

	initialized = true;

	return true;
}

void event_shutdown()
{
	sl::log_info("Successfully shut down the event system.");
}

void event_register(uint16_t event_code)
{
	EventEntry entry = {};
	entry.event_code = event_code;

	registered_events.push_back(entry);
}

void event_fire(uint16_t event_code, event_context ctx)
{
	for (auto& entry : registered_events)
	{
		if (entry.event_code == event_code)
		{
			for (auto& event_cb : entry.listeners)
			{
				(*event_cb)(event_code, ctx);
			}

			return;
		}
	}
}

void event_add_listener(uint16_t event_code, on_event_cb listener)
{
	for (auto& entry : registered_events)
	{
		if (entry.event_code == event_code)
		{
			entry.listeners.push_back(listener);

			return;
		}
	}
}

}
