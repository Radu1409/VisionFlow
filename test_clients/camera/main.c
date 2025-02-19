#include <stdio.h>
#include <stdlib.h>

#include "vf-error.h"
#include "vf-logger.h"
#include "camera.h"

int main() {
    CameraConfig config = {1920, 1080, 30, 1};
    
    if (camera_init(&config) != VF_SUCCESS) {
        log_error("Error initializing the camera!\n");

        return VF_CAMERA_INIT_ERR;
    }

    int buffer_size = config.width * config.height * 3; // RGB
    uint8_t *frame_buffer = (uint8_t *)malloc(buffer_size);
    if (!frame_buffer) {
        log_error("Error allocating buffer!\n");

        return VF_CAMERA_CAPTURE_FRAME_FAILED;
    }

    if (camera_capture_frame(frame_buffer, buffer_size) != VF_SUCCESS) {
        log_error("Error capturing frame!\n");
    }

    free(frame_buffer);
    camera_close();

    return VF_SUCCESS;
}

