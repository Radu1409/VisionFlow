/**
 **************************************************************************************************
 *  @file           : vf-conversion.c
 *  @brief          : VisionFlow Pixel Format Conversion API
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

#include <stdint.h>
#include <string.h>

#include "vf-conversion.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"

/* =========================================================================
 * BT.601 integer arithmetic helpers
 *
 * RGB -> YUV:
 *   Y =  ((66*R + 129*G +  25*B + 128) >> 8) + 16
 *   U = ((-38*R -  74*G + 112*B + 128) >> 8) + 128
 *   V = ((112*R -  94*G -  18*B + 128) >> 8) + 128
 *
 * YUV -> RGB:
 *   C = Y - 16
 *   D = U - 128
 *   E = V - 128
 *   R = clip((298*C           + 409*E + 128) >> 8)
 *   G = clip((298*C - 100*D   - 208*E + 128) >> 8)
 *   B = clip((298*C + 516*D           + 128) >> 8)
 * ========================================================================= */

#define CLIP_U8(x)  ((uint8_t)(((x) < 0) ? 0 : (((x) > 255) ? 255 : (x))))

static
uint8_t rgb_to_y(uint8_t r, uint8_t g, uint8_t b)
{
        int y = ((66 * (int)r + 129 * (int)g + 25 * (int)b + 128) >> 8) + 16;

        return CLIP_U8(y);
}

static
uint8_t rgb_to_u(uint8_t r, uint8_t g, uint8_t b)
{
        int u = ((-38 * (int)r - 74 * (int)g + 112 * (int)b + 128) >> 8) + 128;

        return CLIP_U8(u);
}

static
uint8_t rgb_to_v(uint8_t r, uint8_t g, uint8_t b)
{
        int v = ((112 * (int)r - 94 * (int)g - 18 * (int)b + 128) >> 8) + 128;

        return CLIP_U8(v);
}

static
uint8_t yuv_to_r(int c, int e)
{
        int r = (298 * c + 409 * e + 128) >> 8;

        return CLIP_U8(r);
}

static
uint8_t yuv_to_g(int c, int d, int e)
{
        int g = (298 * c - 100 * d - 208 * e + 128) >> 8;

        return CLIP_U8(g);
}

static
uint8_t yuv_to_b(int c, int d)
{
        int b = (298 * c + 516 * d + 128) >> 8;

        return CLIP_U8(b);
}

/* =========================================================================
 * Validation helpers
 * ========================================================================= */

static
int validate_framebuffers(const vf_framebuffer_t *src, const vf_framebuffer_t *dst)
{
        if ((NULL == src) || (NULL == dst)) {
                log_err("NULL framebuffer: src=%p dst=%p", (void *)src, (void *)dst);

                return -1;
        }

        if ((NULL == src->data) || (NULL == dst->data)) {
                log_err("NULL data: src->data=%p dst->data=%p",
                        (void *)src->data, (void *)dst->data);

                return -1;
        }

        if ((src->params.width != dst->params.width) ||
            (src->params.height != dst->params.height)) {
                log_err("Dimension mismatch: src=%ux%u dst=%ux%u",
                        src->params.width, src->params.height,
                        dst->params.width, dst->params.height);

                return -1;
        }

        return 0;
}

/* =========================================================================
 * RAW8 -> RGB888
 * ========================================================================= */

static
vf_err_t convert_raw8_to_rgb888(const vf_framebuffer_t *src, vf_framebuffer_t *dst)
{
        const uint8_t *src_row = NULL;
        uint8_t       *dst_row = NULL;
        uint32_t       width   = 0U;
        uint32_t       height  = 0U;
        uint32_t       x       = 0U;
        uint32_t       y       = 0U;
        uint8_t        pixel   = 0U;

        if (0 != validate_framebuffers(src, dst)) {
                return VF_CONV_PROCESSING_ERR;
        }

        width  = src->params.width;
        height = src->params.height;

        for (y = 0U; y < height; y++) {
                src_row = src->data + y * src->plane_stride[0];
                dst_row = dst->data + y * dst->plane_stride[0];

                for (x = 0U; x < width; x++) {
                        pixel = src_row[x];

                        dst_row[x * 3U + 0U] = pixel;
                        dst_row[x * 3U + 1U] = pixel;
                        dst_row[x * 3U + 2U] = pixel;
                }
        }

        log_dbg("RAW8 -> RGB888: %ux%u", width, height);

        return VF_SUCCESS;
}

