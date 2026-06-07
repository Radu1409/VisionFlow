/**
 **************************************************************************************************
 *  @file           : vf-service.c
 *  @brief          : VisionFlow Service Lifecycle Manager
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Service lifecycle manager — implements init, start, stop and deinit
 *  for the camera pipeline service. Encapsulates buffer pool allocation,
 *  unit configuration, pipeline construction and graceful shutdown into
 *  a single vf_service_t lifecycle API.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <string.h>

#include "vf-logger.h"
#include "vf-framebuffer.h"
#include "vf-service.h"

#define VF_CAMERA_STR          "camera"
#define VF_CAMERA_IN_STR       "camera_in"
#define VF_STREAM_PROVIDER_STR "stream_provider"
#define VF_CONVERSION_STR      "conversion"
#define VF_FILE_OUT_STR        "file_out"

#define MODULE_NAME "vf_service"

vf_err_t vf_service_init(vf_service_t *svc, const vf_service_cfg_t *cfg)
{
        vf_fb_params_t params_in = {0};
        vf_fb_params_t params_out = {0};
        vf_err_t err = VF_SUCCESS;

        if ((NULL == svc) || (NULL == cfg)) {
                log_err("Invalid params: svc=%p cfg=%p",
                        (void *)svc, (void *)cfg);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(svc, 0, sizeof(*svc));

        /* ----------------------------------------------------------------
         * 1. Setup framebuffer params
         * ---------------------------------------------------------------- */
        params_in.width = cfg->width;
        params_in.height = cfg->height;
        params_in.format = cfg->src_fmt;

        params_out.width = cfg->width;
        params_out.height = cfg->height;
        params_out.format = cfg->dst_fmt;

        /* ----------------------------------------------------------------
         * 2. Init buffer pools
         * ---------------------------------------------------------------- */
        err = vf_buf_pool_init(&svc->pool_in, &params_in, cfg->pool_slot_count);
        if (VF_SUCCESS != err) {
                log_err("Failed to init input pool: %s", vf_err2str(err));

                return err;
        }

        err = vf_buf_pool_init(&svc->pool_out, &params_out, cfg->pool_slot_count);
        if (VF_SUCCESS != err) {
                log_err("Failed to init output pool: %s", vf_err2str(err));

                vf_buf_pool_deinit(&svc->pool_in);

                return err;
        }

        /* ----------------------------------------------------------------
         * 3. Configure units
         * ---------------------------------------------------------------- */

        /* CAMERA_IN */
        (void)snprintf(svc->camera_unit_cfg.camera_cfg.device_path,
                       sizeof(svc->camera_unit_cfg.camera_cfg.device_path),
                       "%s", cfg->device_path);

        svc->camera_unit_cfg.camera_cfg.width = cfg->width;
        svc->camera_unit_cfg.camera_cfg.height = cfg->height;
        svc->camera_unit_cfg.camera_cfg.format = cfg->src_fmt;
        svc->camera_unit_cfg.camera_cfg.buffer_count = VF_CAMERA_DEFAULT_BUFFER_COUNT;
        svc->camera_unit_cfg.pool = &svc->pool_in;

        svc->camera_unit.type = VF_UNIT_TYPE_CAMERA;
        svc->camera_unit.name = VF_CAMERA_IN_STR;
        svc->camera_unit.internal_data = &svc->camera_unit_cfg;

        err = vf_camera_unit_init_operations(&svc->camera_unit.operations);
        if (VF_SUCCESS != err) {
                log_err("Camera unit init operations failed: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        /* STREAM_PROVIDER */
        svc->sp_cfg.consumer_count = 1U;
        svc->sp_cfg.src_pool = &svc->pool_in;

        svc->sp_unit.type = VF_UNIT_TYPE_STREAM_PROVIDER;
        svc->sp_unit.name = VF_STREAM_PROVIDER_STR;
        svc->sp_unit.internal_data = &svc->sp_cfg;

        err = vf_stream_provider_unit_init_operations(&svc->sp_unit.operations);
        if (VF_SUCCESS != err) {
                log_err("Stream provider unit init operations failed: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        /* CONVERSION */
        svc->conv_cfg.src_fmt = cfg->src_fmt;
        svc->conv_cfg.dst_fmt = cfg->dst_fmt;
        svc->conv_cfg.dst_params = params_out;
        svc->conv_cfg.pool = &svc->pool_out;
        svc->conv_cfg.src_pool = &svc->pool_in;

        svc->conv_unit.type = VF_UNIT_TYPE_CONVERSION;
        svc->conv_unit.name = VF_CONVERSION_STR;
        svc->conv_unit.internal_data = &svc->conv_cfg;

        err = vf_conversion_unit_init_operations(&svc->conv_unit.operations);
        if (VF_SUCCESS != err) {
                log_err("Conversion unit init operations failed: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        /* FILE_OUT */
        svc->file_out_cfg.fb_params = params_out;
        svc->file_out_cfg.pool = &svc->pool_out;
        svc->file_out_cfg.mode = VF_FILE_UNIT_MODE_OUT;
        svc->file_out_cfg.split_output = 1;
        svc->file_out_cfg.output_extension = cfg->output_extension;
        svc->file_out_cfg.frame_set = NULL;

        (void)snprintf(svc->file_out_cfg.output_dir,
                       sizeof(svc->file_out_cfg.output_dir),
                       "%s", cfg->output_dir);

        svc->file_out_unit.type = VF_UNIT_TYPE_FILE_OUT;
        svc->file_out_unit.name = VF_FILE_OUT_STR;
        svc->file_out_unit.internal_data = &svc->file_out_cfg;

        err = vf_file_unit_init_operations(&svc->file_out_unit.operations);
        if (VF_SUCCESS != err) {
                log_err("File unit init operations failed: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        /* ----------------------------------------------------------------
         * 4. Build pipeline
         * ---------------------------------------------------------------- */
        err = vf_pipeline_init(&svc->pipeline, VF_CAMERA_STR);
        if (VF_SUCCESS != err) {
                log_err("Failed to init pipeline: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        err = vf_pipeline_add_unit(&svc->pipeline, &svc->camera_unit);
        if (VF_SUCCESS != err) {
                log_err("Failed to add camera_in unit: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        err = vf_pipeline_add_unit(&svc->pipeline, &svc->sp_unit);
        if (VF_SUCCESS != err) {
                log_err("Failed to add stream_provider unit: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        err = vf_pipeline_add_unit(&svc->pipeline, &svc->conv_unit);
        if (VF_SUCCESS != err) {
                log_err("Failed to add conversion unit: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        err = vf_pipeline_add_unit(&svc->pipeline, &svc->file_out_unit);
        if (VF_SUCCESS != err) {
                log_err("Failed to add file_out unit: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        err = vf_pipeline_create(&svc->pipeline);
        if (VF_SUCCESS != err) {
                log_err("Failed to create pipeline: %s", vf_err2str(err));

                goto cleanup_pools;
        }

        svc->initialized = 1;

        log_info("Service initialized successfully");

        return VF_SUCCESS;

cleanup_pools:
        vf_buf_pool_deinit(&svc->pool_out);
        vf_buf_pool_deinit(&svc->pool_in);

        return err;
}

vf_err_t vf_service_start(vf_service_t *svc)
{
        vf_err_t err = VF_SUCCESS;

        if (NULL == svc) {
                log_err("Invalid input: svc = %p", (void *)svc);

                return VF_INVALID_PARAMETER;
        }

        if (0 == svc->initialized) {
                log_err("Service not initialized");

                return VF_INIT_FAILED;
        }

        vf_buf_pool_reset(&svc->pool_in);
        vf_buf_pool_reset(&svc->pool_out);
        vf_buf_queue_reset(&svc->pipeline.queues[0]);
        vf_buf_queue_reset(&svc->pipeline.queues[1]);
        vf_buf_queue_reset(&svc->pipeline.queues[2]);

        err = vf_pipeline_start(&svc->pipeline);
        if (VF_SUCCESS != err) {
                log_err("Failed to start pipeline: %s", vf_err2str(err));

                return err;
        }

        log_info("Service started");

        return VF_SUCCESS;
}

vf_err_t vf_service_stop(vf_service_t *svc)
{
        vf_err_t err = VF_SUCCESS;

        if (NULL == svc) {
                log_err("Invalid input: svc = %p", (void *)svc);

                return VF_INVALID_PARAMETER;
        }

        if (0 == svc->initialized) {
                log_err("Service not initialized");

                return VF_INIT_FAILED;
        }

        vf_buf_pool_shutdown(&svc->pool_in);
        vf_buf_pool_shutdown(&svc->pool_out);

        err = vf_pipeline_stop(&svc->pipeline);
        if (VF_SUCCESS != err) {
                log_err("Failed to stop pipeline: %s", vf_err2str(err));

                return err;
        }

        log_info("Service stopped");

        return VF_SUCCESS;
}

void vf_service_deinit(vf_service_t *svc)
{
        if (NULL == svc) {
                log_err("Invalid input: svc = %p", (void *)svc);

                return;
        }

        if (0 == svc->initialized) {
                log_err("Service not initialized");

                return;
        }

        vf_pipeline_destroy(&svc->pipeline);
        vf_buf_pool_deinit(&svc->pool_out);
        vf_buf_pool_deinit(&svc->pool_in);

        (void)memset(svc, 0, sizeof(*svc));

        log_info("Service deinitialized");
}

