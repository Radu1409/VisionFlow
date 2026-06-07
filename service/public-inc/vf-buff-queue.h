/**
 **************************************************************************************************
 *  @file           : vf-buf-queue.h
 *  @brief          : VisionFlow Buffer Queue API Header
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

#ifndef VF_BUFF_QUEUE_H
#define VF_BUFF_QUEUE_H

#include <pthread.h>
#include <stdint.h>

#include "vf-error.h"
#include "vf-framebuffer.h"

#define VF_BUF_QUEUE_MAX_CAPACITY 16U

typedef struct {
        vf_framebuffer_t *slots[VF_BUF_QUEUE_MAX_CAPACITY];
        uint32_t          head;
        uint32_t          tail;
        uint32_t          count;
        uint32_t          capacity;
        pthread_mutex_t   lock;
        pthread_cond_t    not_empty;
        int               initialized;
} vf_buf_queue_t;

vf_err_t vf_buf_queue_init(vf_buf_queue_t *queue, uint32_t capacity);
vf_err_t vf_buf_queue_push(vf_buf_queue_t *queue, vf_framebuffer_t *fb);
vf_err_t vf_buf_queue_pop(vf_buf_queue_t *queue, vf_framebuffer_t **out_fb);
vf_err_t vf_buf_queue_pop_blocking(vf_buf_queue_t *queue, vf_framebuffer_t **out_fb);
void vf_buf_queue_deinit(vf_buf_queue_t *queue);
uint32_t vf_buf_queue_count(vf_buf_queue_t *queue);
int vf_buf_queue_is_full(vf_buf_queue_t *queue);
int vf_buf_queue_is_empty(vf_buf_queue_t *queue);

#endif /* VF_BUFF_QUEUE_H */

