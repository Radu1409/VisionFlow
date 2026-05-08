/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VF Framebuffer test client
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *      Manual test client for validating the VisionFlow framebuffer module.
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <string.h>

#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"

#define DEFAULT_FB_APP_NAME       "vf_framebuffer_test_client"
#define DEFAULT_FB_WIDTH          320U
#define DEFAULT_FB_HEIGHT         240U
#define DEFAULT_FILENAME_BASE     "vf_fb_test"
#define DEFAULT_PATTERN_MASK      0xFFU
#define DEFAULT_FB_FILENAME_READ  "vf_fb_test_RGB888_320x240.raw"

static
int fill_test_pattern(vf_framebuffer_t *fb)
{
        size_t i = 0U;

        if ((NULL == fb) || (NULL == fb->data)) {
                log_err("Invalid params: fb=%p fb->data=%p",
                        (void *)fb, (void *)fb->data);

                return VF_INVALID_PARAMETER;
        }

        for (i = 0U; i < fb->total_size; ++i) {
                fb->data[i] = (unsigned char)(i & DEFAULT_PATTERN_MASK);
        }

        return VF_SUCCESS;
}

int main(void)
{
        vf_fb_params_t params = {
                .width  = DEFAULT_FB_WIDTH,
                .height = DEFAULT_FB_HEIGHT,
                .format = VF_PIXEL_FMT_RGB888
        };
        vf_framebuffer_t fb = {0};
        vf_framebuffer_t fb_copy = {0};
        size_t calc_size = 0U;
        int rc = VF_SUCCESS;

        rc = vf_logger_init(DEFAULT_FB_APP_NAME, VF_LOG_LEVEL_TRACE);
        if (EOK != rc) {
                fprintf(stderr, "Failed to initialize logger. rc=%d\n", rc);

                return VF_INIT_FAILED;
        }

        log_info("Starting vf_framebuffer test client...");

        calc_size = vf_framebuffer_calculate_size(&params);
        if (0U == calc_size) {
                log_err("vf_framebuffer_calculate_size() failed.");

                vf_logger_deinit();

                return VF_INIT_FAILED;
        }

        log_info("Calculated framebuffer size: %zu bytes", calc_size);

        rc = vf_framebuffer_alloc(&fb, &params);
        if (VF_SUCCESS != rc) {
                log_err("vf_framebuffer_alloc() failed: %s", vf_err2str((vf_err_t)rc));

                vf_logger_deinit();

                return rc;
        }

        log_info("Framebuffer allocated successfully.");
        log_info("Framebuffer format: %s", vf_pixel_fmt_str(params.format));
        log_info("Framebuffer planes: %u", fb.num_planes);
        log_info("Framebuffer total size: %zu", fb.total_size);

        rc = fill_test_pattern(&fb);
        if (VF_SUCCESS != rc) {
                log_err("Failed to fill framebuffer test pattern.");

                vf_framebuffer_free(&fb);

                vf_logger_deinit();

                return rc;
        }

        rc = vf_framebuffer_write_to_file(&fb, DEFAULT_FILENAME_BASE);
        if (VF_SUCCESS != rc) {
                log_err("vf_framebuffer_write_to_file() failed: %s", vf_err2str((vf_err_t)rc));

                vf_framebuffer_free(&fb);

                vf_logger_deinit();

                return rc;
        }

        log_info("Framebuffer written to file successfully.");

        rc = vf_framebuffer_alloc(&fb_copy, &params);
        if (VF_SUCCESS != rc) {
                log_err("Failed to allocate framebuffer copy: %s", vf_err2str((vf_err_t)rc));

                vf_framebuffer_free(&fb);

                vf_logger_deinit();

                return rc;
        }

        rc = vf_framebuffer_copy(&fb_copy, &fb);
        if (VF_SUCCESS != rc) {
                log_err("vf_framebuffer_copy() failed: %s", vf_err2str((vf_err_t)rc));

                vf_framebuffer_free(&fb_copy);
                vf_framebuffer_free(&fb);

                vf_logger_deinit();

                return rc;
        }

        log_info("Framebuffer copied successfully.");

        rc = vf_framebuffer_clear(&fb_copy);
        if (VF_SUCCESS != rc) {
                log_err("vf_framebuffer_clear() failed: %s", vf_err2str((vf_err_t)rc));

                vf_framebuffer_free(&fb_copy);
                vf_framebuffer_free(&fb);

                vf_logger_deinit();

                return rc;
        }

        log_info("Framebuffer cleared successfully.");

        rc = vf_framebuffer_read_from_file(&fb_copy, DEFAULT_FB_FILENAME_READ);
        if (VF_SUCCESS != rc) {
                log_err("vf_framebuffer_read_from_file() failed: %s",
                        vf_err2str((vf_err_t)rc));

                vf_framebuffer_free(&fb_copy);
                vf_framebuffer_free(&fb);

                vf_logger_deinit();

                return rc;
        }

        log_info("Framebuffer read from file successfully.");

        vf_framebuffer_free(&fb_copy);
        vf_framebuffer_free(&fb);

        log_info("All vf_framebuffer tests passed successfully.");

        vf_logger_deinit();

        return EOK;
}

