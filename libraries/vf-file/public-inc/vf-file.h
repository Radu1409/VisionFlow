/**
 **************************************************************************************************
 *  @file           : vf-file.h
 *  @brief          : Vision flow file API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Vision flow file API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_FILE__H
#define VF_FILE__H

#include "vf-file-ctx.h"
#include "vf-framebuffer.h"

int vf_file_lib_init(vf_file_lib_ctx_st_t *ctx);
int vf_file_lib_deinit(vf_file_lib_ctx_st_t *ctx);

int vf_file_lib_write_fb_to_file(vf_file_lib_ctx_st_t *ctx, vf_fb_st_t *fb);
int vf_file_lib_read_fb_from_file(vf_file_lib_ctx_st_t *ctx, vf_fb_st_t *fb);

#endif /* VF_FILE__H */

