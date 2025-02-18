#ifndef VF_LOGGER_H
#define VF_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

typedef enum {
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

void log_message(LogLevel level, const char *file, int line, const char *func, const char *fmt, ...);
void set_log_level(LogLevel level);
void set_log_file(const char *filename);
void enable_console_log(int enabled);

#define log_info(fmt, ...)    log_message(LOG_INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_warning(fmt, ...) log_message(LOG_WARNING, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)   log_message(LOG_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)   log_message(LOG_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...)   log_message(LOG_FATAL, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#endif /* __VF_LOGGER_H__ */