/* =========================================================================
 * RAW8 -> YUV420P
 * ========================================================================= */

static
vf_err_t convert_raw8_to_yuv420p(const vf_framebuffer_t *src, vf_framebuffer_t *dst)
{
        const uint8_t *src_row = NULL;
        uint8_t       *y_row   = NULL;
        uint8_t       *u_plane = NULL;
        uint8_t       *v_plane = NULL;
        uint32_t       width   = 0U;
        uint32_t       height  = 0U;
        uint32_t       x       = 0U;
        uint32_t       y       = 0U;

        if (0 != validate_framebuffers(src, dst)) {
                return VF_CONV_PROCESSING_ERR;
        }

        width   = src->params.width;
        height  = src->params.height;
        u_plane = dst->data + dst->plane_size[0];
        v_plane = u_plane   + dst->plane_size[1];

        /* Fill U and V planes with 128 (neutral chroma for grayscale) */
        (void)memset(u_plane, 128U, dst->plane_size[1]);
        (void)memset(v_plane, 128U, dst->plane_size[2]);

        for (y = 0U; y < height; y++) {
                src_row = src->data + y * src->plane_stride[0];
                y_row   = dst->data + y * dst->plane_stride[0];

                for (x = 0U; x < width; x++) {
                        y_row[x] = src_row[x];
                }
        }

        log_dbg("RAW8 -> YUV420P: %ux%u", width, height);

        return VF_SUCCESS;
}

/* =========================================================================
 * RGB888 -> YUV420P
 * ========================================================================= */

static
vf_err_t convert_rgb888_to_yuv420p(const vf_framebuffer_t *src, vf_framebuffer_t *dst)
{
        const uint8_t *src_row = NULL;
        uint8_t       *y_row   = NULL;
        uint8_t       *u_plane = NULL;
        uint8_t       *v_plane = NULL;
        uint32_t       width   = 0U;
        uint32_t       height  = 0U;
        uint32_t       x       = 0U;
        uint32_t       y       = 0U;
        uint8_t        r       = 0U;
        uint8_t        g       = 0U;
        uint8_t        b       = 0U;

        if (0 != validate_framebuffers(src, dst)) {
                return VF_CONV_PROCESSING_ERR;
        }

        width  = src->params.width;
        height = src->params.height;

        u_plane = dst->data + dst->plane_size[0];
        v_plane = u_plane   + dst->plane_size[1];

        for (y = 0U; y < height; y++) {
                src_row = src->data + y * src->plane_stride[0];
                y_row   = dst->data + y * dst->plane_stride[0];

                for (x = 0U; x < width; x++) {
                        r = src_row[x * 3U + 0U];
                        g = src_row[x * 3U + 1U];
                        b = src_row[x * 3U + 2U];

                        y_row[x] = rgb_to_y(r, g, b);

                        /* U and V sampled once per 2x2 block */
                        if ((0U == (y & 1U)) && (0U == (x & 1U))) {
                                uint32_t uv_x = x / 2U;
                                uint32_t uv_y = y / 2U;

                                u_plane[uv_y * dst->plane_stride[1] + uv_x] = rgb_to_u(r, g, b);
                                v_plane[uv_y * dst->plane_stride[2] + uv_x] = rgb_to_v(r, g, b);
                        }
                }
        }

        log_dbg("RGB888 -> YUV420P: %ux%u", width, height);

        return VF_SUCCESS;
}

/* =========================================================================
 * BGR888 -> YUV420P
 * ========================================================================= */

