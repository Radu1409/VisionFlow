/**
 **************************************************************************************************
 *  @file           : vf-camera.c
 *  @brief          : VisionFlow Camera API implementation
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Camera library implementation using V4L2 (Video4Linux2).
 *  Captures real frames from a physical camera device.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "vf-camera.h"
#include "vf-error.h"
#include "vf-logger.h"

#define MODULE_NAME "vf_camera"

/* =========================================================================
 * Helpers
 * ========================================================================= */

static
uint32_t pixel_fmt_to_v4l2(vf_pixel_fmt_t fmt)
{
        switch (fmt) {
                case VF_PIXEL_FMT_YUYV:
                        return V4L2_PIX_FMT_YUYV;
                case VF_PIXEL_FMT_MJPEG:
                        return V4L2_PIX_FMT_MJPEG;
                default:
                        log_err("Unsupported pixel format: %d", fmt);

                        return V4L2_PIX_FMT_YUYV;
        }
}

static
vf_err_t camera_ioctl(int fd, unsigned long request, void *arg)
{
        int rc = 0;

        do {
                rc = ioctl(fd, request, arg);
        } while (-1 == rc && EINTR == errno);

        if (-1 == rc) {
                return VF_ERROR_MIN;
        }

        return VF_SUCCESS;
}

