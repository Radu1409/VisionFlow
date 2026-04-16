/**
 **************************************************************************************************
 *  @file           : vf-file-unit.h
 *  @brief          : VisionFlow File Unit API Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  File unit — reads frames from disk (FILE_IN) or writes frames to disk (FILE_OUT).
 *  Implements the vf_unit_operations_t interface.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_FILE_UNIT_H
#define VF_FILE_UNIT_H

#include "vf-buff-pool.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-unit-operations.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        VF_FILE_UNIT_MODE_IN  = 0,
        VF_FILE_UNIT_MODE_OUT = 1
} vf_file_unit_mode_t;

typedef struct {
        char                 file_path[256];
        vf_fb_params_t       fb_params;
        vf_buf_pool_t       *pool;
        vf_file_unit_mode_t  mode;
} vf_file_unit_cfg_t;

vf_err_t vf_file_unit_init(void *ctx, ...);
vf_err_t vf_file_unit_deinit(void *ctx, ...);
vf_err_t vf_file_unit_get_data(void *ctx, ...);
vf_err_t vf_file_unit_process_data(void *ctx, ...);
vf_err_t vf_file_unit_send_data(void *ctx, ...);

vf_err_t vf_file_unit_init_operations(vf_unit_operations_t *ops);

#ifdef __cplusplus
}
#endif

#endif /* VF_FILE_UNIT_H */

