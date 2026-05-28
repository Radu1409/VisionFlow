/**
 **************************************************************************************************
 *  @file           : vf-camera-unit.c
 *  @brief          : VisionFlow Camera Unit implementation
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Camera unit — wraps vf_camera library into vf_unit_operations_t.
 *  Captures real frames from a V4L2 device and pushes them into the
 *  pipeline via the notifier.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "vf-camera.h"
#include "vf-camera-unit.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"
#include "vf-notifier.h"
#include "vf-processing-unit.h"

#define MODULE_NAME "vf_camera_unit"

/* =========================================================================
 * Internal data
 * ========================================================================= */

typedef struct {
        vf_camera_t       camera;
        vf_buf_pool_t    *pool;
        vf_framebuffer_t *current_fb;
} vf_camera_unit_data_t;

/* =========================================================================
 * Helpers
 * ========================================================================= */

static
vf_camera_unit_data_t *get_internal_data(vf_unit_t *unit)
{
        if (NULL == unit) {
                log_err("Invalid input: unit = %p", (void *)unit);

                return NULL;
        }

        if (NULL == unit->internal_data) {
                log_err("Camera unit internal data not initialized for unit '%s'",
                        unit->name);

                return NULL;
        }

        return (vf_camera_unit_data_t *)unit->internal_data;
}

static
void print_configuration(const vf_camera_cfg_t *cfg)
{
        if (NULL == cfg) {
                log_err("Invalid input: cfg = %p", (void *)cfg);

                return;
        }

        log_info("Camera unit configuration: device='%s' %ux%u fmt=%s buffers=%u",
                 cfg->device_path,
                 cfg->width,
                 cfg->height,
                 vf_pixel_fmt_str(cfg->format),
                 cfg->buffer_count);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_camera_unit_init(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_camera_unit_data_t *data = NULL;
        vf_camera_unit_cfg_t *cfg  = NULL;
        vf_err_t err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        log_info("Camera unit '%s' initialization started", unit->name);

        cfg = (vf_camera_unit_cfg_t *)unit->internal_data;
        if (NULL == cfg) {
                log_err("Camera unit '%s' has no configuration", unit->name);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == cfg->pool) {
                log_err("Camera unit '%s' has no buffer pool", unit->name);

                return VF_INVALID_PARAMETER;
        }

        print_configuration(&cfg->camera_cfg);

        data = (vf_camera_unit_data_t *)calloc(1U, sizeof(vf_camera_unit_data_t));
        if (NULL == data) {
                log_err("Failed to allocate camera unit data for unit '%s'", unit->name);

                return VF_OOM;
        }

        data->pool = cfg->pool;

        err = vf_camera_init(&data->camera, &cfg->camera_cfg);
        if (VF_SUCCESS != err) {
                log_err("Failed to initialize camera for unit '%s': %s",
                        unit->name, vf_err2str(err));

                free(data);

                return err;
        }

        err = vf_camera_start(&data->camera);
        if (VF_SUCCESS != err) {
                log_err("Failed to start camera for unit '%s': %s",
                        unit->name, vf_err2str(err));

                vf_camera_deinit(&data->camera);
                free(data);

                return err;
        }

        unit->internal_data = data;

        log_info("Camera unit '%s' initialized successfully", unit->name);

        return VF_SUCCESS;
}

vf_err_t vf_camera_unit_deinit(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_camera_unit_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        log_info("Camera unit '%s' de-initialization started", unit->name);

        data = get_internal_data(unit);
        if (NULL == data) {
                return VF_INVALID_PARAMETER;
        }

        err = vf_camera_stop(&data->camera);
        if (VF_SUCCESS != err) {
                log_err("Failed to stop the camera. Error: %d", err);

                return VF_CAMERA_STOP_ERR;
        }

        err = vf_camera_deinit(&data->camera);
        if (VF_SUCCESS != err) {
                log_err("Failed to deinit the camera. Error: %d", err);

                return VF_CAMERA_DEINIT_ERR;
        }

        free(data);

        unit->internal_data = NULL;

        log_info("Camera unit '%s' de-initialized", unit->name);

        return VF_SUCCESS;
}

vf_err_t vf_camera_unit_get_data(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_camera_unit_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        data = get_internal_data(unit);
        if (NULL == data) {
                log_err("Failed to get the internal data. data=%p", (void *)data);

                return VF_INVALID_PARAMETER;
        }

        err = vf_buf_pool_acquire(data->pool, &data->current_fb);
        if (VF_SUCCESS != err) {
                log_err("Failed to acquire framebuffer from pool for unit '%s': %s",
                        unit->name, vf_err2str(err));

                return err;
        }

        err = vf_camera_acquire_frame(&data->camera, data->current_fb);
        if (VF_SUCCESS != err) {
                log_err("Failed to acquire frame for unit '%s': %s",
                        unit->name, vf_err2str(err));

                (void)vf_buf_pool_release(data->pool, data->current_fb);
                data->current_fb = NULL;

                return err;
        }

        return VF_SUCCESS;
}

vf_err_t vf_camera_unit_process_data(void *ctx, ...)
{
        (void)ctx;

        return VF_SUCCESS;
}

vf_err_t vf_camera_unit_send_data(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_camera_unit_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = get_internal_data(unit);
        if (NULL == data) {
                log_err("Failed to get the internal data. data=%p", (void *)data);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == data->current_fb) {
                log_err("Camera unit '%s' has no current frame", unit->name);

                return VF_INVALID_PARAMETER;
        }

        err = vf_buf_queue_push(unit->out_queue, data->current_fb);
        if (VF_SUCCESS != err) {
                log_err("Failed to push frame to out_queue for unit '%s': %s",
                        unit->name, vf_err2str(err));

                (void)vf_buf_pool_release(data->pool, data->current_fb);
                data->current_fb = NULL;

                return err;
        }

        err = vf_notifier_publish(&unit->notifier,
                                  VF_NOTIFIER_EVENT_FRAME_READY,
                                  data->current_fb);
        if (VF_SUCCESS != err) {
                log_err("Failed to publish FRAME_READY for unit '%s': %s",
                        unit->name, vf_err2str(err));
        }

        data->current_fb = NULL;

        return VF_SUCCESS;
}

vf_err_t vf_camera_unit_init_operations(vf_unit_operations_t *ops)
{
        if (NULL == ops) {
                log_err("Invalid input: ops = %p", (void *)ops);

                return VF_INVALID_PARAMETER;
        }

        ops->init = vf_camera_unit_init;
        ops->deinit = vf_camera_unit_deinit;
        ops->get_data = vf_camera_unit_get_data;
        ops->process_data = vf_camera_unit_process_data;
        ops->send_data = vf_camera_unit_send_data;

        log_info("Camera unit operations initialized");

        return VF_SUCCESS;
}

