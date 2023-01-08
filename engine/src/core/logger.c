#include "core/logger.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "platform/platform.h"

int8_t logging_init()
{
	// TODO: Create log file, etc
	return 1;
}

void logging_shutdown()
{
	// TODO: Do shutdown stuff
}

void llog(log_level level, const char* message, ...)
{
	const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};

	int8_t is_error = level <= LOG_LEVEL_ERROR;

	uint64_t log_entry_length = 32000;
	char out_message[log_entry_length];
	memset(out_message, 0, sizeof(out_message));

	__builtin_va_list arg_ptr;
	va_start(arg_ptr, message);
	vsnprintf(out_message, log_entry_length, message, arg_ptr);
	va_end(arg_ptr);

	char out_message2[log_entry_length];
	sprintf(out_message2, "$%s%s\n", level_strings[level], out_message);

	if (is_error)
		platform_console_write_error(out_message2, level);
	else
		platform_console_write(out_message2, level);
}

