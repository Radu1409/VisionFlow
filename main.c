#include "vf-logger.h"
#include "camera.h"

int main() {
    enable_console_log(1);

    log_info("Starting application initialization...\n");

    CameraConfig config1 = {3840, 2160, 30, 1};

    if (camera_init(&config1) != 0) {
        log_error("Error initializing the camera!\n");
        return 1;
    }

    log_info("Camera initialized.\n");

    log_warning("Warning: Resources are not being used correctly!\n");
    log_error("Error: Unable to access the database!\n");
    log_debug("Debug details: function X was called.\n");
    //log_fatal("FATAL: Details function X.\n");

    camera_close();

    log_info("Camera closed.\n");

    return 0;
}

