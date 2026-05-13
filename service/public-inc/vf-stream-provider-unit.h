/**
 **************************************************************************************************
 *  @file           : vf-stream-provider-unit.h
 *  @brief          : VisionFlow Stream Provider Unit API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Stream provider unit — receives frames from camera_unit and distributes
 *  them to downstream consumers using reference counting.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_STREAM_PROVIDER_UNIT_H
#define VF_STREAM_PROVIDER_UNIT_H

#include "vf-buff-pool.h"
#include "vf-error.h"
#include "vf-processing-unit.h"
#include "vf-unit-operations.h"

#define VF_STREAM_PROVIDER_MAX_CONSUMERS 4U

typedef struct {
        uint32_t       consumer_count;
        vf_buf_pool_t *src_pool;
} vf_stream_provider_cfg_t;

vf_err_t vf_stream_provider_unit_init(void *ctx, ...);
vf_err_t vf_stream_provider_unit_deinit(void *ctx, ...);
vf_err_t vf_stream_provider_unit_get_data(void *ctx, ...);
vf_err_t vf_stream_provider_unit_process_data(void *ctx, ...);
vf_err_t vf_stream_provider_unit_send_data(void *ctx, ...);
vf_err_t vf_stream_provider_unit_init_operations(vf_unit_operations_t *ops);

#endif /* VF_STREAM_PROVIDER_UNIT_H */

