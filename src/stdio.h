#pragma once

#include <stdint.h>
#include <stdarg.h>

#ifndef LOG_PREFIX
	#define LOG_PREFIX "GURT"
#endif

#define LOG(...) print("[ " LOG_PREFIX " ] :: " __VA_ARGS__)

int32_t print(const char* format, ...);
int32_t vprint(const char* format, va_list args);