static
vf_err_t convert_bgr888_to_yuv420p(const vf_framebuffer_t *src, vf_framebuffer_t *dst)
{
        const uint8_t *src_row = NULL;
        uint8_t       *y_row   = NULL;
        uint8_t       *u_plane = NULL;
        uint8_t       *v_plane = NULL;
        uint32_t       width   = 0U;
        uint32_t       height  = 0U;
        uint32_t       x       = 0U;
        uint32_t       y       = 0U;
        uint8_t        r       = 0U;
        uint8_t        g       = 0U;
        uint8_t        b       = 0U;

        if (0 != validate_framebuffers(src, dst)) {
                return VF_CONV_PROCESSING_ERR;
        }

        width  = src->params.width;
        height = src->params.height;

        u_plane = dst->data + dst->plane_size[0];
        v_plane = u_plane   + dst->plane_size[1];

        for (y = 0U; y < height; y++) {
                src_row = src->data + y * src->plane_stride[0];
                y_row   = dst->data + y * dst->plane_stride[0];

                for (x = 0U; x < width; x++) {
                        /* BGR order */
                        b = src_row[x * 3U + 0U];
                        g = src_row[x * 3U + 1U];
                        r = src_row[x * 3U + 2U];

                        y_row[x] = rgb_to_y(r, g, b);

                        if ((0U == (y & 1U)) && (0U == (x & 1U))) {
                                uint32_t uv_x = x / 2U;
                                uint32_t uv_y = y / 2U;

                                u_plane[uv_y * dst->plane_stride[1] + uv_x] = rgb_to_u(r, g, b);
                                v_plane[uv_y * dst->plane_stride[2] + uv_x] = rgb_to_v(r, g, b);
                        }
                }
        }

        log_dbg("BGR888 -> YUV420P: %ux%u", width, height);

        return VF_SUCCESS;
}

/* =========================================================================
 * YUV420P -> RGB888
 * ========================================================================= */

static
vf_err_t convert_yuv420p_to_rgb888(const vf_framebuffer_t *src, vf_framebuffer_t *dst)
{
        const uint8_t *y_plane  = NULL;
        const uint8_t *u_plane  = NULL;
        const uint8_t *v_plane  = NULL;
        uint8_t       *dst_row  = NULL;
        uint32_t       width    = 0U;
        uint32_t       height   = 0U;
        uint32_t       x        = 0U;
        uint32_t       y        = 0U;
        int            yv       = 0;
        int            c        = 0;
        int            d        = 0;
        int            e        = 0;

        if (0 != validate_framebuffers(src, dst)) {
                return VF_CONV_PROCESSING_ERR;
        }

        width   = src->params.width;
        height  = src->params.height;

        y_plane = src->data;
        u_plane = y_plane + src->plane_size[0];
        v_plane = u_plane + src->plane_size[1];

        for (y = 0U; y < height; y++) {
                dst_row = dst->data + y * dst->plane_stride[0];

                for (x = 0U; x < width; x++) {
                        uint32_t uv_x = x / 2U;
                        uint32_t uv_y = y / 2U;

                        yv = (int)y_plane[y * src->plane_stride[0] + x];
                        c  = yv - 16;
                        d  = (int)u_plane[uv_y * src->plane_stride[1] + uv_x] - 128;
                        e  = (int)v_plane[uv_y * src->plane_stride[2] + uv_x] - 128;

                        dst_row[x * 3U + 0U] = yuv_to_r(c, e);
                        dst_row[x * 3U + 1U] = yuv_to_g(c, d, e);
                        dst_row[x * 3U + 2U] = yuv_to_b(c, d);
                }
        }

        log_dbg("YUV420P -> RGB888: %ux%u", width, height);

        return VF_SUCCESS;
}

/* =========================================================================
 * RGB888 -> NV12
 * ========================================================================= */

