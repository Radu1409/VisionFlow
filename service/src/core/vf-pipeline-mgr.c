/**
 **************************************************************************************************
 *  @file           : vf-pipeline-mgr.c
 *  @brief          : VisionFlow Pipeline Manager API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Pipeline manager — owns a sequence of vf_unit_t instances connected via buffer queues.
 *  Responsible for unit creation, queue wiring, pipeline execution, and teardown.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <string.h>

#include "vf-error.h"
#include "vf-logger.h"
#include "vf-pipeline-mgr.h"
#include "vf-processing-unit.h"

#define MODULE_NAME "vf_pipeline_mgr"

vf_err_t vf_pipeline_init(vf_pipeline_t *pipeline, const char *name)
{
        if (NULL == pipeline) {
                log_err("Invalid input: pipeline = %p", (void *)pipeline);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(pipeline, 0, sizeof(*pipeline));

        if (NULL != name) {
                (void)snprintf(pipeline->name, sizeof(pipeline->name), "%s", name);
        }

        log_info("Pipeline '%s' initialized", pipeline->name);

        return VF_SUCCESS;
}

vf_err_t vf_pipeline_add_unit(vf_pipeline_t *pipeline, vf_unit_t *unit)
{
        if ((NULL == pipeline) || (NULL == unit)) {
                log_err("Invalid params: pipeline=%p unit=%p",
                        (void *)pipeline, (void *)unit);

                return VF_INVALID_PARAMETER;
        }

        if (pipeline->unit_count >= VF_PIPELINE_MAX_UNITS) {
                log_err("Pipeline '%s' is full (max=%u)", pipeline->name, VF_PIPELINE_MAX_UNITS);

                return VF_INVALID_PARAMETER;
        }

        pipeline->units[pipeline->unit_count] = unit;
        pipeline->unit_count++;

        log_info("Unit '%s' added to pipeline '%s' at index %u",
                 unit->name ? unit->name : "unknown",
                 pipeline->name,
                 pipeline->unit_count - 1U);

        return VF_SUCCESS;
}

vf_err_t vf_pipeline_create(vf_pipeline_t *pipeline)
{
        uint32_t i = 0U;
        uint32_t q_count = 0U;
        vf_err_t err = VF_SUCCESS;

        if (NULL == pipeline) {
                log_err("Invalid input: pipeline = %p", (void *)pipeline);

                return VF_INVALID_PARAMETER;
        }

        if (pipeline->unit_count < 2U) {
                log_err("Pipeline '%s' needs at least 2 units, has %u",
                        pipeline->name, pipeline->unit_count);

                return VF_INVALID_PARAMETER;
        }

        log_info("Creating pipeline '%s' with %u units", pipeline->name, pipeline->unit_count);

        /* Number of queues = number of connections = unit_count - 1 */
        q_count = pipeline->unit_count - 1U;

        /* Initialize queues and wire units */
        for (i = 0U; i < q_count; i++) {
                err = vf_buf_queue_init(&pipeline->queues[i], VF_BUF_QUEUE_MAX_CAPACITY);
                if (VF_SUCCESS != err) {
                        log_err("Failed to init queue[%u]: %s", i, vf_err2str(err));

                        goto cleanup_queues;
                }

                /* out_queue of unit[i] = in_queue of unit[i+1] */
                err = vf_unit_connect_output(pipeline->units[i], &pipeline->queues[i]);
                if (VF_SUCCESS != err) {
                        log_err("Failed to connect output of unit[%u]: %s", i, vf_err2str(err));

                        goto cleanup_queues;
                }

                err = vf_unit_connect_input(pipeline->units[i + 1U], &pipeline->queues[i]);
                if (VF_SUCCESS != err) {
                        log_err("Failed to connect input of unit[%u]: %s", i + 1U, vf_err2str(err));

                        goto cleanup_queues;
                }

                log_dbg("Queue[%u] wired: '%s' -> '%s'",
                        i,
                        pipeline->units[i]->name ? pipeline->units[i]->name : "unknown",
                        pipeline->units[i + 1U]->name ? pipeline->units[i + 1U]->name : "unknown");
        }

        /* Create all units */
        for (i = 0U; i < pipeline->unit_count; i++) {
                err = vf_unit_create(pipeline->units[i]);
                if (VF_SUCCESS != err) {
                        log_err("Failed to create unit[%u] '%s': %s",
                                i,
                                pipeline->units[i]->name ? pipeline->units[i]->name : "unknown",
                                vf_err2str(err));

                        goto cleanup_units;
                }
        }

        pipeline->initialized = 1;

        log_info("Pipeline '%s' created successfully", pipeline->name);

        return VF_SUCCESS;

