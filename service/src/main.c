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

#include <stdio.h>
#include <stdlib.h>

#include "vf-core.h"
#include "vf-error.h"
#include "vf-logger.h"

int main(void)
{
        vf_err_t err = VF_SUCCESS;

        err = vf_logger_init("VisionFlow", VF_LOG_LEVEL_DBG);
        if (VF_SUCCESS != err) {
                (void)fprintf(stderr, "[main] Logger init failed\n");

                return EXIT_FAILURE;
        }

        log_info("=== VisionFlow Service Start ===");

        err = vf_core_run();
        if (VF_SUCCESS != err) {
                log_err("vf_core_run failed: %s", vf_err2str(err));

                vf_logger_deinit();

                return EXIT_FAILURE;
        }

        log_info("=== VisionFlow Service Done ===");

        vf_logger_deinit();

        return EXIT_SUCCESS;
}

