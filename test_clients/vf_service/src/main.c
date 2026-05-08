/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VisionFlow service test client main routine
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *      Service test client used to validate parser integration and frame configuration loading
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>

#include "vf-error.h"
#include "vf-logger.h"
#include "vf-parser.h"

#ifndef VF_VERSION
#define VF_VERSION     "unknown"
#endif

#ifndef VF_BRANCH
#define VF_BRANCH      "unknown"
#endif

#define EOK 0

#define VF_PIXEL_FORMAT_RAW8_STR    "RAW8"
#define VF_PIXEL_FORMAT_RGB888_STR  "RGB888"
#define VF_PIXEL_FORMAT_YUV420_STR  "YUV420"
#define VF_PIXEL_FORMAT_INVALID_STR "INVALID"

#define DEFAULT_CFG_PATH "../../../vf_frames/conf/vf_frames_cfg.json"

#define err(fmt, ...)   fprintf(stderr, "Err: " fmt, ##__VA_ARGS__)
#define info(fmt, ...)  fprintf(stdout, "Info: " fmt, ##__VA_ARGS__)

static
const char *vf_pixel_format_to_str(vf_pixel_format_t format)
{
        switch (format) {
                case VF_PIXEL_FORMAT_RAW8:
                        return VF_PIXEL_FORMAT_RAW8_STR;
                case VF_PIXEL_FORMAT_RGB888:
                        return VF_PIXEL_FORMAT_RGB888_STR;
                case VF_PIXEL_FORMAT_YUV420:
                        return VF_PIXEL_FORMAT_YUV420_STR;
                default:
                        return VF_PIXEL_FORMAT_INVALID_STR;
        }
}

static
void print_usage(char *argv[])
{
        printf("Usage: %s [config_path]\n", argv[0]);
        printf("\n");
        printf("If no config path is provided, default path is used:\n");
        printf("  %s\n", DEFAULT_CFG_PATH);
        printf("\n");
}

static
void print_frames_cfg(const vf_frames_cfg_t *cfg)
{
        uint32_t i = 0U;
        uint32_t j = 0U;

        if (NULL == cfg) {
                log_err("Invalid input: cfg = %p", (void *)cfg);

                return;
        }

        log_info("Parsed configuration contains %u frame sets", cfg->frame_set_count);

        for (i = 0U; i < cfg->frame_set_count; i++) {
                const vf_frame_set_t *set = &cfg->frame_sets[i];

                log_info("Frame set [%u] => name='%s', format='%s', base_path='%s', frames=%u",
                         i,
                         set->name,
                         vf_pixel_format_to_str(set->format),
                         set->base_path,
                         set->frame_count);

                for (j = 0U; j < set->frame_count; j++) {
                        const vf_frame_info_t *frame = &set->frames[j];

                        log_info("  frame[%u] => file='%s', path='%s', %ux%u",
                                 j,
                                 frame->file_name,
                                 frame->full_path,
                                 frame->width,
                                 frame->height);
                }
        }
}

int main(int argc, char *argv[])
{
        vf_frames_cfg_t *cfg = NULL;
        const char *cfg_path = DEFAULT_CFG_PATH;
        int err_code = EOK;
        vf_err_t rc = VF_SUCCESS;

        if (argc > 2) {
                print_usage(argv);

                return VF_INVALID_PARAMETER;
        }

        if (2 == argc) {
                cfg_path = argv[1];
        }

        err_code = vf_logger_init("vf_service_test", VF_LOG_LEVEL_TRACE);
        if (EOK != err_code) {
                err("Failed to initialize logger: err = %d\n", err_code);

                return VF_INIT_FAILED;
        }

        log_info("VisionFlow Service Test Client started");
        log_info("Version: %s", VF_VERSION);
        log_info("Branch: %s", VF_BRANCH);

        log_info("Loading frame configuration from: %s", cfg_path);

        rc = vf_parser_load_frames_cfg(cfg_path, &cfg);
        if (VF_SUCCESS != rc) {
                log_err("Failed to load frame configuration. Error: %s\n", vf_err2str(rc));

                vf_logger_deinit();

                return rc;
        }

        print_frames_cfg(cfg);

        vf_parser_free_frames_cfg(cfg);
        vf_logger_deinit();

        return VF_SUCCESS;
}

