#ifndef VF_ERROR_H
#define VF_ERROR_H

typedef enum {
    VF_ERROR_MIN                   = 0,

    VF_SUCCESS                     = 0,
    VF_INVALID_PARAMETER           = 1,
    VF_OOM                         = 1,
    VF_SYNC_ERROR                  = 1,
    VF_INIT_FAILED                 = 2,
    VF_FILE_ERR                    = 3,

    VF_CAMERA_INIT_ERR             = 4,
    VF_CAMERA_START_ERR            = 5,
    VF_CAMERA_STOP_ERR             = 6,
    VF_CAMERA_CAPTURE_FRAME_FAILED = 7,
    VF_CAMERA_DEINIT_ERR           = 8,

    VF_CONV_INIT_ERR               = 9,
    VF_CONV_PROCESSING_ERR         = 10,

    VF_ERROR_MAX
} vf_err_t;

char *vf_err2str(vf_err_t err_code);

#endif /* __VF_ERROR_H__ */

