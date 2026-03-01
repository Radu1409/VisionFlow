/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VF Logger test client
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *          Test client for testing logger VF API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <errno.h>
#include <stdio.h>

#include "vf-error.h"
#include "vf-logger.h"

#define VF_LOGGER_LIB_NAME "vf_logger_lib"

#define NUM_CAMERAS_ON     2
#define PORT_NUM_ONE       1
#define PORT_NUM_TWO       2
#define FRAME_ID           13
#define DEFAULT_TIMESTAMP  1634500123

int main()
{
        /* Initialize the logger (e.g. set it to log everything up to the DEBUG level) */
        int rc = EOK;
        int num_cameras_on = NUM_CAMERAS_ON;

        rc = vf_logger_init(VF_LOGGER_LIB_NAME, VF_LOG_LEVEL_ERR);
        if (EOK != rc) {
                vf_log_fprintf("Failed to initialize logger. Error: %s", vf_err2str(rc));

                return VF_INIT_FAILED;
        }

        log_info("This message will not be displayed because verbosity level is set to err.");

        vf_logger_set_verbosity(VF_LOG_LEVEL_TRACE);

        log_info("VisionFlow system startup. Connected cameras: %d", num_cameras_on);

        log_dbg("Allocating video buffers...");

        log_err("The video sensor on port %d is not responding!", PORT_NUM_ONE);

        log_trace("Entering process_video_frame(). frame_id=%u, timestamp=%llu ms", FRAME_ID,
                  DEFAULT_TIMESTAMP);

        log_wrn("Memory leaks occurs...");

        /* Test the system's Fallback (what happens if it receives an invalid level - UNKNOWN LEVEL) */
        // vf_log((vf_log_level_t)0, "Received an undefined status code from the hardware component!");

        log_fatal("The system has collapsed!");

        log_info("All VF Logs were tested successfully!");

        vf_logger_deinit();

        return EOK;
}

