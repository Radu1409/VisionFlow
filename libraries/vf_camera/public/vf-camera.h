/**
 **************************************************************************************************
 *  @file           : vf-camera.h
 *  @brief          : VisionFlow Camera API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Camera library API for VisionFlow. Captures real frames from a physical
 *  camera using V4L2 (Video4Linux2).
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CAMERA_H
#define VF_CAMERA_H

#include <stdint.h>

#include "vf-camera-cfg.h"
#include "vf-error.h"
#include "vf-framebuffer.h"

#define VF_CAMERA_MAX_BUFFERS       16U

#define VF_CAMERA_STATE_STOPPED_STR "CAMERA_STATE_STOPPED"
#define VF_CAMERA_STATE_RUNNING_STR "CAMERA_STATE_RUNNING"
#define VF_CAMERA_STATE_ERROR_STR   "CAMERA_STATE_ERROR"
#define VF_CAMERA_STATE_UNKNOWN_STR "CAMERA_STATE_UNKNOWN"

/* *DISABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */
typedef enum {
        VF_CAMERA_STATE_STOPPED = 0,
        VF_CAMERA_STATE_RUNNING = 1,
        VF_CAMERA_STATE_ERROR   = 2,
} vf_camera_state_t;
/* *ENABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */

typedef struct {
        void    *start;
        uint32_t length;
} vf_camera_buffer_t;

typedef struct {
        vf_camera_cfg_t    cfg;
        vf_camera_state_t  state;

        int                fd;

        vf_camera_buffer_t buffers[VF_CAMERA_MAX_BUFFERS];
        uint32_t           buffer_count;

        uint32_t           frame_id;
} vf_camera_t;

vf_err_t vf_camera_init(vf_camera_t *camera, const vf_camera_cfg_t *cfg);
vf_err_t vf_camera_deinit(vf_camera_t *camera);
vf_err_t vf_camera_start(vf_camera_t *camera);
vf_err_t vf_camera_stop(vf_camera_t *camera);
vf_err_t vf_camera_acquire_frame(vf_camera_t *camera, vf_framebuffer_t *fb);
const char *vf_camera_state2str(vf_camera_state_t state);

#endif /* VF_CAMERA_H */

