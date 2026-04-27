/**
 **************************************************************************************************
 *  @file           : vf-core.c
 *  @brief          : VisionFlow Pipeline Core
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Pipeline core — scenario descriptor table and dispatcher. Each scenario
 *  is described as data. A single run_scenario() function builds and runs
 *  any scenario based on its descriptor.
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
#include "vf-conversion.h"
#include "vf-error.h"
#include "vf-file-unit.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"
#include "vf-parser.h"
#include "vf-pipeline-mgr.h"
#include "vf-processing-unit.h"

#define MODULE_NAME      "vf_core"
#define CFG_PATH         "vf_frames/conf/vf_frames_cfg.json"
#define POOL_SLOT_COUNT  4U
#define CONCAT_FILE_NAME "frame_out"

/* =========================================================================
 * Scenario descriptor
 * ========================================================================= */

typedef struct {
        const char     *flag;
        const char     *name;
        const char     *input_set;
        vf_pixel_fmt_t  src_fmt;
        vf_pixel_fmt_t  dst_fmt;
        const char     *output_dir;
        const char     *output_extension;
        int             split_output;
} vf_scenario_t;

static const vf_scenario_t g_scenarios[] = {
        {
                "--rgb_to_yuv_concat",
                "rgb_to_yuv_concat",
                "rgb",
                VF_PIXEL_FMT_RGB888,
                VF_PIXEL_FMT_YUV420P,
                "vf_frames/out/rgb_to_yuv_concat",
                ".yuv",
                0
        },
        {
                "--rgb_to_yuv_split",
                "rgb_to_yuv_split",
                "rgb",
                VF_PIXEL_FMT_RGB888,
                VF_PIXEL_FMT_YUV420P,
                "vf_frames/out/rgb_to_yuv_split",
                ".yuv",
                1
        },
        {
                "--yuv_to_rgb_concat",
                "yuv_to_rgb_concat",
                "yuv",
                VF_PIXEL_FMT_YUV420P,
                VF_PIXEL_FMT_RGB888,
                "vf_frames/out/yuv_to_rgb_concat",
                ".rgb",
                0
        },
        {
                "--yuv_to_rgb_split",
                "yuv_to_rgb_split",
                "yuv",
                VF_PIXEL_FMT_YUV420P,
                VF_PIXEL_FMT_RGB888,
                "vf_frames/out/yuv_to_rgb_split",
                ".rgb",
                1
        },
        {
                "--raw_to_rgb_concat",
                "raw_to_rgb_concat",
                "raw",
                VF_PIXEL_FMT_RAW8,
                VF_PIXEL_FMT_RGB888,
                "vf_frames/out/raw_to_rgb_concat",
                ".rgb",
                0
        },
        {
                "--raw_to_rgb_split",
                "raw_to_rgb_split",
                "raw",
                VF_PIXEL_FMT_RAW8,
                VF_PIXEL_FMT_RGB888,
                "vf_frames/out/raw_to_rgb_split",
                ".rgb",
                1
        },
        {
                "--raw_to_yuv_concat",
                "raw_to_yuv_concat",
                "raw",
                VF_PIXEL_FMT_RAW8,
                VF_PIXEL_FMT_YUV420P,
                "vf_frames/out/raw_to_yuv_concat",
                ".yuv",
                0
        },
        {
                "--raw_to_yuv_split",
                "raw_to_yuv_split",
                "raw",
                VF_PIXEL_FMT_RAW8,
                VF_PIXEL_FMT_YUV420P,
                "vf_frames/out/raw_to_yuv_split",
                ".yuv",
                1
        },
};

#define SCENARIO_COUNT \
        (sizeof(g_scenarios) / sizeof(g_scenarios[0]))

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
                        log_err("Pipeline run failed at frame %u: %s",
                                i, vf_err2str(err));

                        return err;
                }
        }

        return VF_SUCCESS;
}

