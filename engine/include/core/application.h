/**
 * @file application.h
 * @brief This header file contains definitions of structures and functions relating to the application.
 * 
 * This header should only be included by the consumer of the engine. It contains the functions to initialize the
 * engine.
 */
#pragma once

#include "definitions.h"

/**
 * @brief This structure stores function pointers to consumer defined engine functions.
 */
typedef struct lise_game_entry_points
{
	/**
	 * @brief The `initialize` callback gets called once at the initialization phase of the engine.
	 * 
	 * An example of usage of this callback is registering events.
	 */
	bool (*initialize)();

	/**
	 * @brief The `update` callback gets called once every frame from the main engine thread.
	 * 
	 * The intended use of the update callback is for anything non-graphical. Such as physics calculations.
	 * 
	 * @param delta_time The time between frames, in seconds. Also known as delta time.
	 */
	bool (*update)(float delta_time);

	/**
	 * @brief The `render` callback gets called once every frame from the render thread.
	 * 
	 * The intended use of the render callback is for anything graphical. Such as camera controls.
	 * 
	 * @param delta_time The time between frames, in seconds. Also known as delta time.
	 */
	bool (*render)(float delta_time);

	/**
	 * @brief The `on_window_resize` callback gets called every time the game window is resized.
	 * 
	 * @param width The new width of the window.
	 * @param height The new height of the window.
	 */
	void (*on_window_resize)(uint32_t width, uint32_t height);
} lise_game_entry_points;

/**
 * @brief This structure contains configurations for creating the application.
 */
typedef struct lise_application_create_info
{
	/**
	 * @brief The desired x-coordinate of the upper left corner of the window on the screen.
	 */
	int16_t window_pos_x;

	/**
	 * @brief The desired y-coordinate of the upper left corner of the window on the screen.
	 */
	int16_t window_pos_y;

	/**
	 * @brief The desired width of the screen.
	 */
	int16_t window_width;

	/**
	 * @brief The desired height of the screen.
	 */
	int16_t window_height;

	/**
	 * @brief The name of the window.
	 * 
	 * Do not deallocate this allocation ever, as it is used during the entire lifetime of the application.
	 */
	const char* window_name;
	
	/**
	 * @brief A \ref lise_game_engine_entry_points object containing all the "entry_points" or callbacks of the
	 * consumer.
	 */
	lise_game_entry_points entry_points;
} lise_application_create_info;

/**
 * @brief Attempts to create the application.
 * 
 * @param app_create_info A struct containing all the configurations required for creating the application.
 * @return true if the application was successfully created.
 * @return false if there was an error during application creation.
 */
LAPI bool lise_application_create(lise_application_create_info app_create_info);

/**
 * @brief Runs the application, meaning that it will start the engine and transfer ownership of the entire thread to the
 * library: this function will block the thread until the application closes for any reason.
 * 
 * @return true if the application shut down correctly.
 * @return false if the application shut down unsuccessfully.
 */
LAPI bool lise_application_run();
