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

#ifdef __cplusplus
extern "C" {
#endif

vf_err_t vf_core_run_all(void);
vf_err_t vf_core_run_by_flag(const char *flag);

#ifdef __cplusplus
}
#endif

#endif /* VF_CORE_H */

