/**
 **************************************************************************************************
 *  @file           : encoder-unit.c
 *  @brief          : Encoder unit API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Common layer between VF service and encoder library.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include "vf-processing-unit.h"

vf_err_t encoder_unit_init(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t encoder_unit_deinit(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t encoder_unit_get_data(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t encoder_unit_send_data(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t encoder_unit_process_data(void *ctx, ...) {
        return VF_SUCCESS;
}

vf_err_t encoder_unit_init_operations(operations_st_t *ops)
{
        if (NULL == ops) {
                log_error("Invalid input: ops = %p\n", ops);

                return VF_INVALID_PARAMETER;
        }

        ops->init = encoder_unit_init;
        ops->deinit = encoder_unit_deinit;
        ops->get_data = encoder_unit_get_data;
        ops->send_data = encoder_unit_send_data;
        ops->process_data = encoder_unit_process_data;

        log_info("Encoder unit operations initialized!\n");

        return VF_SUCCESS;
}

