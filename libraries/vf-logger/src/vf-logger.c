/**
 **************************************************************************************************
 *  @file           : vf-logger.c
 *  @brief          : VF logger API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  VisionFlow Logger implementation using Linux syslog and stdout
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>
#include <time.h>

#include "vf-error.h"
#include "vf-logger.h"

#define DEFAULT_APP_NAME            "VisionFlow"

#define VF_LOG_TIME_BUF_SIZE        64
#define VF_LOG_SYSLOG_FMT_BUF_SIZE  512

/* ANSI colors for clean output in terminal */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_FATAL   "\x1b[1;31m" /* Red Bold */
#define COLOR_ERR     "\x1b[31m"   /* Red */
#define COLOR_WRN     "\x1b[33m"   /* Yellow */
#define COLOR_INFO    "\x1b[94m"   /* Green */
#define COLOR_DBG     "\x1b[96m"   /* Cyan */
#define COLOR_TRACE   "\x1b[37m"   /* White */

typedef struct {
        vf_log_level_t  max_level;
        pthread_mutex_t lock;
        bool            initialized;
} vf_logger_ctx_t;

/* Global and static logger context */
static vf_logger_ctx_t g_logger = {
        .max_level   = VF_LOG_LEVEL_TRACE,
        .initialized = false
};

/* Map our logging level to the standard levels in syslog */
static
int map_to_syslog_level(vf_log_level_t level)
{
        switch (level) {
            case VF_LOG_LEVEL_FATAL:
                    return LOG_EMERG;
            case VF_LOG_LEVEL_ERR:
                    return LOG_ERR;
            case VF_LOG_LEVEL_WRN:
                    return LOG_WARNING;
            case VF_LOG_LEVEL_INFO: 
                    return LOG_INFO;
            case VF_LOG_LEVEL_DBG:
                    return LOG_DEBUG;
            case VF_LOG_LEVEL_TRACE:
                    return LOG_DEBUG;
            default:
                    return LOG_INFO;
        }
}

/* Format the level text exactly as it was in the original */
static
const char *get_level_str_and_color(vf_log_level_t level, const char **color)
{
        if (NULL == color || NULL == *color) {
                vf_log_fprintf("Invalid input: color = %p, *color = %p", color, *color);

                return NULL;
        }

        switch (level) {
                case VF_LOG_LEVEL_FATAL:
                        *color = COLOR_FATAL;

                        return VF_LOG_FATAL_STR;
                case VF_LOG_LEVEL_ERR:
                        *color = COLOR_ERR;

                        return VF_LOG_ERR_STR;
                case VF_LOG_LEVEL_WRN:
                        *color = COLOR_WRN;

                        return VF_LOG_WRN_STR;
                case VF_LOG_LEVEL_INFO:
                        *color = COLOR_INFO;

                        return VF_LOG_INFO_STR;
                case VF_LOG_LEVEL_DBG:
                        *color = COLOR_DBG;

                        return VF_LOG_DBG_STR;
                case VF_LOG_LEVEL_TRACE:
                        *color = COLOR_TRACE;

                        return VF_LOG_TRACE_STR;
                default:
                        *color = COLOR_RESET;

                        return VF_LOG_UNKNOWN_STR;
        }
}

int vf_logger_init(const char *app_name, vf_log_level_t max_verbosity)
{
        int err = EOK;

        if (true == g_logger.initialized) {
                return EOK; /* Already initialized */
        }

        err = pthread_mutex_init(&g_logger.lock, NULL);
        if (err != EOK) {
                vf_log_fprintf("Failed to initialize logger mutex.");

                return -1;
        }

        g_logger.max_level = max_verbosity;

        if (NULL == app_name) {
                app_name = DEFAULT_APP_NAME;
        }

        openlog(app_name, LOG_PID | LOG_CONS, LOG_USER);

        g_logger.initialized = true;

        return EOK;
}

