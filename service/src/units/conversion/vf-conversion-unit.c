/**
 **************************************************************************************************
 *  @file           : vf-conversion-unit.c
 *  @brief          : VisionFlow Conversion Unit API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Conversion unit — pops a frame from in_queue, converts pixel format,
 *  pushes result to out_queue. Implements the vf_unit_operations_t interface.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "vf-buff-pool.h"
#include "vf-buff-queue.h"
#include "vf-conversion-unit.h"
#include "vf-conversion.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"
#include "vf-processing-unit.h"

#define MODULE_NAME "vf_conversion_unit"

typedef struct {
        vf_conversion_ctx_t  conv_ctx;
        vf_buf_pool_t       *pool;
        vf_buf_pool_t       *src_pool;
        vf_framebuffer_t    *in_fb;
        vf_framebuffer_t    *out_fb;
        vf_fb_params_t       dst_params;
} vf_conversion_unit_data_t;

/* =========================================================================
 * Operations
 * ========================================================================= */

vf_err_t vf_conversion_unit_init(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_conversion_unit_data_t *data = NULL;
        vf_conversion_unit_cfg_t *cfg = NULL;
        vf_err_t err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        cfg = (vf_conversion_unit_cfg_t *)unit->internal_data;

        if (NULL == cfg) {
                log_err("Invalid input: cfg = %p", (void *)cfg);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == cfg->pool) {
                log_err("Conversion unit requires a buffer pool");

                return VF_INVALID_PARAMETER;
        }

        data = (vf_conversion_unit_data_t *)calloc(1U, sizeof(vf_conversion_unit_data_t));
        if (NULL == data) {
                log_err("Failed to allocate conversion unit internal data");

                return VF_OOM;
        }

        err = vf_conversion_init(&data->conv_ctx, cfg->src_fmt, cfg->dst_fmt);
        if (VF_SUCCESS != err) {
                log_err("Failed to initialize conversion context: %s", vf_err2str(err));

                free(data);

                return err;
        }

        data->pool = cfg->pool;
        data->src_pool = cfg->src_pool;
        data->dst_params = cfg->dst_params;

        unit->internal_data = data;

        log_info("Conversion unit '%s' initialized: %s -> %s",
                 unit->name ? unit->name : "unknown",
                 vf_pixel_fmt_str(cfg->src_fmt),
                 vf_pixel_fmt_str(cfg->dst_fmt));

        return VF_SUCCESS;
}

