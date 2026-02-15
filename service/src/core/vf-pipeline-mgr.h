/**
 **************************************************************************************************
 *  @file           : vf-pipeline-mgr.h
 *  @brief          : VF pipelines routine
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Pipelines operating API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_PIPELINE_MGR__H
#define VF_PIPELINE_MGR__H

#include "vf-processing-unit.h"

typedef struct pipeline {
        const char  *name;
        size_t      units_num;
        struct unit *units;
} pipeline_st_t;

typedef struct pipeline_mgr_ctx {
        size_t          pipelines_num;
        struct pipeline *pipelines;
} pipeline_mgr_ctx_st_t;

vf_err_t vf_create_pipelines(pipeline_mgr_ctx_st_t *ctx);
void vf_destroy_pipelines(pipeline_mgr_ctx_st_t *ctx);

#endif /* VF_PIPELINE_MGR__H */

