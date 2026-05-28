/**
 **************************************************************************************************
 *  @file           : vf-camera-unit.h
 *  @brief          : VisionFlow Camera Unit API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Camera unit — wraps vf_camera library into vf_unit_operations_t.
 *  Captures real frames from a V4L2 device and pushes them into the
 *  pipeline via the notifier.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CAMERA_UNIT_H
#define VF_CAMERA_UNIT_H

#include "vf-buff-pool.h"
#include "vf-camera.h"
#include "vf-error.h"
#include "vf-processing-unit.h"

typedef struct {
        vf_camera_cfg_t  camera_cfg;
        vf_buf_pool_t   *pool;
} vf_camera_unit_cfg_t;

vf_err_t vf_camera_unit_init(void *ctx, ...);
vf_err_t vf_camera_unit_deinit(void *ctx, ...);
vf_err_t vf_camera_unit_get_data(void *ctx, ...);
vf_err_t vf_camera_unit_process_data(void *ctx, ...);
vf_err_t vf_camera_unit_send_data(void *ctx, ...);
vf_err_t vf_camera_unit_init_operations(vf_unit_operations_t *ops);

#endif /* VF_CAMERA_UNIT_H */

