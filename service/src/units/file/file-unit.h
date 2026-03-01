/**
 **************************************************************************************************
 *  @file           : file-unit.h
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

#ifndef VF_FILE_UNIT__H
#define VF_FILE_UNIT__H

#include "vf-processing-unit.h"

vf_err_t file_unit_init(void *ctx, ...);
vf_err_t file_unit_deinit(void *ctx, ...);
vf_err_t file_unit_get_data(void *ctx, ...);
vf_err_t file_unit_send_data(void *ctx, ...);
vf_err_t file_unit_process_data(void *ctx, ...);
vf_err_t file_unit_init_operations(operations_st_t *ops);

#endif /* VF_FILE_UNIT__H */

