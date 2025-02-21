#include <string.h>

#include "vf-error.h"
#include "vf-logger.h"
#include "camera.h"

int camera_init(CameraConfig *config) {

    if(NULL == config) {
        return VF_INVALID_PARAMETER;
    }

    log_info("Initialized camera with resolution %dx%d @ %d FPS, format %d\n",
             config->width, config->height, config->fps, config->format);

    return VF_SUCCESS;
}

int camera_capture_frame(uint8_t *buffer, int buffer_size) {

    if (NULL == buffer) {
        return VF_INVALID_PARAMETER;
    }

    memset(buffer, rand() % 256, buffer_size);

    log_info("Frame received: (%d bytes)\n", buffer_size);

    return VF_SUCCESS;
}

void camera_close() {
    log_info("Camera turned off!\n");
}

