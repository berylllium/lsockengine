#include "core/event.h"

#include "core/logger.h"
#include "container/darray.h"

typedef struct lise_event_entry
{
	uint16_t event_code;
	blib_darray listeners; // lise_on_event_cb
} lise_event_entry;

static blib_darray registered_events; // lise_event_entry
static bool INITIALIZED = false;

bool lise_event_init()
{
	registered_events = blib_darray_create(lise_event_entry);

	for (int i = LISE_EVENT_ON_WINDOW_CLOSE; i < LISE_EVENT_MAX_ENUM; i++)
	{
		lise_event_register(i);
	}

	LINFO("Successfully initialized the event subsystem.");

	return true;
}

void lise_event_shutdown()
{
	// Free listeners darrays
	for (int entry_index = 0; entry_index < registered_events.size; entry_index++)
	{
		lise_event_entry* entry = blib_darray_get(&registered_events, entry_index);

		blib_darray_free(&entry->listeners);
	}

	blib_darray_free(&registered_events);

	LINFO("Successfully shut down the event system.");
}

void lise_event_register(uint16_t event_code)
{
	lise_event_entry entry = {};
	entry.event_code = event_code;
	entry.listeners = blib_darray_create(lise_on_event_cb);

	blib_darray_push_back(&registered_events, &entry);
}

void lise_event_fire(uint16_t event_code, lise_event_context ctx)
{
	for (int entry_index = 0; entry_index < registered_events.size; entry_index++)
	{
		lise_event_entry* entry = blib_darray_get(&registered_events, entry_index);

		if (entry->event_code == event_code)
		{
			for (int listener = 0; listener < entry->listeners.size; listener++)
			{
				lise_on_event_cb* event_cb = blib_darray_get(&entry->listeners, listener);

				(*event_cb)(event_code, ctx);
			}

			return;
		}
	}
}

void lise_event_add_listener(uint16_t event_code, lise_on_event_cb listener)
{
	for (int entry_index = 0; entry_index < registered_events.size; entry_index++)
	{
		lise_event_entry* entry = blib_darray_get(&registered_events, entry_index);

		if (entry->event_code == event_code)
		{
			blib_darray_push_back(&entry->listeners, &listener);

			return;
		}
	}
}
