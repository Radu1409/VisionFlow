/**
 **********************************
 *  @file           : vf-framebuffer.c
 *  @brief          : VF framebuffer API
 **********************************
 *  @author         : Radu Purecel
 *
 *  @description:
 *      Implementation of the core framebuffer structure and management functions.
 *      Supports packed and planar pixel formats.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **********************************
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "vf-framebuffer.h"
#include "vf-logger.h"

#define ALIGN_SIZE(x, y)    (((x) + ((y) - 1U)) & ~((y) - 1U))
#define FB_FILENAME_MAX_LEN 256U
#define ONE_LINE            1U

static
void reset_plane_info(vf_framebuffer_t *fb)
{
        (void)memset(fb->plane_width, 0, sizeof(fb->plane_width));
        (void)memset(fb->plane_height, 0, sizeof(fb->plane_height));
        (void)memset(fb->plane_line_size, 0, sizeof(fb->plane_line_size));
        (void)memset(fb->plane_stride, 0, sizeof(fb->plane_stride));
        (void)memset(fb->plane_size, 0, sizeof(fb->plane_size));

        fb->num_planes = 0U;
        fb->total_size = 0U;
}

static
void init_planes_packed(vf_framebuffer_t *fb, const vf_fb_params_t *params)
{
        vf_bpp_t bpp = VF_BPP_INVALID;

        bpp = vf_pixel_fmt_bpp(params->format);

        fb->num_planes = 1U;

        fb->plane_width[0] = params->width;
        fb->plane_height[0] = params->height;
        fb->plane_line_size[0] = params->width * (uint32_t)bpp / 8U;
        fb->plane_stride[0] = ALIGN_SIZE(fb->plane_line_size[0], VF_FB_STRIDE_ALIGN);
        fb->plane_size[0] = fb->plane_stride[0] * fb->plane_height[0];

        fb->total_size = (size_t)fb->plane_size[0];
}

static
void init_planes_yuv420p(vf_framebuffer_t *fb, const vf_fb_params_t *params)
{
        fb->num_planes = 3U;

        /* Y */
        fb->plane_width[0] = params->width;
        fb->plane_height[0] = params->height;
        fb->plane_line_size[0] = params->width;
        fb->plane_stride[0] = ALIGN_SIZE(fb->plane_line_size[0], VF_FB_STRIDE_ALIGN);
        fb->plane_size[0] = fb->plane_stride[0] * fb->plane_height[0];

        /* U */
        fb->plane_width[1] = params->width / 2U;
        fb->plane_height[1] = params->height / 2U;
        fb->plane_line_size[1] = params->width / 2U;
        fb->plane_stride[1] = ALIGN_SIZE(fb->plane_line_size[1], VF_FB_STRIDE_ALIGN);
        fb->plane_size[1] = fb->plane_stride[1] * fb->plane_height[1];

        /* V */
        fb->plane_width[2] = params->width / 2U;
        fb->plane_height[2] = params->height / 2U;
        fb->plane_line_size[2] = params->width / 2U;
        fb->plane_stride[2] = ALIGN_SIZE(fb->plane_line_size[2], VF_FB_STRIDE_ALIGN);
        fb->plane_size[2] = fb->plane_stride[2] * fb->plane_height[2];

        fb->total_size = (size_t)(fb->plane_size[0] + fb->plane_size[1] + fb->plane_size[2]);
}

static
void init_planes_nv12(vf_framebuffer_t *fb, const vf_fb_params_t *params)
{
        fb->num_planes = 2U;

        /* Y */
        fb->plane_width[0] = params->width;
        fb->plane_height[0] = params->height;
        fb->plane_line_size[0] = params->width;
        fb->plane_stride[0] = ALIGN_SIZE(fb->plane_line_size[0], VF_FB_STRIDE_ALIGN);
        fb->plane_size[0] = fb->plane_stride[0] * fb->plane_height[0];

        /* UV interleaved */
        fb->plane_width[1] = params->width;
        fb->plane_height[1] = params->height / 2U;
        fb->plane_line_size[1] = params->width;
        fb->plane_stride[1] = ALIGN_SIZE(fb->plane_line_size[1], VF_FB_STRIDE_ALIGN);
        fb->plane_size[1] = fb->plane_stride[1] * fb->plane_height[1];

        fb->total_size = (size_t)(fb->plane_size[0] + fb->plane_size[1]);
}