void vf_logger_deinit(void)
{
        int err = EOK;

        if (false == g_logger.initialized) {
                return; /* Already deinitialized */
        }

        log_info("Logger shutting down...");

        closelog();

        err = pthread_mutex_destroy(&g_logger.lock);
        if (err != EOK) {
                vf_log_fprintf("Failed to destroy logger mutex.");

                return;
        }

        g_logger.initialized = false;
}

void vf_logger_set_verbosity(vf_log_level_t log_level)
{
        int err = EOK;

        err = pthread_mutex_lock(&g_logger.lock);
        if (err != EOK) {
                vf_log_fprintf("Failed to lock logger mutex.");

                return;
        }

        g_logger.max_level = log_level;

        err = pthread_mutex_unlock(&g_logger.lock);
        if (err != EOK) {
                vf_log_fprintf("Failed to unlock logger mutex.");

                return;
        }

        log_info("Verbosity set successfully!");
}

void vf_logger_record(vf_log_level_t log_level, const char *file, int line, const char *func,
        const char *fmt, ...)
{
        char time_buf[VF_LOG_TIME_BUF_SIZE] = {0};
        char syslog_fmt[VF_LOG_SYSLOG_FMT_BUF_SIZE]= {0};
        const char *color = NULL;
        const char *level_str = NULL;
        va_list args_console = {0};
        va_list args_syslog = {0};
        struct timespec ts = {0};
        struct tm tm_info = {0};
        int syslog_level = 0;
        long millis = 0;
        int err = EOK;

        if (NULL == file || NULL == func || NULL == fmt) {
                vf_log_fprintf("Invalid input: file = %p, func = %p, fmt = %p",
                               (void *)file, (void *)func, (void *)fmt);

                return;
        }

        if (log_level < VF_LOG_LEVEL_FATAL || log_level > VF_LOG_LEVEL_TRACE) {
                vf_log_fprintf("Invalid log level provided: %d. Message dropped.",
                               log_level);

                return;
        }

        if (false == g_logger.initialized || log_level > g_logger.max_level) {
                return;
        }

        /* 1. Get the current time function thread-safe localtime_r */
        err = clock_gettime(CLOCK_REALTIME, &ts);
        if (EOK != err) {
                log_err("Failed to get current time", strerror(errno));

                return;
        }

        (void)localtime_r(&ts.tv_sec, &tm_info);

        /* Calculate miliseconds from nanoseconds */
        millis = ts.tv_nsec / 1000000;

        (void)strftime(time_buf, VF_LOG_TIME_BUF_SIZE, "%Y-%m-%d %H:%M:%S", &tm_info);

        /* 2. Prepare the variable arguments */
        va_start(args_console, fmt);
        va_copy(args_syslog, args_console);

        color = COLOR_RESET;
        level_str = get_level_str_and_color(log_level, &color);
        if (NULL == level_str || NULL == color) {
                vf_log_fprintf("Invalid internal color/level pointers. color = %p, level = %p",
                               (void *)color, (void *)level_str);

                goto cleanup;
        }

        err = pthread_mutex_lock(&g_logger.lock);
        if (err != EOK) {
                vf_log_fprintf("Failed to lock logger mutex.");

                goto cleanup;
        }

        /* 3. Display in the console using your exact original format */
        (void)fprintf(stderr, "[%s.%.3ld] [%s:%d:%s] %s[%s]%s: ",
                      time_buf, millis, file, line, func, color, level_str, COLOR_RESET);
        (void)vfprintf(stderr, fmt, args_console);
        (void)fprintf(stderr, "\n");

        /* 4. Send to syslog (without colors and without explicit time, syslog adds its time) */
        (void)snprintf(syslog_fmt, sizeof(syslog_fmt), "[%s:%d:%s] [%s]: %s", file, line, func,
                       level_str, fmt);

        syslog_level = map_to_syslog_level(log_level);

        vsyslog(syslog_level, syslog_fmt, args_syslog);

        err = pthread_mutex_unlock(&g_logger.lock);
        if (err != EOK) {
                vf_log_fprintf("Failed to unlock logger mutex.");
        }

cleanup:
        va_end(args_console);
        va_end(args_syslog);
}

