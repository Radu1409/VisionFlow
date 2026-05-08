/**
 **************************************************************************************************
 *  @file           : vf-pixel-fmt.h
 *  @brief          : VisionFlow pixel format definitions
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_PIXEL_FMT_H
#define VF_PIXEL_FMT_H

/* *DISABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */
typedef enum {
        VF_PIXEL_FMT_UNKNOWN  = 0,
        VF_PIXEL_FMT_RGB888   = 1,
        VF_PIXEL_FMT_BGR888   = 2,
        VF_PIXEL_FMT_RGBA8888 = 3,
        VF_PIXEL_FMT_YUV420P  = 4,
        VF_PIXEL_FMT_NV12     = 5,
        VF_PIXEL_FMT_RAW8     = 6,
        VF_PIXEL_FMT_YUYV     = 7,    /* Native V4L2 webcam format */
        VF_PIXEL_FMT_MJPEG    = 8,    /* Native V4L2 webcam format */
        VF_PIXEL_FMT_LAST
} vf_pixel_fmt_t;
/* *ENABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */

#endif /* VF_PIXEL_FMT_H */