static
vf_err_t convert_rgb888_to_nv12(const vf_framebuffer_t *src, vf_framebuffer_t *dst)
{
        const uint8_t *src_row  = NULL;
        uint8_t       *y_row    = NULL;
        uint8_t       *uv_plane = NULL;
        uint32_t       width    = 0U;
        uint32_t       height   = 0U;
        uint32_t       x        = 0U;
        uint32_t       y        = 0U;
        uint8_t        r        = 0U;
        uint8_t        g        = 0U;
        uint8_t        b        = 0U;

        if (0 != validate_framebuffers(src, dst)) {
                return VF_CONV_PROCESSING_ERR;
        }

        width   = src->params.width;
        height  = src->params.height;
        uv_plane = dst->data + dst->plane_size[0];

        for (y = 0U; y < height; y++) {
                src_row = src->data + y * src->plane_stride[0];
                y_row   = dst->data + y * dst->plane_stride[0];

                for (x = 0U; x < width; x++) {
                        r = src_row[x * 3U + 0U];
                        g = src_row[x * 3U + 1U];
                        b = src_row[x * 3U + 2U];

                        y_row[x] = rgb_to_y(r, g, b);

                        /* UV interleaved: U at even, V at odd, sampled per 2x2 block */
                        if ((0U == (y & 1U)) && (0U == (x & 1U))) {
                                uint32_t uv_x = x;
                                uint32_t uv_y = y / 2U;

                                uv_plane[uv_y * dst->plane_stride[1] + uv_x + 0U] = rgb_to_u(r, g, b);
                                uv_plane[uv_y * dst->plane_stride[1] + uv_x + 1U] = rgb_to_v(r, g, b);
                        }
                }
        }

        log_dbg("RGB888 -> NV12: %ux%u", width, height);

        return VF_SUCCESS;
}

/* =========================================================================
 * NV12 -> RGB888
 * ========================================================================= */

static
vf_err_t convert_nv12_to_rgb888(const vf_framebuffer_t *src, vf_framebuffer_t *dst)
{
        const uint8_t *y_plane  = NULL;
        const uint8_t *uv_plane = NULL;
        uint8_t       *dst_row  = NULL;
        uint32_t       width    = 0U;
        uint32_t       height   = 0U;
        uint32_t       x        = 0U;
        uint32_t       y        = 0U;
        int            c        = 0;
        int            d        = 0;
        int            e        = 0;

        if (0 != validate_framebuffers(src, dst)) {
                return VF_CONV_PROCESSING_ERR;
        }

        width   = src->params.width;
        height  = src->params.height;

        y_plane  = src->data;
        uv_plane = y_plane + src->plane_size[0];

        for (y = 0U; y < height; y++) {
                dst_row = dst->data + y * dst->plane_stride[0];

                for (x = 0U; x < width; x++) {
                        uint32_t uv_y = y / 2U;
                        uint32_t uv_x = (x & ~1U); /* align to even */

                        c = (int)y_plane[y * src->plane_stride[0] + x] - 16;
                        d = (int)uv_plane[uv_y * src->plane_stride[1] + uv_x + 0U] - 128;
                        e = (int)uv_plane[uv_y * src->plane_stride[1] + uv_x + 1U] - 128;

                        dst_row[x * 3U + 0U] = yuv_to_r(c, e);
                        dst_row[x * 3U + 1U] = yuv_to_g(c, d, e);
                        dst_row[x * 3U + 2U] = yuv_to_b(c, d);
                }
        }

        log_dbg("NV12 -> RGB888: %ux%u", width, height);

        return VF_SUCCESS;
}

/* =========================================================================
 * Conversion table lookup
 * ========================================================================= */

typedef struct {
        vf_pixel_fmt_t  src_fmt;
        vf_pixel_fmt_t  dst_fmt;
        vf_convert_fn_t fn;
} vf_conversion_entry_t;

static const vf_conversion_entry_t g_conversion_table[] = {
        { VF_PIXEL_FMT_RGB888,  VF_PIXEL_FMT_YUV420P, convert_rgb888_to_yuv420p },
        { VF_PIXEL_FMT_BGR888,  VF_PIXEL_FMT_YUV420P, convert_bgr888_to_yuv420p },
        { VF_PIXEL_FMT_YUV420P, VF_PIXEL_FMT_RGB888,  convert_yuv420p_to_rgb888 },
        { VF_PIXEL_FMT_RGB888,  VF_PIXEL_FMT_NV12,    convert_rgb888_to_nv12    },
        { VF_PIXEL_FMT_NV12,    VF_PIXEL_FMT_RGB888,  convert_nv12_to_rgb888    },
        { VF_PIXEL_FMT_RAW8,    VF_PIXEL_FMT_RGB888,  convert_raw8_to_rgb888    },
        { VF_PIXEL_FMT_RAW8,    VF_PIXEL_FMT_YUV420P, convert_raw8_to_yuv420p   },
};

