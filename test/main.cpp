#include <entry.hpp>
#include <core/logger.hpp>

#include "game.hpp"

int8_t create_game(lise::game* out_game)
{
	out_game->app_create_config.window_pos_x = 100;
    out_game->app_create_config.window_pos_y = 100;
    out_game->app_create_config.window_width = 1280;
    out_game->app_create_config.window_height = 720;
    out_game->app_create_config.window_name = "LiSE Testing";
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->initialize = game_initialize;
    out_game->on_window_resize = game_on_resize;

    return true;
}

