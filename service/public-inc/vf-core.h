/**
 **************************************************************************************************
 *  @file           : vf-core.h
 *  @brief          : VisionFlow Pipeline Core Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Entry point for the VisionFlow pipeline execution.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CORE_H
#define VF_CORE_H

#include "vf-error.h"

vf_err_t vf_core_run_all(void);
vf_err_t vf_core_run_by_flag(const char *flag);
vf_err_t vf_core_run_camera(void);

#endif /* VF_CORE_H */

