/**
 **************************************************************************************************
 *  @file           : vf-file-ctx.h
 *  @brief          : Vision Flow file API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *      Vision Flow file API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#ifndef VF_FILE__CTX__H
#define VF_FILE__CTX__H

#include <stdio.h>

#define FILE_PATH_LENGTH        512

typedef enum {
        FILE_MODE_READ  = 0,
        FILE_MODE_WRITE = 1,
} file_mode_t;

typedef struct vf_file_lib_config_st {
        int         frames_limit;
        char        file_name[FILE_PATH_LENGTH];
        file_mode_t file_mode;
} vf_file_lib_config_st_t;

typedef struct vf_file_lib_ctx_st {
        vf_file_lib_config_st_t config;
        FILE                     *file;
        int                      frames_count;
} vf_file_lib_ctx_st_t;

#endif /* VF_FILE__CTX__H */