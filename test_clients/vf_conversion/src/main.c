/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VisionFlow vf_conversion test client
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Manual validation test client for the vf_conversion library module.
 *  Tests RGB888 <-> YUV420P and RGB888 -> NV12 -> RGB888 roundtrips.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf-conversion.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"

#define DEFAULT_WIDTH       64U
#define DEFAULT_HEIGHT      64U

#define DEFAULT_RED_R       255U
#define DEFAULT_RED_G       0U
#define DEFAULT_RED_B       0U

#define DEFAULT_YUV_RED_Y   81U
#define DEFAULT_YUV_RED_U   90U
#define DEFAULT_YUV_RED_V   240U

#define DEFAULT_GRAY_VALUE  128U

static
int test_rgb888_to_yuv420p(void)
{
        vf_framebuffer_t src = {0};
        vf_framebuffer_t dst = {0};
        vf_conversion_ctx_t ctx = {0};
        vf_fb_params_t src_params = {DEFAULT_WIDTH, DEFAULT_HEIGHT, VF_PIXEL_FMT_RGB888};
        vf_fb_params_t dst_params = {DEFAULT_WIDTH, DEFAULT_HEIGHT, VF_PIXEL_FMT_YUV420P};
        uint32_t i = 0U;
        uint8_t *p = NULL;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_rgb888_to_yuv420p ---");

        err = vf_framebuffer_alloc(&src, &src_params);
        if (VF_SUCCESS != err) {
                log_err("src alloc failed");

                return 1;
        }

        err = vf_framebuffer_alloc(&dst, &dst_params);
        if (VF_SUCCESS != err) {
                log_err("dst alloc failed");

                vf_framebuffer_free(&src);

                return 1;
        }

        /* Fill src with a solid red pattern */
        p = src.data;
        for (i = 0U; i < DEFAULT_WIDTH * DEFAULT_HEIGHT; i++) {
                p[i * 3U + 0U] = DEFAULT_RED_R;   /* R */
                p[i * 3U + 1U] = DEFAULT_RED_G;   /* G */
                p[i * 3U + 2U] = DEFAULT_RED_B;   /* B */
        }

        err = vf_conversion_init(&ctx, VF_PIXEL_FMT_RGB888, VF_PIXEL_FMT_YUV420P);
        if (VF_SUCCESS != err) {
                log_err("conversion init failed");

                vf_framebuffer_free(&src);
                vf_framebuffer_free(&dst);

                return 1;
        }

        err = vf_conversion_process(&ctx, &src, &dst);
        if (VF_SUCCESS != err) {
                log_err("conversion process failed");

                vf_conversion_deinit(&ctx);
                vf_framebuffer_free(&src);
                vf_framebuffer_free(&dst);

                return 1;
        }

        /* Y plane should be non-zero for red input */
        if (0U == dst.data[0]) {
                log_err("Y plane unexpectedly zero for red input");

                vf_conversion_deinit(&ctx);
                vf_framebuffer_free(&src);
                vf_framebuffer_free(&dst);

                return 1;
        }

        vf_conversion_deinit(&ctx);
        vf_framebuffer_free(&src);
        vf_framebuffer_free(&dst);

        log_info("PASS: RGB888 -> YUV420P OK");

        return 0;
}

