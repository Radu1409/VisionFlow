#include "vf-logger.h"

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

void log_message(LogLevel level, const char *message, const char *file, int line) {
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

    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));

    FILE *log_file = fopen(log_filename, "a");
    if (log_file) {
        fprintf(log_file, "[%s] [%s:%d] [%s]: %s\n", timestamp, file, line, level_str, message);
        fclose(log_file);
    }

    if (console_logging_enabled) {
        printf("[%s] [%s:%d] %s[%s]%s: %s\n", timestamp, file, line, color, level_str, COLOR_RESET, message);
    }

    if (level == LOG_FATAL) {
        exit(EXIT_FAILURE);
    }
}