const char *vf_camera_state2str(vf_camera_state_t state)
{
        switch (state) {
                case VF_CAMERA_STATE_STOPPED:
                        return VF_CAMERA_STATE_STOPPED_STR;
                case VF_CAMERA_STATE_RUNNING:
                        return VF_CAMERA_STATE_RUNNING_STR;
                case VF_CAMERA_STATE_ERROR:
                        return VF_CAMERA_STATE_ERROR_STR;
                default:
                        return VF_CAMERA_STATE_UNKNOWN_STR;
        }
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_camera_init(vf_camera_t *camera, const vf_camera_cfg_t *cfg)
{
        struct v4l2_capability cap = {0};
        struct v4l2_format fmt = {0};
        struct v4l2_requestbuffers req = {0};
        struct v4l2_buffer buf = {0};
        uint32_t i = 0U;
        vf_err_t err = VF_SUCCESS;

        if (NULL == camera || NULL == cfg) {
                log_err("Invalid input: camera = %p, cfg = %p",
                        (void *)camera, (void *)cfg);

                return VF_INVALID_PARAMETER;
        }

        if (cfg->buffer_count > VF_CAMERA_MAX_BUFFERS) {
                log_err("buffer_count %u exceeds maximum %u",
                        cfg->buffer_count, VF_CAMERA_MAX_BUFFERS);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(camera, 0, sizeof(vf_camera_t));

        camera->cfg = *cfg;
        camera->state = VF_CAMERA_STATE_STOPPED;
        camera->fd = -1;

        /* Open device */
        camera->fd = open(cfg->device_path, O_RDWR | O_NONBLOCK);
        if (-1 == camera->fd) {
                log_err("Failed to open device '%s': %s",
                        cfg->device_path, strerror(errno));

                return VF_INIT_FAILED;
        }

        log_info("Opened camera device '%s'", cfg->device_path);

        /* Query capabilities */
        err = camera_ioctl(camera->fd, VIDIOC_QUERYCAP, &cap);
        if (VF_SUCCESS != err) {
                log_err("VIDIOC_QUERYCAP failed: %s", strerror(errno));

                goto close_fd;
        }

        if (0 == (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                log_err("Device '%s' does not support video capture",
                        cfg->device_path);

                err = VF_INIT_FAILED;

                goto close_fd;
        }

        if (0 == (cap.capabilities & V4L2_CAP_STREAMING)) {
                log_err("Device '%s' does not support streaming",
                        cfg->device_path);

                err = VF_INIT_FAILED;

                goto close_fd;
        }

        log_info("Camera capabilities verified: driver='%s' card='%s'",
                 cap.driver, cap.card);

        /* Set format */
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = cfg->width;
        fmt.fmt.pix.height = cfg->height;
        fmt.fmt.pix.pixelformat = pixel_fmt_to_v4l2(cfg->format);
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        err = camera_ioctl(camera->fd, VIDIOC_S_FMT, &fmt);
        if (VF_SUCCESS != err) {
                log_err("VIDIOC_S_FMT failed: %s", strerror(errno));

                goto close_fd;
        }

        log_info("Camera format set: %ux%u fmt=%u",
                 fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);

        /* Request buffers */
        req.count = cfg->buffer_count;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        err = camera_ioctl(camera->fd, VIDIOC_REQBUFS, &req);
        if (VF_SUCCESS != err) {
                log_err("VIDIOC_REQBUFS failed: %s", strerror(errno));

                goto close_fd;
        }

        if (0U == req.count) {
                log_err("Driver returned 0 buffers");

                err = VF_OOM;

                goto close_fd;
        }

        if (req.count < cfg->buffer_count) {
                log_wrn("Requested %u buffers, driver allocated %u — continuing",
                        cfg->buffer_count, req.count);
        }

        camera->buffer_count = req.count;

        /* mmap buffers */
        for (i = 0U; i < camera->buffer_count; i++) {
                struct v4l2_buffer mmap_buf = {0};

                mmap_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                mmap_buf.memory = V4L2_MEMORY_MMAP;
                mmap_buf.index = i;

                err = camera_ioctl(camera->fd, VIDIOC_QUERYBUF, &mmap_buf);
                if (VF_SUCCESS != err) {
                        log_err("VIDIOC_QUERYBUF failed for buffer %u: %s",
                                i, strerror(errno));

                        goto unmap_buffers;
                }

                camera->buffers[i].length = mmap_buf.length;
                camera->buffers[i].start  = mmap(NULL, mmap_buf.length,
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_SHARED,
                                                 camera->fd,
                                                 mmap_buf.m.offset);

                if (MAP_FAILED == camera->buffers[i].start) {
                        log_err("mmap failed for buffer %u: %s", i, strerror(errno));

                        err = VF_OOM;

                        goto unmap_buffers;
                }

                log_dbg("Buffer %u mapped: length=%u", i, camera->buffers[i].length);
        }

        /* Queue all buffers */
        for (i = 0U; i < camera->buffer_count; i++) {
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                err = camera_ioctl(camera->fd, VIDIOC_QBUF, &buf);
                if (VF_SUCCESS != err) {
                        log_err("VIDIOC_QBUF failed for buffer %u: %s",
                                i, strerror(errno));

                        goto unmap_buffers;
                }
        }

        log_info("Camera '%s' initialized: %ux%u, %u buffers",
                 cfg->device_path, cfg->width, cfg->height, camera->buffer_count);

        return VF_SUCCESS;

unmap_buffers:
        for (i = 0U; i < camera->buffer_count; i++) {
                if (NULL != camera->buffers[i].start &&
                    MAP_FAILED != camera->buffers[i].start) {
                        (void)munmap(camera->buffers[i].start,
                                     camera->buffers[i].length);
                        camera->buffers[i].start = NULL;
                }
        }

close_fd:
        (void)close(camera->fd);

        camera->fd = -1;
        camera->state = VF_CAMERA_STATE_ERROR;

        return err;
}

vf_err_t vf_camera_start(vf_camera_t *camera)
{
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vf_err_t err = VF_SUCCESS;

        if (NULL == camera) {
                log_err("Invalid input: camera = %p", (void *)camera);

                return VF_INVALID_PARAMETER;
        }

        if (VF_CAMERA_STATE_STOPPED != camera->state) {
                log_err("Camera must be in STOPPED state to start, current: %s",
                        vf_camera_state2str(camera->state));

                return VF_INIT_FAILED;
        }

        err = camera_ioctl(camera->fd, VIDIOC_STREAMON, &type);
        if (VF_SUCCESS != err) {
                log_err("VIDIOC_STREAMON failed: %s", strerror(errno));

                camera->state = VF_CAMERA_STATE_ERROR;

                return VF_ERROR_MIN;
        }

        camera->state = VF_CAMERA_STATE_RUNNING;

        log_info("Camera '%s' started", camera->cfg.device_path);

        return VF_SUCCESS;
}

vf_err_t vf_camera_stop(vf_camera_t *camera)
{
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vf_err_t err = VF_SUCCESS;

        if (NULL == camera) {
                log_err("Invalid input: camera = %p", (void *)camera);

                return VF_INVALID_PARAMETER;
        }

        if (VF_CAMERA_STATE_RUNNING != camera->state) {
                log_wrn("Camera is not running, state: %s",
                        vf_camera_state2str(camera->state));

                return VF_SUCCESS;
        }

        err = camera_ioctl(camera->fd, VIDIOC_STREAMOFF, &type);
        if (VF_SUCCESS != err) {
                log_err("VIDIOC_STREAMOFF failed: %s", strerror(errno));

                camera->state = VF_CAMERA_STATE_ERROR;

                return VF_ERROR_MIN;
        }

        camera->state = VF_CAMERA_STATE_STOPPED;

        log_info("Camera '%s' stopped", camera->cfg.device_path);

        return VF_SUCCESS;
}

vf_err_t vf_camera_acquire_frame(vf_camera_t *camera, vf_framebuffer_t *fb)
{
        struct v4l2_buffer buf = {0};
        fd_set fds = {0};
        struct timeval tv = {0};
        size_t copy_size = 0U;
        int rc = 0;
        vf_err_t err = VF_SUCCESS;

        if (NULL == camera || NULL == fb) {
                log_err("Invalid input: camera = %p, fb = %p",
                        (void *)camera, (void *)fb);

                return VF_INVALID_PARAMETER;
        }

        if (VF_CAMERA_STATE_RUNNING != camera->state) {
                log_err("Camera is not running, state: %s",
                        vf_camera_state2str(camera->state));

                return VF_CAMERA_INIT_ERR;
        }

        /* Wait for frame with timeout 2s */
        FD_ZERO(&fds);
        FD_SET(camera->fd, &fds);
        tv.tv_sec  = 2;
        tv.tv_usec = 0;

        rc = select(camera->fd + 1, &fds, NULL, NULL, &tv);
        if (-1 == rc) {
                log_err("select() failed: %s", strerror(errno));

                return VF_ERROR_MIN;
        }

        if (0 == rc) {
                log_err("select() timeout — no frame received");

                return VF_ERROR_MIN;
        }

        /* Dequeue buffer */
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        err = camera_ioctl(camera->fd, VIDIOC_DQBUF, &buf);
        if (VF_SUCCESS != err) {
                log_err("VIDIOC_DQBUF failed: %s", strerror(errno));

                return VF_ERROR_MIN;
        }

        /* Copy frame data into vf_framebuffer_t */
        if (buf.bytesused < fb->total_size) {
                copy_size = buf.bytesused;
        }
        else {
                copy_size = fb->total_size;
        }

        (void)memcpy(fb->data, camera->buffers[buf.index].start, copy_size);

        /* Set metadata */
        vf_framebuffer_set_meta(fb, camera->frame_id, camera->frame_id, camera->cfg.device_path,
                                camera->cfg.width, camera->cfg.height, camera->cfg.format);

        camera->frame_id++;

        log_dbg("Frame acquired: index=%u bytes=%u frame_id=%u",
                buf.index, buf.bytesused, camera->frame_id - 1U);

        /* Re-queue buffer */
        err = camera_ioctl(camera->fd, VIDIOC_QBUF, &buf);
        if (VF_SUCCESS != err) {
                log_err("VIDIOC_QBUF failed: %s", strerror(errno));

                return VF_ERROR_MIN;
        }

        return VF_SUCCESS;
}

vf_err_t vf_camera_deinit(vf_camera_t *camera)
{
        uint32_t i = 0U;
        vf_err_t err = VF_SUCCESS;

        if (NULL == camera) {
                log_err("Invalid input: camera = %p", (void *)camera);

                return VF_INVALID_PARAMETER;
        }

        if (-1 == camera->fd) {
                log_info("Camera already deinitialized, skipping");

                return VF_SUCCESS;
        }

        /* Stop streaming if running */
        if (VF_CAMERA_STATE_RUNNING == camera->state) {
                err = vf_camera_stop(camera);
                if (VF_SUCCESS != err) {
                        log_err("Failed to stop the camera: %s", vf_err2str(err));

                        return err;
                }
        }

        /* Unmap buffers */
        for (i = 0U; i < camera->buffer_count; i++) {
                if (NULL != camera->buffers[i].start &&
                    MAP_FAILED != camera->buffers[i].start) {
                        (void)munmap(camera->buffers[i].start,
                                     camera->buffers[i].length);
                        camera->buffers[i].start = NULL;
                }
        }

        (void)close(camera->fd);

        camera->fd = -1;
        camera->state = VF_CAMERA_STATE_STOPPED;

        log_info("Camera '%s' deinitialized", camera->cfg.device_path);

        return VF_SUCCESS;
}