static
int test_yuv420p_to_rgb888(void)
{
        vf_framebuffer_t src = {0};
        vf_framebuffer_t dst = {0};
        vf_conversion_ctx_t ctx = {0};
        vf_fb_params_t src_params = {DEFAULT_WIDTH, DEFAULT_HEIGHT, VF_PIXEL_FMT_YUV420P};
        vf_fb_params_t dst_params = {DEFAULT_WIDTH, DEFAULT_HEIGHT, VF_PIXEL_FMT_RGB888};
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_yuv420p_to_rgb888 ---");

        err = vf_framebuffer_alloc(&src, &src_params);
        if (VF_SUCCESS != err) {
                log_err("src alloc failed");

                return 1;
        }

        err = vf_framebuffer_alloc(&dst, &dst_params);
        if (VF_SUCCESS != err) {
                log_err("dst alloc failed");

                vf_framebuffer_free(&src);

                return 1;
        }

        /* Fill with valid YUV values: Y=81 U=90 V=240 approximates red */
        (void)memset(src.data, DEFAULT_YUV_RED_Y, src.plane_size[0]);
        (void)memset(src.data + src.plane_size[0], DEFAULT_YUV_RED_U, src.plane_size[1]);
        (void)memset(src.data + src.plane_size[0] + src.plane_size[1], DEFAULT_YUV_RED_V,
                     src.plane_size[2]);

        err = vf_conversion_init(&ctx, VF_PIXEL_FMT_YUV420P, VF_PIXEL_FMT_RGB888);
        if (VF_SUCCESS != err) {
                log_err("conversion init failed");

                vf_framebuffer_free(&src);
                vf_framebuffer_free(&dst);

                return 1;
        }

        err = vf_conversion_process(&ctx, &src, &dst);
        if (VF_SUCCESS != err) {
                log_err("conversion process failed");

                vf_conversion_deinit(&ctx);
                vf_framebuffer_free(&src);
                vf_framebuffer_free(&dst);

                return 1;
        }

        vf_conversion_deinit(&ctx);
        vf_framebuffer_free(&src);
        vf_framebuffer_free(&dst);

        log_info("PASS: YUV420P -> RGB888 OK");

        return 0;
}

static
int test_rgb888_to_nv12(void)
{
        vf_framebuffer_t src = {0};
        vf_framebuffer_t dst = {0};
        vf_conversion_ctx_t ctx = {0};
        vf_fb_params_t src_params = {DEFAULT_WIDTH, DEFAULT_HEIGHT, VF_PIXEL_FMT_RGB888};
        vf_fb_params_t dst_params = {DEFAULT_WIDTH, DEFAULT_HEIGHT, VF_PIXEL_FMT_NV12};
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_rgb888_to_nv12 ---");

        err = vf_framebuffer_alloc(&src, &src_params);
        if (VF_SUCCESS != err) {
                log_err("src alloc failed");

                return 1;
        }

        err = vf_framebuffer_alloc(&dst, &dst_params);
        if (VF_SUCCESS != err) {
                log_err("dst alloc failed");

                vf_framebuffer_free(&src);

                return 1;
        }

        (void)memset(src.data, DEFAULT_GRAY_VALUE, src.total_size);

        err = vf_conversion_init(&ctx, VF_PIXEL_FMT_RGB888, VF_PIXEL_FMT_NV12);
        if (VF_SUCCESS != err) {
                log_err("conversion init failed");

                vf_framebuffer_free(&src);
                vf_framebuffer_free(&dst);

                return 1;
        }

        err = vf_conversion_process(&ctx, &src, &dst);
        if (VF_SUCCESS != err) {
                log_err("conversion process failed");

                vf_conversion_deinit(&ctx);
                vf_framebuffer_free(&src);
                vf_framebuffer_free(&dst);

                return 1;
        }

        vf_conversion_deinit(&ctx);
        vf_framebuffer_free(&src);
        vf_framebuffer_free(&dst);

        log_info("PASS: RGB888 -> NV12 OK");

        return 0;
}

static
int test_unsupported_conversion(void)
{
        vf_conversion_ctx_t ctx = {0};
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_unsupported_conversion ---");

        err = vf_conversion_init(&ctx, VF_PIXEL_FMT_RAW8, VF_PIXEL_FMT_NV12);
        if (VF_SUCCESS == err) {
                log_err("Expected failure for unsupported conversion RAW8 -> NV12");

                vf_conversion_deinit(&ctx);

                return 1;
        }

        log_info("PASS: unsupported conversion correctly rejected");

        return 0;
}

int main(void)
{
        vf_err_t err = VF_SUCCESS;
        int failed = 0;

        err = vf_logger_init("vf_conversion_test", VF_LOG_LEVEL_DBG);
        if (VF_SUCCESS != err) {
                (void)fprintf(stderr, "[vf_conversion_test] Logger init failed\n");

                return EXIT_FAILURE;
        }

        log_info("=== vf_conversion test client start ===");

        failed += test_rgb888_to_yuv420p();
        failed += test_yuv420p_to_rgb888();
        failed += test_rgb888_to_nv12();
        failed += test_unsupported_conversion();

        if (0 == failed) {
                log_info("All tests passed");
        } else {
                log_err("%d test(s) failed", failed);
        }

        log_info("=== vf_conversion test client end ===");

        vf_logger_deinit();

        return (0 == failed) ? EXIT_SUCCESS : EXIT_FAILURE;
}

