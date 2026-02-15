/**
 **************************************************************************************************
 *  @file           : vf-parser.c
 *  @brief          : Parser API implementation
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Parser API for VF service configuration
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include "vf-logger.h"
#include "vf-parser.h"

#define PIPELINES_NUM           1
#define UNITS_NUM               2

#define DISPLAY_UNIT_0_ID       "display_unit_0_recv"

char *camera_unit_0_receiver_ids[1] = {DISPLAY_UNIT_0_ID};
int camera_unit_0_coinds[1];

// const notifier_ctx_st_t camera_unit_0_notifier = {
//         .sender = {
//                 .receiver_ids = camera_unit_0_receiver_ids,
//                 .coids = camera_unit_0_coinds,
//                 .receivers_num = 1,
//         },
//         .mode = NOTIFIER_MODE_SENDER,
// };

// const notifier_ctx_st_t display_unit_0_notifier = {
//         .receiver = {
//                 .id = DISPLAY_UNIT_0_ID,
//         },
//         .mode = NOTIFIER_MODE_RECEIVER,
// };

unit_st_t units_array[UNITS_NUM] = {
        {
                .type = UNIT_TYPE_CAMERA,
                .name = "Camera unit",
                .operations = {0},
                // .notifier = camera_unit_0_notifier,
                .settings = {},
        },
        {
                .type = UNIT_TYPE_DISPLAY,
                .name = "Display unit",
                .operations = {0},
                // .notifier = display_unit_0_notifier,
                .settings = {},
        }
};

pipeline_st_t pipelines_array[PIPELINES_NUM] = {
        {
                .name = "Camera->Display",
                .units_num = UNITS_NUM,
                .units = units_array,
        }
};

pipeline_mgr_ctx_st_t ctx = {
        .pipelines_num = PIPELINES_NUM,
        .pipelines = pipelines_array,
};

static
vf_err_t init_pipeline_mgr_ctx(pipeline_mgr_ctx_st_t **pipeline_mgr_ctx)
{
        if (NULL == pipeline_mgr_ctx) {
                log_error("Invalid input: pipeline_mgr_ctx = %p\n", pipeline_mgr_ctx);

                return VF_INVALID_PARAMETER;
        }

        *pipeline_mgr_ctx = &ctx;

        return VF_SUCCESS;
}

vf_err_t vf_get_pipeline_mgr_ctx(pipeline_mgr_ctx_st_t **pipeline_mgr_ctx)
{
        vf_err_t rc = VF_SUCCESS;

        if (NULL == pipeline_mgr_ctx) {
                log_error("Invalid input: pipeline_mgr_ctx = %p\n", pipeline_mgr_ctx);

                return VF_INVALID_PARAMETER;
        }

        rc = init_pipeline_mgr_ctx(pipeline_mgr_ctx);
        if (VF_SUCCESS != rc) {
                log_error("Failed to initialize pipeline manager context. Error: %s\n",
                        vf_err2str(rc));

                return rc;
        }

        log_info("Pipeline manager context initialized\n");

        return VF_SUCCESS;
}