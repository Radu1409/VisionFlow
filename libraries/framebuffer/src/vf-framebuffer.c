/**
 **************************************************************************************************
 *  @file           : vf-framebuffer.c
 *  @brief          : Vision Flow framebuffer source
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *  Implementation of allocation and storage used for frames in Vision Flow library
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"
#include "vf-mem.h"

#define BITS_IN_BYTE                    8
#define MAX_PLANE_COUNT                 4
#define STRIDE_ALIGN_VAL                128
#define HEIGHT_ALIGN_VAL                32
#define ALIGN_SIZE(x, y)                (((x) + ((y) - 1U)) & ~((y) - 1U))

#define VF_COLOR_FMT_INVALID_STR        "UNKWN_FMT"
#define VF_COLOR_FMT_422_YUYV_STR       "422_YUYV"
#define VF_COLOR_FMT_420_Y_UV_STR       "420_Y_UV"
#define VF_COLOR_FMT_888_RGB_STR        "888_RGB"

vf_color_fmt_bpp_t vf_framebuffer_get_color_fmt_bpp(vf_color_fmt_t color_fmt)
{
        switch (color_fmt) {
        case VF_COLOR_FMT_420_Y_UV:
                return VF_COLOR_FMT_BPP_12;
        case VF_COLOR_FMT_422_YUYV:
                return VF_COLOR_FMT_BPP_16;
        case VF_COLOR_FMT_888_RGB:
                return VF_COLOR_FMT_BPP_24;
        case VF_COLOR_FMT_INVALID:
        default:
                log_error("Invalid input: wrong or unsupported color format: %d\n", color_fmt);

                return VF_COLOR_FMT_BPP_INVALID;
        }
}

static
void init_framebuffer_data(vf_fb_st_t *fb, const vf_fb_params_st_t *p)
{
        int i = 0;

        if (NULL == fb || NULL == p) {
                log_error("Invalid input: fb = %p, p = %p\n", fb, p);

                return;
        }

        memset(fb, 0, sizeof(vf_fb_st_t));

        fb->params = *p;

        switch (p->color_fmt) {
        case VF_COLOR_FMT_420_Y_UV:
                fb->plane_width[0] = p->width;
                fb->plane_height[0] = ALIGN_SIZE(p->height, HEIGHT_ALIGN_VAL);
                fb->plane_stride[0] = ALIGN_SIZE(fb->plane_width[0], STRIDE_ALIGN_VAL);
                fb->plane_size[0] = fb->plane_stride[0] *
                                    ALIGN_SIZE(fb->plane_height[0], HEIGHT_ALIGN_VAL);

                fb->plane_width[1] = p->width;
                fb->plane_height[1] = ALIGN_SIZE(p->height / 2, HEIGHT_ALIGN_VAL);
                fb->plane_stride[1] = ALIGN_SIZE(fb->plane_width[1], STRIDE_ALIGN_VAL);
                fb->plane_size[1] = fb->plane_stride[1] *
                                    ALIGN_SIZE(fb->plane_height[1], HEIGHT_ALIGN_VAL);

                break;
        case VF_COLOR_FMT_422_YUYV:
                fb->plane_width[0] = p->width;
                fb->plane_height[0] = p->height;
                fb->plane_stride[0] = ALIGN_SIZE(fb->plane_width[0] *
                                                 vf_framebuffer_get_color_fmt_bpp(p->color_fmt) /
                                                 BITS_IN_BYTE, STRIDE_ALIGN_VAL);
                fb->plane_size[0] = fb->plane_stride[0] *
                                    ALIGN_SIZE(fb->plane_height[0], HEIGHT_ALIGN_VAL);

                break;
        case VF_COLOR_FMT_888_RGB:
                fb->plane_width[0] = p->width;
                fb->plane_height[0] = p->height;
                fb->plane_stride[0] = ALIGN_SIZE(fb->plane_width[0] *
                                                 vf_framebuffer_get_color_fmt_bpp(p->color_fmt) /
                                                 BITS_IN_BYTE, STRIDE_ALIGN_VAL);
                fb->plane_size[0] = fb->plane_stride[0] *
                                    ALIGN_SIZE(fb->plane_height[0], HEIGHT_ALIGN_VAL);

                break;
        case VF_COLOR_FMT_INVALID:
        default:
                log_error("Invalid input: wrong or unsupported color format: %d\n",
                        p->color_fmt);

                break;
        }

        for (i = 0; i < MAX_PLANE_COUNT; i++) {
                fb->total_size += fb->plane_size[i];

                log_info("Plane[%d] width = %u, height = %u, stride = %u, size = %u\n",
                         i,
                         fb->plane_width[i],
                         fb->plane_height[i],
                         fb->plane_stride[i],
                         fb->plane_size[i]);
        }
}

int vf_init_framebuffer(vf_fb_st_t *fb, const vf_fb_params_st_t *params, void *virt_addr)
{
        if (NULL == fb || NULL == params || NULL == virt_addr) {
            log_error("Invalid input: fb = %p, params = %p, virt_addr = %p\n",
                    fb, params, virt_addr);

            return EINVAL;
        }

        init_framebuffer_data(fb, params);

        fb->virt_addr = virt_addr;
        fb->phys_addr = NULL;
        fb->handle = NULL;

        if (NULL == fb->phys_addr) {
                log_error("Failed to get physical address for pmem buffer\n");

                return EINVAL;
        }

        log_info("Buffer initialized with parameters: "
                 "virt_addr = %p, color_fmt = %u, total_size = %zu\n", virt_addr,
                 fb->params.color_fmt,
                 fb->total_size);

        return 0;
}

vf_fb_st_t *vf_alloc_framebuffer(const vf_fb_params_st_t *params)
{
        vf_fb_st_t *fb = NULL;

        if (NULL == params) {
                log_error("Invalid input param: params = %p\n", params);

                return NULL;
        }

        fb = alloc_data(1, vf_fb_st_t);
        if (NULL == fb) {
                log_error("Failed to allocate framebuffer\n");

                return NULL;
        }

        fb->params = *params;
        init_framebuffer_data(fb, params);

        if (0 == fb->total_size) {
                log_error("Failed to allocate framebuffer\n");

                free(fb);

                return NULL;
        }

        fb->virt_addr = malloc(fb->total_size);
        if (NULL == fb->virt_addr) {
                log_error("Failed to allocate buffer via pmem\n");

                // goto clear_fb_on_fail;
                free(fb);

                return NULL;
        }

        memset(fb->virt_addr, 0, fb->total_size);

        fb->phys_addr = NULL;
        fb->handle = NULL;

        log_info("Buffer allocated: virt_addr = %p, color_fmt = %u, total_size = %zu\n",
                 fb->virt_addr,
                 fb->params.color_fmt,
                 fb->total_size);

        return fb;

// clear_pmem_buf_on_fail:
//         pmem_free(fb->virt_addr);
// clear_fb_on_fail:
//         clear_data(&fb);

//         return NULL;
}

void vf_free_framebuffer(vf_fb_st_t **fb)
{
        if (NULL == fb) {
                log_error("Invalid input param: fb = %p\n", fb);

                return;
        }

        if (NULL == *fb) {
                log_error("Invalid input param: *fb = %p\n", *fb);

                return;
        }

        if ((*fb)->virt_addr != NULL) {
                free((*fb)->virt_addr);
                (*fb)->virt_addr = NULL;
        }

        (*fb)->virt_addr = NULL;
        (*fb)->phys_addr = NULL;

        free(*fb);
        *fb = NULL;
}

static
const char *get_color_fmt_str(vf_color_fmt_t color_fmt)
{
        switch (color_fmt) {
        case VF_COLOR_FMT_420_Y_UV:
                return VF_COLOR_FMT_420_Y_UV_STR;
        case VF_COLOR_FMT_422_YUYV:
                return VF_COLOR_FMT_422_YUYV_STR;
        case VF_COLOR_FMT_888_RGB:
                return VF_COLOR_FMT_888_RGB_STR;
        case VF_COLOR_FMT_INVALID:
        default:
                log_error("Invalid input: wrong or unsupported color format: %d\n", color_fmt);

                return VF_COLOR_FMT_INVALID_STR;
        }
}

int vf_framebuffer_write_to_file(vf_fb_st_t *fb, const char *filename_prefix)
{
        char file_name[128] = {0};
        const char *format = NULL;
        FILE *fptr = NULL;
        int i = 0;
        uint8_t *base_ptr = NULL;
        size_t file_size = 0;
        size_t offset = 0;
        size_t plane_size = 0;
        uint8_t *plane_ptr = NULL;
        size_t written = 0;

        if (NULL == fb || NULL == filename_prefix) {
                log_error("Invalid input: fb = %p, filename_prefix = %p\n", fb, filename_prefix);

                return EINVAL;
        }

        if (NULL == fb->virt_addr) {
                log_error("Invalid framebuffer: virt_addr is NULL.\n");

                return EINVAL;
        }

        format = get_color_fmt_str(fb->params.color_fmt);

        sprintf(file_name, "%s_%s_%dx%d.raw",
                filename_prefix,
                format,
                fb->params.width,
                fb->params.height);

        fptr = fopen(file_name, "wb");
        if (NULL == fptr) {
                log_error("Failed to open a file (file_name = %s). Error: %s\n",
                        file_name, strerror(errno));

                return EINVAL;
        }

        base_ptr = (uint8_t *)fb->virt_addr;
        file_size = 0;
        offset = 0;

        /* for each valid plane, we write plane_size[i] bytes, in order */
        for (i = 0; i < MAX_PLANE_COUNT; i++) {
                plane_size = fb->plane_size[i];

                if (plane_size == 0) {
                        /* we no longer have valid plans*/
                        break;
                }

                plane_ptr = base_ptr + offset;

                written = fwrite(plane_ptr, 1, plane_size, fptr);
                if (written != plane_size) {
                        log_error("Failed to write plane %d to file '%s'. "
                                "Expected=%zu, written=%zu\n",
                                i, file_name, plane_size, written);

                        fclose(fptr);

                        return EINVAL;
                }

                offset += plane_size;
                file_size += plane_size;
        }

        fclose(fptr);

        log_info("Successfully dumped frame to file '%s' size = %lu bytes\n", file_name, file_size);

        return 0;
}

