/**
 **************************************************************************************************
 *  @file           : encoder-unit.h
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

#ifndef VF_ENCODER_UNIT__H
#define VF_ENCODER_UNIT__H

#include "vf-processing-unit.h"

vf_err_t encoder_unit_init(void *ctx, ...);
vf_err_t encoder_unit_deinit(void *ctx, ...);
vf_err_t encoder_unit_get_data(void *ctx, ...);
vf_err_t encoder_unit_send_data(void *ctx, ...);
vf_err_t encoder_unit_process_data(void *ctx, ...);
vf_err_t encoder_unit_init_operations(operations_st_t *ops);

#endif /* VF_ENCODER_UNIT__H */

