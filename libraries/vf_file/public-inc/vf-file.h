/**
 **************************************************************************************************
 *  @file           : vf-file.h
 *  @brief          : VisionFlow File I/O API Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  VisionFlow thin wrapper over standard file I/O operations with consistent
 *  vf_err_t error handling
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_FILE_H
#define VF_FILE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vf-error.h"

#define VF_FILE_PATH_MAX_LEN  256U

typedef struct {
        FILE *fp;
        char  path[VF_FILE_PATH_MAX_LEN];
        int   is_open;
} vf_file_t;

vf_err_t vf_file_open(vf_file_t *file, const char *path, const char *mode);
vf_err_t vf_file_close(vf_file_t *file);

vf_err_t vf_file_read(vf_file_t *file, void *buf, size_t size, size_t *bytes_read);
vf_err_t vf_file_write(vf_file_t *file, const void *buf, size_t size, size_t *bytes_written);

vf_err_t vf_file_size(vf_file_t *file, size_t *out_size);
vf_err_t vf_file_exists(const char *path, int *out_exists);
vf_err_t vf_file_seek(vf_file_t *file, long offset, int whence);
vf_err_t vf_file_tell(vf_file_t *file, long *out_pos);

#endif /* VF_FILE_H */

