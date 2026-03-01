/**
 **************************************************************************************************
 *  @file           : vf-file.c
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

#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "vf-file-ctx.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"

#define STR_MODE_READ   "rb"
#define STR_MODE_WRITE  "wb"

#define SAFE_STR(str)   (char *)((str) == NULL ? "NULL" : (str))

static void lib_constructor(void) __attribute__((constructor));
static void lib_destructor(void) __attribute__((destructor));

static
void lib_constructor(void)
{
        int rc = EOK;

        rc = vf_log_init(APP_NAME);
        if (EOK != rc) {
                printf("Failed to initialize logger. Error: %d\n", rc);
        }

        log_debug("Library constructor finished\n");
}

static
void lib_destructor(void)
{
        int rc = EOK;

        rc = vf_log_deinit();
        if (EOK != rc) {
                printf("Failed to deinitialize logger. Error: %d\n", rc);
        }

        log_debug("Library destructor finished\n");
}

static
const char *file_lib_mode2str(file_mode_t file_mode)
{
        switch (file_mode) {
        case FILE_MODE_READ:
                return STR_MODE_READ;
        case FILE_MODE_WRITE:
                return STR_MODE_WRITE;
        default:
                log_error("Failed to convert file mode: %d\n", file_mode);

                return NULL;
        }
}

static
int file_lib_open_file(vf_file_lib_ctx_st_t *ctx)
{
        const char *file_mode = NULL;

        if (NULL == ctx) {
                log_error("Invalid input: ctx = %p\n", ctx);

                return EINVAL;
        }

        log_debug("Trying to open file '%s'\n", SAFE_STR(ctx->config.file_name));

        file_mode = file_lib_mode2str(ctx->config.file_mode);
        if (NULL == file_mode) {
                log_error("Failed to convert file mode\n");

                return EINVAL;
        }

        ctx->file = fopen(ctx->config.file_name, file_mode);
        if (NULL == ctx->file) {
                log_error("Failed to open file '%s'. Error: %s\n", ctx->config.file_name,
                        strerror(errno));

                return errno;
        }

        log_debug("Successfully opened file '%s'\n", ctx->config.file_name);

        return EOK;
}

static
int file_lib_close_file(vf_file_lib_ctx_st_t *ctx)
{
        int rc = EOK;

        if (NULL == ctx) {
                log_error("Invalid input: ctx = %p\n", ctx);

                return EINVAL;
        }

        log_debug("Trying to close file '%s'\n", SAFE_STR(ctx->config.file_name));

        rc = fclose(ctx->file);
        if (EOK != rc) {
                log_error("Failed to close file '%s'. Error: %s\n", SAFE_STR(ctx->config.file_name),
                        strerror(errno));

                return errno;
        }

        log_debug("Successfully closed file '%s'\n", SAFE_STR(ctx->config.file_name));

        return EOK;
}

static
void file_lib_set_ctx_defaults(vf_file_lib_ctx_st_t *ctx)
{
        if (NULL == ctx) {
                log_error("Invalid input: ctx = %p\n", ctx);

                return;
        }

        ctx->frames_count = 0;
}

static
int file_lib_validate_config(vf_file_lib_config_st_t *config)
{
        if (NULL == config) {
                log_error("Invalid input: config = %p\n", config);

                return EINVAL;
        }

        if (0 == config->file_name) {
                log_error("Incorrect configuration: file name is empty\n");

                return EINVAL;
        }

        log_debug("Configuration is valid\n");

        return EOK;
}

int vf_file_lib_init(vf_file_lib_ctx_st_t *ctx)
{
        int rc = EOK;

        if (NULL == ctx) {
                log_error("Invalid input: ctx = %p\n", ctx);

                return EINVAL;
        }

        rc = file_lib_validate_config(&ctx->config);
        if (EOK != rc) {
                log_error("Failed to check configuration. Error: %d\n", rc);

                return rc;
        }

        rc = file_lib_open_file(ctx);
        if (EOK != rc) {
                log_error("Failed to open file. Error: %d\n", rc);

                return rc;
        }

        file_lib_set_ctx_defaults(ctx);

        log_debug("Successfully initialized file library\n");

        return EOK;
}

int vf_file_lib_deinit(vf_file_lib_ctx_st_t *ctx)
{
        int rc = EOK;

        if (NULL == ctx) {
                log_error("Invalid input: ctx = %p\n", ctx);

                return EINVAL;
        }

        rc = file_lib_close_file(ctx);
        if (EOK != rc) {
                log_error("Failed to close file. Error: %d\n", rc);

                return rc;
        }

        log_debug("Successfully deinitialized file library\n");

        return EOK;
}

int vf_file_lib_write_fb_to_file(vf_file_lib_ctx_st_t *ctx, vf_fb_st_t *fb)
{
        int i = 0;
        uint8_t *plane_ptr = NULL;
        size_t plane_size = 0;
        size_t file_size = 0;
        size_t size = 0;

        if (NULL == ctx || NULL == fb) {
                log_error("Invalid input: ctx = %p, fb = %p\n", ctx, fb);

                return EINVAL;
        }

        for (i = 0; i < MAX_PLANE_COUNT; i++) {
                plane_ptr = fb->virt_addr + plane_size;
                plane_size = fb->plane_size[i];
                file_size += fb->plane_size[i];

                if (0 == plane_size) {
                        continue;
                }

                size = fwrite(plane_ptr, 1, plane_size, ctx->file);
                if (size != plane_size) {
                        log_error("Failed to write to file. Write size: %zd, plane size: %d\n", size,
                                plane_size);

                        return EINVAL;
                }
        }

        log_debug("Successfully wrote frame: %d\n", ctx->frames_count);

        /* Do not overwrite the file after N frames */
        if (0 == ctx->config.frames_limit) {
                return EOK;
        }

        ctx->frames_count++;

        if (ctx->frames_count == ctx->config.frames_limit) {
                ctx->frames_count = 0;

                fseek(ctx->file, 0, SEEK_SET);
        }

        return EOK;
}

int vf_file_lib_read_fb_from_file(vf_file_lib_ctx_st_t *ctx, vf_fb_st_t *fb)
{
        int i = 0;
        size_t size = 0;
        uint8_t *plane_ptr = NULL;
        size_t plane_size = 0;
        size_t file_size = 0;

        if (NULL == ctx || NULL == fb) {
                log_error("Invalid input: ctx = %p, fb = %p\n", ctx, fb);

                return EINVAL;
        }

        for (i = 0; i < MAX_PLANE_COUNT; i++) {
                plane_ptr = fb->virt_addr + plane_size;
                plane_size = fb->plane_size[i];
                file_size += fb->plane_size[i];

                if (0 == plane_size) {
                        continue;
                }

                size = fread(plane_ptr, 1, plane_size, ctx->file);
                if (size != plane_size) {
                        log_error("Failed to read from file. Read size: %zd, plane size: %zd\n", size,
                                plane_size);

                        return EINVAL;
                }
        }

        log_debug("Successfully read frame: %d\n", ctx->frames_count);

        /* Read all frames from file */
        if (0 == ctx->config.frames_limit) {
                return EOK;
        }

        ctx->frames_count++;

        if (ctx->frames_count == ctx->config.frames_limit) {
                ctx->frames_count = 0;

                fseek(ctx->file, 0, SEEK_SET);
        }

        return EOK;
}

