/**
 **************************************************************************************************
 *  @file           : conversion-unit.h
 *  @brief          : Conversion unit API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Common layer between VF service and conversion library.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include "conversion-unit.h"
#include "vf-logger.h"

vf_err_t conversion_unit_init(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t conversion_unit_deinit(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t conversion_unit_get_data(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t conversion_unit_send_data(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t conversion_unit_process_data(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t conversion_unit_init_operations(operations_st_t *ops)
{
        if (NULL == ops) {
                log_error("Invalid input: ops = %p\n", ops);

                return VF_INVALID_PARAMETER;
        }

        ops->init = conversion_unit_init;
        ops->deinit = conversion_unit_deinit;
        ops->get_data = conversion_unit_get_data;
        ops->send_data = conversion_unit_send_data;
        ops->process_data = conversion_unit_process_data;

        log_info("Conversion unit operations initialized!\n");

        return VF_SUCCESS;
}

