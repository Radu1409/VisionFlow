/**
 **************************************************************************************************
 *  @file           : vf-framebuffer-queue.c
 *  @brief          : Vision Flow framebuffer queue implementation
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @description:
 *  Implementation of queue of framebuffers used as a storage for framebuffers
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "vf-error.h"
#include "vf-framebuffer-queue.h"
#include "vf-mem.h"

vf_err_t vf_fb_queue_init(vf_fb_queue_st_t *queue)
{
        int err = EOK;

        if (NULL == queue) {
                log_error("Invalid input: queue = %p\n", queue);

                return VF_INVALID_PARAMETER;
        }

        list_initialize(&queue->buffers);

        err = pthread_mutex_init(&queue->mutex, NULL);
        if (EOK != err) {
                log_error("Failed to initialize queue mutex. Error: %s\n", strerror(err));

                return VF_INIT_FAILED;
        }

        return VF_SUCCESS;
}

vf_err_t vf_fb_queue_destroy(vf_fb_queue_st_t *queue)
{
        int err = EOK;
        vf_fb_node_st_t *curr = NULL;
        vf_fb_node_st_t *next = NULL;

        if (NULL == queue) {
                log_error("Invalid input: queue = %p\n", queue);

                return VF_INVALID_PARAMETER;
        }

        err = pthread_mutex_lock(&queue->mutex);
        if (EOK != err) {
                log_error("Failed to lock queue mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        list_for_every_entry_safe(&queue->buffers, curr, next, vf_fb_node_st_t, node) {
                vf_free_framebuffer(&curr->fb);

                list_delete(&curr->node);
                clear_data(&curr);
        }

        err = pthread_mutex_unlock(&queue->mutex);
        if (EOK != err) {
                log_error("Failed to unlock queue mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        err = pthread_mutex_destroy(&queue->mutex);
        if (EOK != err) {
                log_error("Failed to destroy queue mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}

vf_err_t vf_fb_queue_push_back(vf_fb_queue_st_t *queue, vf_fb_st_t *fb)
{
        int err = EOK;
        vf_err_t rc = VF_SUCCESS;
        vf_fb_node_st_t *queue_node = NULL;

        if (NULL == queue || NULL == fb) {
                log_error("Invalid input: queue = %p, fb = %p\n", queue, fb);

                return VF_INVALID_PARAMETER;
        }

        queue_node = alloc_data(1, vf_fb_node_st_t);
        if (NULL == queue_node) {
                log_error("Failed to allocate memory for the framebuffer queue node\n");

                return VF_OOM;
        }

        queue_node->fb = fb;

        err = pthread_mutex_lock(&queue->mutex);
        if (EOK != err) {
                log_error("Failed to lock queue mutex. Error: %s\n", strerror(err));

                rc = VF_SYNC_ERROR;

                goto mutex_lock_failed;
        }

        list_add_tail(&queue->buffers, &queue_node->node);

        err = pthread_mutex_unlock(&queue->mutex);
        if (EOK != err) {
                log_error("Failed to unlock queue mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        log_debug("Successfully added a framebuffer to the queue: width = %d, height = %d\n",
                fb->params.width, fb->params.height);

        return VF_SUCCESS;

mutex_lock_failed:
        clear_data(&queue_node);

        return rc;
}

vf_err_t vf_fb_queue_pop_front(vf_fb_queue_st_t *queue, vf_fb_st_t **fb)
{
        int err = EOK;
        vf_fb_node_st_t *front_node = NULL;

        if (NULL == queue || NULL == fb) {
                log_error("Invalid input: queue = %p, fb = %p\n", queue, fb);

                return VF_INVALID_PARAMETER;
        }

        err = pthread_mutex_lock(&queue->mutex);
        if (EOK != err) {
            log_error("Failed to lock queue mutex. Error: %s\n", strerror(err));

            return VF_SYNC_ERROR;
        }

        front_node = list_remove_head_type(&queue->buffers, vf_fb_node_st_t, node);
        if (NULL != front_node) {
                *fb = front_node->fb;
                clear_data(&front_node);
        } else {
                *fb = NULL;
        }

        err = pthread_mutex_unlock(&queue->mutex);
        if (EOK != err) {
                log_error("Failed to unlock queue mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}
