#include "vf-error.h"

#define VF_STR_SUCCESS                     "Operation completed successfully"
#define VF_STR_INVALID_PARAM               "Provided parameter is invalid"
#define VF_STR_INIT_FAILED                 "Failed to initialize component"
#define VF_STR_FILE_ERR                    "Error encountered while handling file"
#define VF_STR_UNDEFINED_ERROR             "An unknown error has occured"

#define VF_STR_CAMERA_INIT_ERR             "Camera could not be started"
#define VF_STR_CAMERA_CAPTURE_FRAME_FAILED "Camera could not received frame"
#define VF_STR_CAMERA_DEINIT_ERR           "Error during camera de-initialization"

char *vf_err2str(vf_err_t err_code)
{
    switch (err_code) {
        case VF_SUCCESS:
            return VF_STR_SUCCESS;
        case VF_INVALID_PARAMETER:
            return VF_STR_INVALID_PARAM;
        case VF_INIT_FAILED:
            return VF_STR_INIT_FAILED;
        case VF_FILE_ERR:
            return VF_STR_FILE_ERR;
        case VF_CAMERA_INIT_ERR:
            return VF_STR_CAMERA_INIT_ERR;
        case VF_CAMERA_CAPTURE_FRAME_FAILED:
            return VF_STR_CAMERA_CAPTURE_FRAME_FAILED;
        case VF_CAMERA_DEINIT_ERR:
            return VF_STR_CAMERA_DEINIT_ERR;
        default:
            return VF_STR_UNDEFINED_ERROR;
    }

    return VF_STR_UNDEFINED_ERROR;
}

