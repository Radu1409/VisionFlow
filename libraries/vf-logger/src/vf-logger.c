#include <string.h>

#include "vf-logger.h"

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

static LogLevel current_log_level = LOG_INFO;
static const char *log_filename = "log.txt";

static int console_logging_enabled = 1;

void enable_console_log(int enabled) {
    console_logging_enabled = enabled;
}

void set_log_level(LogLevel level) {
    current_log_level = level;
}

void set_log_file(const char *filename) {
    log_filename = filename;
}

static void get_timestamp(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

void log_message(LogLevel level, const char *file, int line, const char *func, const char *fmt, ...) {
    if (level < current_log_level) {
        return;
    }

    const char *level_str;
    const char *color;

    switch (level) {
        case LOG_INFO:
            level_str = VF_LOG_INFO_STR;
            color = COLOR_INFO;
            break;
        case LOG_WARNING:
            level_str = VF_LOG_WARNING_STR;
            color = COLOR_WARNING;
            break;
        case LOG_ERROR:
            level_str = VF_LOG_ERROR_STR;
            color = COLOR_ERROR;
            break;
        case LOG_DEBUG:
            level_str = VF_LOG_DEBUG_STR;
            color = COLOR_DEBUG;
            break;
        case LOG_FATAL:
            level_str = VF_LOG_FATAL_STR;
            color = COLOR_FATAL;
            break;
        default:
            level_str = VF_LOG_UNKNOWN_STR;
            color = COLOR_RESET;
            break;
    }

    const char *filename = strrchr(file, '/');
    if (filename) {
        filename++;
    } else {
        filename = file;
    }

    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));

    char message[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    FILE *log_file = fopen(log_filename, "a");
    if (log_file) {
        fprintf(log_file, "[%s] [%s:%d:%s] [%s]: %s", timestamp, filename, line, func, level_str, message);
        fclose(log_file);
    }

    if (console_logging_enabled) {
        printf("[%s] [%s:%d:%s] %s[%s]%s: %s", timestamp, filename, line, func, color, level_str, COLOR_RESET, message);
    }

    if (level == LOG_FATAL) {
        exit(EXIT_FAILURE);
    }
}

