/**
 **************************************************************************************************
 *  @file           : vf-processing-unit.h
 *  @brief          : VF processing unit routine
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Processing unit operating API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_PROCESSING_UNIT__H
#define VF_PROCESSING_UNIT__H

#include <pthread.h>
#include <sys/types.h>

#include "vf-error.h"

#define UNIT_TYPE_INVALID_STR        "Invalid"
#define UNIT_TYPE_CAMERA_STR         "Camera"
#define UNIT_TYPE_DISPLAY_STR        "Display"
#define UNIT_TYPE_ENCODER_STR        "Encoder"
#define UNIT_TYPE_CONVERSION_STR     "Conversion"

typedef vf_err_t (*operation_fp_t)(void *, ...);

typedef struct operations {
        operation_fp_t init;
        operation_fp_t deinit;
        operation_fp_t get_data;
        operation_fp_t send_data;
        operation_fp_t process_data;
} operations_st_t;

typedef enum unit_type {
        UNIT_TYPE_INVALID    = -1,
        UNIT_TYPE_CAMERA     = 0,
        UNIT_TYPE_DISPLAY    = 1,
        UNIT_TYPE_ENCODER    = 2,
        UNIT_TYPE_CONVERSION = 3,
} unit_type_t;

typedef enum unit_state {
        UNIT_STATE_UNKNOWN          = -1,
        UNIT_STATE_STOPPED          = 0,
        UNIT_STATE_RUNNING          = 1,
        UNIT_STATE_DEINITIALIZATION = 2,
        UNIT_STATE_DEINITIALIZED    = 3,
        UNIT_STATE_INITIALIZATION   = 4,
        UNIT_STATE_INITIALIZED      = 5,        
} unit_state_t;

typedef struct unit_stats {
        uint64_t obtained_frames_cnt;
        uint64_t sent_frames_cnt;
        double   avg_obtained_fps;
        double   avg_processed_fps;
        double   avg_sent_fps;
} unit_stats_st_t;

typedef struct unit {
        unit_type_t       type;
        unit_state_t      state;
        const char        *name;
        struct operations operations;
        struct unit_stats stats;
        union {
        } settings;

        pthread_t         tid;
} unit_st_t;

vf_err_t vf_create_unit(unit_st_t *unit);
void vf_destroy_unit(unit_st_t *unit);

#endif /* VF_PROCESSING_UNIT__H */

