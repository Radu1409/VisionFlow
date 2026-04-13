/**
 **************************************************************************************************
 *  @file           : vf-parser.h
 *  @brief          : VisionFlow parser API Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  VisionFlow parser used to load frame configuration from JSON files
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_PARSER__H
#define VF_PARSER__H

#include <stdint.h>

#include "vf-error.h"

#define VF_PARSER_MAX_NAME_LEN        64
#define VF_PARSER_MAX_BASE_PATH_LEN   256
#define VF_PARSER_MAX_FILE_NAME_LEN   128
#define VF_PARSER_MAX_FULL_PATH_LEN   (VF_PARSER_MAX_BASE_PATH_LEN + VF_PARSER_MAX_FILE_NAME_LEN + 2)

typedef enum {
        VF_PIXEL_FORMAT_INVALID = 0,
        VF_PIXEL_FORMAT_RAW8,
        VF_PIXEL_FORMAT_RGB888,
        VF_PIXEL_FORMAT_YUV420
} vf_pixel_format_t;

typedef struct {
        char     file_name[VF_PARSER_MAX_FILE_NAME_LEN];
        char     full_path[VF_PARSER_MAX_FULL_PATH_LEN];
        uint32_t width;
        uint32_t height;
} vf_frame_info_t;

typedef struct {
        char              name[VF_PARSER_MAX_NAME_LEN];
        char              base_path[VF_PARSER_MAX_BASE_PATH_LEN];
        vf_pixel_format_t format;
        vf_frame_info_t  *frames;
        uint32_t          frame_count;
} vf_frame_set_t;

typedef struct {
        vf_frame_set_t *frame_sets;
        uint32_t        frame_set_count;
} vf_frames_cfg_t;

vf_err_t vf_parser_load_frames_cfg(const char *cfg_path, vf_frames_cfg_t **frames_cfg);
void vf_parser_free_frames_cfg(vf_frames_cfg_t *frames_cfg);

const char *vf_parser_pixel_format_to_str(vf_pixel_format_t format);

#endif /* VF_PARSER__H */

