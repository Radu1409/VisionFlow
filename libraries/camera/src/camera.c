#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include "vf-error.h"
#include "vf-logger.h"
#include "camera.h"

/** Macro to quickly clean up a structure (set everything to 0). */
#define CLEAR(x) memset(&(x), 0, sizeof(x))

/** Static helper function for repeated ioctl calls until interrupted. */
static
int xioctl(int fd, int request, void *arg) {
    int r = 0;

    do {
            r = ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);

    return r;
}

vf_err_t camera_init(Camera *cam, const char *device, int width, int height, uint32_t pixel_format, enum io_method io) {
    struct v4l2_capability cap;
    struct v4l2_format fmt;

    cam->fd = open(device, O_RDWR);
    if (cam->fd < 0) {
        log_error("Error opening device %s: %s\n", device, strerror(errno));

        return VF_CAMERA_INIT_ERR;
    }

    cam->io = io;
    cam->width = width;
    cam->height = height;
    cam->pixel_format = pixel_format;
    cam->buffers = NULL;
    cam->n_buffers = 0;
    
    // Get device capabilities
    if (xioctl(cam->fd, VIDIOC_QUERYCAP, &cap) < 0) {
        log_error("Unable to obtain device capabilities: %s\n", strerror(errno));

        close(cam->fd);

        return VF_CAMERA_INIT_ERR;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        log_error("The device does not support video capture.\n");

        close(cam->fd);

        return VF_CAMERA_INIT_ERR;
    }
    if (io == IO_METHOD_READ && !(cap.capabilities & V4L2_CAP_READWRITE)) {
        log_error("The device does not support the read method.\n");

        close(cam->fd);

        return VF_CAMERA_INIT_ERR;
    }
    if (io == IO_METHOD_MMAP && !(cap.capabilities & V4L2_CAP_STREAMING)) {
        log_error("The device does not support the streaming method (mmap).\n");

        close(cam->fd);

        return VF_CAMERA_INIT_ERR;
    }
    
    // Set the video format (dimensions and pixel format)
    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixel_format;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (xioctl(cam->fd, VIDIOC_S_FMT, &fmt) < 0) {
        log_error("Error setting video format %dx%d (pixelformat 0x%08x): %s\n",
                  width, height, pixel_format, strerror(errno));

        close(cam->fd);

        return VF_CAMERA_INIT_ERR;
    }

    // Actual format may be different (driver may adjust), update if necessary:
    cam->width = fmt.fmt.pix.width;
    cam->height = fmt.fmt.pix.height;
    cam->pixel_format = fmt.fmt.pix.pixelformat;

    // Image buffer size (frame) in bytes
    size_t image_size = fmt.fmt.pix.sizeimage;
    
    if (io == IO_METHOD_READ) {
        // Allocate a buffer for reading
        cam->n_buffers = 1;

        cam->buffers = calloc(1, sizeof(struct buffer));
        if (!cam->buffers) {
            log_error("Error allocating read buffer\n");

            close(cam->fd);

            return VF_CAMERA_INIT_ERR;
        }

        cam->buffers[0].length = image_size;
        cam->buffers[0].start = malloc(image_size);
        if (!cam->buffers[0].start) {
            log_error("Error allocating memory for buffer\n");

            free(cam->buffers);
            close(cam->fd);

            return VF_CAMERA_INIT_ERR;
        }

        // Note: in read mode, no need for VIDIOC_REQBUFS or VIDIOC_STREAMON
        log_info("Camera %s initialized in READ mode (frame %dx%d, format 0x%08x).\n",
                 device, cam->width, cam->height, cam->pixel_format);
    } else if (io == IO_METHOD_MMAP) {
        // Ask the driver to allocate buffers (e.g. 4 frame buffers)
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        
        if (xioctl(cam->fd, VIDIOC_REQBUFS, &req) < 0) {
            if (errno == EINVAL) {
                log_error("The device does not support mapped memory.\n");
            } else {
                log_error("Buffer request error: %s\n", strerror(errno));
            }

            close(cam->fd);

            return VF_CAMERA_INIT_ERR;
        }
        if (req.count < 2) {
            log_error("Could not allocate enough buffers (only %d available).\n", req.count);

            close(cam->fd);

            return VF_CAMERA_INIT_ERR;
        }

        cam->n_buffers = req.count;
        cam->buffers = calloc(req.count, sizeof(struct buffer));
        if (!cam->buffers) {
            log_error("Error allocating buffer vector\n");

            close(cam->fd);

            return VF_CAMERA_INIT_ERR;
        }
        // Map each buffer into memory
        for (unsigned int i = 0; i < req.count; ++i) {
            struct v4l2_buffer buf;

            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (xioctl(cam->fd, VIDIOC_QUERYBUF, &buf) < 0) {
                log_error("Buffer query error %d: %s\n", i, strerror(errno));

                camera_close(cam);

                return VF_CAMERA_INIT_ERR;
            }

            cam->buffers[i].length = buf.length;
            cam->buffers[i].start =
                mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam->fd, buf.m.offset);
            if (cam->buffers[i].start == MAP_FAILED) {
                log_error("Error mapping buffer %d to memory: %s\n", i, strerror(errno));

                camera_close(cam);

                return VF_CAMERA_INIT_ERR;
            }
        }

        // Enable frame capture (STREAMON) and queue buffers
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        for (unsigned int i = 0; i < cam->n_buffers; ++i) {
            struct v4l2_buffer buf;

            CLEAR(buf);

            buf.type = type;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (xioctl(cam->fd, VIDIOC_QBUF, &buf) < 0) {
                log_error("Buffer queue error %d: %s\n", i, strerror(errno));

                camera_close(cam);

                return VF_CAMERA_INIT_ERR;
            }
        }
    } else {
        log_error("The specified I/O method is not supported.\n");

        close(cam->fd);

        return VF_CAMERA_INIT_ERR;
    }

    return VF_SUCCESS;
}

