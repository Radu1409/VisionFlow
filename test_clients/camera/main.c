/*
* main_mmap.c - V4L2 video capture demonstration using memory mapped (MMAP).
* Initializes the camera (/dev/video0) to 640x480 resolution, YUYV format and captures 10 frames using mmap buffers.
* For each frame, print the size in bytes and save the raw data to separate files (frame_0.raw, frame_1.raw, etc.).
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <linux/videodev2.h>
#include "vf-error.h"
#include "vf-logger.h"
#include "camera.h"

int main() {
    const char *device = "/dev/video10";
    Camera camera = {0};
    vf_err_t rc = VF_SUCCESS;

    // Initialize the camera: YUV format 1280x720, capture method MMAP
    rc = camera_init(&camera, device, 1280, 720, V4L2_PIX_FMT_YUV420, IO_METHOD_MMAP);
    if (VF_SUCCESS != rc) {
        log_error("Camera initialization error.\n");

        camera_close(&camera);

        return VF_CAMERA_INIT_ERR;
    }

    // Start capturing the video stream
    rc = camera_start(&camera, device);
    if (VF_SUCCESS != rc) {
        log_error("Error starting capture.\n");

        camera_close(&camera);

        return VF_CAMERA_START_ERR;
    }

    const void *frame = NULL;
    size_t frame_size = 0;
    char filename[32];

    int fps = 30;
    int duration_seconds = 6; // round to 6 for safety
    int frames_num = fps * duration_seconds;

    // Capture 10 frames
    for (int i = 0; i < frames_num; ++i) {
        // Capture a frame (wait until the frame is available)
        rc = camera_capture_frame(&camera, &frame, &frame_size);
        if (VF_SUCCESS != rc) {
            log_error("Error capturing frame: %d\n", i);

            break;
        }

        log_info("Frame %d captured - %zu bytes\n", i, frame_size);

        // Prepare the output file name (ex: frame_0.raw, frame_1.raw, ...)
        snprintf(filename, sizeof(filename), "frame_%d.raw", i);

        FILE *fptr = fopen(filename, "wb");
        if (!fptr) {
            log_error("Cannot create the file %s\n", filename);

            continue;
        }
        // Write the raw frame data to the .raw file
        fwrite(frame, frame_size, 1, fptr);
        fclose(fptr);
    }

    // Stop capturing and close the camera
    rc = camera_stop(&camera);
    if (VF_SUCCESS != rc) {
        log_error("Error stopping camera!\n");

        return rc;
    }
    camera_close(&camera);

    return rc;
}

