#include <entry.h>
#include <core/logger.h>

#include "game.h"

int8_t create_game(game* out_game)
{
	out_game->app_create_config.window_pos_x = 100;
    out_game->app_create_config.window_pos_y = 100;
    out_game->app_create_config.window_width = 1280;
    out_game->app_create_config.window_height = 720;
    out_game->app_create_config.window_name = "Kohi Engine Testbed";
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->initialize = game_initialize;
    out_game->on_window_resize = game_on_resize;
}

