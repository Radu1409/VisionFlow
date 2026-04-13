/**
 **************************************************************************************************
 *  @file           : vf-parser.c
 *  @brief          : VisionFlow parser API
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf-parser.h"
#include "vf-logger.h"
#include "vf-error.h"

#define MAX_FILE_SIZE 65536

static
vf_pixel_format_t vf_parser_str_to_format(const char *str)
{
        if (0 == strcmp(str, "RAW8")) {
                return VF_PIXEL_FORMAT_RAW8;
        }

        if (0 == strcmp(str, "RGB888")) {
                return VF_PIXEL_FORMAT_RGB888;
        }

        if (0 == strcmp(str, "YUV420")) {
                return VF_PIXEL_FORMAT_YUV420;
        }

        return VF_PIXEL_FORMAT_INVALID;
}

static
char *vf_parser_read_file(const char *path)
{
        FILE *f = NULL;
        char *buffer = NULL;
        size_t read_size = 0U;

        f = fopen(path, "r");
        if (NULL == f) {
                log_err("Failed to open config file: %s", path);

                return NULL;
        }

        buffer = malloc(MAX_FILE_SIZE);
        if (NULL == buffer) {
                log_err("Out of memory while reading config file");

                fclose(f);

                return NULL;
        }

        read_size = fread(buffer, 1U, MAX_FILE_SIZE - 1U, f);
        buffer[read_size] = '\0';

        fclose(f);

        return buffer;
}

static
int vf_parser_extract_string(char *src, char *out, size_t max_len)
{
        char *colon = NULL;
        char *start = NULL;
        char *end = NULL;
        size_t len = 0U;

        if (NULL == src || NULL == out || 0U == max_len) {
                return -1;
        }

        colon = strchr(src, ':');
        if (NULL == colon) {
                return -1;
        }

        start = colon + 1;

        while (' ' == *start || '\t' == *start || '\n' == *start || '\r' == *start) {
                start++;
        }

        if ('\"' != *start) {
                return -1;
        }

        start++;

        end = strchr(start, '\"');
        if (NULL == end) {
                return -1;
        }

        len = (size_t)(end - start);
        if (len >= max_len) {
                return -1;
        }

        (void)strncpy(out, start, len);
        out[len] = '\0';

        return 0;
}

static
int vf_parser_extract_int(char *src, int *value)
{
    char *colon = NULL;

    if (NULL == src || NULL == value) {
        return -1;
    }

    colon = strchr(src, ':');
    if (NULL == colon) {
        return -1;
    }

    *value = atoi(colon + 1);

    return 0;
}

vf_err_t vf_parser_load_frames_cfg(const char *cfg_path, vf_frames_cfg_t **out_cfg)
{
        char *file_data = NULL;
        char *cursor = NULL;
        vf_frames_cfg_t *cfg = NULL;
        int set_count = 0;
        int i = 0;

        if (NULL == cfg_path || NULL == out_cfg) {
                log_err("Invalid parameters: cfg_path = %p, out_cfg = %p",
                        (void *)cfg_path, (void *)out_cfg);

                return VF_INVALID_PARAMETER;
        }

        *out_cfg = NULL;

        file_data = vf_parser_read_file(cfg_path);
        if (NULL == file_data) {
                return VF_FILE_ERR;
        }

        cfg = calloc(1U, sizeof(*cfg));
        if (NULL == cfg) {
                free(file_data);

                return VF_OOM;
        }

        cursor = file_data;

        while (NULL != (cursor = strstr(cursor, "\"name\""))) {
                set_count++;
                cursor++;
        }

        if (0 == set_count) {
                log_err("No frame sets found in config");

                free(file_data);
                free(cfg);

                return VF_FILE_ERR;
        }

        cfg->frame_sets = calloc((size_t)set_count, sizeof(vf_frame_set_t));
        if (NULL == cfg->frame_sets) {
                free(file_data);
                free(cfg);

                return VF_OOM;
        }

        cfg->frame_set_count = (uint32_t)set_count;
        cursor = file_data;

        for (i = 0; i < set_count; i++) {
                vf_frame_set_t *set = &cfg->frame_sets[i];
                char fmt_str[32] = {0};
                char *frames_start = NULL;
                char *frames_end = NULL;
                char *tmp = NULL;
                int frame_count = 0;
                int j = 0;

                cursor = strstr(cursor, "\"name\"");
                if (NULL == cursor || 0 != vf_parser_extract_string(cursor, set->name, sizeof(set->name))) {
                        log_err("Failed to parse frame set name");

                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_FILE_ERR;
                }

                cursor = strstr(cursor, "\"base_path\"");
                if (NULL == cursor ||
                    0 != vf_parser_extract_string(cursor, set->base_path, sizeof(set->base_path))) {
                        log_err("Failed to parse base_path for frame set: %s", set->name);

                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_FILE_ERR;
                }

                cursor = strstr(cursor, "\"format\"");
                if (NULL == cursor || 0 != vf_parser_extract_string(cursor, fmt_str, sizeof(fmt_str))) {
                        log_err("Failed to parse format for frame set: %s", set->name);

                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_FILE_ERR;
                }

                set->format = vf_parser_str_to_format(fmt_str);
                if (VF_PIXEL_FORMAT_INVALID == set->format) {
                        log_err("Invalid pixel format: %s", fmt_str);

                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_FILE_ERR;
                }

                frames_start = strstr(cursor, "\"frames\"");
                if (NULL == frames_start) {
                        log_err("Missing frames array for frame set: %s", set->name);

                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_FILE_ERR;
                }

                frames_end = strchr(frames_start, ']');
                if (NULL == frames_end) {
                        log_err("Invalid frames array for frame set: %s", set->name);

                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_FILE_ERR;
                }

                tmp = frames_start;
                while (NULL != (tmp = strstr(tmp, "\"file\"")) && tmp < frames_end) {
                        frame_count++;
                        tmp++;
                }

                if (0 == frame_count) {
                        log_err("No frames found for frame set: %s", set->name);

                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_FILE_ERR;
                }

                set->frames = calloc((size_t)frame_count, sizeof(vf_frame_info_t));
                if (NULL == set->frames) {
                        vf_parser_free_frames_cfg(cfg);
                        free(file_data);

                        return VF_OOM;
                }

                set->frame_count = (uint32_t)frame_count;

                for (j = 0; j < frame_count; j++) {
                        vf_frame_info_t *frame = &set->frames[j];
                        int width = 0;
                        int height = 0;

                        cursor = strstr(cursor, "\"file\"");
                        if (NULL == cursor ||
                            0 != vf_parser_extract_string(cursor, frame->file_name, sizeof(frame->file_name))) {
                                log_err("Failed to parse file name for frame index %d in set %s", j, set->name);

                                vf_parser_free_frames_cfg(cfg);
                                free(file_data);

                                return VF_FILE_ERR;
                        }

                        cursor = strstr(cursor, "\"width\"");
                        if (NULL == cursor || 0 != vf_parser_extract_int(cursor, &width) || width <= 0) {
                                log_err("Invalid width for frame %s", frame->file_name);

                                vf_parser_free_frames_cfg(cfg);
                                free(file_data);

                                return VF_FILE_ERR;
                        }

                        cursor = strstr(cursor, "\"height\"");
                        if (NULL == cursor || 0 != vf_parser_extract_int(cursor, &height) || height <= 0) {
                                log_err("Invalid height for frame %s", frame->file_name);

                                vf_parser_free_frames_cfg(cfg);
                                free(file_data);

                                return VF_FILE_ERR;
                        }

                        frame->width = (uint32_t)width;
                        frame->height = (uint32_t)height;

                        (void)snprintf(frame->full_path,
                                    sizeof(frame->full_path),
                                    "%s/%s",
                                    set->base_path,
                                    frame->file_name);
                }

                log_info("Parsed frame set '%s' with %u frames", set->name, set->frame_count);
        }

        free(file_data);

        *out_cfg = cfg;

        log_info("Config parsed successfully: %u frame sets", cfg->frame_set_count);

        return VF_SUCCESS;
}

void vf_parser_free_frames_cfg(vf_frames_cfg_t *cfg)
{
    uint32_t i = 0U;

    if (NULL == cfg) {
            return;
    }

    for (i = 0U; i < cfg->frame_set_count; i++) {
            free(cfg->frame_sets[i].frames);
    }

        free(cfg->frame_sets);
        free(cfg);
}