/**
 **************************************************************************************************
 *  @file           : vf-parser.c
 *  @brief          : Parser API implementation
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Parser API for VF service configuration
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_PARSER__H
#define VF_PARSER__H

#include "vf-pipeline-mgr.h"

vf_err_t vf_get_pipeline_mgr_ctx(pipeline_mgr_ctx_st_t **pipeline_mgr_ctx);

#endif /* VF_PARSER__H */

