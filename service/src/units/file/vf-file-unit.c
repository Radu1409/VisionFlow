/**
 **************************************************************************************************
 *  @file           : vf-file-unit.c
 *  @brief          : VisionFlow File Unit API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  File unit — reads frames from disk (FILE_IN) or writes frames to disk (FILE_OUT).
 *  Implements the vf_unit_operations_t interface.
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
#include "vf-file-unit.h"
#include "vf-file.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"
#include "vf-processing-unit.h"

#define MODULE_NAME "vf_file_unit"

typedef struct {
        vf_file_t            file;
        vf_buf_pool_t       *pool;
        vf_framebuffer_t    *current_fb;
        vf_fb_params_t       fb_params;
        vf_file_unit_mode_t  mode;
        char                 file_path[256];
} vf_file_unit_data_t;

/* =========================================================================
 * Operations
 * ========================================================================= */

vf_err_t vf_file_unit_init(void *ctx, ...)
{
        vf_unit_t            *unit      = NULL;
        vf_file_unit_data_t  *data      = NULL;
        vf_file_unit_cfg_t   *cfg       = NULL;
        va_list               args;
        vf_err_t              err       = VF_SUCCESS;
        const char           *open_mode = NULL;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;

        va_start(args, ctx);
        cfg = va_arg(args, vf_file_unit_cfg_t *);
        va_end(args);

        if (NULL == cfg) {
                log_err("Invalid input: cfg = %p", (void *)cfg);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == cfg->pool && VF_FILE_UNIT_MODE_IN == cfg->mode) {
                log_err("FILE_IN unit requires a buffer pool");

                return VF_INVALID_PARAMETER;
        }

        data = (vf_file_unit_data_t *)calloc(1U, sizeof(vf_file_unit_data_t));
        if (NULL == data) {
                log_err("Failed to allocate file unit internal data");

                return VF_OOM;
        }

        (void)snprintf(data->file_path, sizeof(data->file_path), "%s", cfg->file_path);

        data->fb_params = cfg->fb_params;
        data->pool      = cfg->pool;
        data->mode      = cfg->mode;

        open_mode = (VF_FILE_UNIT_MODE_IN == cfg->mode) ? "rb" : "wb";

        err = vf_file_open(&data->file, data->file_path, open_mode);
        if (VF_SUCCESS != err) {
                log_err("Failed to open file '%s': %s", data->file_path, vf_err2str(err));

                free(data);

                return err;
        }

        unit->internal_data = data;

        log_info("File unit '%s' initialized: path='%s' mode=%s",
                 unit->name ? unit->name : "unknown",
                 data->file_path,
                 (VF_FILE_UNIT_MODE_IN == data->mode) ? "FILE_IN" : "FILE_OUT");

        return VF_SUCCESS;
}

