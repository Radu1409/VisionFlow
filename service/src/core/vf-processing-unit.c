/**
 **************************************************************************************************
 *  @file           : vf-processing-unit.c
 *  @brief          : VisionFlow Processing Unit Interface
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Implementation of the common processing unit lifecycle API.
 *  Dispatches calls to unit-specific operations set via init_operations pattern.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <string.h>

#include "vf-error.h"
#include "vf-logger.h"
#include "vf-processing-unit.h"

#define MODULE_NAME "vf_processing_unit"

vf_err_t vf_unit_create(vf_unit_t *unit)
{
        vf_err_t err = VF_SUCCESS;

        if (NULL == unit) {
                log_err("Invalid input: unit = %p", (void *)unit);

                return VF_INVALID_PARAMETER;
        }

        log_info("Creating unit '%s' type='%s'",
                 unit->name ? unit->name : "unknown",
                 vf_unit_type_str(unit->type));

        err = vf_notifier_init(&unit->notifier, 
                               unit->name ? unit->name : "unknown",
                               VF_NOTIFIER_MODE_BIDIRECTIONAL);
        if (VF_SUCCESS != err) {
                log_err("Failed to init notifier for unit '%s'",
                        unit->name ? unit->name : "unknown");

                return err;
        }

        if (NULL == unit->operations.init) {
                log_err("Unit '%s' has no init operation — call init_operations first",
                        unit->name ? unit->name : "unknown");

                return VF_INIT_FAILED;
        }

        err = unit->operations.init(unit);
        if (VF_SUCCESS != err) {
                log_err("Unit '%s' init failed: %s",
                        unit->name ? unit->name : "unknown",
                        vf_err2str(err));
                
                (void)vf_notifier_deinit(&unit->notifier);

                return err;
        }

        unit->initialized = 1;

        log_info("Unit '%s' created successfully", unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

void vf_unit_destroy(vf_unit_t *unit)
{
        if (NULL == unit) {
                log_wrn("vf_unit_destroy called with NULL unit");

                return;
        }

        if (0 == unit->initialized) {
                return;
        }

        log_info("Destroying unit '%s'", unit->name ? unit->name : "unknown");

        if (NULL != unit->operations.deinit) {
                unit->operations.deinit(unit);
        }

        unit->initialized   = 0;
        unit->in_queue      = NULL;
        unit->out_queue     = NULL;
        unit->internal_data = NULL;
        
        (void)vf_notifier_deinit(&unit->notifier);

        log_info("Unit '%s' destroyed", unit->name ? unit->name : "unknown");
}

vf_err_t vf_unit_run(vf_unit_t *unit)
{
        vf_err_t err = VF_SUCCESS;

        if (NULL == unit) {
                log_err("Invalid input: unit = %p", (void *)unit);

                return VF_INVALID_PARAMETER;
        }

        if (0 == unit->initialized) {
                log_err("Unit '%s' not initialized", unit->name ? unit->name : "unknown");

                return VF_INIT_FAILED;
        }

        /* get_data → process_data → send_data */

        if (NULL != unit->operations.get_data) {
                err = unit->operations.get_data(unit);
                if (VF_SUCCESS != err) {
                        log_err("Unit '%s' get_data failed: %s",
                                unit->name ? unit->name : "unknown",
                                vf_err2str(err));

                        return err;
                }
        }

        if (NULL != unit->operations.process_data) {
                err = unit->operations.process_data(unit);
                if (VF_SUCCESS != err) {
                        log_err("Unit '%s' process_data failed: %s",
                                unit->name ? unit->name : "unknown",
                                vf_err2str(err));

                        return err;
                }
        }

        if (NULL != unit->operations.send_data) {
                err = unit->operations.send_data(unit);
                if (VF_SUCCESS != err) {
                        log_err("Unit '%s' send_data failed: %s",
                                unit->name ? unit->name : "unknown",
                                vf_err2str(err));

                        return err;
                }
        }

        return VF_SUCCESS;
}

vf_err_t vf_unit_connect_input(vf_unit_t *unit, vf_buf_queue_t *queue)
{
        if ((NULL == unit) || (NULL == queue)) {
                log_err("Invalid params: unit=%p queue=%p",
                        (void *)unit, (void *)queue);

                return VF_INVALID_PARAMETER;
        }

        unit->in_queue = queue;

        log_dbg("Unit '%s' input connected", unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

vf_err_t vf_unit_connect_output(vf_unit_t *unit, vf_buf_queue_t *queue)
{
        if ((NULL == unit) || (NULL == queue)) {
                log_err("Invalid params: unit=%p queue=%p",
                        (void *)unit, (void *)queue);

                return VF_INVALID_PARAMETER;
        }

        unit->out_queue = queue;

        log_dbg("Unit '%s' output connected", unit->name ? unit->name : "unknown");

        return VF_SUCCESS;
}

const char *vf_unit_type_str(vf_unit_type_t type)
{
        switch (type) {
                case VF_UNIT_TYPE_FILE_IN:
                        return "FILE_IN";
                case VF_UNIT_TYPE_FILE_OUT:
                        return "FILE_OUT";
                case VF_UNIT_TYPE_CONVERSION:
                        return "CONVERSION";
                case VF_UNIT_TYPE_CAMERA:
                        return "CAMERA";
                case VF_UNIT_TYPE_ENCODER:
                        return "ENCODER";
                case VF_UNIT_TYPE_DISPLAY:
                        return "DISPLAY";
                case VF_UNIT_TYPE_UNKNOWN:
                case VF_UNIT_TYPE_MAX:
                default:
                        return "UNKNOWN";
        }
}

