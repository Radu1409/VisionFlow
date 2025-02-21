#ifndef VF_LOGGER_H
#define VF_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define COLOR_RESET        "\033[0m"
#define COLOR_INFO         "\033[94m"
#define COLOR_WARNING      "\033[33m"
#define COLOR_DEBUG        "\033[96m"
#define COLOR_ERROR        "\033[31m"
#define COLOR_FATAL        "\033[91m"

#define VF_LOG_INFO_STR    "INFO"
#define VF_LOG_WARNING_STR "WARNING"
#define VF_LOG_DEBUG_STR   "DEBUG"
#define VF_LOG_ERROR_STR   "ERROR"
#define VF_LOG_FATAL_STR   "FATAL"
#define VF_LOG_UNKNOWN_STR "UNKNOWN"

typedef enum {
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

void log_message(LogLevel level, const char *message, const char *file, int line);
void set_log_level(LogLevel level);
void set_log_file(const char *filename);
void enable_console_log(int enabled);

#define LOG_MESSAGE(level, message) log_message(level, message, __FILE__, __LINE__)

#endif /* __VF_LOGGER_H__ */

