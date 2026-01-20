/**
 **************************************************************************************************
 *  @file           : vf-framebuffer.h
 *  @brief          : Vision Flow framebuffer API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *  API for allocation and storage used for frames in Vision Flow library
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_FRAMEBUFFER__H
#define VF_FRAMEBUFFER__H

#include <stdint.h>

#define MAX_PLANE_COUNT         4

typedef enum {
        VF_COLOR_FMT_INVALID  = 0,
        VF_COLOR_FMT_422_YUYV = 1,
        VF_COLOR_FMT_420_Y_UV = 2,
        VF_COLOR_FMT_888_RGB  = 3,
        VF_COLOR_FMT_LAST,
} vf_color_fmt_t;

typedef enum {
        VF_COLOR_FMT_BPP_INVALID = 0,
        VF_COLOR_FMT_BPP_12      = 12,
        VF_COLOR_FMT_BPP_16      = 16,
        VF_COLOR_FMT_BPP_24      = 24,
} vf_color_fmt_bpp_t;

/*
 * Parameters of framebuffer used to allocate and identify it
 */
typedef struct {
        uint32_t        width;
        uint32_t        height;
        vf_color_fmt_t color_fmt;
} vf_fb_params_st_t;

/*
 * VF framebuffer, allocated via pmem
 */
typedef struct {
        void               *virt_addr;
        void               *phys_addr;
        void               *handle;
        uint16_t           stream_id;
        uint32_t           plane_width[MAX_PLANE_COUNT];
        uint32_t           plane_height[MAX_PLANE_COUNT];
        uint32_t           plane_stride[MAX_PLANE_COUNT];
        uint32_t           plane_size[MAX_PLANE_COUNT];
        size_t             total_size;
        vf_fb_params_st_t  params;
} vf_fb_st_t;

/**
 * @FUNCTION vf_framebuffer_get_color_fmt_bpp
 *
 * @brief Returns bits per pixel for color format
 *
 * @param [in] format color color format code
 *
 * @return bits per pixel, 0 if format invalid
 */
vf_color_fmt_bpp_t vf_framebuffer_get_color_fmt_bpp(vf_color_fmt_t color_fmt);

/*
 * @FUNCTION vf_alloc_framebuffer
 *
 * @brief         Allocates buffer for frames with pmem
 *
 * @param[in]     params     Parameters which describe data in buffer
 *
 * @return        Pointer to vf_fb_st_t structure
 */
vf_fb_st_t *vf_alloc_framebuffer(const vf_fb_params_st_t *params);

/*
 * @FUNCTION vf_copy_framebuffer
 *
 * @brief         Allocates memory and copy from fb to new mem
 *
 * @param[in]     fb     Pointer to framebuffer
 *
 * @return        Pointer to vf_fb_st_t structure
 */
vf_fb_st_t *vf_copy_framebuffer(const vf_fb_params_st_t *fb);

/*
 * @FUNCTION vf_free_framebuffer
 *
 * @brief         Frees pmem buffer and vf_fb_st_t structure
 *
 * @param[in]     fb         Pointer to the framebuffer pointer which should be freed
 *
 * @return        Nothing
 */
void vf_free_framebuffer(vf_fb_st_t **fb);

/*
 * @FUNCTION vf_init_framebuffer
 *
 * @brief         Initialize buffer from pmem allocated memory
 *
 * @param[in]     fb         Framebuffer pointer for initialize
 * @param[in]     params     Parameters which describe data in buffer
 * @param[in]     virt_add   Virtual address of pmem allocated buffer
 *
 * @return        Status
 */
int vf_init_framebuffer(vf_fb_st_t *fb, const vf_fb_params_st_t *params, void *virt_addr);

/*
 * @FUNCTION vf_fb_write_to_file
 *
 * @brief         Dump framebuffer to file
 *
 * @param[in]     fb                    Framebuffer pointer to dump
 * @param[in]     filename_prefix       Filename prefix
 *
 * @return        Status
 */
int vf_framebuffer_write_to_file(vf_fb_st_t *fb, const char *filename_prefix);

#endif /* VF_FRAMEBUFFER__H */

