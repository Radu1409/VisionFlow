/**
 **************************************************************************************************
 *  @file           : vf-conversion.c
 *  @brief          : Framebuffer conversions API routines
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *  Implementation of color format conversions API.
 *  The purpose is to  convert the source color format to the destination color format.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include <string.h>

#include "vf-conversion.h"
#include "vf-logger.h"

typedef enum {
        VF_COLOR_SPACE_INVALID = -1,
        VF_COLOR_SPACE_RGB     = 0,
        VF_COLOR_SPACE_YUV     = 1,
} vf_color_space_t;

// clamp helper
static inline uint8_t clamp_int_to_u8(int x)
{
    if (x < 0) {
        return 0;     
    }

    if (x > UINT8_MAX) {
        return UINT8_MAX;
    }

    return (uint8_t)x;
}

vf_err_t vf_yuv420_to_rgb24(const uint8_t *yuv, size_t yuv_size, uint8_t *rgb,
        size_t rgb_size, uint32_t width, uint32_t height)
{
    size_t y_plane_size = 0;
    size_t chroma_plane_size = 0;
    size_t expected_yuv_size = 0;
    size_t expected_rgb_size = 0;
    size_t y_index = 0, uv_index = 0;

    if (NULL == yuv || NULL == rgb) {
        log_error("Invalid input: yuv = %p, rgb = %p\n", yuv, rgb);

        return VF_INVALID_PARAMETER;
    }

    if (width == 0 || height == 0) {
        log_error("Invalid width or height: width = %d, height = %d\n", width, height);

        return VF_INVALID_PARAMETER;
    }

    // YUV420 asks even dimensions
    if ((width % 2u) != 0u || (height % 2u) != 0u) {
        log_error("Invalid width or height: Resolution must be even sizes.\n");

        return VF_INVALID_PARAMETER;
    }

    y_plane_size = (size_t)width * (size_t)height;
    chroma_plane_size = (size_t)(width / 2u) * (size_t)(height / 2u);
    expected_yuv_size = y_plane_size + 2u * chroma_plane_size;
    expected_rgb_size = (size_t)width * (size_t)height * 3u;

    if (yuv_size < expected_yuv_size || rgb_size < expected_rgb_size) {
        log_error("Lower sizes than expected: yuv_size = %d, expected_yuv_size = %d, "
                  "rgb_size = %d, expected_rgb_size = %d\n",
                  yuv_size, expected_yuv_size, rgb_size, expected_rgb_size);

        return VF_INVALID_PARAMETER;
    }

    const uint8_t *y_plane = yuv;
    const uint8_t *u_plane = yuv + y_plane_size;
    const uint8_t *v_plane = u_plane + chroma_plane_size;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            y_index = (size_t)y * width + x;
            uv_index = (size_t)(y / 2u) * (width / 2u) + (x / 2u);

            uint8_t Y = y_plane[y_index];
            uint8_t U = u_plane[uv_index];
            uint8_t V = v_plane[uv_index];

            // BT.601 integer aprox
            int C = (int)Y - 16;
            int D = (int)U - 128;
            int E = (int)V - 128;

            if (C < 0) {
                C = 0;
            } 

            int R = (298 * C + 409 * E + 128) >> 8;
            int G = (298 * C - 100 * D - 208 * E + 128) >> 8;
            int B = (298 * C + 516 * D + 128) >> 8;

            uint8_t r8 = clamp_int_to_u8(R);
            uint8_t g8 = clamp_int_to_u8(G);
            uint8_t b8 = clamp_int_to_u8(B);

            size_t rgb_index = y_index * 3u;
            rgb[rgb_index + 0] = r8;
            rgb[rgb_index + 1] = g8;
            rgb[rgb_index + 2] = b8;
        }
    }

    return VF_SUCCESS;
}

vf_err_t
vf_yuv420_copy(const uint8_t *yuv_src,
               size_t yuv_src_size,
               uint8_t *yuv_dst,
               size_t yuv_dst_size,
               uint32_t width,
               uint32_t height)
{
    size_t y_plane_size = 0;
    size_t chroma_plane_size = 0;
    size_t expected_yuv_size = 0;

    if (!yuv_src || !yuv_dst) {
        log_error("Invalid input: yuv_src = %p, yuv_dst = %p\n", yuv_src, yuv_dst);

        return VF_INVALID_PARAMETER;
    }

    if (width == 0 || height == 0) {
        log_error("Invalid input: width = %d, height = %d\n", width, height);

        return VF_INVALID_PARAMETER;
    }

    if ((width % 2u) != 0u || (height % 2u) != 0u) {
        log_error("Invalid width or height: Resolution must be even sizes.\n");

        return VF_INVALID_PARAMETER;
    }

    y_plane_size = (size_t)width * (size_t)height;
    chroma_plane_size = (size_t)(width / 2u) * (size_t)(height / 2u);
    expected_yuv_size = y_plane_size + 2u * chroma_plane_size;

    if (yuv_src_size < expected_yuv_size || yuv_dst_size < expected_yuv_size) {
        log_error("Lower sizes than expected: yuv_src_size = %d, yuv_dst_size = %d, "
                  "expected_yuv_size = %d\n",
                  yuv_src_size, expected_yuv_size, yuv_dst_size, expected_yuv_size);

        return VF_INVALID_PARAMETER;
    }

    memcpy(yuv_dst, yuv_src, expected_yuv_size);

    return VF_SUCCESS;
}

vf_err_t
vf_rgb24_resize_nearest(const uint8_t *src,
                        size_t src_size,
                        uint8_t *dst,
                        size_t dst_size,
                        uint32_t src_width,
                        uint32_t src_height,
                        uint32_t dst_width,
                        uint32_t dst_height)
{
    size_t expected_src = 0;
    size_t expected_dst = 0;
    size_t src_index = 0;
    size_t dst_index = 0;
    uint32_t src_y = 0, src_x = 0;

    if (!src || !dst) {
        log_error("Invalid input: src = %p, dst = %p\n", src, dst);

        return VF_INVALID_PARAMETER;
    }

    if (src_width == 0 || src_height == 0 || dst_width == 0 || dst_height == 0) {
        log_error("Invalid input: src_width = %d, src_height = %d, dst_width = %d, "
                  "dst_height = %d\n", src_width, src_height, dst_width, dst_height);

        return VF_INVALID_PARAMETER;
    }

    expected_src = (size_t)src_width * (size_t)src_height * 3u;
    expected_dst = (size_t)dst_width * (size_t)dst_height * 3u;

    if (src_size < expected_src || dst_size < expected_dst) {
        log_error("Lower sizes than expected: src_size = %d, expected_src = %d, dst_size = %d, "
                  "expected_dst = %d\n", src_size, expected_src, dst_size, expected_dst);

        return VF_INVALID_PARAMETER;
    }

    for (uint32_t y = 0; y < dst_height; ++y) {
        src_y = (uint32_t)((uint64_t)y * src_height / dst_height);
        for (uint32_t x = 0; x < dst_width; ++x) {
            src_x = (uint32_t)((uint64_t)x * src_width / dst_width);

            src_index = ((size_t)src_y * src_width + src_x) * 3u;
            dst_index = ((size_t)y * dst_width + x) * 3u;

            dst[dst_index + 0] = src[src_index + 0]; // R
            dst[dst_index + 1] = src[src_index + 1]; // G
            dst[dst_index + 2] = src[src_index + 2]; // B
        }
    }

    return VF_SUCCESS;
}