vf_err_t vf_file_unit_deinit(void *ctx, ...)
{
        vf_unit_t           *unit = NULL;
        vf_file_unit_data_t *data = NULL;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = (vf_file_unit_data_t *)unit->internal_data;

        if (NULL == data) {
                return VF_SUCCESS;
        }

        (void)vf_file_close(&data->file);

        if ((NULL != data->current_fb) && (NULL != data->pool)) {
                (void)vf_buf_pool_release(data->pool, data->current_fb);
                data->current_fb = NULL;
        }

        free(data);

        unit->internal_data = NULL;

        log_info("File unit '%s' deinitialized", unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

vf_err_t vf_file_unit_get_data(void *ctx, ...)
{
        vf_unit_t           *unit = NULL;
        vf_file_unit_data_t *data = NULL;
        vf_err_t             err  = VF_SUCCESS;
        size_t               read = 0U;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = (vf_file_unit_data_t *)unit->internal_data;

        if (NULL == data) {
                log_err("Unit '%s' has no internal data", unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        if (VF_FILE_UNIT_MODE_IN == data->mode) {
                /* Acquire buffer from pool and read frame from file */
                err = vf_buf_pool_acquire(data->pool, &data->current_fb);
                if (VF_SUCCESS != err) {
                        log_err("Failed to acquire buffer from pool: %s", vf_err2str(err));

                        return err;
                }

                err = vf_framebuffer_read_from_fptr(data->current_fb,
                                                    data->file.fp,
                                                    &read);
                if (VF_SUCCESS != err) {
                        log_err("Failed to read frame from file: %s", vf_err2str(err));

                        (void)vf_buf_pool_release(data->pool, data->current_fb);

                        data->current_fb = NULL;

                        return err;
                }

                log_dbg("FILE_IN: read %zu bytes from '%s'", read, data->file_path);
        } else {
                /* FILE_OUT: pop frame from in_queue */
                if (NULL == unit->in_queue) {
                        log_err("FILE_OUT unit '%s' has no in_queue",
                                unit->name ? unit->name : "unknown");

                        return VF_INVALID_PARAMETER;
                }

                err = vf_buf_queue_pop(unit->in_queue, &data->current_fb);
                if (VF_SUCCESS != err) {
                        log_dbg("in_queue empty for unit '%s'",
                                unit->name ? unit->name : "unknown");

                        return err;
                }
        }

        return VF_SUCCESS;
}

vf_err_t vf_file_unit_process_data(void *ctx, ...)
{
        /* File unit does no processing — passthrough */
        (void)ctx;

        return VF_SUCCESS;
}

vf_err_t vf_file_unit_send_data(void *ctx, ...)
{
        vf_unit_t           *unit    = NULL;
        vf_file_unit_data_t *data    = NULL;
        vf_err_t             err     = VF_SUCCESS;
        size_t               written = 0U;

        if (NULL == ctx) {
                log_err("Invalid input: ctx = %p", ctx);

                return VF_INVALID_PARAMETER;
        }

        unit = (vf_unit_t *)ctx;
        data = (vf_file_unit_data_t *)unit->internal_data;

        if (NULL == data) {
                log_err("Unit '%s' has no internal data", unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        if (NULL == data->current_fb) {
                log_err("Unit '%s' has no current frame buffer",
                        unit->name ? unit->name : "unknown");

                return VF_INVALID_PARAMETER;
        }

        if (VF_FILE_UNIT_MODE_IN == data->mode) {
                /* Push frame to out_queue */
                if (NULL == unit->out_queue) {
                        log_err("FILE_IN unit '%s' has no out_queue",
                                unit->name ? unit->name : "unknown");

                        (void)vf_buf_pool_release(data->pool, data->current_fb);

                        data->current_fb = NULL;

                        return VF_INVALID_PARAMETER;
                }

                err = vf_buf_queue_push(unit->out_queue, data->current_fb);
                if (VF_SUCCESS != err) {
                        log_err("Failed to push frame to out_queue: %s", vf_err2str(err));

                        (void)vf_buf_pool_release(data->pool, data->current_fb);

                        data->current_fb = NULL;

                        return err;
                }

                log_dbg("FILE_IN: frame pushed to out_queue");

        } else {
                /* Write frame to file then release back to pool */
                err = vf_framebuffer_write_to_fptr(data->current_fb,
                                                   data->file.fp,
                                                   &written);
                if (VF_SUCCESS != err) {
                        log_err("Failed to write frame to file: %s", vf_err2str(err));
                }

                log_dbg("FILE_OUT: wrote %zu bytes to '%s'", written, data->file_path);
        }

        data->current_fb = NULL;

        return err;
}

vf_err_t vf_file_unit_init_operations(vf_unit_operations_t *ops)
{
        if (NULL == ops) {
                log_err("Invalid input: ops = %p", (void *)ops);

                return VF_INVALID_PARAMETER;
        }

        ops->init         = vf_file_unit_init;
        ops->deinit       = vf_file_unit_deinit;
        ops->get_data     = vf_file_unit_get_data;
        ops->process_data = vf_file_unit_process_data;
        ops->send_data    = vf_file_unit_send_data;

        log_info("File unit operations initialized");

        return VF_SUCCESS;
}

