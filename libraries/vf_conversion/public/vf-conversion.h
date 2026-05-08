/**
 **************************************************************************************************
 *  @file           : vf-conversion.h
 *  @brief          : VisionFlow Pixel Format Conversion API Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  VisionFlow pixel format conversion module. Supports RGB888, BGR888 and YUV420P, NV12
 *  conversions using integer BT.601 arithmetic. Operates on vf_framebuffer_t and respects
 *  plane stride layout.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CONVERSION_H
#define VF_CONVERSION_H

#include <stdint.h>

#include "vf-error.h"
#include "vf-framebuffer.h"

typedef struct vf_conversion_ctx_t vf_conversion_ctx_t;

typedef vf_err_t (*vf_convert_fn_t)(const vf_framebuffer_t *src, vf_framebuffer_t *dst);

struct vf_conversion_ctx_t {
        vf_pixel_fmt_t src_fmt;
        vf_pixel_fmt_t dst_fmt;
        vf_convert_fn_t convert_fn;
        int initialized;
};

vf_err_t vf_conversion_init(vf_conversion_ctx_t *ctx, vf_pixel_fmt_t src_fmt,
                            vf_pixel_fmt_t dst_fmt);
vf_err_t vf_conversion_process(vf_conversion_ctx_t *ctx, const vf_framebuffer_t *src,
                               vf_framebuffer_t *dst);
void vf_conversion_deinit(vf_conversion_ctx_t *ctx);
int vf_conversion_is_supported(vf_pixel_fmt_t src_fmt, vf_pixel_fmt_t dst_fmt);

#endif /* VF_CONVERSION_H */

