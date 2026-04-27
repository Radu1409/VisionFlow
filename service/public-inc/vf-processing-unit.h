/**
 **************************************************************************************************
 *  @file           : vf-processing-unit.h
 *  @brief          : VisionFlow Processing Unit Interface Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Defines the common interface for all VisionFlow pipeline processing units.
 *  Each unit exposes its operations via vf_unit_init_operations().
 *  The pipeline manager creates and destroys units through this interface.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_PROCESSING_UNIT_H
#define VF_PROCESSING_UNIT_H

#include <stdint.h>

#include "vf-buff-queue.h"
#include "vf-error.h"
#include "vf-notifier.h"
#include "vf-unit-operations.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        VF_UNIT_TYPE_UNKNOWN    = 0,
        VF_UNIT_TYPE_FILE_IN    = 1,
        VF_UNIT_TYPE_FILE_OUT   = 2,
        VF_UNIT_TYPE_CONVERSION = 3,
        VF_UNIT_TYPE_CAMERA     = 4,
        VF_UNIT_TYPE_ENCODER    = 5,
        VF_UNIT_TYPE_DISPLAY    = 6,
        VF_UNIT_TYPE_MAX
} vf_unit_type_t;

typedef struct {
        uint64_t total_get_data_ms;
        uint64_t total_process_data_ms;
        uint64_t total_send_data_ms;
        uint64_t total_ms;
        uint32_t frames_processed;
} vf_unit_stats_t;

typedef struct vf_unit {
        vf_unit_type_t       type;
        const char          *name;

        vf_unit_operations_t operations;

        vf_buf_queue_t      *in_queue;
        vf_buf_queue_t      *out_queue;

        void                *internal_data;

        int                  initialized;

        vf_notifier_t        notifier;

        vf_unit_stats_t      stats;
} vf_unit_t;

vf_err_t vf_unit_create(vf_unit_t *unit);
void     vf_unit_destroy(vf_unit_t *unit);

vf_err_t vf_unit_run(vf_unit_t *unit);

vf_err_t vf_unit_connect_input(vf_unit_t *unit, vf_buf_queue_t *queue);
vf_err_t vf_unit_connect_output(vf_unit_t *unit, vf_buf_queue_t *queue);

const char *vf_unit_type_str(vf_unit_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* VF_PROCESSING_UNIT_H */

