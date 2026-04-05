/**
 **************************************************************************************************
 *  @file           : vf-error.h
 *  @brief          : Vision flow error API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Vision flow error API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/


#ifndef VF_ERROR_H
#define VF_ERROR_H

typedef enum {
    VF_ERROR_MIN                   = 0,

    VF_SUCCESS                     = 0,
    VF_INVALID_PARAMETER           = 1,
    VF_OOM                         = 2,
    VF_SYNC_ERROR                  = 3,
    VF_INIT_FAILED                 = 4,
    VF_FILE_ERR                    = 5,

    VF_CAMERA_INIT_ERR             = 6,
    VF_CAMERA_START_ERR            = 7,
    VF_CAMERA_STOP_ERR             = 8,
    VF_CAMERA_CAPTURE_FRAME_FAILED = 9,
    VF_CAMERA_DEINIT_ERR           = 10,

    VF_CONV_INIT_ERR               = 11,
    VF_CONV_PROCESSING_ERR         = 12,

    VF_ERROR_MAX
} vf_err_t;

const char *vf_err2str(vf_err_t err_code);

#endif /* __VF_ERROR_H__ */

