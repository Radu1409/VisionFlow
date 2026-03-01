/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VF File lib test application
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *      VF File lib test application
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 *  @copyright  (c) 2023 Hitachi Astemo Ltd
 *
 **************************************************************************************************
 **/

#include <stdlib.h>
#include <string.h>

#include "vf-file.h"
#include "vf-logger.h"

#define EOK             0
#define BUILD_VERSION   "1.00"
#define SAFE_STR(str)   (char *)((str) == NULL ? "(null)" : (str))

int main(int argc, char **argv)
{
        int rc = EOK;
        vf_file_lib_ctx_st_t file_lib_ctx_wr = {0};
        vf_file_lib_ctx_st_t file_lib_ctx_rd = {0};
        vf_fb_st_t *fb = NULL;
        vf_fb_params_st_t params = {
                .width = 1920,
                .height = 1080,
                .color_fmt = VF_COLOR_FMT_888_RGB,
        };

        printf("Run vf file test client. Version '%s'\n", SAFE_STR(BUILD_VERSION));

        strcpy(file_lib_ctx_wr.config.file_name, "/var/output_file.raw");
        file_lib_ctx_wr.config.file_mode = FILE_MODE_WRITE;

        strcpy(file_lib_ctx_rd.config.file_name, "/var/input_file.raw");
        file_lib_ctx_rd.config.file_mode = FILE_MODE_READ;

        rc = vf_log_init(APP_NAME);
        if (EOK != rc) {
                fprintf(stderr, "Failed to initialize logger: err = %d\n", rc);

                return rc;
        }

        log_debug("Logger initialized successfully\n");

        fb = vf_alloc_framebuffer(&params);
        if (EOK != rc) {
                log_error("Failed to init file library. Error: %d\n", rc);

                return rc;
        }

        rc = vf_file_lib_init(&file_lib_ctx_wr);
        if (EOK != rc) {
                log_error("Failed to open file library. Error: %d\n", rc);

                return rc;
        }

        rc = vf_file_lib_init(&file_lib_ctx_rd);
        if (EOK != rc) {
                log_error("Failed to open file library. Error: %d\n", rc);

                return rc;
        }

        rc = vf_file_lib_read_fb_from_file(&file_lib_ctx_rd, fb);
        if (EOK != rc) {
                log_error("Failed to read frame buffer. Error: %d\n", rc);

                return rc;
        }

        rc = vf_file_lib_write_fb_to_file(&file_lib_ctx_wr, fb);
        if (EOK != rc) {
                log_error("Failed to write frame buffer. Error: %d\n", rc);

                return rc;
        }

        rc =  vf_file_lib_deinit(&file_lib_ctx_wr);
        if (EOK != rc) {
                log_error("Failed to close file library. Error: %d\n", rc);

                return rc;
        }

        rc =  vf_file_lib_deinit(&file_lib_ctx_rd);
        if (EOK != rc) {
                log_error("Failed to close file library. Error: %d\n", rc);

                return rc;
        }

        printf("Successfully finished\n");

        return EXIT_SUCCESS;
}