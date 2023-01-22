#include "core/logger.hpp"

#include <string>
#include <sstream>
#include <cstdarg>

#include "platform/platform.hpp"

namespace lise
{

bool logger_init()
{
	// TODO: Create log file, etc
	return true;
}

void logging_shutdown()
{
	// TODO: Do shutdown stuff
}

void llog(log_level level, const char* message, ...)
{
	const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};

	int8_t is_error = level <= LOG_LEVEL_ERROR;

	std::string formatted;

	va_list args, args_copy;
	
	va_start(args, message);
	va_copy(args_copy, args);

	int len = vsnprintf(nullptr, 0, message, args);

	if (len < 0)
	{
		LERROR("Encoding error in llog, vsnprintf.");

		va_end(args);
		va_end(args_copy);

		return;
	}	

	if (len > 0)
	{
		formatted.resize(len);

		vsnprintf(formatted.data(), len+1, message, args_copy);
	}

	va_end(args);
	va_end(args_copy);

	std::stringstream ss;
	
	ss << level_strings[level] << formatted << "\n";

	if (is_error)
		platform_console_write_error(ss.str().c_str(), level);
	else
		platform_console_write(ss.str().c_str(), level);
}

}
