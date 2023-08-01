#include "core/engine.hpp"

#include <simple-logger.hpp>

#include "core/clock.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
#include "renderer/renderer.hpp"
#include "platform/platform.hpp"

namespace lise
{

struct engine_state
{
	bool is_initialized;

	ConsumerEntryPoints entry_points;

	bool is_running;
	bool is_suspended;

	int16_t width;
	int16_t height;

	clock delta_clock;
	double delta_time;
};

void on_window_close(uint16_t event_code, event_context ctx);

static engine_state app_state;

bool engine_create(EngineCreateInfo app_create_info)
{
	if (app_state.is_initialized)
	{
		sl::log_error("'engine_create' has been called more than once.");
		return false;
	}

	sl::log_info("Creating the engine / starting the engine...");

	app_state.entry_points = app_create_info.entry_points;

	app_state.width = app_create_info.window_width;
	app_state.height = app_create_info.window_height;

	// Initialize subsystems
	sl::set_log_to_file(true);
	sl::set_log_time(true);

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
		sl::log_fatal("Error setting platform up.");
		return false;
	}

	app_state.delta_clock.reset();

	if (!renderer_initialize(app_create_info.window_name))
	{
		sl::log_fatal("Failed to initialize renderer submodule.");
		return false;
	}

	// Run consumer initialization function
	if (!app_state.entry_points.initialize())
	{
		sl::log_fatal("Consumer failed to initialize");
		return false;
	}

	// Register events
	event_add_listener(event_codes::ON_WINDOW_CLOSE, on_window_close);

	app_state.is_initialized = true;

	sl::log_info("Successfully created the engine / engine.");

	return true;
}

bool engine_run()
{
	sl::log_info("Starting the engine.");

	while (app_state.is_running)
	{
		// Calculate delta time.
		app_state.delta_time = app_state.delta_clock.get_elapsed_time();
		app_state.delta_clock.reset();

		if (!platform_poll_messages())
		{
			sl::log_fatal("Failed to poll platform messages");
			app_state.is_running = false;
		}
		
		if (!app_state.is_suspended)
		{
			if (!app_state.entry_points.update(app_state.delta_time))
			{
				sl::log_fatal("Consumer game update failed.");
				app_state.is_running = false;
				break;
			}

			if (!app_state.entry_points.render(app_state.delta_time))
			{
				sl::log_fatal("Consumer game render failed.");
				app_state.is_running = false;
				break;
			}

			if (!renderer_draw_frame(app_state.delta_time))
			{
				sl::log_fatal("Failed to draw the next frame.");
				app_state.is_running = false;
				break;
			}

			input_update();
		}
	}

	app_state.is_running = false;

	// Perform shutdown code
	sl::log_info("Shutting down the engine...");

	event_shutdown();

	renderer_shutdown();
	
	platform_shutdown();

	sl::log_info("Successfully shut down the engine.");

	return true;
}

void on_window_close(uint16_t event_code, event_context ctx)
{
	app_state.is_running = false;
}

}
