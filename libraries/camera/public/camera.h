#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>

typedef struct {
    int width;
    int height;
    int fps;
    int format;  // Ex: YUV, RGB
} CameraConfig;

int camera_init(CameraConfig *config);
int camera_capture_frame(uint8_t *buffer, int buffer_size);
void camera_close();

#endif // CAMERA_H

