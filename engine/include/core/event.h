/**
 * @file event.h
 * @brief This header file contains the definition of structures and functions relating to event handeling.
 */
#pragma once

#include "definitions.h"

/**
 * @brief This enum contains all the types of engine specific events.
 */
typedef enum lise_event_codes
{
	/**
	 * @brief Called when the game window should close.
	 * 
	 * Provides an empty event context.
	 */
	LISE_EVENT_ON_WINDOW_CLOSE,

	/**
	 * @brief Called when the left mouse button is pushed down.
	 * 
	 * Provides an empty event context.
	 */
	LISE_EVENT_ON_MOUSE_BUTTON_DOWN,

	/**
	 * @brief Called when the left mouse button is released.
	 * 
	 * Provides an empty event context.
	 */
	LISE_EVENT_ON_MOUSE_BUTTON_UP,
	
	/**
	 * @brief Called when the mouse is moved.
	 * 
	 * `i32[0]` will contain the x-cooordinate of the new mouse position. <br>
	 * `i32[1]` will contain the y-cooordinate of the new mouse position.
	 */
	LISE_EVENT_ON_MOUSE_MOVE,

	/**
	 * @brief Called when the mouse wheel is moved.
	 * 
	 * Mouse wheel movement is normalized.
	 * 
	 * `i8[0]` will contain the direction of movement. `-1` if the mouse wheel moved down and `1` if it moved up.
	 */
	LISE_EVENT_ON_MOUSE_WHEEL_MOVE,

	/**
	 * @brief Called when a key is pressed.
	 * 
	 * `u32[0]` will contain the key that was pressed. See \ref lise_keys for all available keys.
	 */
	LISE_EVENT_ON_KEY_DOWN,

	/**
	 * @brief Called when a key is released.
	 * 
	 * `u32[0]` will contain the key that was released. See \ref lise_keys for all available keys.
	 */
	LISE_EVENT_ON_KEY_UP,

	/**
	 * @brief Called when the window is resized.
	 * 
	 * `u32[0]` will contain the new width of the window. <br>
	 * `u32[1]` will contain the new height of the window.
	 */
	LISE_EVENT_ON_WINDOW_RESIZE,

	LISE_EVENT_MAX_ENUM
} lise_event_codes;

/**
 * @brief The context of an event.
 * 
 * Stores the "parameters" for an event. The size of an event context is 128 bytes. These 128 bytes are split up
 * in different ways to allow for easier access to parameter.
 * 
 * It is possible to store the following:
 * - 2 signed 64-bit integers
 * - 2 unsigned 64-bit integers
 * - 2 doubles
 * - 4 signed 32-bit integers
 * - 4 unsigned 32-bit integers
 * - 4 floats
 * - 8 signed 16-bit integers
 * - 8 unsigned 16-bit integers
 * - 16 signed 8-bit integers
 * - 16 unsigned 8-bit integers
 * - 16 characters
 */
typedef struct lise_event_context
{
	union
	{
		int64_t i64[2];
		uint64_t u64[2];
		double f64[2];

		int32_t i32[4];
		uint32_t u32[4];
		float f32[4];

		int16_t i16[8];
		uint16_t u16[8];

		int8_t i8[16];
		uint8_t u8[16];
		char c[16];
	} data;
} lise_event_context;

/**
 * @brief The on event callback. This is the format that event callbacks use.
 * 
 * @param event_code The event code. This can be a custom event code or an engine event code. Engine event codes
 * can be found in the \ref lise_event_codes enum.
 * @param ctx The passed event context.
 */
typedef void (*lise_on_event_cb)(uint16_t event_code, lise_event_context ctx);

/**
 * @brief Initializes the event subsystem.
 * 
 * @return true if the initialization was successfull.
 * @return false if there was an error during initialization.
 */
bool lise_event_init();

/**
 * @brief Shuts down the event subsystem, cleaning up any dynamic resources.
 */
void lise_event_shutdown();

/**
 * @brief Registers an event code.
 * 
 * @param event_code The event code to register.
 * 
 * @note The event codes 0 up to and including 1024 are preserved for engine event codes. It is still possible to
 * register an event within this range because not every code within the range is in use yet, but it is not recommended
 * to do so because it could create conflicts with future updates if a new engine event has been registered.
 */
LAPI void lise_event_register(uint16_t event_code);

/**
 * @brief Fires an event.
 * 
 * @param event_code The event code to fire.
 * @param ctx The ctx that gets passed to all listeners.
 */
LAPI void lise_event_fire(uint16_t event_code, lise_event_context ctx);

/**
 * @brief Adds a listener to an event.
 * 
 * @param event_code The event code to add a listener to.
 * @param listener The listener to add.
 */
LAPI void lise_event_add_listener(uint16_t event_code, lise_on_event_cb listener);
