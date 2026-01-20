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

#ifndef VF_CONVERSION_UNIT__H
#define VF_CONVERSION_UNIT__H

#include "vf-processing-unit.h"

vf_err_t conversion_unit_init(void *ctx, ...);
vf_err_t conversion_unit_deinit(void *ctx, ...);
vf_err_t conversion_unit_get_data(void *ctx, ...);
vf_err_t conversion_unit_send_data(void *ctx, ...);
vf_err_t conversion_unit_process_data(void *ctx, ...);
vf_err_t conversion_unit_init_operations(operations_st_t *ops);

#endif /* VF_CONVERSION_UNIT__H */

