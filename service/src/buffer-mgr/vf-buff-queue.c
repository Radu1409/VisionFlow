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
        int ret = 0;

        if (NULL == queue) {
                log_err("Invalid param: queue=NULL");

                return VF_INVALID_PARAMETER;
        }

        if ((0U == capacity) || (capacity > VF_BUF_QUEUE_MAX_CAPACITY)) {
                log_err("Invalid capacity=%u (max=%u)", capacity, VF_BUF_QUEUE_MAX_CAPACITY);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(queue, 0, sizeof(*queue));

        ret = pthread_mutex_init(&queue->lock, NULL);
        if (0 != ret) {
                log_err("pthread_mutex_init failed: %d", ret);

                return VF_SYNC_ERROR;
        }

        queue->capacity    = capacity;
        queue->head        = 0U;
        queue->tail        = 0U;
        queue->count       = 0U;
        queue->initialized = 1;

        log_info("Buffer queue initialized: capacity=%u", capacity);

        return VF_SUCCESS;
}

vf_err_t vf_buf_queue_push(vf_buf_queue_t   *queue,
                            vf_framebuffer_t *fb)
{
        int ret = 0;

        if ((NULL == queue) || (NULL == fb)) {
                log_err("Invalid params: queue=%p fb=%p",
                        (void *)queue, (void *)fb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == queue->initialized) {
                log_err("Queue not initialized");

                return VF_INIT_FAILED;
        }

        ret = pthread_mutex_lock(&queue->lock);
        if (0 != ret) {
                log_err("pthread_mutex_lock failed: %d", ret);

                return VF_SYNC_ERROR;
        }

        if (queue->count >= queue->capacity) {
                log_wrn("Queue full: capacity=%u", queue->capacity);

                (void)pthread_mutex_unlock(&queue->lock);

                return VF_OOM;
        }

        queue->slots[queue->tail] = fb;
        queue->tail = (queue->tail + 1U) % queue->capacity;
        queue->count++;

        log_dbg("Frame pushed: count=%u/%u", queue->count, queue->capacity);

        ret = pthread_mutex_unlock(&queue->lock);
        if (0 != ret) {
                log_err("pthread_mutex_unlock failed: %d", ret);

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}

vf_err_t vf_buf_queue_pop(vf_buf_queue_t    *queue,
                           vf_framebuffer_t **out_fb)
{
        int ret = 0;

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

        ret = pthread_mutex_lock(&queue->lock);
        if (0 != ret) {
                log_err("pthread_mutex_lock failed: %d", ret);

                return VF_SYNC_ERROR;
        }

        if (0U == queue->count) {
                log_dbg("Queue empty");

                (void)pthread_mutex_unlock(&queue->lock);

                return VF_FILE_ERR; /* reusing as "nothing available" — see note below */
        }

        *out_fb    = queue->slots[queue->head];
        queue->slots[queue->head] = NULL;
        queue->head  = (queue->head + 1U) % queue->capacity;
        queue->count--;

        log_dbg("Frame popped: count=%u/%u", queue->count, queue->capacity);

        ret = pthread_mutex_unlock(&queue->lock);
        if (0 != ret) {
                log_err("pthread_mutex_unlock failed: %d", ret);

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}

void vf_buf_queue_deinit(vf_buf_queue_t *queue)
{
        if (NULL == queue) {
                log_wrn("vf_buf_queue_deinit called with NULL queue");

                return;
        }

        if (0 == queue->initialized) {
                return;
        }

        if (0U < queue->count) {
                log_wrn("Queue deinit with %u frame(s) still in queue", queue->count);
        }

        (void)pthread_mutex_destroy(&queue->lock);

        (void)memset(queue, 0, sizeof(*queue));

        log_dbg("Buffer queue deinitialized");
}

uint32_t vf_buf_queue_count(vf_buf_queue_t *queue)
{
        uint32_t count = 0U;

        if ((NULL == queue) || (0 == queue->initialized)) {
                return 0U;
        }

        (void)pthread_mutex_lock(&queue->lock);
        count = queue->count;
        (void)pthread_mutex_unlock(&queue->lock);

        return count;
}

int vf_buf_queue_is_full(vf_buf_queue_t *queue)
{
        int full = 0;

        if ((NULL == queue) || (0 == queue->initialized)) {
                return 0;
        }

        (void)pthread_mutex_lock(&queue->lock);
        full = (queue->count >= queue->capacity) ? 1 : 0;
        (void)pthread_mutex_unlock(&queue->lock);

        return full;
}

int vf_buf_queue_is_empty(vf_buf_queue_t *queue)
{
        int empty = 0;

        if ((NULL == queue) || (0 == queue->initialized)) {
                return 1;
        }

        (void)pthread_mutex_lock(&queue->lock);
        empty = (0U == queue->count) ? 1 : 0;
        (void)pthread_mutex_unlock(&queue->lock);

        return empty;
}

