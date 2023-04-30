#include "core/logger.hpp"

#include <cstring>
#include <cstdio>
#include <cstdarg>

#include "platform/platform.hpp"

namespace lise
{

bool logger_init()
{
	// TODO: Create log file, etc
	return true;
}

void logger_shutdown()
{
	// TODO: Do shutdown stuff
}

void log(log_level level, const char* message, ...)
{
	const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ",
									"[TRACE]: "};

	bool is_error = level <= log_level::LOG_LEVEL_ERROR;

	uint64_t log_entry_length = 32000;
	char out_message[log_entry_length];
	memset(out_message, 0, sizeof(out_message));

	__builtin_va_list arg_ptr;
	va_start(arg_ptr, message);
	vsnprintf(out_message, log_entry_length, message, arg_ptr);
	va_end(arg_ptr);

	char out_message2[log_entry_length];
	sprintf(out_message2, "$%s%s\n", level_strings[static_cast<int>(level)], out_message);

	if (is_error)
	{
		platform_console_write_error(out_message2, static_cast<uint8_t>(level));
	}
	else
	{
		platform_console_write(out_message2, static_cast<uint8_t>(level));
	}
	
}

}
