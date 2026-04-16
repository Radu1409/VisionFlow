/**
 **************************************************************************************************
 *  @file           : vf-unit-operations.h
 *  @brief          : VisionFlow Unit Operations API Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Defines the operations vtable structure used by all VisionFlow pipeline units.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_UNIT_OPERATIONS_H
#define VF_UNIT_OPERATIONS_H

#include "vf-error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef vf_err_t (*vf_unit_op_fn_t)(void *ctx, ...);

typedef struct {
        vf_unit_op_fn_t init;
        vf_unit_op_fn_t deinit;
        vf_unit_op_fn_t get_data;
        vf_unit_op_fn_t process_data;
        vf_unit_op_fn_t send_data;
} vf_unit_operations_t;

#ifdef __cplusplus
}
#endif

#endif /* VF_UNIT_OPERATIONS_H */

