/**
 **************************************************************************************************
 *  @file           : vf-framebuffer-queue.h
 *  @brief          : Vision Flow framebuffer FIFO queue API
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @description:
 *  API for FIFO queue of framebuffers used as a storage of buffers
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_FRAMEBUFFER_QUEUE__H
#define VF_FRAMEBUFFER_QUEUE__H

#include <pthread.h>
#include <stdint.h>

#include "list.h"

#include "vf-error.h"
#include "vf-framebuffer.h"

typedef struct {
        vf_fb_st_t      *fb;
        struct list_node node;
} vf_fb_node_st_t;

typedef struct {
        struct list_node buffers;
        pthread_mutex_t  mutex;
} vf_fb_queue_st_t;

/*
 * @FUNCTION vf_fb_queue_init
 *
 * @brief         Queue initialization
 *
 * @param[in]     queue     Framebuffers queue structure
 *
 * @return        status
 */
vf_err_t vf_fb_queue_init(vf_fb_queue_st_t *queue);

/*
 * @FUNCTION vf_fb_queue_destroy
 *
 * @brief         Queue deinitialization and deallocate of all framebuffers
 *
 * @param[in]     queue     Framebuffers queue structure
 *
 * @return        status
 */
vf_err_t vf_fb_queue_destroy(vf_fb_queue_st_t *queue);

/*
 * @FUNCTION vf_fb_queue_push_back
 *
 * @brief         Push back framebuffer to queue and obtain ownership
 *
 * @param[in]     queue     Framebuffers queue structure
 * @param[in]     fb        Pointer to framebuffer
 *
 * @return        status
 */
vf_err_t vf_fb_queue_push_back(vf_fb_queue_st_t *queue, vf_fb_st_t *fb);

/*
 * @FUNCTION vf_fb_queue_pop_front
 *
 * @brief         Pop framebuffer from queue head and transfer ownership
 *
 * @param[in]     queue     Framebuffers queue structure
 * @param[out]    fb        Pointer to framebuffer
 *
 * @return        status
 */
vf_err_t vf_fb_queue_pop_front(vf_fb_queue_st_t *queue, vf_fb_st_t **fb);

#endif /* VF_FRAMEBUFFER_QUEUE__H */