static
void on_frame_ready(vf_notifier_event_t event, void *data, void *ctx)
{
        (void)data;
        (void)ctx;

        log_info("Notifier: event received — '%s'", vf_notifier_event2str(event));
}

/* =========================================================================
 * Single scenario runner
 * ========================================================================= */

static
vf_err_t run_scenario(const vf_scenario_t  *scenario,
                      const vf_frames_cfg_t *cfg)
{
        vf_err_t                  err              = VF_SUCCESS;
        const vf_frame_set_t     *frame_set        = NULL;
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
        char                      concat_path[VF_PARSER_MAX_FULL_PATH_LEN] = { 0 };

        log_info("=== Running scenario: %s ===", scenario->name);

        /* ----------------------------------------------------------------
         * 1. Find input frame set
         * ---------------------------------------------------------------- */
        frame_set = find_frame_set(cfg, scenario->input_set);
        if (NULL == frame_set) {
                log_err("Frame set '%s' not found in config", scenario->input_set);

                return VF_FILE_ERR;
        }

        log_info("Input frame set '%s': %u frames, %ux%u",
                 frame_set->name,
                 frame_set->frame_count,
                 frame_set->frames[0].width,
                 frame_set->frames[0].height);

        /* ----------------------------------------------------------------
         * 2. Setup framebuffer params
         * ---------------------------------------------------------------- */
        params_in.width  = frame_set->frames[0].width;
        params_in.height = frame_set->frames[0].height;
        params_in.format = scenario->src_fmt;

        params_out.width  = params_in.width;
        params_out.height = params_in.height;
        params_out.format = scenario->dst_fmt;

        /* ----------------------------------------------------------------
         * 3. Init buffer pools
         * ---------------------------------------------------------------- */
        err = vf_buf_pool_init(&pool_in, &params_in, POOL_SLOT_COUNT);
        if (VF_SUCCESS != err) {
                log_err("Failed to init input pool: %s", vf_err2str(err));

                return err;
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
        file_in_cfg.frame_set        = frame_set;
        file_in_cfg.fb_params        = params_in;
        file_in_cfg.pool             = &pool_in;
        file_in_cfg.mode             = VF_FILE_UNIT_MODE_IN;
        file_in_cfg.split_output     = 0;
        file_in_cfg.output_extension = NULL;

        (void)snprintf(file_in_cfg.file_path, sizeof(file_in_cfg.file_path),
                       "%s", frame_set->frames[0].full_path);

        file_in_unit.type = VF_UNIT_TYPE_FILE_IN;
        file_in_unit.name = "file_in";
        (void)vf_file_unit_init_operations(&file_in_unit.operations);

        /* CONVERSION */
        conv_cfg.src_fmt    = scenario->src_fmt;
        conv_cfg.dst_fmt    = scenario->dst_fmt;
        conv_cfg.dst_params = params_out;
        conv_cfg.pool       = &pool_out;
        conv_cfg.src_pool   = &pool_in;

        conv_unit.type = VF_UNIT_TYPE_CONVERSION;
        conv_unit.name = "conversion";
        (void)vf_conversion_unit_init_operations(&conv_unit.operations);

        /* FILE_OUT */
        file_out_cfg.frame_set        = frame_set;
        file_out_cfg.fb_params        = params_out;
        file_out_cfg.pool             = &pool_out;
        file_out_cfg.mode             = VF_FILE_UNIT_MODE_OUT;
        file_out_cfg.split_output     = scenario->split_output;
        file_out_cfg.output_extension = scenario->output_extension;

        (void)snprintf(file_out_cfg.output_dir, sizeof(file_out_cfg.output_dir),
                       "%s", scenario->output_dir);

        if (0 == scenario->split_output) {
                (void)snprintf(concat_path, sizeof(concat_path), "%s/%s%s",
                               scenario->output_dir,
                               CONCAT_FILE_NAME,
                               scenario->output_extension);

                (void)snprintf(file_out_cfg.file_path, sizeof(file_out_cfg.file_path),
                               "%s", concat_path);
        }

        file_out_unit.type = VF_UNIT_TYPE_FILE_OUT;
        file_out_unit.name = "file_out";
        (void)vf_file_unit_init_operations(&file_out_unit.operations);

        /* ----------------------------------------------------------------
         * 5. Store configs in internal_data before pipeline_create
         * ---------------------------------------------------------------- */
        file_in_unit.internal_data  = &file_in_cfg;
        conv_unit.internal_data     = &conv_cfg;
        file_out_unit.internal_data = &file_out_cfg;

        /* ----------------------------------------------------------------
         * 6. Build pipeline
         * ---------------------------------------------------------------- */
        err = vf_pipeline_init(&pipeline, scenario->name);
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

        err = vf_pipeline_create(&pipeline);
        if (VF_SUCCESS != err) {
                log_err("Failed to create pipeline: %s", vf_err2str(err));

                goto cleanup_pool_out;
        }

        pipeline_created = 1;

        (void)vf_notifier_subscribe(&file_in_unit.notifier,
                                    VF_NOTIFIER_EVENT_FRAME_READY,
                                    on_frame_ready, NULL);

        /* ----------------------------------------------------------------
         * 7. Run pipeline for each frame
         * ---------------------------------------------------------------- */
        err = run_pipeline_for_frames(&pipeline, frame_set);
        if (VF_SUCCESS != err) {
                log_err("Pipeline execution failed: %s", vf_err2str(err));
        } else {
                log_info("Scenario '%s' completed — %u frame(s) processed",
                         scenario->name, frame_set->frame_count);
        }

        /* ----------------------------------------------------------------
         * 8. Cleanup
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

        return err;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_core_run_all(void)
{
        vf_err_t       err      = VF_SUCCESS;
        vf_frames_cfg_t *cfg   = NULL;
        size_t          i      = 0U;
        int             failed = 0;

        err = vf_parser_load_frames_cfg(CFG_PATH, &cfg);
        if (VF_SUCCESS != err) {
                log_err("Failed to parse config '%s': %s", CFG_PATH, vf_err2str(err));

                return err;
        }

        for (i = 0U; i < SCENARIO_COUNT; i++) {
                err = run_scenario(&g_scenarios[i], cfg);
                if (VF_SUCCESS != err) {
                        log_err("Scenario '%s' failed: %s",
                                g_scenarios[i].name, vf_err2str(err));
                        failed++;
                }
        }

        vf_parser_free_frames_cfg(cfg);

        if (0 != failed) {
                log_err("%d scenario(s) failed", failed);

                return VF_FILE_ERR;
        }

        log_info("All %zu scenario(s) completed successfully", SCENARIO_COUNT);

        return VF_SUCCESS;
}

vf_err_t vf_core_run_by_flag(const char *flag)
{
        vf_err_t        err = VF_SUCCESS;
        vf_frames_cfg_t *cfg = NULL;
        size_t           i   = 0U;

        if (NULL == flag) {
                log_err("Invalid input: flag = NULL");

                return VF_INVALID_PARAMETER;
        }

        for (i = 0U; i < SCENARIO_COUNT; i++) {
                if (0 == strcmp(g_scenarios[i].flag, flag)) {
                        err = vf_parser_load_frames_cfg(CFG_PATH, &cfg);
                        if (VF_SUCCESS != err) {
                                log_err("Failed to parse config: %s", vf_err2str(err));

                                return err;
                        }

                        err = run_scenario(&g_scenarios[i], cfg);

                        vf_parser_free_frames_cfg(cfg);

                        return err;
                }
        }

        log_err("Unknown flag: '%s'", flag);
        log_info("Available flags:");

        for (i = 0U; i < SCENARIO_COUNT; i++) {
                log_info("  %s", g_scenarios[i].flag);
        }

        return VF_INVALID_PARAMETER;
}

