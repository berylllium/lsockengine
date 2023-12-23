#include "core/engine.hpp"

#include <simple-logger.hpp>

#include "core/clock.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
#include "renderer/renderer.hpp"
#include "platform/platform.hpp"

namespace lise
{

struct EngineState
{
	bool is_initialized;

	ConsumerEntryPoints entry_points;

	bool is_running;
	bool is_suspended;

	int16_t width;
	int16_t height;

	Clock delta_clock;
	double delta_time;
};

void on_window_close(uint16_t event_code, event_context ctx);

static EngineState engine_state;

bool engine_create(EngineCreateInfo app_create_info)
{
	sl::set_log_to_file(true);
	sl::set_log_time(true);

	if (engine_state.is_initialized)
	{
		sl::log_error("'engine_create' has been called more than once.");
		return false;
	}

	sl::log_info("Creating the engine / starting the engine...");

	engine_state.entry_points = app_create_info.entry_points;

	engine_state.width = app_create_info.window_width;
	engine_state.height = app_create_info.window_height;

	// Initialize subsystems
	event_init();
	
	engine_state.is_running = true;
	engine_state.is_suspended = false;

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

	engine_state.delta_clock.reset();

	if (!renderer_initialize(app_create_info.window_name))
	{
		sl::log_fatal("Failed to initialize renderer submodule.");
		return false;
	}

	// Run consumer initialization function
	if (!engine_state.entry_points.initialize())
	{
		sl::log_fatal("Consumer failed to initialize");
		return false;
	}

	// Register events
	event_add_listener(EventCodes::ON_WINDOW_CLOSE, on_window_close);

	engine_state.is_initialized = true;

	sl::log_info("Successfully created the engine / engine.");

	return true;
}

bool engine_run()
{
	sl::log_info("Starting the engine.");

	while (engine_state.is_running)
	{
		// Calculate delta time.
		engine_state.delta_time = engine_state.delta_clock.get_elapsed_time();
		engine_state.delta_clock.reset();

		if (!platform_poll_messages())
		{
			sl::log_fatal("Failed to poll platform messages");
			engine_state.is_running = false;
		}
		
		if (!engine_state.is_suspended)
		{
			if (!engine_state.entry_points.update(engine_state.delta_time))
			{
				sl::log_fatal("Consumer game update failed.");
				engine_state.is_running = false;
				break;
			}

			if (!engine_state.entry_points.render(engine_state.delta_time))
			{
				sl::log_fatal("Consumer game render failed.");
				engine_state.is_running = false;
				break;
			}

			if (!renderer_draw_frame(engine_state.delta_time))
			{
				sl::log_fatal("Failed to draw the next frame.");
				engine_state.is_running = false;
				break;
			}

			input_update();
		}
	}

	engine_state.is_running = false;

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
	engine_state.is_running = false;
}

}
