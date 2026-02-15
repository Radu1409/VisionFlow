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

#ifndef __VF_BUFFER_MGR__H__
#define __VF_BUFFER_MGR__H__

#include <stdint.h>
#include <string.h>

#include "vf-logger.h"
#include "vf-error.h"
#include "vf-framebuffer.h"
#include "vf-mem.h"

#define VF_QUEUE_NAME_LEN      32
#define VF_MAX_QUEUE_CNT       16

typedef uint32_t vf_queue_descr_st_t;

/*
 * @FUNCTION buffer_mgr_init
 *
 * @brief         Buffer manager initialization
 *
 * @return        status
 */
vf_err_t buffer_mgr_init(void);

/*
 * @FUNCTION buffer_mgr_deinit
 *
 * @brief         Buffer manager deinit and resources release
 *
 * @return        none
 */
void buffer_mgr_deinit(void);

/*
 * @FUNCTION buffer_mgr_get_queue_id
 *
 * @brief         Get queue id which is used by buffer manager clients
 *
 * @param[out]    id     Queue id
 *
 * @return        status
 */
vf_err_t buffer_mgr_get_queue_id(vf_queue_descr_st_t *id);

/*
 * @FUNCTION buffer_mgr_send_fb
 *
 * @brief         Copy fb and store it in queue.
 *
 * @param[in]     id     Queue id
 * @param[in]     fb     Frame buffer to copy
 *
 * @return        status
 */
vf_err_t buffer_mgr_send_fb(vf_queue_descr_st_t id, const vf_fb_st_t *fb);

/*
 * @FUNCTION buffer_mgr_get_fb
 *
 * @brief         Get fb from queue by id.
 *                Transfers ownership
 *
 * @param[in]     id     Queue id
 * @param[out]    fb     Frame buffer
 *
 * @return        status
 */
vf_err_t buffer_mgr_get_fb(vf_queue_descr_st_t id, vf_fb_st_t **fb);

#endif /* __VF_BUFFER_MGR__H__ */
