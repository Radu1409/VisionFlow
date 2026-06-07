/**
 **************************************************************************************************
 *  @file           : vf-buf-pool.h
 *  @brief          : VisionFlow Buffer Pool API Header
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

#ifndef VF_BUFF_POOL_H
#define VF_BUFF_POOL_H

#include <pthread.h>
#include <stdint.h>

#include "vf-error.h"
#include "vf-framebuffer.h"

#define VF_BUF_POOL_MAX_SLOTS 16U

typedef struct {
        vf_framebuffer_t  slots[VF_BUF_POOL_MAX_SLOTS];
        uint8_t           in_use[VF_BUF_POOL_MAX_SLOTS];
        uint32_t          capacity;
        uint32_t          available;
        pthread_mutex_t   lock;
        pthread_cond_t    slot_available;
        int               initialized;
} vf_buf_pool_t;

vf_err_t vf_buf_pool_init(vf_buf_pool_t *pool, const vf_fb_params_t *params, uint32_t slot_count);
vf_err_t vf_buf_pool_acquire(vf_buf_pool_t *pool, vf_framebuffer_t **out_fb);
vf_err_t vf_buf_pool_acquire_blocking(vf_buf_pool_t *pool, vf_framebuffer_t **out_fb);
vf_err_t vf_buf_pool_release(vf_buf_pool_t *pool, vf_framebuffer_t *fb);
void vf_buf_pool_deinit(vf_buf_pool_t *pool);
uint32_t vf_buf_pool_available(vf_buf_pool_t *pool);

#endif /* VF_BUFF_POOL_H */

