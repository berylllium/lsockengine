#include "core/application.hpp"

#include "game_type.hpp"
#include "core/logger.hpp"
#include "core/event.hpp"
#include "platform/platform.hpp"

namespace lise
{

struct application_state
{
	bool is_initialized;
	game* game_instance;
	bool is_running;
	bool is_suspended;
	platform_state platform;
	int16_t width;
	int16_t height;
	double last_time;
};

void on_window_close(uint16_t event_code, event_context ctx);

static application_state app_state;

bool application_create(game* game_instance)
{
	if (app_state.is_initialized)
	{
		LERROR("'application_create' has been called more than once.");
		return false;
	}

	app_state.game_instance = game_instance;

	// Initialize subsystems
	logger_init();
	event_init();
	
	app_state.is_running = true;
	app_state.is_suspended = false;

	if (!platform_init(
				&app_state.platform,
				game_instance->app_create_config.window_name,
				game_instance->app_create_config.window_pos_x,
				game_instance->app_create_config.window_pos_y,
				game_instance->app_create_config.window_width,
				game_instance->app_create_config.window_height))
	{
		LFATAL("Error setting platform up.");
		return false;
	}

	// Run consumer initialization function
	if (!app_state.game_instance->initialize(game_instance))
	{
		LFATAL("Consumer failed to initialize");
		return false;
	}

	// Register events
	event_add_listener(engine_event_codes::ON_WINDOW_CLOSE, on_window_close);

	app_state.is_initialized = true;

	return true;
}

bool application_run()
{
	while (app_state.is_running)
	{
		if (!platform_poll_messages(&app_state.platform))
		{
			LFATAL("Failed to poll platform messages");
			app_state.is_running = false;
		}
		
		if (!app_state.is_suspended)
		{
			if (!app_state.game_instance->update(app_state.game_instance, 0.0f))
			{
				LFATAL("Consumer game update failed.");
				app_state.is_running = false;
				break;
			}

			if (!app_state.game_instance->render(app_state.game_instance, 0.0f))
			{
				LFATAL("Consumer game render failed.");
				app_state.is_running = false;
				break;
			}
		}
	}

	app_state.is_running = false;

	// Perform shutdown code
	event_shutdown();
	
	platform_shutdown(&app_state.platform);

	return true;
}

void on_window_close(uint16_t event_code, event_context ctx)
{
	app_state.is_running = false;
}

}
