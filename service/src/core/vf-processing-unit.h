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

#include <sys/types.h>

#include "vf-error.h"

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

typedef struct unit {
        unit_type_t       type;
        const char        *name;
        struct operations operations;
        union {
        } settings;
} unit_st_t;

#endif /* VF_PROCESSING_UNIT__H */

