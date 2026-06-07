/**
 **************************************************************************************************
 *  @file           : vf-buf-pool.c
 *  @brief          : VisionFlow Buffer Pool API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Fixed-size pool of pre-allocated vf_framebuffer_t slots. All framebuffers are
 *  allocated at init time. No dynamic allocation occurs at runtime. Thread-safe.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <string.h>

#include "vf-buff-pool.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-logger.h"

#define MODULE_NAME "vf_buf_pool"

vf_err_t vf_buf_pool_init(vf_buf_pool_t *pool, const vf_fb_params_t *params,
                          uint32_t slot_count)
{
        uint32_t i = 0U;
        int rc = 0;
        vf_err_t err = VF_SUCCESS;

        if ((NULL == pool) || (NULL == params)) {
                log_err("Invalid params: pool=%p params=%p",
                        (void *)pool, (void *)params);

                return VF_INVALID_PARAMETER;
        }

        if ((0U == slot_count) || (slot_count > VF_BUF_POOL_MAX_SLOTS)) {
                log_err("Invalid slot_count=%u (max=%u)", slot_count, VF_BUF_POOL_MAX_SLOTS);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(pool, 0, sizeof(*pool));

        atomic_store(&pool->shutdown, false);

        rc = pthread_mutex_init(&pool->lock, NULL);
        if (EOK != rc) {
                log_err("Failed to initialize pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        rc = pthread_cond_init(&pool->slot_available, NULL);
        if (EOK != rc) {
                log_err("Failed to initialize pool cond. Error: %d", rc);

                rc = pthread_mutex_destroy(&pool->lock);
                if (EOK != rc) {
                        log_err("Failed to destroy pool mutex. Error: %d", rc);
                }

                return VF_SYNC_ERROR;
        }

        for (i = 0U; i < slot_count; i++) {
                err = vf_framebuffer_alloc(&pool->slots[i], params);
                if (VF_SUCCESS != err) {
                        log_err("Failed to alloc slot %u: %s", i, vf_err2str(err));

                        /* Free already allocated slots */
                        while (i > 0U) {
                                i--;
                                vf_framebuffer_free(&pool->slots[i]);
                        }

                        rc = pthread_mutex_destroy(&pool->lock);
                        if (EOK != rc) {
                                log_err("Failed to destroy pool mutex. Error: %d", rc);
                        }

                        return err;
                }

                pool->in_use[i] = 0U;
        }

        pool->capacity = slot_count;
        pool->available = slot_count;
        pool->initialized = 1;

        log_info("Buffer pool initialized: %u slots, format=%s, %ux%u",
                 slot_count,
                 vf_pixel_fmt_str(params->format),
                 params->width, params->height);

        return VF_SUCCESS;
}

