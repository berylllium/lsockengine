#include "core/application.h"

#include "core/logger.h"
#include "core/clock.h"
#include "core/event.h"
#include "core/input.h"
#include "renderer/renderer.h"
#include "platform/platform.h"

typedef struct application_state
{
	bool is_initialized;

	lise_game_entry_points entry_points;

	bool is_running;
	bool is_suspended;

	lise_platform_state platform;

	int16_t width;
	int16_t height;

	lise_clock delta_clock;
	double delta_time;
} application_state;

void on_window_close(uint16_t event_code, lise_event_context ctx);

static application_state app_state;

bool lise_application_create(lise_application_create_info app_create_info)
{
	if (app_state.is_initialized)
	{
		LERROR("'application_create' has been called more than once.");
		return false;
	}

	app_state.entry_points = app_create_info.entry_points;

	app_state.width = app_create_info.window_width;
	app_state.height = app_create_info.window_height;

	lise_clock_reset(&app_state.delta_clock);

	// Initialize subsystems
	lise_logger_init();
	lise_event_init();
	
	app_state.is_running = true;
	app_state.is_suspended = false;

	if (!lise_platform_init(
				&app_state.platform,
				app_create_info.window_name,
				app_create_info.window_pos_x,
				app_create_info.window_pos_y,
				app_create_info.window_width,
				app_create_info.window_height))
	{
		LFATAL("Error setting platform up.");
		return false;
	}

	if (!lise_renderer_initialize(app_create_info.window_name))
	{
		LFATAL("Failed to initialize renderer submodule.");
		return false;
	}

	// Run consumer initialization function
	if (!app_state.entry_points.initialize())
	{
		LFATAL("Consumer failed to initialize");
		return false;
	}

	// Register events
	lise_event_add_listener(LISE_EVENT_ON_WINDOW_CLOSE, on_window_close);

	app_state.is_initialized = true;

	return true;
}

bool lise_application_run()
{
	while (app_state.is_running)
	{
		// Calculate delta time.
		app_state.delta_time = lise_clock_get_elapsed_time(app_state.delta_clock);
		lise_clock_reset(&app_state.delta_clock);

		if (!lise_platform_poll_messages(&app_state.platform))
		{
			LFATAL("Failed to poll platform messages");
			app_state.is_running = false;
		}
		
		if (!app_state.is_suspended)
		{
			if (!app_state.entry_points.update(app_state.delta_time))
			{
				LFATAL("Consumer game update failed.");
				app_state.is_running = false;
				break;
			}

			if (!app_state.entry_points.render(app_state.delta_time))
			{
				LFATAL("Consumer game render failed.");
				app_state.is_running = false;
				break;
			}

			lise_input_update();
		}
	}

	app_state.is_running = false;

	// Perform shutdown code
	lise_event_shutdown();
	lise_logger_shutdown();

	lise_renderer_shutdown();
	
	lise_platform_shutdown(&app_state.platform);

	return true;
}

void on_window_close(uint16_t event_code, lise_event_context ctx)
{
	app_state.is_running = false;
}
