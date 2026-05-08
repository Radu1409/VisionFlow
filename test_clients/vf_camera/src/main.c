/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VisionFlow vf_camera test client
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Manual test client for the vf_camera library.
 *  Captures real frames from a V4L2 camera device, logs metadata
 *  and saves raw frames to disk.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf-camera.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"

#define DEFAULT_APP_NAME           "vf_camera_test_client"
#define DEFAULT_DEVICE_PATH        VF_CAMERA_DEFAULT_DEVICE
#define DEFAULT_WIDTH              640U
#define DEFAULT_HEIGHT             480U
#define DEFAULT_FORMAT             VF_PIXEL_FMT_YUYV
#define DEFAULT_BUFFER_COUNT       VF_CAMERA_DEFAULT_BUFFER_COUNT
#define DEFAULT_MAX_FRAMES         10U
#define DEFAULT_FRAMES_OUT_DIR     "frames_out"
#define DEFAULT_FRAME_FILENAME_MAX 128U

static
vf_err_t save_frame(const vf_framebuffer_t *fb, uint32_t frame_id)
{
        char filename[DEFAULT_FRAME_FILENAME_MAX] = {0};
        FILE *fp = NULL;
        size_t written = 0U;

        if (NULL == fb) {
                log_err("Invalid input: fb = %p", (void *)fb);

                return VF_INVALID_PARAMETER;
        }

        (void)snprintf(filename, sizeof(filename),
                       "%s/frame_%03u_%ux%u.raw",
                       DEFAULT_FRAMES_OUT_DIR,
                       frame_id,
                       fb->meta.width,
                       fb->meta.height);

        fp = fopen(filename, "wb");
        if (NULL == fp) {
                log_err("Failed to open output file '%s'", filename);

                return VF_ERROR_MIN;
        }

        written = fwrite(fb->data, 1U, fb->total_size, fp);

        (void)fclose(fp);

        if (written != fb->total_size) {
                log_err("Incomplete write: expected %zu bytes, wrote %zu",
                        fb->total_size, written);

                return VF_ERROR_MIN;
        }

        log_info("Frame saved: %s (%zu bytes)", filename, written);

        return VF_SUCCESS;
}

static
int run_clean(void)
{
        int rc = 0;

        rc = system("rm -f " DEFAULT_FRAMES_OUT_DIR "/*.raw");
        if (0 != rc) {
                (void)fprintf(stderr, "Failed to clean frames_out directory\n");

                return EXIT_FAILURE;
        }

        (void)printf("Cleaned all raw frames from '%s'\n", DEFAULT_FRAMES_OUT_DIR);

        return EXIT_SUCCESS;
}

static
vf_err_t run_capture(void)
{
        vf_camera_t camera = {0};
        vf_framebuffer_t fb = {0};
        vf_camera_cfg_t cfg = {0};
        vf_fb_params_t fb_params = {0};
        vf_err_t err = VF_SUCCESS;
        uint32_t i = 0U;

        /* Configure camera */
        (void)snprintf(cfg.device_path, sizeof(cfg.device_path),
                       "%s", DEFAULT_DEVICE_PATH);

        cfg.width = DEFAULT_WIDTH;
        cfg.height = DEFAULT_HEIGHT;
        cfg.format = DEFAULT_FORMAT;
        cfg.buffer_count = DEFAULT_BUFFER_COUNT;

        /* Allocate framebuffer for captured frame */
        fb_params.width = DEFAULT_WIDTH;
        fb_params.height = DEFAULT_HEIGHT;
        fb_params.format = DEFAULT_FORMAT;

        err = vf_framebuffer_alloc(&fb, &fb_params);
        if (VF_SUCCESS != err) {
                log_err("Failed to allocate framebuffer: %s", vf_err2str(err));

                return err;
        }

        /* Init camera */
        err = vf_camera_init(&camera, &cfg);
        if (VF_SUCCESS != err) {
                log_err("Failed to initialize camera: %s", vf_err2str(err));

                vf_framebuffer_free(&fb);

                return err;
        }

        /* Start camera */
        err = vf_camera_start(&camera);
        if (VF_SUCCESS != err) {
                log_err("Failed to start camera: %s", vf_err2str(err));

                vf_camera_deinit(&camera);
                vf_framebuffer_free(&fb);

                return err;
        }

        log_info("Camera started — capturing %u frames...", DEFAULT_MAX_FRAMES);

        /* Capture loop */
        for (i = 0U; i < DEFAULT_MAX_FRAMES; i++) {
                log_info("Capturing frame [%u/%u]...", i + 1U, DEFAULT_MAX_FRAMES);

                err = vf_camera_acquire_frame(&camera, &fb);
                if (VF_SUCCESS != err) {
                        log_err("Failed to acquire frame %u: %s", i, vf_err2str(err));

                        break;
                }

                log_info("Frame captured: frame_id=%u ts=%llums fmt=%s %ux%u size=%zu",
                         fb.meta.frame_id,
                         (unsigned long long)fb.meta.timestamp_ms,
                         vf_pixel_fmt_str(fb.meta.format),
                         fb.meta.width,
                         fb.meta.height,
                         fb.total_size);

                err = save_frame(&fb, fb.meta.frame_id);
                if (VF_SUCCESS != err) {
                        log_err("Failed to save frame %u", i);

                        break;
                }
        }

        /* Cleanup */
        vf_camera_stop(&camera);
        vf_camera_deinit(&camera);
        vf_framebuffer_free(&fb);

        if (VF_SUCCESS == err) {
                log_info("Capture complete — %u frame(s) saved to '%s'",
                         i, DEFAULT_FRAMES_OUT_DIR);
        }

        return err;
}

int main(int argc, char *argv[])
{
        vf_err_t err = VF_SUCCESS;
        int rc = EXIT_SUCCESS;

        if (2 == argc && 0 == strcmp(argv[1], "clean")) {
                return run_clean();
        }

        err = vf_logger_init(DEFAULT_APP_NAME, VF_LOG_LEVEL_DBG);
        if (VF_SUCCESS != err) {
                (void)fprintf(stderr, "Failed to initialize logger\n");

                return EXIT_FAILURE;
        }

        log_info("=== vf_camera test client start ===");

        err = run_capture();
        if (VF_SUCCESS != err) {
                log_err("Capture failed: %s", vf_err2str(err));

                rc = EXIT_FAILURE;
        }

        log_info("=== vf_camera test client end ===");

        vf_logger_deinit();

        return rc;
}

