/**
 **************************************************************************************************
 *  @file           : vf-buffer-mgr.h
 *  @brief          : Vision flow thread-safe buffer manager
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Buffer manager provide and store framebuffer queues for clients.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <pthread.h>

#include "vf-buffer-mgr.h"
#include "vf-framebuffer-queue.h"

typedef struct {
        vf_fb_queue_st_t  queue[VF_MAX_QUEUE_CNT];
        uint32_t          queue_cnt;
        pthread_mutex_t   mutex;
} vf_buffm_st_t;

static vf_buffm_st_t buff_mgr;

vf_err_t buffer_mgr_init(void)
{
        vf_err_t rc = VF_SUCCESS;
        int i = 0;
        int err = 0;

        memset(&buff_mgr, 0, sizeof(vf_buffm_st_t));

        for (i = 0; i < VF_MAX_QUEUE_CNT; i++) {
                rc = vf_fb_queue_init(&buff_mgr.queue[i]);
                if (VF_SUCCESS != rc) {
                        log_error("Failed to initialize queue: id = %u. Error: %s\n",
                                  buff_mgr.queue_cnt, vf_err2str(rc));

                        return err;
                }
        }

        err = pthread_mutex_init(&buff_mgr.mutex, NULL);
        if (EOK != err) {
                log_error("Failed to initialize buff_mgr mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        return VF_SUCCESS;
}

void buffer_mgr_deinit(void)
{
        vf_err_t rc = VF_SUCCESS;
        int err = 0;
        int i = 0;

        err = pthread_mutex_lock(&buff_mgr.mutex);
        if (EOK != err) {
                log_error("Failed to lock buff_mtx mutex. Error: %s\n", strerror(err));

                return;
        }

        for (i = 0; i < VF_MAX_QUEUE_CNT; i++) {
                rc = vf_fb_queue_destroy(&buff_mgr.queue[i]);
                if (VF_SUCCESS != rc) {
                        log_error("Failed to deinit queue: id = %u. Error: %s\n",
                                buff_mgr.queue_cnt, vf_err2str(rc));
                }
        }

        err = pthread_mutex_unlock(&buff_mgr.mutex);
        if (EOK != err) {
                log_error("Failed to unlock buff_mtx mutex. Error: %s\n", strerror(err));

                return;
        }

        err = pthread_mutex_destroy(&buff_mgr.mutex);
        if (EOK != err) {
                log_error("Failed to destroy buff_mtx mutex. Error: %s\n", strerror(err));
        }
}

vf_err_t buffer_mgr_get_queue_id(vf_queue_descr_st_t *id)
{
        int err = 0;

        if (NULL == id) {
                log_error("Invalid input: id = %p\n", id);

                return VF_INVALID_PARAMETER;
        }

        err = pthread_mutex_lock(&buff_mgr.mutex);
        if (EOK != err) {
                log_error("Failed to lock buff_mtx mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        *id = buff_mgr.queue_cnt++;

        err = pthread_mutex_unlock(&buff_mgr.mutex);
        if (EOK != err) {
                log_error("Failed to unlock buff_mtx mutex. Error: %s\n", strerror(err));

                return VF_SYNC_ERROR;
        }

        log_info("Registered queue: id = %u\n", *id);

        return VF_SUCCESS;
}

vf_err_t buffer_mgr_send_fb(vf_queue_descr_st_t id, const vf_fb_st_t *fb)
{
        vf_err_t rc = VF_SUCCESS;
        vf_fb_st_t *fb_copy = NULL;

        if (NULL == fb) {
                log_error("Invalid input: fb = %p\n", fb);

                return VF_INVALID_PARAMETER;
        }

        if (VF_MAX_QUEUE_CNT >= id) {
                log_error("Invalid input: id = %u\n", id);

                return VF_INVALID_PARAMETER;
        }

        fb_copy = vf_copy_framebuffer(fb);
        if (NULL == fb_copy) {
                log_error("Failed to copy frame buffer with id = %u\n", id);

                return VF_OOM;
        }

        rc = vf_fb_queue_push_back(&buff_mgr.queue[id], fb_copy);
        if (VF_SUCCESS != rc) {
                vf_free_framebuffer(&fb_copy);

                log_error("Failed to push back frame buffer into queue: id = %u. Error: %s\n",
                          id, vf_err2str(rc));

                return rc;
        }

        return VF_SUCCESS;
}

vf_err_t buffer_mgr_get_fb(vf_queue_descr_st_t id, vf_fb_st_t **fb)
{
        vf_err_t rc = VF_SUCCESS;

        if (VF_MAX_QUEUE_CNT >= id || NULL == fb) {
                log_error("Invalid input: id = %u, fb = %p\n", id, fb);

                return VF_INVALID_PARAMETER;
        }

        rc = vf_fb_queue_pop_front(&buff_mgr.queue[id], fb);
        if (VF_SUCCESS != rc) {
                log_error("Failed to pop front frame buffer from queue: id = %u. Error: %s\n",
                          id, vf_err2str(rc));

                return rc;
        }

        return VF_SUCCESS;
}
