#include <core/application.h>
#include <core/logger.h>

#include "game.h"

int main()
{
	lise_application_create_info app_create_info = {};

	app_create_info.window_pos_x = 100;
	app_create_info.window_pos_y = 100;
	app_create_info.window_width = 1280;
	app_create_info.window_height = 720;
	app_create_info.window_name = "LiSE Testing";
	app_create_info.entry_points.update = game_update;
	app_create_info.entry_points.render = game_render;
	app_create_info.entry_points.initialize = game_initialize;
	app_create_info.entry_points.on_window_resize = game_on_resize;

	if (!lise_application_create(app_create_info))
	{
		LFATAL("Could not create application.");
		return -1;
	}

	LINFO("hii");

	if (!lise_application_run())
	{
		LFATAL("Application did not shut down correctly.");
		return -1;
	}

	return 0;
}