#define CONVERSION_TABLE_SIZE \
        (sizeof(g_conversion_table) / sizeof(g_conversion_table[0]))

static
vf_convert_fn_t lookup_convert_fn(vf_pixel_fmt_t src_fmt, vf_pixel_fmt_t dst_fmt)
{
        size_t i = 0U;

        for (i = 0U; i < CONVERSION_TABLE_SIZE; i++) {
                if ((g_conversion_table[i].src_fmt == src_fmt) &&
                    (g_conversion_table[i].dst_fmt == dst_fmt)) {
                        return g_conversion_table[i].fn;
                }
        }

        return NULL;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_conversion_init(vf_conversion_ctx_t *ctx,
                             vf_pixel_fmt_t       src_fmt,
                             vf_pixel_fmt_t       dst_fmt)
{
        vf_convert_fn_t fn = NULL;

        if (NULL == ctx) {
                log_err("Invalid param: ctx=NULL");

                return VF_INVALID_PARAMETER;
        }

        (void)memset(ctx, 0, sizeof(*ctx));

        fn = lookup_convert_fn(src_fmt, dst_fmt);
        if (NULL == fn) {
                log_err("Unsupported conversion: %s -> %s",
                        vf_pixel_fmt_str(src_fmt),
                        vf_pixel_fmt_str(dst_fmt));

                return VF_CONV_INIT_ERR;
        }

        ctx->src_fmt     = src_fmt;
        ctx->dst_fmt     = dst_fmt;
        ctx->convert_fn  = fn;
        ctx->initialized = 1;

        log_info("Conversion initialized: %s -> %s",
                 vf_pixel_fmt_str(src_fmt),
                 vf_pixel_fmt_str(dst_fmt));

        return VF_SUCCESS;
}

vf_err_t vf_conversion_process(vf_conversion_ctx_t    *ctx,
                                const vf_framebuffer_t *src,
                                vf_framebuffer_t       *dst)
{
        if (NULL == ctx) {
                log_err("Invalid param: ctx=NULL");

                return VF_INVALID_PARAMETER;
        }

        if (0 == ctx->initialized) {
                log_err("Conversion context not initialized");

                return VF_CONV_PROCESSING_ERR;
        }

        if ((NULL == src) || (NULL == dst)) {
                log_err("Invalid params: src=%p dst=%p", (void *)src, (void *)dst);

                return VF_INVALID_PARAMETER;
        }

        if (src->params.format != ctx->src_fmt) {
                log_err("Source format mismatch: expected %s, got %s",
                        vf_pixel_fmt_str(ctx->src_fmt),
                        vf_pixel_fmt_str(src->params.format));

                return VF_CONV_PROCESSING_ERR;
        }

        if (dst->params.format != ctx->dst_fmt) {
                log_err("Destination format mismatch: expected %s, got %s",
                        vf_pixel_fmt_str(ctx->dst_fmt),
                        vf_pixel_fmt_str(dst->params.format));

                return VF_CONV_PROCESSING_ERR;
        }

        return ctx->convert_fn(src, dst);
}

void vf_conversion_deinit(vf_conversion_ctx_t *ctx)
{
        if (NULL == ctx) {
                log_wrn("vf_conversion_deinit called with NULL ctx");

                return;
        }

        (void)memset(ctx, 0, sizeof(*ctx));

        log_dbg("Conversion context deinitialized");
}

int vf_conversion_is_supported(vf_pixel_fmt_t src_fmt, vf_pixel_fmt_t dst_fmt)
{
        return (NULL != lookup_convert_fn(src_fmt, dst_fmt)) ? 1 : 0;
}

