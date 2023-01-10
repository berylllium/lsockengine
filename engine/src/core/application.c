#include "core/application.h"

#include "game_type.h"
#include "core/logger.h"
#include "platform/platform.h"

typedef struct application_state
{
	int8_t is_initialized;
	game* game_instance;
	int8_t is_running;
	int8_t is_suspended;
	platform_state platform;
	int16_t width;
	int16_t height;
	double last_time;
} application_state;

static application_state app_state;

int8_t application_create(game* game_instance)
{
	if (app_state.is_initialized)
	{
		LERROR("'application_create' has been called more than once.");
		return 0;
	}

	app_state.game_instance = game_instance;

	// Initialize subsystems
	logger_init();
	
	app_state.is_running = 1;
	app_state.is_suspended = 0;

	if (!platform_init(
				&app_state.platform,
				game_instance->app_create_config.window_name,
				game_instance->app_create_config.window_pos_x,
				game_instance->app_create_config.window_pos_y,
				game_instance->app_create_config.window_width,
				game_instance->app_create_config.window_height))
	{
		LFATAL("Error setting platform up.");
		return 0;
	}

	// Run consumer initialization function
	if (!app_state.game_instance->initialize(game_instance))
	{
		LFATAL("Consumer failed to initialize");
		return 0;
	}

	app_state.is_initialized = 1;

	return 1;
}

int8_t application_run()
{
	while (app_state.is_running)
	{
		if (!platform_poll_messages(&app_state.platform))
		{
			LFATAL("Failed to poll platform messages");
			app_state.is_running = 0;
		}
		
		if (!app_state.is_suspended)
		{
			if (!app_state.game_instance->update(app_state.game_instance, 0.0f))
			{
				LFATAL("Consumer game update failed.");
				app_state.is_running = 0;
				break;
			}

			if (!app_state.game_instance->render(app_state.game_instance, 0.0f))
			{
				LFATAL("Consumer game render failed.");
				app_state.is_running = 0;
				break;
			}
		}
	}

	app_state.is_running = 0;

	// Perform shutdown code
	
	platform_shutdown(&app_state.platform);

	return 1;
}