cleanup_units:
        while (i > 0U) {
                i--;
                vf_unit_destroy(pipeline->units[i]);
        }

cleanup_queues:
        for (i = 0U; i < q_count; i++) {
                vf_buf_queue_deinit(&pipeline->queues[i]);
        }

        return err;
}

vf_err_t vf_pipeline_run_once(vf_pipeline_t *pipeline)
{
        uint32_t i = 0U;
        vf_err_t err = VF_SUCCESS;

        if (NULL == pipeline) {
                log_err("Invalid input: pipeline = %p", (void *)pipeline);

                return VF_INVALID_PARAMETER;
        }

        if (0 == pipeline->initialized) {
                log_err("Pipeline '%s' not initialized", pipeline->name);

                return VF_INIT_FAILED;
        }

        for (i = 0U; i < pipeline->unit_count; i++) {
                err = vf_unit_run(pipeline->units[i]);
                if (VF_SUCCESS != err) {
                        log_err("Pipeline '%s': unit[%u] '%s' failed: %s",
                                pipeline->name,
                                i,
                                pipeline->units[i]->name ? pipeline->units[i]->name : "unknown",
                                vf_err2str(err));

                        return err;
                }
        }

        log_dbg("Pipeline '%s': run_once complete", pipeline->name);

        return VF_SUCCESS;
}

vf_err_t vf_pipeline_start(vf_pipeline_t *pipeline)
{
        uint32_t i = 0U;
        vf_err_t err = VF_SUCCESS;

        if (NULL == pipeline) {
                log_err("Invalid input: pipeline = %p", (void *)pipeline);

                return VF_INVALID_PARAMETER;
        }

        if (0 == pipeline->initialized) {
                log_err("Pipeline '%s' not initialized", pipeline->name);

                return VF_INIT_FAILED;
        }

        log_info("Starting pipeline '%s' — launching %u thread(s)",
                 pipeline->name, pipeline->unit_count);

        for (i = 0U; i < pipeline->unit_count; i++) {
                err = vf_unit_start(pipeline->units[i]);
                if (VF_SUCCESS != err) {
                        log_err("Failed to start unit '%s': %s",
                                pipeline->units[i]->name ? pipeline->units[i]->name : "unknown",
                                vf_err2str(err));

                        /* Stop units already started */
                        while (i > 0U) {
                                i--;
                                err = vf_unit_stop(pipeline->units[i]);
                                if (VF_SUCCESS != err) {
                                        log_err("Failed to stop unit '%s': %s",
                                                pipeline->units[i]->name ? pipeline->units[i]->name : "unknown",
                                                vf_err2str(err));
                                }
                        }

                        return err;
                }
        }

        log_info("Pipeline '%s' started successfully", pipeline->name);

        return VF_SUCCESS;
}

vf_err_t vf_pipeline_stop(vf_pipeline_t *pipeline)
{
        uint32_t i = 0U;
        vf_err_t err = VF_SUCCESS;

        if (NULL == pipeline) {
                log_err("Invalid input: pipeline = %p", (void *)pipeline);

                return VF_INVALID_PARAMETER;
        }

        if (0 == pipeline->initialized) {
                log_err("Pipeline '%s' not initialized", pipeline->name);

                return VF_INIT_FAILED;
        }

        log_info("Stopping pipeline '%s'", pipeline->name);

        for (i = 0U; i < pipeline->unit_count; i++) {
                err = vf_unit_stop(pipeline->units[i]);
                if (VF_SUCCESS != err) {
                        log_err("Failed to stop unit '%s': %s",
                                pipeline->units[i]->name ? pipeline->units[i]->name : "unknown",
                                vf_err2str(err));
                }
        }

        log_info("Pipeline '%s' stopped", pipeline->name);

        return VF_SUCCESS;
}

void vf_pipeline_destroy(vf_pipeline_t *pipeline)
{
        uint32_t i = 0U;
        uint32_t q_count = 0U;

        if (NULL == pipeline) {
                log_err("vf_pipeline_destroy called with NULL pipeline");

                return;
        }

        if (0 == pipeline->initialized) {
                log_err("Pipeline not initialized");

                return;
        }

        log_info("Destroying pipeline '%s'", pipeline->name);

        for (i = 0U; i < pipeline->unit_count; i++) {
                vf_unit_destroy(pipeline->units[i]);
        }

        q_count = (pipeline->unit_count > 0U) ? (pipeline->unit_count - 1U) : 0U;

        for (i = 0U; i < q_count; i++) {
                vf_buf_queue_deinit(&pipeline->queues[i]);
        }

        pipeline->initialized = 0;

        log_info("Pipeline '%s' destroyed", pipeline->name);
}

