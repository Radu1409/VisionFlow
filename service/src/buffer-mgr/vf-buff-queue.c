/**
 **************************************************************************************************
 *  @file           : vf-buf-queue.c
 *  @brief          : VisionFlow Buffer Queue API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Fixed-capacity circular FIFO queue of vf_framebuffer_t pointers for inter-unit
 *  frame passing. No dynamic allocation. Thread-safe.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <string.h>

#include "vf-buff-queue.h"
#include "vf-error.h"
#include "vf-logger.h"

#define MODULE_NAME "vf_buf_queue"

vf_err_t vf_buf_queue_init(vf_buf_queue_t *queue, uint32_t capacity)
{
        int rc = 0;

        if (NULL == queue) {
                log_err("Invalid param: queue=NULL");

                return VF_INVALID_PARAMETER;
        }

        if ((0U == capacity) || (capacity > VF_BUF_QUEUE_MAX_CAPACITY)) {
                log_err("Invalid capacity=%u (max=%u)", capacity, VF_BUF_QUEUE_MAX_CAPACITY);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(queue, 0, sizeof(*queue));

        rc = pthread_mutex_init(&queue->lock, NULL);
        if (EOK != rc) {
                log_err("Failed to initialize queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        rc = pthread_cond_init(&queue->not_empty, NULL);
        if (EOK != rc) {
                log_err("Failed to initialize queue condition variable. Error: %d", rc);

                rc = pthread_mutex_destroy(&queue->lock);
                if (EOK != rc) {
                        log_err("Failed to destroy queue mutex. Error: %d", rc);
                }

                return VF_SYNC_ERROR;
        }

        queue->capacity = capacity;
        queue->head = 0U;
        queue->tail = 0U;
        queue->count = 0U;
        queue->initialized = 1;

        atomic_store(&queue->shutdown, false);

        log_info("Buffer queue initialized: capacity=%u", capacity);

        return VF_SUCCESS;
}

vf_err_t vf_buf_queue_push(vf_buf_queue_t *queue, vf_framebuffer_t *fb)
{
        int rc = 0;

        if ((NULL == queue) || (NULL == fb)) {
                log_err("Invalid params: queue=%p fb=%p",
                        (void *)queue, (void *)fb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return VF_INIT_FAILED;
        }

        rc = pthread_mutex_lock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to lock queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        if (queue->count >= queue->capacity) {
                log_wrn("Queue full: capacity=%u", queue->capacity);

                rc = pthread_mutex_unlock(&queue->lock);
                if (EOK != rc) {
                        log_err("Failed to unlock queue mutex. Error: %d", rc);

                        return VF_SYNC_ERROR;
                }

                return VF_OOM;
        }

        queue->slots[queue->tail] = fb;
        queue->tail = (queue->tail + 1U) % queue->capacity;
        queue->count++;

        rc = pthread_cond_signal(&queue->not_empty);
        if (EOK != rc) {
                log_err("Failed to signal queue condition variable. Error: %d", rc);
        }

        log_dbg("Frame pushed: count=%u/%u", queue->count, queue->capacity);

        rc = pthread_mutex_unlock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to unlock queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}

vf_err_t vf_buf_queue_pop(vf_buf_queue_t *queue, vf_framebuffer_t **out_fb)
{
        int rc = 0;

        if ((NULL == queue) || (NULL == out_fb)) {
                log_err("Invalid params: queue=%p out_fb=%p",
                        (void *)queue, (void *)out_fb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return VF_INIT_FAILED;
        }

        *out_fb = NULL;

        rc = pthread_mutex_lock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to lock queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        if (0U == queue->count) {
                log_dbg("Queue empty");

                rc = pthread_mutex_unlock(&queue->lock);
                if (EOK != rc) {
                        log_err("Failed to unlock queue mutex. Error: %d", rc);

                        return VF_SYNC_ERROR;
                }

                return VF_FILE_ERR; /* reusing as "nothing available" — see note below */
        }

        *out_fb = queue->slots[queue->head];
        queue->slots[queue->head] = NULL;
        queue->head  = (queue->head + 1U) % queue->capacity;
        queue->count--;

        log_dbg("Frame popped: count=%u/%u", queue->count, queue->capacity);

        rc = pthread_mutex_unlock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to unlock queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}

void vf_buf_queue_shutdown(vf_buf_queue_t *queue)
{
        int rc = 0;

        if (NULL == queue) {
                log_err("Invalid input: queue = %p", (void *)queue);

                return;
        }

        rc = pthread_mutex_lock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to lock queue mutex. Error: %d", rc);

                return;
        }

        atomic_store(&queue->shutdown, true);

        rc = pthread_cond_broadcast(&queue->not_empty);
        if (EOK != rc) {
                log_err("Failed to broadcast queue condition. Error: %d", rc);
        }

        rc = pthread_mutex_unlock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to unlock queue mutex. Error: %d", rc);

                return;
        }
}

