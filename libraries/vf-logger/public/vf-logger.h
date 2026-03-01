/**
 **************************************************************************************************
 *  @file           : vf-logger.h
 *  @brief          : VF logger API Header
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

#ifndef VF_LOGGER__H
#define VF_LOGGER__H

#include <stdio.h>
#include <string.h>

#ifndef EOK
#define EOK 0
#endif

#define VF_LOG_FATAL_STR   "FATAL"
#define VF_LOG_ERR_STR     "ERROR"
#define VF_LOG_WRN_STR     "WARNING"
#define VF_LOG_INFO_STR    "INFO"
#define VF_LOG_DBG_STR     "DEBUG"
#define VF_LOG_TRACE_STR   "TRACE"
#define VF_LOG_UNKNOWN_STR "UNKNOWN"

/* Extract only the file name, without absolute path */
#define VF_FILE_NAME_NO_PATH(filepath) \
        (strrchr((filepath), '/') ? strrchr((filepath), '/') + 1 : (filepath))

/* Base macro for logging dynamically by level */
#define vf_log(log_level, fmt, ...)                                                                \
        do {                                                                                       \
                vf_logger_record(log_level,                                                        \
                                 VF_FILE_NAME_NO_PATH(__FILE__),                                   \
                                 __LINE__,                                                         \
                                 __func__,                                                         \
                                 fmt,                                                              \
                                 ## __VA_ARGS__);                                                  \
        } while (0)

/* Fallback macro to print directly to standard (useful before logger is initialized) */
#define vf_log_fprintf(fmt, ...)                                                                   \
        do {                                                                                       \
                fprintf(stderr,                                                                    \
                        "[%s:%d:%s] [%s]: " fmt "\n",                                              \
                        VF_FILE_NAME_NO_PATH(__FILE__),                                            \
                        __LINE__,                                                                  \
                        __func__,                                                                  \
                        VF_LOG_ERR_STR,                                                            \
                        ## __VA_ARGS__);                                                           \
        } while (0)

typedef enum {
        VF_LOG_LEVEL_NONE  = 0,
        VF_LOG_LEVEL_FATAL = 1,
        VF_LOG_LEVEL_ERR   = 2,
        VF_LOG_LEVEL_WRN   = 3,
        VF_LOG_LEVEL_INFO  = 4,
        VF_LOG_LEVEL_DBG   = 5,
        VF_LOG_LEVEL_TRACE = 6
} vf_log_level_t;

/* Main functions exposed by the API */
int vf_logger_init(const char *app_name, vf_log_level_t max_verbosity);
void vf_logger_deinit(void);
void vf_logger_set_verbosity(vf_log_level_t log_level);
void vf_logger_record(vf_log_level_t log_level, const char *file, int line, const char *func,
                      const char *fmt, ...);

/* Macros for easy use in code */
#define log_fatal(fmt, ...) vf_log(VF_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define log_err(fmt, ...)   vf_log(VF_LOG_LEVEL_ERR,   fmt, ##__VA_ARGS__)
#define log_wrn(fmt, ...)   vf_log(VF_LOG_LEVEL_WRN,   fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  vf_log(VF_LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define log_dbg(fmt, ...)   vf_log(VF_LOG_LEVEL_DBG,   fmt, ##__VA_ARGS__)
#define log_trace(fmt, ...) vf_log(VF_LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)

#endif /* VF_LOGGER__H */

