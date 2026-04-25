/**
 **************************************************************************************************
 *  @file           : vf-core.c
 *  @brief          : VisionFlow Pipeline Core
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Pipeline core — wires parser output, buffer pools, units, and pipeline manager
 *  into a single executable flow. Entry point for the VisionFlow MVP.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf-buff-pool.h"
#include "vf-conversion-unit.h"
#include "vf-error.h"
#include "vf-file-unit.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"
#include "vf-parser.h"
#include "vf-pipeline-mgr.h"
#include "vf-processing-unit.h"

#define MODULE_NAME        "vf_core"
#define CFG_PATH           "vf_frames/conf/vf_frames_cfg.json"
#define OUTPUT_FILE_PATH   "vf_frames/out/frame_out.yuv"
#define POOL_SLOT_COUNT    4U
#define FRAME_SET_NAME_IN  "rgb"
#define FRAME_SET_NAME_OUT "yuv"

/* =========================================================================
 * Internal helpers
 * ========================================================================= */

static
const vf_frame_set_t *find_frame_set(const vf_frames_cfg_t *cfg, const char *name)
{
        uint32_t i = 0U;

        if ((NULL == cfg) || (NULL == name)) {
                return NULL;
        }

        for (i = 0U; i < cfg->frame_set_count; i++) {
                if (0 == strcmp(cfg->frame_sets[i].name, name)) {
                        return &cfg->frame_sets[i];
                }
        }

        return NULL;
}

