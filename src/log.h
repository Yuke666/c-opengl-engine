#ifndef LOG_DEF
#define LOG_DEF

enum {
	LOG_NORMAL = 0,
	LOG_RED = 31,
	LOG_GREEN = 32,
	LOG_YELLOW = 33,
	LOG_BLUE = 34,
	LOG_MAGENTA = 35,
	LOG_CYAN = 36,
};

void Log_FormattedInfo(int color, const char *file, int line, const char *format, ...);
void Log_UnformattedInfo(int color, const char *file, int line, const char *text);
void Log_Unformatted(int color, const char *text);
void Log_Formatted(int color, const char *format, ...);

#define INFO_LOGF(color, format, ...) Log_FormattedInfo(color, __FILE__, __LINE__, format, __VA_ARGS__);
#define INFO_LOG(color, text) Log_UnformattedInfo(color, __FILE__, __LINE__, text);

#define LOGF(color, format, ...) Log_Formatted(color, format, __VA_ARGS__);
#define LOG(color, text) Log_Unformatted(color, text);

#endif