#pragma once

#include "definitions.h"

typedef enum lise_event_codes
{
	LISE_EVENT_ON_WINDOW_CLOSE,

	LISE_EVENT_ON_MOUSE_BUTTON_DOWN,
	LISE_EVENT_ON_MOUSE_BUTTON_UP,
	LISE_EVENT_ON_MOUSE_MOVE,
	LISE_EVENT_ON_MOUSE_WHEEL_MOVE,

	LISE_EVENT_ON_KEY_DOWN,
	LISE_EVENT_ON_KEY_UP,

	LISE_EVENT_ON_WINDOW_RESIZE
} lise_event_codes;

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

typedef void (*lise_on_event_cb)(uint16_t event_code, lise_event_context ctx);

bool lise_event_init();
void lise_event_shutdown();

LAPI void lise_event_register(uint16_t event_code);

LAPI void lise_event_fire(uint16_t event_code, lise_event_context ctx);

LAPI void lise_event_add_listener(uint16_t event_code, lise_on_event_cb listener);
