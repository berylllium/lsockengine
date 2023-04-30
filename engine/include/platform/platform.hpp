#pragma once

#include "definitions.hpp"

namespace lise
{

LAPI bool platform_init(
	const char* application_name,
	int32_t x, int32_t y,
	int32_t width, int32_t height
);

LAPI void platform_shutdown();

LAPI bool platform_poll_messages();

void platform_console_write(const char* message, uint8_t color);
void platform_console_write_error(const char* message, uint8_t color);

double platform_get_absolute_time();

void platform_sleep(uint64_t ms);

const char** platform_get_required_instance_extensions(uint32_t* out_extension_count);

}