vf_err_t vf_buf_queue_pop_blocking(vf_buf_queue_t *queue, vf_framebuffer_t **out_fb)
{
        int rc = 0;

        if ((NULL == queue) || (NULL == out_fb)) {
                log_err("Invalid params: queue=%p out_fb=%p",
                        (void *)queue, (void *)out_fb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return VF_INIT_FAILED;
        }

        *out_fb = NULL;

        rc = pthread_mutex_lock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to lock queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        while (0U == queue->count) {
                if (atomic_load(&queue->shutdown)) {
                        rc = pthread_mutex_unlock(&queue->lock);
                        if (EOK != rc) {
                                log_err("Failed to unlock queue mutex. Error: %d", rc);

                                return VF_SYNC_ERROR;
                        }

                        return VF_QUEUE_SHUTDOWN;
                }

                rc = pthread_cond_wait(&queue->not_empty, &queue->lock);
                if (EOK != rc) {
                        log_err("Failed to wait on condition variable. Error: %d", rc);

                        rc = pthread_mutex_unlock(&queue->lock);
                        if (EOK != rc) {
                                log_err("Failed to unlock pool mutex. Error: %d", rc);

                                return VF_SYNC_ERROR;
                        }

                        return VF_SYNC_ERROR;
                }
        }

        *out_fb = queue->slots[queue->head];
        queue->slots[queue->head] = NULL;
        queue->head  = (queue->head + 1U) % queue->capacity;
        queue->count--;

        log_dbg("Frame popped (blocking): count=%u/%u", queue->count, queue->capacity);

        rc = pthread_mutex_unlock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to unlock queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}

void vf_buf_queue_reset(vf_buf_queue_t *queue)
{
        if (NULL == queue) {
                log_err("Invalid input param: queue=%p", (void*)queue);

                return;
        }

        atomic_store(&queue->shutdown, false);
}

void vf_buf_queue_deinit(vf_buf_queue_t *queue)
{
        int rc = 0;

        if (NULL == queue) {
                log_wrn("vf_buf_queue_deinit called with NULL queue");

                return;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return;
        }

        if (0U < queue->count) {
                log_wrn("Queue deinit with %u frame(s) still in queue", queue->count);
                /* Frames are not unref'd here — queue does not own pool context.
                 * Remaining frames will be freed by vf_buf_pool_deinit.
                 * TODO: drain queue with release callback before deinit. */
        }

        rc = pthread_mutex_destroy(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to destroy queue mutex. Error: %d", rc);

                return;
        }

        rc = pthread_cond_destroy(&queue->not_empty);
        if (EOK != rc) {
                log_err("Failed to destroy queue condition variable. Error: %d", rc);
        }

        (void)memset(queue, 0, sizeof(*queue));

        log_dbg("Buffer queue deinitialized");
}

uint32_t vf_buf_queue_count(vf_buf_queue_t *queue)
{
        uint32_t count = 0U;
        int rc = 0;

        if (NULL == queue) {
                log_err("Invalid params: queue=%p", (void *)queue);

                return 0U;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return 0U;
        }

        rc = pthread_mutex_lock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to lock queue mutex. Error: %d", rc);
        }

        count = queue->count;

        rc = pthread_mutex_unlock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to unlock queue mutex. Error: %d", rc);
        }

        return count;
}

int vf_buf_queue_is_full(vf_buf_queue_t *queue)
{
        int full = 0;
        int rc = 0;

        if (NULL == queue) {
                log_err("Invalid params: queue=%p", (void *)queue);

                return 0U;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return 0U;    
        }

        rc = pthread_mutex_lock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to lock queue mutex. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        full = (queue->count >= queue->capacity) ? 1 : 0;

        rc = pthread_mutex_unlock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to unlock queue mutex. Error: %d", rc);
        }

        return full;
}

int vf_buf_queue_is_empty(vf_buf_queue_t *queue)
{
        int empty = 0;
        int rc = 0;

        if (NULL == queue) {
                log_err("Invalid params: queue=%p", (void *)queue);

                return 1;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return 1;   
        }

        rc = pthread_mutex_lock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to lock queue mutex. Error: %d", rc);
        }

        if (0U == queue->count) {
                empty = 1;
        }
        else {
                empty = 0;
        }

        rc = pthread_mutex_unlock(&queue->lock);
        if (EOK != rc) {
                log_err("Failed to unlock queue mutex. Error: %d", rc);
        }

        return empty;
}

