#pragma once

#include "definitions.h"

LAPI bool lise_platform_init(
	const char* application_name,
	int32_t x, int32_t y,
	int32_t width, int32_t height);

LAPI void lise_platform_shutdown();

LAPI bool lise_platform_poll_messages();

void lise_platform_console_write(const char* message, uint8_t color);
void lise_platform_console_write_error(const char* message, uint8_t color);

double lise_platform_get_absolute_time();

void lise_platform_sleep(uint64_t ms);

const char** lise_platform_get_required_instance_extensions(uint32_t* out_extension_count);
