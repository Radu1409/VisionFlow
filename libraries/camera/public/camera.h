#ifndef VF_CAMERA_H
#define VF_CAMERA_H

#include <linux/videodev2.h>
#include <stddef.h>
#include <stdint.h>

#include "vf-error.h"

#ifdef __cplusplus
extern "C" {
#endif

/** I/O method for capture: direct read or memory mapped (mmap). */
enum io_method {
        IO_METHOD_READ,
        IO_METHOD_MMAP
};

/** Structure for allocated buffer (frame). */
struct buffer {
        void   *start;   /**< Pointer to the beginning of the frame's memory area */
        size_t length;   /**< Size of allocated memory area */
};

/** Structure for the video camera. */
typedef struct {
        int fd;                        /**< File descriptor for video device */
        enum io_method io;             /**< I/O method used (read or mmap) */
        struct buffer *buffers;        /**< Vector of frame buffers (used for mmap) */
        unsigned int n_buffers;        /**< Number of allocated buffers */
        uint32_t pixel_format;         /**< Pixel format used (e.g. V4L2_PIX_FMT_YUYV) */
        int width;                     /**< Frame width in pixels */
        int height;                    /**< Frame height in pixels */
} Camera;

/** 
 * Initialize the camera: open the device and configure the video format and I/O method.
 * @param cam - pointer to the Camera structure to be initialized
 * @param device - path to the device (ex: "/dev/video0")
 * @param width - desired frame width
 * @param height - desired frame height
 * @param pixel_format - pixel format (ex: V4L2_PIX_FMT_RGB24, V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG)
 * @param io - I/O method (IO_METHOD_READ or IO_METHOD_MMAP)
 * @return 0 if initialization succeeded, -1 if an error occurred
 */
vf_err_t camera_init(Camera *cam, const char *device, int width, int height, uint32_t pixel_format, enum io_method io);

/**
 * Captures a single frame from the camera.
 * @param cam - pointer to the initialized camera
 * @param frame_data - on output, will point to the raw data of the captured frame
 * @param frame_size - on output, the size in bytes of the frame data
 * @return 0 if captured successfully, -1 on error
 *
 * Note: For IO_METHOD_MMAP, the data is in an internal driver buffer
 * mapped to memory. This buffer will be reused; copy the data if you want to keep it.
 */
vf_err_t camera_capture_frame(Camera *cam, const void **frame_data, size_t *frame_size);

/**
 * Starts continuous capture over a number of frames, using an internal circular buffer.
 * @param cam - pointer to the initialized camera (already streaming if in mmap mode)
 * @param frame_count - number of frames to capture (0 for unlimited)
 * @return number of frames captured (or -1 on error)
 *
 * Note: the function will iterate and capture successive frames. In mmap mode, allocated buffers are reused circularly.
 */
vf_err_t camera_capture_stream(Camera *cam, unsigned int frame_count);

/**
 * Starts continuous streaming of the camera.
 * @param cam - pointer to the initialized camera
 * @param device - pointer to the device
 * @return 0 if the start was successful, -1 otherwise
 */
vf_err_t camera_start(Camera *cam, const char *device);

/**
 * Stops continuous streaming (if enabled) of the camera.
 * @param cam - pointer to the initialized camera
 * @return 0 if the stop succeeded, -1 otherwise
 */
vf_err_t camera_stop(Camera *cam);

/**
 * Releases camera resources and closes the device.
 * @param cam - pointer to the Camera structure
 */
void camera_close(Camera *cam);

#ifdef __cplusplus
}
#endif

#endif /* VF_CAMERA_H */