vf_err_t camera_capture_frame(Camera *cam, const void **frame_data, size_t *frame_size) 
{
    if (cam->io == IO_METHOD_READ) {
        // Read a frame directly into the user-space buffer
        ssize_t bytes_read = read(cam->fd, cam->buffers[0].start, cam->buffers[0].length);
        if (bytes_read < 0) {
            if (errno == EAGAIN) {
                // No frame available yet
                return VF_SUCCESS;
            }

            log_error("Error reading frame: %s\n", strerror(errno));

            return VF_CAMERA_CAPTURE_FRAME_FAILED;
        }

        *frame_data = cam->buffers[0].start;
        *frame_size = (size_t) bytes_read;
        // For read mode, each read already gets a full frame
        return VF_SUCCESS;
    } else if (cam->io == IO_METHOD_MMAP) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // Dequeue a buffer full of the last frame's data
        if (xioctl(cam->fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) {
                // If we use non-blocking, the frame is not yet available
                return VF_SUCCESS;
            }
            log_error("Error decoding buffer (DQBUF): %s\n", strerror(errno));

            return VF_CAMERA_CAPTURE_FRAME_FAILED;
        }

        // Make sure the index is valid
        if (buf.index >= cam->n_buffers) {
            log_error("Invalid buffer index: (%d)\n", buf.index);

            return VF_CAMERA_CAPTURE_FRAME_FAILED;
        }
        *frame_data = cam->buffers[buf.index].start;
        *frame_size = buf.bytesused;
        // MJPEG frames can have variable sizes (bytesused reflects the actual frame size)
        // The buffer now contains the raw data of the captured frame.

        // (Here we could process the frame or copy the data if needed)

        // Put the buffer back for reuse (enqueue)
        if (xioctl(cam->fd, VIDIOC_QBUF, &buf) < 0) {
            log_error("Error reinserting the queue buffer (QBUF): %s\n", strerror(errno));

            return VF_CAMERA_CAPTURE_FRAME_FAILED;
        }

        return VF_SUCCESS;
    }

    return VF_CAMERA_CAPTURE_FRAME_FAILED;
}

vf_err_t camera_capture_stream(Camera *cam, unsigned int frame_count) {
    unsigned int count = 0;
    const void *frame_data;
    size_t frame_size;

    while (1) {
        if (camera_capture_frame(cam, &frame_data, &frame_size) < 0) {
            log_error("Error capturing frame %u\n", count);

            break;
        }
        if (frame_size == 0) {
            // If a frame was not available, try again (non-blocking)
            continue;
        }

        count++;

        // Debug message for each captured frame
        log_info("Frame %u captured (%zu bytes)\n", count, frame_size);

        // Here you can process or store frame data using frame_data
        if (frame_count != 0 && count >= frame_count) {
            break;
        }
    }

    log_info("All frames were captured: %d.\n", count);

    return (count > 0) ? (int)count : -1;
}

vf_err_t camera_start(Camera *cam, const char *device)
{
    if (cam->io == IO_METHOD_MMAP) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (xioctl(cam->fd, VIDIOC_STREAMON, &type) < 0) {
            log_error("Error starting capture (STREAMON): %s\n", strerror(errno));

            camera_close(cam);

            return VF_CAMERA_START_ERR;
        }

        log_info("Camera %s initialized in MMAP mode (frame %dx%d, format 0x%08x, %d buffers).\n",
                 device, cam->width, cam->height, cam->pixel_format, cam->n_buffers);
    }

    log_info("The stream has been successfully activated!\n");

    return VF_SUCCESS;
}

vf_err_t camera_stop(Camera *cam) 
{
    if (cam->io == IO_METHOD_MMAP) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (xioctl(cam->fd, VIDIOC_STREAMOFF, &type) < 0) {
            log_error("Error stopping capture (STREAMOFF): %s\n", strerror(errno));

            return VF_CAMERA_STOP_ERR;
        }
    }

    log_info("The stream has been successfully stopped!\n");

    return VF_SUCCESS;
}

void camera_close(Camera *cam) 
{
    // If streaming is active, stop it
    // enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // if (xioctl(cam->fd, VIDIOC_DQBUF, &type) == 0) {
    // log_error("Stream still active!\n");
    // }

    // Demap memory and free buffers
    if (cam->io == IO_METHOD_MMAP && cam->buffers) {
        for (unsigned int i = 0; i < cam->n_buffers; ++i) {
            if (cam->buffers[i].start && cam->buffers[i].start != MAP_FAILED) {
                munmap(cam->buffers[i].start, cam->buffers[i].length);
            }
        }
    }

    if (cam->io == IO_METHOD_READ && cam->buffers) {
        // Free the buffer allocated for reading
        if (cam->buffers[0].start) {
            free(cam->buffers[0].start);
        }
    }

    // Free the buffer array
    if (cam->buffers) {
        free(cam->buffers);
    }

    // Close the camera file descriptor
    if (cam->fd != -1) {
        close(cam->fd);
    }

    // Reinitialize the Camera structure to 0 (optional)
    cam->fd = -1;
    cam->buffers = NULL;
    cam->n_buffers = 0;

    log_info("The room was closed and the resources released.\n");
}

