/**
 **************************************************************************************************
 *  @file           : vf-stream-provider-unit.c
 *  @brief          : VisionFlow Stream Provider Unit implementation
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Stream provider unit — receives frames from camera_unit and distributes
 *  them to downstream consumers using reference counting on vf_framebuffer_t.
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
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"
#include "vf-processing-unit.h"
#include "vf-stream-provider-unit.h"

#define MODULE_NAME "vf_stream_provider_unit"

/* =========================================================================
 * Internal data
 * ========================================================================= */

typedef struct {
        uint32_t          consumer_count;
        vf_buf_pool_t    *src_pool;
        vf_framebuffer_t *current_fb;
} vf_stream_provider_data_t;

/* =========================================================================
 * Helpers
 * ========================================================================= */

static
vf_stream_provider_data_t *get_internal_data(vf_unit_t *unit)
{
        if (NULL == unit) {
                log_err("Invalid input: unit = %p", (void *)unit);

                return NULL;
        }

        if (NULL == unit->internal_data) {
                log_err("Stream provider unit '%s' has no internal data", unit->name);

                return NULL;
        }

        return (vf_stream_provider_data_t *)unit->internal_data;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_stream_provider_unit_init(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_stream_provider_data_t *data = NULL;
        vf_stream_provider_cfg_t *cfg = NULL;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        log_info("Stream provider unit '%s' initialization started", unit->name);

        cfg = (vf_stream_provider_cfg_t *)unit->internal_data;
        if (NULL == cfg) {
                log_err("Stream provider unit '%s' has no configuration", unit->name);

                return VF_INVALID_PARAMETER;
        }

        if (0U == cfg->consumer_count ||
            cfg->consumer_count > VF_STREAM_PROVIDER_MAX_CONSUMERS) {
                log_err("Invalid consumer_count: %u (max=%u)",
                        cfg->consumer_count, VF_STREAM_PROVIDER_MAX_CONSUMERS);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == cfg->src_pool) {
                log_err("Stream provider unit '%s' has no src_pool", unit->name);

                return VF_INVALID_PARAMETER;
        }

        data = (vf_stream_provider_data_t *)calloc(1U, sizeof(vf_stream_provider_data_t));
        if (NULL == data) {
                log_err("Failed to allocate stream provider data for unit '%s'", unit->name);

                return VF_OOM;
        }

        data->consumer_count = cfg->consumer_count;
        data->src_pool = cfg->src_pool;
        data->current_fb = NULL;

        unit->internal_data = data;

        log_info("Stream provider unit '%s' initialized: %u consumer(s)",
                 unit->name, data->consumer_count);

        return VF_SUCCESS;
}

vf_err_t vf_stream_provider_unit_deinit(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_stream_provider_data_t *data = NULL;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        log_info("Stream provider unit '%s' de-initialization started", unit->name);

        data = get_internal_data(unit);
        if (NULL == data) {
                log_err("Stream provider unit '%s' has no internal data", unit->name);

                return VF_INVALID_PARAMETER;
        }

        free(data);

        unit->internal_data = NULL;

        log_info("Stream provider unit '%s' de-initialized", unit->name);

        return VF_SUCCESS;
}

vf_err_t vf_stream_provider_unit_get_data(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_stream_provider_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;
        uint32_t i = 0U;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        data = get_internal_data(unit);
        if (NULL == data) {
                log_err("Stream provider unit '%s' has no internal data", unit->name);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == unit->in_queue) {
                log_err("Stream provider unit '%s' has no in_queue", unit->name);

                return VF_INVALID_PARAMETER;
        }

        err = vf_buf_queue_pop(unit->in_queue, &data->current_fb);
        if (VF_SUCCESS != err) {
                log_dbg("in_queue empty for stream provider unit '%s'", unit->name);

                return err;
        }

        /* Set ref_count = consumer_count — one ref per consumer */
        for (i = 0U; i < data->consumer_count; i++) {
                vf_framebuffer_ref(data->current_fb);
        }

        log_dbg("Stream provider unit '%s': frame received, ref_count set to %u",
                unit->name, data->consumer_count);

        return VF_SUCCESS;
}

vf_err_t vf_stream_provider_unit_process_data(void *ctx, ...)
{
        (void)ctx;

        return VF_SUCCESS;
}

vf_err_t vf_stream_provider_unit_send_data(void *ctx, ...)
{
        vf_unit_t *unit = NULL;
        vf_stream_provider_data_t *data = NULL;
        vf_err_t err = VF_SUCCESS;
        vf_err_t fb_err = VF_SUCCESS;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        data = get_internal_data(unit);
        if (NULL == data) {
                log_err("Stream provider unit '%s' has no internal data", unit->name);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == data->current_fb) {
                log_err("Stream provider unit '%s' has no current frame", unit->name);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == unit->out_queue) {
                log_err("Stream provider unit '%s' has no out_queue", unit->name);

                fb_err = vf_framebuffer_unref(data->current_fb,
                                              data->src_pool,
                                              (vf_fb_release_fn_t)vf_buf_pool_release);
                if (VF_SUCCESS != fb_err) {
                        log_err("Failed to unref framebuffer for unit '%s': %s",
                                unit->name, vf_err2str(fb_err));
                }

                data->current_fb = NULL;

                return VF_INVALID_PARAMETER;
        }

        err = vf_buf_queue_push(unit->out_queue, data->current_fb);
        if (VF_SUCCESS != err) {
                log_err("Failed to push frame to out_queue for unit '%s': %s",
                        unit->name, vf_err2str(err));

                fb_err = vf_framebuffer_unref(data->current_fb,
                                              data->src_pool,
                                              (vf_fb_release_fn_t)vf_buf_pool_release);
                if (VF_SUCCESS != fb_err) {
                        log_err("Failed to unref framebuffer for unit '%s': %s",
                                unit->name, vf_err2str(fb_err));
                }

                data->current_fb = NULL;

                return err;
        }

        log_dbg("Stream provider unit '%s': frame pushed to out_queue", unit->name);

        data->current_fb = NULL;

        return VF_SUCCESS;
}

vf_err_t vf_stream_provider_unit_init_operations(vf_unit_operations_t *ops)
{
        if (NULL == ops) {
                log_err("Invalid input: ops = %p", (void *)ops);

                return VF_INVALID_PARAMETER;
        }

        ops->init = vf_stream_provider_unit_init;
        ops->deinit = vf_stream_provider_unit_deinit;
        ops->get_data = vf_stream_provider_unit_get_data;
        ops->process_data = vf_stream_provider_unit_process_data;
        ops->send_data = vf_stream_provider_unit_send_data;

        log_info("Stream provider unit operations initialized");

        return VF_SUCCESS;
}

