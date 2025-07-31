#include "stdio.h"
#include "serial_port.h"

int32_t vprint(const char* format, va_list args)
{
	(void)format;
	(void)args;

	com1_writestring(format);

	return 0;
}

int32_t print(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	int32_t bytes_written = vprint(format, args);

	va_end(args);

	return bytes_written;
}
