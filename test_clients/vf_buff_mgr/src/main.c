/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VisionFlow buffer manager test client
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Manual validation test client for vf_buf_pool and vf_buf_queue modules.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>

#include "vf-buff-pool.h"
#include "vf-buff-queue.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"

#define DEFAULT_POOL_SLOTS   4U
#define DEFAULT_QUEUE_CAP    4U
#define DEFAULT_FRAME_W      320U
#define DEFAULT_FRAME_H      240U

static
int test_pool_acquire_release(void)
{
        vf_buf_pool_t pool = {0};
        vf_fb_params_t params = {DEFAULT_FRAME_W, DEFAULT_FRAME_H, VF_PIXEL_FMT_RGB888};
        vf_framebuffer_t *fb0 = NULL;
        vf_framebuffer_t *fb1 = NULL;
        uint32_t buff_pool_available = 0;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_pool_acquire_release ---");

        err = vf_buf_pool_init(&pool, &params, DEFAULT_POOL_SLOTS);
        if (VF_SUCCESS != err) {
                log_err("pool init failed");

                return 1;
        }

        if (DEFAULT_POOL_SLOTS != vf_buf_pool_available(&pool)) {
                log_err("Expected %u available, got %u", DEFAULT_POOL_SLOTS,
                        vf_buf_pool_available(&pool));

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_pool_acquire(&pool, &fb0);
        if ((VF_SUCCESS != err) || (NULL == fb0)) {
                log_err("acquire fb0 failed");

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_pool_acquire(&pool, &fb1);
        if ((VF_SUCCESS != err) || (NULL == fb1)) {
                log_err("acquire fb1 failed");

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        buff_pool_available = vf_buf_pool_available(&pool);
        if ((DEFAULT_POOL_SLOTS - 2U) != buff_pool_available) {
                log_err("Expected %u available after 2 acquires", DEFAULT_POOL_SLOTS - 2U);

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_pool_release(&pool, fb0);
        if (VF_SUCCESS != err) {
                log_err("release fb0 failed");

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_pool_release(&pool, fb1);
        if (VF_SUCCESS != err) {
                log_err("release fb1 failed");

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        buff_pool_available = vf_buf_pool_available(&pool);
        if (DEFAULT_POOL_SLOTS != buff_pool_available) {
                log_err("Expected all slots available after release");

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        vf_buf_pool_deinit(&pool);

        log_info("PASS: pool acquire/release OK");

        return 0;
}

static
int test_pool_exhaustion(void)
{
        vf_buf_pool_t pool = {0};
        vf_fb_params_t params = {DEFAULT_FRAME_W, DEFAULT_FRAME_H, VF_PIXEL_FMT_RGB888};
        vf_framebuffer_t *fb = NULL;
        uint32_t i = 0U;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_pool_exhaustion ---");

        err = vf_buf_pool_init(&pool, &params, DEFAULT_POOL_SLOTS);
        if (VF_SUCCESS != err) {
                log_err("pool init failed");

                return 1;
        }

        for (i = 0U; i < DEFAULT_POOL_SLOTS; i++) {
                err = vf_buf_pool_acquire(&pool, &fb);
                if (VF_SUCCESS != err) {
                        log_err("acquire failed at slot %u", i);

                        vf_buf_pool_deinit(&pool);

                        return 1;
                }
        }

        /* Next acquire must fail */
        err = vf_buf_pool_acquire(&pool, &fb);
        if (VF_OOM != err) {
                log_err("Expected VF_OOM on exhausted pool, got %s", vf_err2str(err));

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        vf_buf_pool_deinit(&pool);

        log_info("PASS: pool exhaustion correctly detected");

        return 0;
}

static
int test_queue_push_pop(void)
{
        vf_buf_pool_t pool = {0};
        vf_buf_queue_t queue = {0};
        vf_fb_params_t params = {DEFAULT_FRAME_W, DEFAULT_FRAME_H, VF_PIXEL_FMT_RGB888};
        vf_framebuffer_t *fb_in = NULL;
        vf_framebuffer_t *fb_out = NULL;
        vf_err_t err = VF_SUCCESS;

        log_info("--- test_queue_push_pop ---");

        err = vf_buf_pool_init(&pool, &params, DEFAULT_POOL_SLOTS);
        if (VF_SUCCESS != err) {
                log_err("pool init failed");

                return 1;
        }

        err = vf_buf_queue_init(&queue, DEFAULT_QUEUE_CAP);
        if (VF_SUCCESS != err) {
                log_err("queue init failed");

                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_pool_acquire(&pool, &fb_in);
        if ((VF_SUCCESS != err) || (NULL == fb_in)) {
                log_err("acquire failed");

                vf_buf_queue_deinit(&queue);
                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_queue_push(&queue, fb_in);
        if (VF_SUCCESS != err) {
                log_err("push failed");

                vf_buf_queue_deinit(&queue);
                vf_buf_pool_deinit(&pool);

                return 1;
        }

        if (1U != vf_buf_queue_count(&queue)) {
                log_err("Expected count=1 after push");

                vf_buf_queue_deinit(&queue);
                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_queue_pop(&queue, &fb_out);
        if ((VF_SUCCESS != err) || (NULL == fb_out)) {
                log_err("pop failed");

                vf_buf_queue_deinit(&queue);
                vf_buf_pool_deinit(&pool);

                return 1;
        }

        if (fb_in != fb_out) {
                log_err("Popped frame != pushed frame");

                vf_buf_queue_deinit(&queue);
                vf_buf_pool_deinit(&pool);

                return 1;
        }

        err = vf_buf_pool_release(&pool, fb_out);
        if (VF_SUCCESS != err) {
                log_err("Failed to release output framebuffer after conversion failure: %s",
                        vf_err2str(err));
        }

        vf_buf_queue_deinit(&queue);
        vf_buf_pool_deinit(&pool);

        log_info("PASS: queue push/pop OK");

        return 0;
}

int main(void)
{
        vf_err_t err = VF_SUCCESS;
        int failed = 0;

        err = vf_logger_init("vf_buf_mgr_test", VF_LOG_LEVEL_DBG);
        if (VF_SUCCESS != err) {
                (void)fprintf(stderr, "[vf_buf_mgr_test] Logger init failed\n");

                return EXIT_FAILURE;
        }

        log_info("=== vf_buf_mgr test client start ===");

        failed += test_pool_acquire_release();
        failed += test_pool_exhaustion();
        failed += test_queue_push_pop();

        if (0 == failed) {
                log_info("All tests passed");
        } else {
                log_err("%d test(s) failed", failed);
        }

        log_info("=== vf_buf_mgr test client end ===");

        vf_logger_deinit();

        return (0 == failed) ? EXIT_SUCCESS : EXIT_FAILURE;
}

