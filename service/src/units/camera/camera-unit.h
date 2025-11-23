/**
 **************************************************************************************************
 *  @file           : camera-unit.h
 *  @brief          : Camera unit API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Common layer between VF service and camera library.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_CAMERA_UNIT__H
#define VF_CAMERA_UNIT__H

#include "vf-error.h"

vf_err_t camera_unit_init(void *ctx, ...);
vf_err_t camera_unit_deinit(void *ctx, ...);
vf_err_t camera_unit_get_data(void *ctx, ...);
vf_err_t camera_unit_send_data(void *ctx, ...);
vf_err_t camera_unit_process_data(void *ctx, ...);

#endif /* VF_CAMERA_UNIT__H */