vf_err_t vf_conversion_unit_deinit(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_conversion_unit_data_t *data = NULL;
        vf_err_t buff_release_err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = (vf_conversion_unit_data_t *)unit->internal_data;

        if (NULL == data) {
                return VF_SUCCESS;
        }

        vf_conversion_deinit(&data->conv_ctx);

        if ((NULL != data->in_fb) && (NULL != data->src_pool)) {
                buff_release_err = vf_buf_pool_release(data->src_pool, data->in_fb);
                if (VF_SUCCESS != buff_release_err) {
                        log_err("Failed to release framebuffer back to pool: %s",
                                vf_err2str(buff_release_err));
                }

                data->in_fb = NULL;
        }

        if ((NULL != data->out_fb) && (NULL != data->pool)) {
                vf_buf_pool_release(data->pool, data->out_fb);
                if (VF_SUCCESS != buff_release_err) {
                        log_err("Failed to release framebuffer back to pool: %s",
                                vf_err2str(buff_release_err));
                }

                data->out_fb = NULL;
        }

        free(data);

        unit->internal_data = NULL;

        log_info("Conversion unit '%s' deinitialized", unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

vf_err_t vf_conversion_unit_get_data(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_conversion_unit_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = (vf_conversion_unit_data_t *)unit->internal_data;

        if (NULL == data) {
                log_err("Unit '%s' has no internal data", unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        if (NULL == unit->in_queue) {
                log_err("Conversion unit '%s' has no in_queue",
                        unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        err = vf_buf_queue_pop(unit->in_queue, &data->in_fb);
        if (VF_SUCCESS != err) {
                log_dbg("in_queue empty for unit '%s'", unit->name ? unit->name : "unknown");

                return err;
        }

        log_dbg("Conversion unit '%s': frame popped from in_queue",
                unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

vf_err_t vf_conversion_unit_process_data(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_conversion_unit_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;
        vf_err_t buff_release_err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = (vf_conversion_unit_data_t *)unit->internal_data;

        if (NULL == data) {
                log_err("Unit '%s' has no internal data", unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        if (NULL == data->in_fb) {
                log_err("Unit '%s' has no input frame", unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        /* Acquire output buffer from pool */
        err = vf_buf_pool_acquire(data->pool, &data->out_fb);
        if (VF_SUCCESS != err) {
                log_err("Failed to acquire output buffer from pool: %s", vf_err2str(err));

                return err;
        }

        err = vf_conversion_process(&data->conv_ctx, data->in_fb, data->out_fb);
        if (VF_SUCCESS != err) {
                log_err("Conversion failed: %s", vf_err2str(err));

                buff_release_err = vf_buf_pool_release(data->pool, data->out_fb);
                if (VF_SUCCESS != buff_release_err) {
                        log_err("Failed to release framebuffer back to pool: %s",
                                vf_err2str(buff_release_err));
                }

                data->out_fb = NULL;

                return err;
        }

        log_dbg("Conversion unit '%s': frame converted", unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

vf_err_t vf_conversion_unit_send_data(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_conversion_unit_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;
        vf_err_t buff_release_err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = (vf_conversion_unit_data_t *)unit->internal_data;

        if (NULL == data) {
                log_err("Unit '%s' has no internal data", unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        if (NULL == data->out_fb) {
                log_err("Unit '%s' has no output frame", unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        if (NULL == unit->out_queue) {
                log_err("Conversion unit '%s' has no out_queue",
                        unit->name ? unit->name : "unknown");

                buff_release_err = vf_buf_pool_release(data->pool, data->out_fb);
                if (VF_SUCCESS != buff_release_err) {
                        log_err("Failed to release framebuffer back to pool: %s",
                                vf_err2str(buff_release_err));
                }

                data->out_fb = NULL;

                return VF_INVALID_PARAMETER;
        }

        err = vf_buf_queue_push(unit->out_queue, data->out_fb);
        if (VF_SUCCESS != err) {
                log_err("Failed to push converted frame to out_queue: %s", vf_err2str(err));

                buff_release_err = vf_buf_pool_release(data->pool, data->out_fb);
                if (VF_SUCCESS != buff_release_err) {
                        log_err("Failed to release framebuffer back to pool: %s",
                                vf_err2str(buff_release_err));
                }

                data->out_fb = NULL;

                return err;
        }

        if (NULL != data->in_fb) {
                buff_release_err = vf_buf_pool_release(data->src_pool, data->in_fb);
                if (VF_SUCCESS != buff_release_err) {
                        log_err("Failed to release framebuffer back to pool: %s",
                                vf_err2str(buff_release_err));
                }

                data->in_fb = NULL;
        }

        data->out_fb = NULL;

        log_dbg("Conversion unit '%s': frame pushed to out_queue",
                unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

vf_err_t vf_conversion_unit_init_operations(vf_unit_operations_t *ops)
{
        if (NULL == ops) {
                log_err("Invalid input: ops = %p", (void *)ops);

                return VF_INVALID_PARAMETER;
        }

        ops->init = vf_conversion_unit_init;
        ops->deinit = vf_conversion_unit_deinit;
        ops->get_data = vf_conversion_unit_get_data;
        ops->process_data = vf_conversion_unit_process_data;
        ops->send_data = vf_conversion_unit_send_data;

        log_info("Conversion unit operations initialized");

        return VF_SUCCESS;
}

