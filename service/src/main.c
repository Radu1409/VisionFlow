/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VisionFlow Service Entry Point
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  VisionFlow service entry point. Initializes logger and runs the pipeline core.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf-core.h"
#include "vf-error.h"
#include "vf-logger.h"

#define OPT_ALL               1
#define OPT_RGB_TO_YUV_CONCAT 2
#define OPT_RGB_TO_YUV_SPLIT  3
#define OPT_YUV_TO_RGB_CONCAT 4
#define OPT_YUV_TO_RGB_SPLIT  5
#define OPT_RAW_TO_RGB_CONCAT 6
#define OPT_RAW_TO_RGB_SPLIT  7
#define OPT_RAW_TO_YUV_CONCAT 8
#define OPT_RAW_TO_YUV_SPLIT  9

/* *DISABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */
static struct option g_long_options[] = {
        { "all",              no_argument, 0, OPT_ALL              },
        { "rgb_to_yuv_concat",no_argument, 0, OPT_RGB_TO_YUV_CONCAT},
        { "rgb_to_yuv_split", no_argument, 0, OPT_RGB_TO_YUV_SPLIT },
        { "yuv_to_rgb_concat",no_argument, 0, OPT_YUV_TO_RGB_CONCAT},
        { "yuv_to_rgb_split", no_argument, 0, OPT_YUV_TO_RGB_SPLIT },
        { "raw_to_rgb_concat",no_argument, 0, OPT_RAW_TO_RGB_CONCAT},
        { "raw_to_rgb_split", no_argument, 0, OPT_RAW_TO_RGB_SPLIT },
        { "raw_to_yuv_concat",no_argument, 0, OPT_RAW_TO_YUV_CONCAT},
        { "raw_to_yuv_split", no_argument, 0, OPT_RAW_TO_YUV_SPLIT },
        { 0,                  0,           0, 0                    }
};
/* *ENABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */

static
void print_usage(const char *app_name)
{
        (void)fprintf(stderr, "Usage: %s [option]\n", app_name);
        (void)fprintf(stderr, "Options:\n");
        (void)fprintf(stderr, "  --all               Run all conversion scenarios\n");
        (void)fprintf(stderr, "  --rgb_to_yuv_concat Run RGB888 -> YUV420P (concatenated output)\n");
        (void)fprintf(stderr, "  --rgb_to_yuv_split  Run RGB888 -> YUV420P (split output)\n");
        (void)fprintf(stderr, "  --yuv_to_rgb_concat Run YUV420P -> RGB888 (concatenated output)\n");
        (void)fprintf(stderr, "  --yuv_to_rgb_split  Run YUV420P -> RGB888 (split output)\n");
        (void)fprintf(stderr, "  --raw_to_rgb_concat Run RAW8 -> RGB888 (concatenated output)\n");
        (void)fprintf(stderr, "  --raw_to_rgb_split  Run RAW8 -> RGB888 (split output)\n");
        (void)fprintf(stderr, "  --raw_to_yuv_concat Run RAW8 -> YUV420P (concatenated output)\n");
        (void)fprintf(stderr, "  --raw_to_yuv_split  Run RAW8 -> YUV420P (split output)\n");
}

int main(int argc, char *argv[])
{
        vf_err_t err     = VF_SUCCESS;
        int      opt     = 0;
        int      longidx = 0;
        int      found   = 0;
        const char *flag = NULL;

        err = vf_logger_init("VisionFlow", VF_LOG_LEVEL_DBG);
        if (VF_SUCCESS != err) {
                (void)fprintf(stderr, "[main] Logger init failed\n");

                return EXIT_FAILURE;
        }

        log_info("=== VisionFlow Service Start ===");

        if (argc < 2) {
                print_usage(argv[0]);

                vf_logger_deinit();

                return EXIT_FAILURE;
        }

        opt = getopt_long(argc, argv, "", g_long_options, &longidx);

        switch (opt) {
                case OPT_ALL:
                        err   = vf_core_run_all();
                        found = 1;
                        break;
                case OPT_RGB_TO_YUV_CONCAT:
                        flag  = "--rgb_to_yuv_concat";
                        found = 1;
                        break;
                case OPT_RGB_TO_YUV_SPLIT:
                        flag  = "--rgb_to_yuv_split";
                        found = 1;
                        break;
                case OPT_YUV_TO_RGB_CONCAT:
                        flag  = "--yuv_to_rgb_concat";
                        found = 1;
                        break;
                case OPT_YUV_TO_RGB_SPLIT:
                        flag  = "--yuv_to_rgb_split";
                        found = 1;
                        break;
                case OPT_RAW_TO_RGB_CONCAT:
                        flag  = "--raw_to_rgb_concat";
                        found = 1;
                        break;
                case OPT_RAW_TO_RGB_SPLIT:
                        flag  = "--raw_to_rgb_split";
                        found = 1;
                        break;
                case OPT_RAW_TO_YUV_CONCAT:
                        flag  = "--raw_to_yuv_concat";
                        found = 1;
                        break;
                case OPT_RAW_TO_YUV_SPLIT:
                        flag  = "--raw_to_yuv_split";
                        found = 1;
                        break;
                default:
                        print_usage(argv[0]);

                        vf_logger_deinit();

                        return EXIT_FAILURE;
        }

        if ((1 == found) && (NULL != flag)) {
                err = vf_core_run_by_flag(flag);
        }

        if (VF_SUCCESS != err) {
                log_err("vf_core failed: %s", vf_err2str(err));

                vf_logger_deinit();

                return EXIT_FAILURE;
        }

        log_info("=== VisionFlow Service Done ===");

        vf_logger_deinit();

        return EXIT_SUCCESS;
}