vf_err_t vf_buf_pool_acquire(vf_buf_pool_t *pool, vf_framebuffer_t **out_fb)
{
        uint32_t i = 0U;
        int rc = 0;

        if ((NULL == pool) || (NULL == out_fb)) {
                log_err("Invalid params: pool=%p out_fb=%p",
                        (void *)pool, (void *)out_fb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == pool->initialized) {
                log_err("Pool not initialized");

                return VF_INIT_FAILED;
        }

        *out_fb = NULL;

        rc = pthread_mutex_lock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to lock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        if (0U == pool->available) {
                log_wrn("Pool exhausted: no free slots");

                rc = pthread_mutex_unlock(&pool->lock);
                if (EOK != rc) {
                        log_err("Failed to unlock pool mutex. Error: %d", rc);

                        return VF_SYNC_ERROR;
                }

                return VF_OOM;
        }

        for (i = 0U; i < pool->capacity; i++) {
                if (0U == pool->in_use[i]) {
                        pool->in_use[i] = 1U;
                        pool->available--;
                        *out_fb = &pool->slots[i];

                        break;
                }
        }

        rc = pthread_mutex_unlock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to unlock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        log_dbg("Slot acquired: index=%u available=%u", i, pool->available);

        return VF_SUCCESS;
}

vf_err_t vf_buf_pool_acquire_blocking(vf_buf_pool_t *pool, vf_framebuffer_t **out_fb)
{
        uint32_t i = 0U;
        int rc = 0;

        if ((NULL == pool) || (NULL == out_fb)) {
                log_err("Invalid params: pool=%p out_fb=%p",
                        (void *)pool, (void *)out_fb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == pool->initialized) {
                log_err("Pool not initialized");

                return VF_INIT_FAILED;
        }

        *out_fb = NULL;

        rc = pthread_mutex_lock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to lock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        while (0U == pool->available) {
                if (atomic_load(&pool->shutdown)) {
                        rc = pthread_mutex_unlock(&pool->lock);
                        if (EOK != rc) {
                                log_err("Failed to unlock pool mutex. Error: %d", rc);

                                return VF_SYNC_ERROR;
                        }

                        return VF_QUEUE_SHUTDOWN;
                }

                rc = pthread_cond_wait(&pool->slot_available, &pool->lock);
                if (EOK != rc) {
                        log_err("Failed to wait on pool cond. Error: %d", rc);

                        rc = pthread_mutex_unlock(&pool->lock);
                        if (EOK != rc) {
                                log_err("Failed to unlock pool mutex. Error: %d", rc);

                                return VF_SYNC_ERROR;
                        }

                        return VF_SYNC_ERROR;
                }
        }

        for (i = 0U; i < pool->capacity; i++) {
                if (0U == pool->in_use[i]) {
                        pool->in_use[i] = 1U;
                        pool->available--;
                        *out_fb = &pool->slots[i];

                        break;
                }
        }

        rc = pthread_mutex_unlock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to unlock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        log_dbg("Slot acquired (blocking): index=%u available=%u", i, pool->available);

        return VF_SUCCESS;
}

void vf_buf_pool_shutdown(vf_buf_pool_t *pool)
{
        int rc = 0;

        if (NULL == pool) {
                log_err("Invalid input: pool = %p", (void*)pool);

                return;
        }

        atomic_store(&pool->shutdown, true);

        rc = pthread_cond_broadcast(&pool->slot_available);
        if (EOK != rc) {
                log_err("Failed to broadcast pool condition variable. Error: %d", rc);
        }
}

vf_err_t vf_buf_pool_release(vf_buf_pool_t *pool, vf_framebuffer_t *fb)
{
        uint32_t i = 0U;
        int found = 0;
        int rc = 0;

        if ((NULL == pool) || (NULL == fb)) {
                log_err("Invalid params: pool=%p fb=%p",
                        (void *)pool, (void *)fb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == pool->initialized) {
                log_err("Pool not initialized");

                return VF_INIT_FAILED;
        }

        rc = pthread_mutex_lock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to lock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        for (i = 0U; i < pool->capacity; i++) {
                if (fb == &pool->slots[i]) {
                        if (0U == pool->in_use[i]) {
                                log_wrn("Slot %u already released", i);
                        } else {
                                pool->in_use[i] = 0U;
                                pool->available++;

                                rc = pthread_cond_signal(&pool->slot_available);
                                if (EOK != rc) {
                                        log_err("Failed to signal pool condition variable. "
                                                "Error: %d", rc);
                                }

                        }

                        found = 1;

                        break;
                }
        }

        rc = pthread_mutex_unlock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to unlock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        if (0 == found) {
                log_err("Framebuffer %p does not belong to this pool", (void *)fb);

                return VF_INVALID_PARAMETER;
        }

        log_dbg("Slot released: index=%u available=%u", i, pool->available);

        return VF_SUCCESS;
}

void vf_buf_pool_reset(vf_buf_pool_t *pool)
{
        if (NULL == pool) {
                log_err("Invalid input param: pool=%p", (void*)pool);

                return;
        }

        atomic_store(&pool->shutdown, false);
}

void vf_buf_pool_deinit(vf_buf_pool_t *pool)
{
        uint32_t i = 0U;
        int rc = 0;

        if (NULL == pool) {
                log_wrn("vf_buf_pool_deinit called with NULL pool");

                return;
        }

        if (0 == pool->initialized) {
                log_err("Pool not initialized");

                return;
        }

        for (i = 0U; i < pool->capacity; i++) {
                if (1U == pool->in_use[i]) {
                        log_wrn("Slot %u still in use at deinit", i);
                }

                vf_framebuffer_free(&pool->slots[i]);
        }

        rc = pthread_mutex_destroy(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to destroy pool mutex. Error: %d", rc);
        }

        rc = pthread_cond_destroy(&pool->slot_available);
        if (EOK != rc) {
                log_err("Failed to destroy pool cond. Error: %d", rc);
        }

        (void)memset(pool, 0, sizeof(*pool));

        log_dbg("Buffer pool deinitialized");
}

uint32_t vf_buf_pool_available(vf_buf_pool_t *pool)
{
        uint32_t count = 0U;
        int rc = 0;

        if (NULL == pool) {
                log_err("Invalid params: pool=%p", (void *)pool);

                return 0U;
        }

        if(0 == pool->initialized) {
                log_err("Pool not initialized");

                return 0U;
        }

        rc = pthread_mutex_lock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to lock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        count = pool->available;

        rc = pthread_mutex_unlock(&pool->lock);
        if (EOK != rc) {
                log_err("Failed to unlock pool mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        return count;
}

