/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VisionFlow vf_file test client
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Manual validation test client for the vf_file library module
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf-error.h"
#include "vf-file.h"
#include "vf-logger.h"

#define DEFAULT_FILE_PATH  "vf_file_test.bin"
#define DEFAULT_DATA       "VisionFlow vf_file test payload\n"
#define DEFAULT_BUFFER_LEN 128

static
int test_write_read(void)
{
        vf_file_t file = {0};
        const char *data = DEFAULT_DATA;
        size_t data_sz = 0U;
        char rbuf[DEFAULT_BUFFER_LEN]= {0};
        size_t n = 0U;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_write_read ---");

        data_sz = strlen(data);

        err = vf_file_open(&file, DEFAULT_FILE_PATH, "wb");
        if (VF_SUCCESS != err) {
                log_err("open write failed");

                return 1;
        }

        err = vf_file_write(&file, data, data_sz, &n);
        if (VF_SUCCESS != err || n != data_sz) {
                log_err("write failed");

                return 1;
        }

        err = vf_file_close(&file);
        if (VF_SUCCESS != err) {
                log_err("close after write failed");

                return 1;
        }

        err = vf_file_open(&file, DEFAULT_FILE_PATH, "rb");
        if (VF_SUCCESS != err) {
                log_err("open read failed");

                return 1;
        }

        err = vf_file_read(&file, rbuf, data_sz, &n);
        if (VF_SUCCESS != err || n != data_sz) {
                log_err("read failed");

                (void)vf_file_close(&file);

                return 1;
        }

        rbuf[n] = '\0';

        if (0 != memcmp(rbuf, data, data_sz)) {
                log_err("Data mismatch: got '%s'", rbuf);

                (void)vf_file_close(&file);

                return 1;
        }

        (void)vf_file_close(&file);

        log_info("PASS: write/read roundtrip OK");

        return 0;
}

static
int test_exists(void)
{
        int exists = 0;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_exists ---");

        err = vf_file_exists(DEFAULT_FILE_PATH, &exists);
        if (0 == exists) {
                log_err("File should exist: %s", DEFAULT_FILE_PATH);

                return 1;
        }

        err = vf_file_exists("nonexistent_file.bin", &exists);
        if (0 != exists) {
                log_err("File should not exist");

                return 1;
        }

        log_info("PASS: exists check OK");

        return 0;
}

static
int test_size(void)
{
        vf_file_t file = {0};
        size_t sz = 0U;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_size ---");

        err = vf_file_open(&file, DEFAULT_FILE_PATH, "rb");
        if (VF_SUCCESS != err) {
                log_err("open failed");

                return 1;
        }

        err = vf_file_size(&file, &sz);
        if (VF_SUCCESS != err) {
                log_err("size query failed");

                (void)vf_file_close(&file);

                return 1;
        }

        if (sz != strlen(DEFAULT_DATA)) {
                log_err("Expected size %zu, got %zu", strlen(DEFAULT_DATA), sz);

                (void)vf_file_close(&file);

                return 1;
        }

        (void)vf_file_close(&file);

        log_info("PASS: size=%zu OK", sz);

        return 0;
}

static
int test_seek_tell(void)
{
        vf_file_t file = {0};
        long pos = 0;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_seek_tell ---");

        err = vf_file_open(&file, DEFAULT_FILE_PATH, "rb");
        if (VF_SUCCESS != err) {
                log_err("open failed");

                return 1;
        }

        err = vf_file_seek(&file, 4, SEEK_SET);
        if (VF_SUCCESS != err) {
                log_err("seek failed");

                (void)vf_file_close(&file);

                return 1;
        }

        err = vf_file_tell(&file, &pos);
        if (VF_SUCCESS != err || 4 != pos) {
                log_err("Expected pos=4, got %ld", pos);

                (void)vf_file_close(&file);

                return 1;
        }

        (void)vf_file_close(&file);

        log_info("PASS: seek/tell OK");

        return 0;
}

int main(void)
{
        int failed = 0;
        vf_err_t err = VF_SUCCESS;

        err = vf_logger_init("vf_file_test", VF_LOG_LEVEL_DBG);
        if (VF_SUCCESS != err) {
                (void)fprintf(stderr, "[vf_file_test] Logger init failed\n");

                return EXIT_FAILURE;
        }

        log_info("=== vf_file test client start ===");

        failed += test_write_read();
        failed += test_exists();
        failed += test_size();
        failed += test_seek_tell();

        if (0 == failed) {
                log_info("All tests passed");
        } else {
                log_err("%d test(s) failed", failed);
        }

        log_info("=== vf_file test client end ===");

        vf_logger_deinit();

        return (0 == failed) ? EXIT_SUCCESS : EXIT_FAILURE;
}

