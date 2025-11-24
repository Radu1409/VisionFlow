/**
 **************************************************************************************************
 *  @file           : vf-conversion.h
 *  @brief          : Framebuffer conversions API header
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *  Framebuffer color format, size and merge conversions API for Vision Flow service
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_CONVERSION__H
#define VF_CONVERSION__H

#include <stdint.h>
#include <stddef.h>
#include "vf-error.h"
#include "vf-framebuffer.h"

typedef enum {
    VF_CONVERSION_OK = 0,
    VF_CONVERSION_ERR_INVALID_ARG = -1,
    VF_CONVERSION_ERR_SIZE_MISMATCH = -2
} vf_conversion_status_t;

/** 
* Conversion from planar YUV420 (Y + U + V) to RGB24 (3 bytes/pixel). 
* 
* Layout input: 
* - Y plane: width * height bytes 
* - U plane: (width/2) * (height/2) bytes 
* - V plane: (width/2) * (height/2) bytes 
* Buffer is: [Y][U][V] 
*/
vf_err_t
vf_yuv420_to_rgb24(const uint8_t *yuv,
                   size_t yuv_size,
                   uint8_t *rgb,
                   size_t rgb_size,
                   uint32_t width,
                   uint32_t height);

/**
* Simple planar YUV420 copy (useful if you just want to rewrite the files
* to a .yuv extension, but the format remains the same YUV420).
*/
vf_err_t
vf_yuv420_copy(const uint8_t *yuv_src,
               size_t yuv_src_size,
               uint8_t *yuv_dst,
               size_t yuv_dst_size,
               uint32_t width,
               uint32_t height);

vf_err_t
vf_rgb24_resize_nearest(const uint8_t *src,
                        size_t src_size,
                        uint8_t *dst,
                        size_t dst_size,
                        uint32_t src_width,
                        uint32_t src_height,
                        uint32_t dst_width,
                        uint32_t dst_height);

#endif /* VF_CONVERSION__H */

