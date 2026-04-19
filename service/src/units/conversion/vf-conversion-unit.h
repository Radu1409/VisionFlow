/**
 **************************************************************************************************
 *  @file           : vf-conversion-unit.h
 *  @brief          : VisionFlow Conversion Unit API Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Conversion unit — pops a frame from in_queue, converts pixel format,
 *  pushes result to out_queue. Implements the vf_unit_operations_t interface.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CONVERSION_UNIT_H
#define VF_CONVERSION_UNIT_H

#include "vf-buff-pool.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-unit-operations.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        vf_pixel_fmt_t  src_fmt;
        vf_pixel_fmt_t  dst_fmt;
        vf_fb_params_t  dst_params;
        vf_buf_pool_t  *pool;
        vf_buf_pool_t  *src_pool;
} vf_conversion_unit_cfg_t;

vf_err_t vf_conversion_unit_init(void *ctx, ...);
vf_err_t vf_conversion_unit_deinit(void *ctx, ...);
vf_err_t vf_conversion_unit_get_data(void *ctx, ...);
vf_err_t vf_conversion_unit_process_data(void *ctx, ...);
vf_err_t vf_conversion_unit_send_data(void *ctx, ...);

vf_err_t vf_conversion_unit_init_operations(vf_unit_operations_t *ops);

#ifdef __cplusplus
}
#endif

#endif /* VF_CONVERSION_UNIT_H */

