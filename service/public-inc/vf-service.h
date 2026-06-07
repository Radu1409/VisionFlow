/**
 **************************************************************************************************
 *  @file           : vf-service.h
 *  @brief          : VisionFlow Service Lifecycle Manager Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Service lifecycle manager — encapsulates camera pipeline resources
 *  (buffer pools, units, pipeline) into a single vf_service_t object
 *  with init, start, stop and deinit operations.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_SERVICE_H
#define VF_SERVICE_H

#include "vf-buff-pool.h"
#include "vf-camera-unit.h"
#include "vf-conversion-unit.h"
#include "vf-error.h"
#include "vf-file-unit.h"
#include "vf-pipeline-mgr.h"
#include "vf-processing-unit.h"
#include "vf-stream-provider-unit.h"

typedef struct {
        const char     *device_path;
        uint32_t        width;
        uint32_t        height;
        vf_pixel_fmt_t  src_fmt;
        vf_pixel_fmt_t  dst_fmt;
        uint32_t        pool_slot_count;
        const char     *output_dir;
        const char     *output_extension;
} vf_service_cfg_t;

typedef struct {
        vf_buf_pool_t             pool_in;
        vf_buf_pool_t             pool_out;
        vf_pipeline_t             pipeline;
        vf_unit_t                 camera_unit;
        vf_unit_t                 sp_unit;
        vf_unit_t                 conv_unit;
        vf_unit_t                 file_out_unit;
        vf_camera_unit_cfg_t      camera_unit_cfg;
        vf_stream_provider_cfg_t  sp_cfg;
        vf_conversion_unit_cfg_t  conv_cfg;
        vf_file_unit_cfg_t        file_out_cfg;
        int                       initialized;
} vf_service_t;

vf_err_t vf_service_init(vf_service_t *svc, const vf_service_cfg_t *cfg);
vf_err_t vf_service_start(vf_service_t *svc);
vf_err_t vf_service_stop(vf_service_t *svc);
void vf_service_deinit(vf_service_t *svc);

#endif /* VF_SERVICE_H */