static
vf_err_t init_planes(vf_framebuffer_t *fb, const vf_fb_params_t *params)
{
        if ((NULL == fb) || (NULL == params)) {
                log_err("Invalid input: fb=%p, params=%p.", (void *)fb, (void *)params);

                return VF_INVALID_PARAMETER;
        }

        reset_plane_info(fb);

        switch (params->format) {
                case VF_PIXEL_FMT_RGB888:
                case VF_PIXEL_FMT_BGR888:
                case VF_PIXEL_FMT_RGBA8888:
                case VF_PIXEL_FMT_RAW8:
                        init_planes_packed(fb, params);

                        break;

                case VF_PIXEL_FMT_YUV420P:
                        init_planes_yuv420p(fb, params);

                        break;

                case VF_PIXEL_FMT_NV12:
                        init_planes_nv12(fb, params);

                        break;

                case VF_PIXEL_FMT_UNKNOWN:
                case VF_PIXEL_FMT_LAST:
                default:
                        log_err("Invalid pixel format: %d", (int)params->format);

                        return VF_INVALID_PARAMETER;
        }

        log_dbg("Planes initialized: num_planes=%u, total_size=%zu, format=%s.",
                fb->num_planes, fb->total_size, vf_pixel_fmt_str(params->format));

        return VF_SUCCESS;
}

vf_err_t vf_framebuffer_alloc(vf_framebuffer_t *fb, const vf_fb_params_t *params)
{
        vf_err_t rc = VF_SUCCESS;

        if ((NULL == fb) || (NULL == params)) {
                log_err("Invalid input: fb=%p, params=%p.", (void *)fb, (void *)params);

                return VF_INVALID_PARAMETER;
        }

        if ((0U == params->width) || (0U == params->height)) {
                log_err("Invalid dimensions: width=%u, height=%u.", params->width, params->height);

                return VF_INVALID_PARAMETER;
        }

        if ((VF_PIXEL_FMT_UNKNOWN == params->format) || (VF_PIXEL_FMT_LAST == params->format)) {
                log_err("Invalid pixel format.");

                return VF_INVALID_PARAMETER;
        }

        (void)memset(fb, 0, sizeof(*fb));
        fb->params = *params;

        rc = init_planes(fb, params);
        if (VF_SUCCESS != rc) {
                return rc;
        }

        fb->data = (uint8_t *)malloc(fb->total_size);
        if (NULL == fb->data) {
                log_err("Failed to allocate %zu bytes for framebuffer.", fb->total_size);

                return VF_OOM;
        }

        (void)memset(fb->data, 0, fb->total_size);
        fb->owns_memory = 1U;

        log_dbg("Framebuffer allocated: %ux%u, format=%s, total_size=%zu bytes.",
                params->width, params->height, vf_pixel_fmt_str(params->format), fb->total_size);

        return VF_SUCCESS;
}

vf_err_t vf_framebuffer_wrap(vf_framebuffer_t *fb, uint8_t *data, const vf_fb_params_t *params,
                             size_t total_size)
{
        vf_err_t rc = VF_SUCCESS;

        if ((NULL == fb) || (NULL == data) || (NULL == params)) {
                log_err("Invalid input: fb=%p, data=%p, params=%p.",
                        (void *)fb, (void *)data, (void *)params);

                return VF_INVALID_PARAMETER;
        }

        if (0U == total_size) {
                log_err("Invalid total_size: 0.");

                return VF_INVALID_PARAMETER;
        }

        (void)memset(fb, 0, sizeof(*fb));
        fb->params = *params;
        fb->data = data;
        fb->owns_memory = 0U;

        rc = init_planes(fb, params);
        if (VF_SUCCESS != rc) {
                fb->data = NULL;

                return rc;
        }

        fb->total_size = total_size;

        log_dbg("Framebuffer wrapped: %ux%u, format=%s, total_size=%zu bytes.",
                params->width, params->height, vf_pixel_fmt_str(params->format), total_size);

        return VF_SUCCESS;
}

void vf_framebuffer_free(vf_framebuffer_t *fb)
{
        if (NULL == fb) {
                log_wrn("vf_framebuffer_free called with NULL framebuffer.");

                return;
        }

        if ((NULL != fb->data) && (1U == fb->owns_memory)) {
                free(fb->data);
        }

        fb->data = NULL;
        fb->total_size = 0U;
        fb->owns_memory = 0U;
        fb->params.width = 0U;
        fb->params.height = 0U;
        fb->params.format = VF_PIXEL_FMT_UNKNOWN;

        reset_plane_info(fb);

        log_dbg("Framebuffer freed.");
}