static
vf_err_t run_pipeline_for_frames(vf_pipeline_t        *pipeline,
                                  const vf_frame_set_t *frame_set)
{
        vf_err_t err = VF_SUCCESS;
        uint32_t i   = 0U;

        for (i = 0U; i < frame_set->frame_count; i++) {
                log_info("Processing frame [%u/%u]: '%s'",
                         i + 1U, frame_set->frame_count,
                         frame_set->frames[i].file_name);

                err = vf_pipeline_run_once(pipeline);
                if (VF_SUCCESS != err) {
                        log_err("Pipeline run failed at frame %u: %s", i, vf_err2str(err));

                        return err;
                }
        }

        return VF_SUCCESS;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_core_run(void)
{
        vf_err_t                  err              = VF_SUCCESS;
        vf_frames_cfg_t          *cfg              = NULL;
        const vf_frame_set_t     *frame_set_in     = NULL;
        vf_buf_pool_t             pool_in          = { 0 };
        vf_buf_pool_t             pool_out         = { 0 };
        vf_fb_params_t            params_in        = { 0 };
        vf_fb_params_t            params_out       = { 0 };
        vf_file_unit_cfg_t        file_in_cfg      = { 0 };
        vf_file_unit_cfg_t        file_out_cfg     = { 0 };
        vf_conversion_unit_cfg_t  conv_cfg         = { 0 };
        vf_unit_t                 file_in_unit     = { 0 };
        vf_unit_t                 conv_unit        = { 0 };
        vf_unit_t                 file_out_unit    = { 0 };
        vf_pipeline_t             pipeline         = { 0 };
        int                       pool_in_init     = 0;
        int                       pool_out_init    = 0;
        int                       pipeline_created = 0;

        /* ----------------------------------------------------------------
         * 1. Parse config
         * ---------------------------------------------------------------- */
        err = vf_parser_load_frames_cfg(CFG_PATH, &cfg);
        if (VF_SUCCESS != err) {
                log_err("Failed to parse config '%s': %s", CFG_PATH, vf_err2str(err));

                return err;
        }

        frame_set_in = find_frame_set(cfg, FRAME_SET_NAME_IN);
        if (NULL == frame_set_in) {
                log_err("Frame set '%s' not found in config", FRAME_SET_NAME_IN);

                err = VF_FILE_ERR;

                goto cleanup_cfg;
        }

        log_info("Using frame set '%s': %u frames, format=%s, %ux%u",
                 frame_set_in->name,
                 frame_set_in->frame_count,
                 frame_set_in->frames[0].file_name,
                 frame_set_in->frames[0].width,
                 frame_set_in->frames[0].height);

        /* ----------------------------------------------------------------
         * 2. Setup framebuffer params
         * ---------------------------------------------------------------- */
        params_in.width  = frame_set_in->frames[0].width;
        params_in.height = frame_set_in->frames[0].height;
        params_in.format = VF_PIXEL_FMT_RGB888;

        params_out.width  = params_in.width;
        params_out.height = params_in.height;
        params_out.format = VF_PIXEL_FMT_YUV420P;

        /* ----------------------------------------------------------------
         * 3. Init buffer pools
         * ---------------------------------------------------------------- */
        err = vf_buf_pool_init(&pool_in, &params_in, POOL_SLOT_COUNT);
        if (VF_SUCCESS != err) {
                log_err("Failed to init input pool: %s", vf_err2str(err));

                goto cleanup_cfg;
        }

        pool_in_init = 1;

        err = vf_buf_pool_init(&pool_out, &params_out, POOL_SLOT_COUNT);
        if (VF_SUCCESS != err) {
                log_err("Failed to init output pool: %s", vf_err2str(err));

                goto cleanup_pool_in;
        }

        pool_out_init = 1;

        /* ----------------------------------------------------------------
         * 4. Configure units
         * ---------------------------------------------------------------- */

        /* FILE_IN */
        (void)snprintf(file_in_cfg.file_path, sizeof(file_in_cfg.file_path),
                       "%s", frame_set_in->frames[0].full_path);
        file_in_cfg.fb_params = params_in;
        file_in_cfg.pool      = &pool_in;
        file_in_cfg.mode      = VF_FILE_UNIT_MODE_IN;
        file_in_cfg.frame_set = frame_set_in;

        file_in_unit.type = VF_UNIT_TYPE_FILE_IN;
        file_in_unit.name = "file_in";
        (void)vf_file_unit_init_operations(&file_in_unit.operations);

        /* CONVERSION */
        conv_cfg.src_fmt    = VF_PIXEL_FMT_RGB888;
        conv_cfg.dst_fmt    = VF_PIXEL_FMT_YUV420P;
        conv_cfg.dst_params = params_out;
        conv_cfg.pool       = &pool_out;
        conv_cfg.src_pool   = &pool_in;

        conv_unit.type = VF_UNIT_TYPE_CONVERSION;
        conv_unit.name = "conversion";
        (void)vf_conversion_unit_init_operations(&conv_unit.operations);

        /* FILE_OUT */
        (void)snprintf(file_out_cfg.file_path, sizeof(file_out_cfg.file_path),
                       "%s", OUTPUT_FILE_PATH);
        file_out_cfg.fb_params = params_out;
        file_out_cfg.pool      = &pool_out;
        file_out_cfg.mode      = VF_FILE_UNIT_MODE_OUT;
        file_out_cfg.split_output     = 0;
        file_out_cfg.output_extension = ".yuv";

        file_out_unit.type = VF_UNIT_TYPE_FILE_OUT;
        file_out_unit.name = "file_out";
        (void)vf_file_unit_init_operations(&file_out_unit.operations);

        /* ----------------------------------------------------------------
         * 5. Build pipeline
         * ---------------------------------------------------------------- */
        err = vf_pipeline_init(&pipeline, "mvp_pipeline");
        if (VF_SUCCESS != err) {
                log_err("Failed to init pipeline: %s", vf_err2str(err));

                goto cleanup_pool_out;
        }

        err = vf_pipeline_add_unit(&pipeline, &file_in_unit);
        if (VF_SUCCESS != err) { goto cleanup_pool_out; }

        err = vf_pipeline_add_unit(&pipeline, &conv_unit);
        if (VF_SUCCESS != err) { goto cleanup_pool_out; }

        err = vf_pipeline_add_unit(&pipeline, &file_out_unit);
        if (VF_SUCCESS != err) { goto cleanup_pool_out; }

        file_in_unit.internal_data  = &file_in_cfg;
        conv_unit.internal_data     = &conv_cfg;
        file_out_unit.internal_data = &file_out_cfg;

        err = vf_pipeline_create(&pipeline);
        if (VF_SUCCESS != err) {
                log_err("Failed to create pipeline: %s", vf_err2str(err));

                goto cleanup_pool_out;
        }

        pipeline_created = 1;

        /* ----------------------------------------------------------------
         * 6. Run pipeline for each frame
         * ---------------------------------------------------------------- */
        err = run_pipeline_for_frames(&pipeline, frame_set_in);
        if (VF_SUCCESS != err) {
                log_err("Pipeline execution failed: %s", vf_err2str(err));
        } else {
                log_info("Pipeline completed successfully — %u frame(s) processed",
                         frame_set_in->frame_count);
        }

        /* ----------------------------------------------------------------
         * 7. Cleanup
         * ---------------------------------------------------------------- */
        if (1 == pipeline_created) {
                vf_pipeline_destroy(&pipeline);
        }

cleanup_pool_out:
        if (1 == pool_out_init) {
                vf_buf_pool_deinit(&pool_out);
        }

cleanup_pool_in:
        if (1 == pool_in_init) {
                vf_buf_pool_deinit(&pool_in);
        }

cleanup_cfg:
        vf_parser_free_frames_cfg(cfg);

        return err;
}

