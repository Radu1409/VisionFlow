/**
 **************************************************************************************************
 *  @file           : file-unit.c
 *  @brief          : File unit API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *      Common layer between VF service and file library.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include "vf-logger.h"
#include "vf-processing-unit.h"

vf_err_t file_unit_init(void *ctx, ...)
{
        return VF_SUCCESS;
}

vf_err_t file_unit_deinit(void *ctx, ...)
{
        return VF_SUCCESS;
}

vf_err_t file_unit_get_data(void *ctx, ...)
{
        return VF_SUCCESS;
}

vf_err_t file_unit_send_data(void *ctx, ...)
{
        return VF_SUCCESS;
}

vf_err_t file_unit_process_data(void *ctx, ...)
{
        return VF_SUCCESS;
}

vf_err_t file_unit_init_operations(operations_st_t *ops)
{
        if (NULL == ops) {
                log_err("Invalid input: ops = %p\n", ops);

                return VF_INVALID_PARAMETER;
        }

        ops->init = file_unit_init;
        ops->deinit = file_unit_deinit;
        ops->get_data = file_unit_get_data;
        ops->send_data = file_unit_send_data;
        ops->process_data = file_unit_process_data;

        log_info("File unit operations initialized\n");

        return VF_SUCCESS;
}