vf_err_t vf_framebuffer_copy(vf_framebuffer_t *dst, const vf_framebuffer_t *src)
{
        if ((NULL == dst) || (NULL == src)) {
                log_err("Invalid input: dst=%p, src=%p.", (void *)dst, (void *)src);

                return VF_INVALID_PARAMETER;
        }

        if ((NULL == dst->data) || (NULL == src->data)) {
                log_err("NULL data: dst->data=%p, src->data=%p.",
                        (void *)dst->data, (void *)src->data);

                return VF_INVALID_PARAMETER;
        }

        if (dst->total_size != src->total_size) {
                log_err("Size mismatch: dst=%zu, src=%zu.", dst->total_size, src->total_size);

                return VF_INVALID_PARAMETER;
        }

        if (dst->params.format != src->params.format) {
                log_err("Format mismatch: dst=%s, src=%s.",
                        vf_pixel_fmt_str(dst->params.format),
                        vf_pixel_fmt_str(src->params.format));

                return VF_INVALID_PARAMETER;
        }

        (void)memcpy(dst->data, src->data, src->total_size);

        log_dbg("Framebuffer copied: %ux%u, format=%s, size=%zu bytes.",
                src->params.width, src->params.height,
                vf_pixel_fmt_str(src->params.format), src->total_size);

        return VF_SUCCESS;
}

