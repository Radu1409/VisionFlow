/**
 **************************************************************************************************
 *  @file           : vf-file.c
 *  @brief          : VisionFlow File I/O API
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

#include <errno.h>
#include <string.h>

#include "vf-error.h"
#include "vf-file.h"
#include "vf-logger.h"

#define MODULE_NAME "vf_file"

vf_err_t vf_file_open(vf_file_t *file, const char *path, const char *mode)
{
        if (NULL == file || NULL == path || NULL == mode) {
                log_err("Invalid params: file=%p path=%p mode=%p",
                        (void *)file, (void *)path, (void *)mode);

                return VF_INVALID_PARAMETER;
        }

        if (1 == file->is_open) {
                log_wrn("File already open: %s", file->path);

                return VF_FILE_ERR;
        }

        file->fp = fopen(path, mode);
        if (NULL == file->fp) {
                log_err("Failed to open file: %s (mode=%s)", path, mode);

                return VF_FILE_ERR;
        }

        (void)snprintf(file->path, sizeof(file->path), "%s", path);

        file->is_open = 1;

        log_dbg("Opened file: %s (mode=%s)", file->path, mode);

        return VF_SUCCESS;
}

vf_err_t vf_file_close(vf_file_t *file)
{
        int rc = 0;

        if (NULL == file) {
                log_err("Invalid params: file=%p", (void *)file);

                return VF_INVALID_PARAMETER;
        }

        if (0 == file->is_open) {
                log_wrn("File already closed");

                return VF_SUCCESS;
        }

        rc = fclose(file->fp);
        if (EOK != rc) {
                log_err("fclose failed for: %s", file->path);

                return VF_FILE_ERR;
        }

        log_dbg("Closed file: %s", file->path);

        file->fp = NULL;
        file->is_open = 0;
        file->path[0] = '\0';

        return VF_SUCCESS;
}

vf_err_t vf_file_read(vf_file_t *file, void *buf, size_t size, size_t *bytes_read)
{
        size_t ret_size = 0U;

        if (NULL == file || NULL == buf || 0U == size) {
                log_err("Invalid params: file=%p buf=%p size=%zu",
                        (void *)file, buf, size);

                return VF_INVALID_PARAMETER;
        }

        if (0 == file->is_open) {
                log_err("File not open");

                return VF_FILE_ERR;
        }

        ret_size = fread(buf, 1U, size, file->fp);

        if (NULL != bytes_read) {
                *bytes_read = ret_size;
        }

        if (ret_size != size && ferror(file->fp)) {
                log_err("fread error on: %s", file->path);

                return VF_FILE_ERR;
        }

        log_dbg("Read %zu/%zu bytes from: %s", ret_size, size, file->path);

        return VF_SUCCESS;
}

vf_err_t vf_file_write(vf_file_t *file, const void *buf, size_t size, size_t *bytes_written)
{
        size_t ret_size = 0U;

        if (NULL == file || NULL == buf || 0U == size) {
                log_err("Invalid params: file=%p buf=%p size=%zu",
                        (void *)file, buf, size);

                return VF_INVALID_PARAMETER;
        }

        if (0 == file->is_open) {
                log_err("File not open");

                return VF_FILE_ERR;
        }

        ret_size = fwrite(buf, 1U, size, file->fp);

        if (NULL != bytes_written) {
                *bytes_written = ret_size;
        }

        if (ret_size != size) {
                log_err("fwrite incomplete on: %s (wrote %zu/%zu)", file->path, ret_size, size);

                return VF_FILE_ERR;
        }

        log_dbg("Wrote %zu bytes to: %s", ret_size, file->path);

        return VF_SUCCESS;
}

vf_err_t vf_file_size(vf_file_t *file, size_t *out_size)
{
        long current = 0;
        long size = 0;

        if (NULL == file || NULL == out_size) {
                log_err("Invalid params: file=%p out_size=%p",
                        (void *)file, (void *)out_size);

                return VF_INVALID_PARAMETER;
        }

        if (0 == file->is_open) {
                log_err("File not open");

                return VF_FILE_ERR;
        }

        current = ftell(file->fp);
        if (current < 0) {
                log_err("ftell failed for: %s", file->path);

                return VF_FILE_ERR;
        }

        if (EOK != fseek(file->fp, 0, SEEK_END)) {
                log_err("fseek SEEK_END failed for: %s", file->path);

                return VF_FILE_ERR;
        }

        size = ftell(file->fp);
        if (size < 0) {
                log_err("ftell (SEEK_END) failed for: %s", file->path);

                return VF_FILE_ERR;
        }

        if (EOK != fseek(file->fp, current, SEEK_SET)) {
                log_err("fseek restore failed for: %s", file->path);

                return VF_FILE_ERR;
        }

        *out_size = (size_t)size;

        return VF_SUCCESS;
}

vf_err_t vf_file_exists(const char *path, int *out_exists)
{
        FILE *fp = NULL;

        if (NULL == path || NULL == out_exists) {
                log_err("Invalid params: path=%p out_exists=%p",
                        (void *)path, (void *)out_exists);

                return VF_INVALID_PARAMETER;
        }

        fp = fopen(path, "r");
        if(NULL != fp) {
                log_err("Failed to open file %s. Error: %s\n", (void *)fp, strerror(errno));

                return VF_FILE_ERR;
        }

        *out_exists = (NULL != fp) ? 1 : 0;

        if (NULL != fp) {
                (void)fclose(fp);
        }

        return VF_SUCCESS;
}

vf_err_t vf_file_seek(vf_file_t *file, long offset, int whence)
{
        if (NULL == file) {
                log_err("Invalid params: file=%p", (void *)file);

                return VF_INVALID_PARAMETER;
        }

        if (0 == file->is_open) {
                log_err("File not open");

                return VF_FILE_ERR;
        }

        if (EOK != fseek(file->fp, offset, whence)) {
                log_err("fseek failed for: %s", file->path);

                return VF_FILE_ERR;
        }

        return VF_SUCCESS;
}

vf_err_t vf_file_tell(vf_file_t *file, long *out_pos)
{
        long pos = 0;

        if (NULL == file || NULL == out_pos) {
                log_err("Invalid params: file=%p out_pos=%p", (void *)file, (void *)out_pos);

                return VF_INVALID_PARAMETER;
        }

        if (0 == file->is_open) {
                log_err("File not open");

                return VF_FILE_ERR;
        }

        pos = ftell(file->fp);
        if (pos < 0) {
                log_err("ftell failed for: %s", file->path);

                return VF_FILE_ERR;
        }

        *out_pos = pos;

        return VF_SUCCESS;
}

