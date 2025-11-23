/**
 **************************************************************************************************
 *  @file           : display-unit.h
 *  @brief          : Display unit API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Common layer between VF service and display library.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_DISPLAY_UNIT__H
#define VF_DISPLAY_UNIT__H

#include "vf-error.h"

vf_err_t display_unit_init(void *ctx, ...);
vf_err_t display_unit_deinit(void *ctx, ...);
vf_err_t display_unit_get_data(void *ctx, ...);
vf_err_t display_unit_send_data(void *ctx, ...);
vf_err_t display_unit_process_data(void *ctx, ...);

#endif /* VF_DISPLAY_UNIT__H */