vf_err_t vf_framebuffer_clear(vf_framebuffer_t *fb)
{
        if ((NULL == fb) || (NULL == fb->data)) {
                log_err("Invalid input: fb=%p.", (void *)fb);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(fb->data, 0, fb->total_size);

        log_dbg("Framebuffer cleared: size=%zu bytes.", fb->total_size);

        return VF_SUCCESS;
}

size_t vf_framebuffer_calculate_size(const vf_fb_params_t *params)
{
        vf_framebuffer_t fb = {0};
        vf_err_t rc = VF_SUCCESS;

        if (NULL == params) {
                log_err("Invalid input: params is NULL.");

                return 0U;
        }

        fb.params = *params;

        rc = init_planes(&fb, params);
        if (VF_SUCCESS != rc) {
                return 0U;
        }

        return fb.total_size;
}

vf_err_t vf_framebuffer_write_to_fptr(const vf_framebuffer_t *fb, FILE *fptr,
                                      size_t *written_bytes)
{
        uint8_t  *plane_ptr = NULL;
        uint8_t  *line_ptr = NULL;
        size_t    written = 0U;
        uint32_t  i = 0U;
        uint32_t  j = 0U;

        if ((NULL == fb) || (NULL == fptr) || (NULL == written_bytes)) {
                log_err("Invalid input: fb=%p, fptr=%p, written_bytes=%p.",
                        (void *)fb, (void *)fptr, (void *)written_bytes);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == fb->data) {
                log_err("Framebuffer data is NULL.");

                return VF_INVALID_PARAMETER;
        }

        *written_bytes = 0U;
        plane_ptr = fb->data;

        for (i = 0U; i < VF_FB_MAX_PLANE_COUNT; i++) {
                if (0U == fb->plane_size[i]) {
                        break;
                }

                line_ptr = plane_ptr;

                for (j = 0U; j < fb->plane_height[i]; j++) {
                        written = fwrite(line_ptr, fb->plane_line_size[i], ONE_LINE, fptr);
                        if (ONE_LINE != written) {
                                log_err("Write failed at plane=%u, line=%u.", i, j);

                                return VF_FILE_ERR;
                        }

                        line_ptr += fb->plane_stride[i];
                        *written_bytes += fb->plane_line_size[i];
                }

                plane_ptr += fb->plane_size[i];
        }

        return VF_SUCCESS;
}

vf_err_t vf_framebuffer_write_to_file(const vf_framebuffer_t *fb, const char *filename_prefix)
{
        char   filename[FB_FILENAME_MAX_LEN] = {0};
        FILE  *fptr = NULL;
        size_t written_bytes = 0U;
        int    len = 0;
        vf_err_t rc = VF_SUCCESS;

        if ((NULL == fb) || (NULL == filename_prefix)) {
                log_err("Invalid input: fb=%p, filename_prefix=%p.",
                        (void *)fb, (void *)filename_prefix);

                return VF_INVALID_PARAMETER;
        }

        len = snprintf(filename, sizeof(filename), "%s_%s_%ux%u.raw",
                       filename_prefix,
                       vf_pixel_fmt_str(fb->params.format),
                       fb->params.width,
                       fb->params.height);

        if ((len < 0) || ((size_t)len >= sizeof(filename))) {
                log_err("Filename truncated or formatting failed.");

                return VF_INVALID_PARAMETER;
        }

        fptr = fopen(filename, "wb");
        if (NULL == fptr) {
                log_err("Failed to open file '%s': %s.", filename, strerror(errno));

                return VF_FILE_ERR;
        }

        rc = vf_framebuffer_write_to_fptr(fb, fptr, &written_bytes);

        (void)fclose(fptr);

        if (VF_SUCCESS == rc) {
                log_info("Frame written to '%s': %zu bytes.", filename, written_bytes);
        }

        return rc;
}

vf_err_t vf_framebuffer_read_from_fptr(vf_framebuffer_t *fb, FILE *fptr, size_t *read_bytes)
{
        uint8_t  *plane_ptr = NULL;
        uint8_t  *line_ptr = NULL;
        size_t    count = 0U;
        uint32_t  i = 0U;
        uint32_t  j = 0U;

        if ((NULL == fb) || (NULL == fptr) || (NULL == read_bytes)) {
                log_err("Invalid input: fb=%p, fptr=%p, read_bytes=%p.",
                        (void *)fb, (void *)fptr, (void *)read_bytes);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == fb->data) {
                log_err("Framebuffer data is NULL.");

                return VF_INVALID_PARAMETER;
        }

        *read_bytes = 0U;
        plane_ptr = fb->data;

        for (i = 0U; i < VF_FB_MAX_PLANE_COUNT; i++) {
                if (0U == fb->plane_size[i]) {
                        break;
                }

                line_ptr = plane_ptr;

                for (j = 0U; j < fb->plane_height[i]; j++) {
                        count = fread(line_ptr, fb->plane_line_size[i], ONE_LINE, fptr);
                        if (ONE_LINE != count) {
                                log_err("Read failed at plane=%u, line=%u.", i, j);

                                return VF_FILE_ERR;
                        }

                        line_ptr += fb->plane_stride[i];
                        *read_bytes += fb->plane_line_size[i];
                }

                plane_ptr += fb->plane_size[i];
        }

        return VF_SUCCESS;
}

vf_err_t vf_framebuffer_read_from_file(vf_framebuffer_t *fb, const char *filename)
{
        FILE    *fptr = NULL;
        size_t   read_bytes = 0U;
        vf_err_t rc = VF_SUCCESS;

        if ((NULL == fb) || (NULL == filename)) {
                log_err("Invalid input: fb=%p, filename=%p.", (void *)fb, (void *)filename);

                return VF_INVALID_PARAMETER;
        }

        fptr = fopen(filename, "rb");
        if (NULL == fptr) {
                log_err("Failed to open file '%s': %s.", filename, strerror(errno));

                return VF_FILE_ERR;
        }

        rc = vf_framebuffer_read_from_fptr(fb, fptr, &read_bytes);

        (void)fclose(fptr);

        if (VF_SUCCESS == rc) {
                log_info("Frame read from '%s': %zu bytes.", filename, read_bytes);
        }

        return rc;
}

vf_bpp_t vf_pixel_fmt_bpp(vf_pixel_fmt_t format)
{
        switch (format) {
                case VF_PIXEL_FMT_RAW8:
                        return VF_BPP_8;
                case VF_PIXEL_FMT_RGB888:
                case VF_PIXEL_FMT_BGR888:
                        return VF_BPP_24;
                case VF_PIXEL_FMT_RGBA8888:
                        return VF_BPP_32;
                case VF_PIXEL_FMT_YUV420P:
                case VF_PIXEL_FMT_NV12:
                case VF_PIXEL_FMT_UNKNOWN:
                case VF_PIXEL_FMT_LAST:
                default:
                        return VF_BPP_INVALID;
        }
}

bool vf_pixel_fmt_is_yuv(vf_pixel_fmt_t format)
{
        switch (format) {
                case VF_PIXEL_FMT_YUV420P:
                case VF_PIXEL_FMT_NV12:
                        return true;

                case VF_PIXEL_FMT_RGB888:
                case VF_PIXEL_FMT_BGR888:
                case VF_PIXEL_FMT_RGBA8888:
                case VF_PIXEL_FMT_RAW8:
                case VF_PIXEL_FMT_UNKNOWN:
                case VF_PIXEL_FMT_LAST:
                default:
                        return false;
        }
}

const char *vf_pixel_fmt_str(vf_pixel_fmt_t format)
{
        switch (format) {
                case VF_PIXEL_FMT_RGB888:
                        return "RGB888";
                case VF_PIXEL_FMT_BGR888:
                        return "BGR888";
                case VF_PIXEL_FMT_RGBA8888:
                        return "RGBA8888";
                case VF_PIXEL_FMT_YUV420P:
                        return "YUV420P";
                case VF_PIXEL_FMT_NV12:
                        return "NV12";
                case VF_PIXEL_FMT_RAW8:
                        return "RAW8";
                case VF_PIXEL_FMT_UNKNOWN:
                case VF_PIXEL_FMT_LAST:
                default:
                        return "UNKNOWN";
        }
}

