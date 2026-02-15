/**
 **************************************************************************************************
 *  @file           : vf-pipeline-mgr.c
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

#include "vf-generic.h"
#include "vf-logger.h"
#include "vf-pipeline-mgr.h"

static
vf_err_t create_pipeline(pipeline_st_t *pipe)
{
        vf_err_t rc = VF_SUCCESS;
        int i = 0;

        if (NULL == pipe) {
                log_error("Invalid input: pipe = %p\n", pipe);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == pipe->units) {
                log_error("Invalid input: units = %p\n", pipe->units);

                return VF_INVALID_PARAMETER;
        }

        log_info("Creating pipeline (\"%s\")\n", SAFE_STR(pipe->name));
        log_info("Found (%d) units in pipeline\n", pipe->units_num);

        for (i = 0; i < pipe->units_num; i++) {
                rc = vf_create_unit(&pipe->units[i]);
                if (VF_SUCCESS != rc) {
                        log_error("Failed to create unit. Error: %s\n", vf_err2str(rc));

                        return rc;
                }
        }

        log_info("Pipeline (\"%s\") initialization finished successfully\n", SAFE_STR(pipe->name));

        return rc;
}

static
void destroy_pipeline(pipeline_st_t *pipe)
{
        int i = 0;

        if (NULL == pipe) {
                log_error("Invalid input: pipe = %p\n", pipe);

                return;
        }

        if (NULL == pipe->units) {
                log_error("Invalid input: units = %p\n", pipe->units);

                return;
        }

        log_info("Pipeline (\"%s\") de-initialization started\n", SAFE_STR(pipe->name));

        for (i = 0; i < pipe->units_num; i++) {
                vf_destroy_unit(&pipe->units[i]);
        }

        log_info("Pipeline (\"%s\") de-initialization finished\n", SAFE_STR(pipe->name));
}

vf_err_t vf_create_pipelines(pipeline_mgr_ctx_st_t *ctx)
{
        vf_err_t rc = VF_SUCCESS;
        int i = 0;

        if (NULL == ctx) {
                log_error("Invalid input: ctx = %p\n", ctx);

                return VF_INVALID_PARAMETER;
        }

        if (NULL == ctx->pipelines) {
                log_error("Invalid input: pipelines = %p\n", ctx->pipelines);

                return VF_INVALID_PARAMETER;
        }

        log_info("Found (%d) pipelines\n", ctx->pipelines_num);

        for (i = 0; i < ctx->pipelines_num; i++) {
                rc = create_pipeline(&ctx->pipelines[i]);
                if (VF_SUCCESS != rc) {
                        log_error("Failed to create pipeline. Error: %s\n", vf_err2str(rc));

                        return rc;
                }
        }

        log_info("Pipelines initialization finished successfully");

        return rc;
}

void vf_destroy_pipelines(pipeline_mgr_ctx_st_t *ctx)
{
        int i = 0;

        if (NULL == ctx) {
                log_error("Invalid input: ctx = %p\n", ctx);

                return;
        }

        if (NULL == ctx->pipelines) {
                log_error("Invalid input: pipelines = %p\n", ctx->pipelines);

                return;
        }

        log_info("Pipelines de-initialization started\n");

        for (i = 0; i < ctx->pipelines_num; i++) {
                destroy_pipeline(&ctx->pipelines[i]);
        }

        log_info("Pipelines de-initialization finished\n");
}