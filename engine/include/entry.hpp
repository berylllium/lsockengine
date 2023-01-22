#pragma once

#include "definitions.hpp"

#include "core/logger.hpp"
#include "game_type.hpp"

int8_t create_game(lise::game* out_game);

int main(void)
{
	// Request game instance from the library consumer
	lise::game game_instance;
	if (!create_game(&game_instance))
	{
		LFATAL("Could not create game.");
		return -1;
	}

	// Inlise::itialization
	if (!lise::application_create(&game_instance))
	{
		LFATAL("Could not create application.");
		return -1;
	}

	// Belise::gin game loop
	if (!lise::application_run())
	{
		LFATAL("Application did not shut down correctly.");
		return -1;
	}

	return 0;
}

