#include "log.h"
#include <stdio.h>
#include <stdarg.h>

void Log_FormattedInfo(int color, const char *file, int line, const char *format, ...){

	printf("\x1b[%im%s, Line %i\n", color, file, line);

	va_list args;
	va_start(args, format);

	vprintf(format, args);

	va_end(args);

	printf("\x1b[0m");
}

void Log_UnformattedInfo(int color, const char *file, int line, const char *text){

	printf("\x1b[33m%s, Line %i\n\x1b[%im%s\x1b[0m", file, line, color, text);
}

void Log_Formatted(int color, const char *format, ...){

	printf("\x1b[%im", color);

	va_list args;
	va_start(args, format);

	vprintf(format, args);

	va_end(args);

	printf("\x1b[0m");
}

void Log_Unformatted(int color, const char *text){

	printf("\x1b[%im%s\x1b[0m", color, text);
}