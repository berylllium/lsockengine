#pragma once

#include "definitions.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#ifdef L_RELEASE
	#define LOG_DEBUG_ENABLED 0
	#define LOG_TRACE_ENABLED 0
#endif

enum log_level
{
	LISE_LOG_LEVEL_FATAL = 0,
	LISE_LOG_LEVEL_ERROR = 1,
	LISE_LOG_LEVEL_WARN = 2,
	LISE_LOG_LEVEL_INFO = 3,
	LISE_LOG_LEVEL_DEBUG = 4,
	LISE_LOG_LEVEL_TRACE = 5
};

bool lise_logger_init();
void lise_logger_shutdown();

LAPI void lise_log(enum log_level level, const char* message, ...);

#ifndef LFATAL
// Logs a fatal-level message.
#define LFATAL(message, ...) lise_log(LISE_LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#endif

#ifndef LERROR
// Logs an error-level message.
#define LERROR(message, ...) lise_log(LISE_LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
// Logs a warning-level message.
#define LWARN(message, ...) lise_log(LISE_LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define LWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
// Logs a info-level message.
#define LINFO(message, ...) lise_log(LISE_LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define LINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs a debug-level message.
#define LDEBUG(message, ...) lise_log(LISE_LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define LDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
// Logs a trace-level message.
#define LTRACE(message, ...) lise_log(LISE_LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define LTRACE(message, ...)
#endif
