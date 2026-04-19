/**
 **************************************************************************************************
 *  @file           : vf-pipeline-mgr.h
 *  @brief          : VisionFlow Pipeline Manager API Header
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

#ifndef VF_PIPELINE_MGR_H
#define VF_PIPELINE_MGR_H

#include <stdint.h>

#include "vf-buff-queue.h"
#include "vf-error.h"
#include "vf-processing-unit.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VF_PIPELINE_MAX_UNITS  8U
#define VF_PIPELINE_NAME_LEN   64U

typedef struct {
        char           name[VF_PIPELINE_NAME_LEN];
        vf_unit_t     *units[VF_PIPELINE_MAX_UNITS];
        vf_buf_queue_t queues[VF_PIPELINE_MAX_UNITS];
        uint32_t       unit_count;
        int            initialized;
} vf_pipeline_t;

vf_err_t vf_pipeline_init(vf_pipeline_t *pipeline, const char *name);

vf_err_t vf_pipeline_add_unit(vf_pipeline_t *pipeline, vf_unit_t *unit);

vf_err_t vf_pipeline_create(vf_pipeline_t *pipeline);

vf_err_t vf_pipeline_run_once(vf_pipeline_t *pipeline);

void     vf_pipeline_destroy(vf_pipeline_t *pipeline);

#ifdef __cplusplus
}
#endif

#endif /* VF_PIPELINE_MGR_H */

