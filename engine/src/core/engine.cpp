#include "core/engine.hpp"

#include "core/logger.hpp"
#include "core/clock.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
//#include "renderer/renderer.h"
#include "platform/platform.hpp"

namespace lise
{

struct engine_state
{
	bool is_initialized;

	consumer_entry_points entry_points;

	bool is_running;
	bool is_suspended;

	int16_t width;
	int16_t height;

	clock delta_clock;
	double delta_time;
};

void on_window_close(uint16_t event_code, event_context ctx);

static engine_state app_state;

bool engine_create(engine_create_info app_create_info)
{
	if (app_state.is_initialized)
	{
		LERROR("'engine_create' has been called more than once.");
		return false;
	}

	LINFO("Creating the engine / starting the engine...");

	app_state.entry_points = app_create_info.entry_points;

	app_state.width = app_create_info.window_width;
	app_state.height = app_create_info.window_height;

	// Initialize subsystems
	logger_init();
	event_init();
	
	app_state.is_running = true;
	app_state.is_suspended = false;

	if (!platform_init(
		app_create_info.window_name,
		app_create_info.window_pos_x,
		app_create_info.window_pos_y,
		app_create_info.window_width,
		app_create_info.window_height
	))
	{
		LFATAL("Error setting platform up.");
		return false;
	}

	app_state.delta_clock.reset();

//	if (!renderer_initialize(app_create_info.window_name))
//	{
//		LFATAL("Failed to initialize renderer submodule.");
//		return false;
//	}

	// Run consumer initialization function
	if (!app_state.entry_points.initialize())
	{
		LFATAL("Consumer failed to initialize");
		return false;
	}

	// Register events
	event_add_listener(event_codes::ON_WINDOW_CLOSE, on_window_close);

	app_state.is_initialized = true;

	LINFO("Successfully created the engine / engine.");

	return true;
}

bool engine_run()
{
	LINFO("Starting the engine.");

	while (app_state.is_running)
	{
		// Calculate delta time.
		app_state.delta_time = app_state.delta_clock.get_elapsed_time();
		app_state.delta_clock.reset();

		if (!platform_poll_messages())
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

//			if (!renderer_draw_frame(app_state.delta_time))
//			{
//				LFATAL("Failed to draw the next frame.");
//				app_state.is_running = false;
//				break;
//			}

			input_update();
		}
	}

	app_state.is_running = false;

	// Perform shutdown code
	LINFO("Shutting down the engine...");

	event_shutdown();
	logger_shutdown();

//	renderer_shutdown();
	
	platform_shutdown();

	LINFO("Successfully shut down the engine.");

	return true;
}

void on_window_close(uint16_t event_code, event_context ctx)
{
	app_state.is_running = false;
}

}
