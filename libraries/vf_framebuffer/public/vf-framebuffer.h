/**
 **************************************************************************************************
 *  @file           : vf-framebuffer.h
 *  @brief          : VF framebuffer API Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *      Defines the core framebuffer structure and API used across all VisionFlow modules.
 *      A framebuffer represents a single video frame in memory and supports both packed
 *      (RGB, RGBA, RAW8) and planar (YUV420P, NV12) pixel formats.
 *
 *      All pipeline units exchange data via vf_framebuffer_t pointers in order to enable
 *      zero-copy frame passing whenever possible.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_FRAMEBUFFER_H
#define VF_FRAMEBUFFER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vf-error.h"
#include "vf-pixel-fmt.h"

#define VF_FB_MAX_PLANE_COUNT     4U
#define VF_FB_STRIDE_ALIGN        16U

#define VF_PIXEL_FMT_RGB888_STR   "RGB888"
#define VF_PIXEL_FMT_BGR888_STR   "BGR888"
#define VF_PIXEL_FMT_RGBA8888_STR "RGBA8888"
#define VF_PIXEL_FMT_YUV420P_STR  "YUV420P"
#define VF_PIXEL_FMT_NV12_STR     "NV12"
#define VF_PIXEL_FMT_YUYV_STR     "YUYV"
#define VF_PIXEL_FMT_MJPEG_STR    "MJPEG"
#define VF_PIXEL_FMT_RAW8_STR     "RAW8"
#define VF_PIXEL_FMT_UNKNOWN_STR  "UNKNOWN"

typedef vf_err_t (*vf_fb_release_fn_t)(void *pool, void *fb);

typedef enum {
        VF_BPP_INVALID = 0,
        VF_BPP_8       = 8,
        VF_BPP_16      = 16,
        VF_BPP_24      = 24,
        VF_BPP_32      = 32
} vf_bpp_t;

typedef struct {
        uint32_t       width;
        uint32_t       height;
        vf_pixel_fmt_t format;
} vf_fb_params_t;

typedef struct {
        uint32_t       frame_id;
        uint32_t       sequence_index;
        uint64_t       timestamp_ms;
        const char    *source_name;
        uint32_t       width;
        uint32_t       height;
        vf_pixel_fmt_t format;
} vf_frame_meta_t;

typedef struct {
        uint8_t        *data;
        size_t          total_size;
        uint8_t         owns_memory;

        vf_fb_params_t  params;

        uint32_t        num_planes;
        uint32_t        plane_width[VF_FB_MAX_PLANE_COUNT];
        uint32_t        plane_height[VF_FB_MAX_PLANE_COUNT];
        uint32_t        plane_line_size[VF_FB_MAX_PLANE_COUNT];
        uint32_t        plane_stride[VF_FB_MAX_PLANE_COUNT];
        uint32_t        plane_size[VF_FB_MAX_PLANE_COUNT];

        vf_frame_meta_t meta;

        atomic_uint ref_count;
} vf_framebuffer_t;

vf_err_t vf_framebuffer_alloc(vf_framebuffer_t *fb, const vf_fb_params_t *params);
vf_err_t vf_framebuffer_wrap(vf_framebuffer_t *fb, uint8_t *data, const vf_fb_params_t *params,
                             size_t total_size);
void vf_framebuffer_free(vf_framebuffer_t *fb);
vf_err_t vf_framebuffer_copy(vf_framebuffer_t *dst, const vf_framebuffer_t *src);
vf_err_t vf_framebuffer_clear(vf_framebuffer_t *fb);
size_t vf_framebuffer_calculate_size(const vf_fb_params_t *params);
vf_err_t vf_framebuffer_write_to_file(const vf_framebuffer_t *fb, const char *filename_prefix);
vf_err_t vf_framebuffer_write_to_fptr(const vf_framebuffer_t *fb, FILE *fptr,
                                      size_t *written_bytes);
vf_err_t vf_framebuffer_read_from_file(vf_framebuffer_t *fb, const char *filename);
vf_err_t vf_framebuffer_read_from_fptr(vf_framebuffer_t *fb, FILE *fptr, size_t *read_bytes);
void vf_framebuffer_set_meta(vf_framebuffer_t *fb, uint32_t frame_id, uint32_t sequence_index,
                             const char *source_name, uint32_t width, uint32_t height,
                             vf_pixel_fmt_t format);
void vf_framebuffer_ref(vf_framebuffer_t *fb);
vf_err_t vf_framebuffer_unref(vf_framebuffer_t *fb, void *pool, vf_fb_release_fn_t release_fn);

vf_bpp_t vf_pixel_fmt_bpp(vf_pixel_fmt_t format);
bool vf_pixel_fmt_is_yuv(vf_pixel_fmt_t format);
const char *vf_pixel_fmt_str(vf_pixel_fmt_t format);

#endif /* VF_FRAMEBUFFER_H */

