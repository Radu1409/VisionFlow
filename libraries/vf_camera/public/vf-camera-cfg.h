/**
 **************************************************************************************************
 *  @file           : vf-camera-cfg.h
 *  @brief          : VisionFlow Camera configuration structures
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Camera configuration structures for VisionFlow camera library.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CAMERA_CFG_H
#define VF_CAMERA_CFG_H

#include <stdint.h>

#include "vf-pixel-fmt.h"

#define VF_CAMERA_DEVICE_PATH_MAX_LEN   64U
#define VF_CAMERA_DEFAULT_BUFFER_COUNT  4U
#define VF_CAMERA_DEFAULT_DEVICE        "/dev/video0"

typedef struct {
        char           device_path[VF_CAMERA_DEVICE_PATH_MAX_LEN];
        uint32_t       width;
        uint32_t       height;
        vf_pixel_fmt_t format;
        uint32_t       buffer_count;
} vf_camera_cfg_t;

#endif /* VF_CAMERA_CFG_H */

