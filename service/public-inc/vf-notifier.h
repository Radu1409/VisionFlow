/**
 **************************************************************************************************
 *  @file           : vf-notifier.h
 *  @brief          : VisionFlow Notifier API
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Notifier API declaration. Provides intra-process event-based communication
 *  between pipeline units. 
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_NOTIFIER_H
#define VF_NOTIFIER_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "vf-error.h"

#define VF_NOTIFIER_MAX_SUBSCRIBERS          8U
#define VF_NOTIFIER_ID_MAX_LEN               64U

#define VF_NOTIFIER_EVENT_FRAME_READY_STR    "VF_NOTIFIER_EVENT_FRAME_READY"
#define VF_NOTIFIER_EVENT_FRAME_RECEIVED_STR "VF_NOTIFIER_EVENT_FRAME_RECEIVED"
#define VF_NOTIFIER_EVENT_UNKNOWN_STR        "VF_NOTIFIER_EVENT_UNKNOWN"

/* *DISABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */
typedef enum {
        VF_NOTIFIER_MODE_RECEIVER      = 0,
        VF_NOTIFIER_MODE_SENDER        = 1,
        VF_NOTIFIER_MODE_BIDIRECTIONAL = 2,
} vf_notifier_mode_t;

typedef enum {
        VF_NOTIFIER_EVENT_FRAME_READY    = 0,
        VF_NOTIFIER_EVENT_FRAME_RECEIVED = 1,
        VF_NOTIFIER_EVENT_UNKNOWN
} vf_notifier_event_t;
/* *ENABLE FORMATTER* - DO NOT REMOVE. Formatter rule exception! */

typedef void (*vf_notifier_cb_t)(vf_notifier_event_t event, void *data, void *ctx);

typedef struct {
        vf_notifier_event_t  event;
        vf_notifier_cb_t     cb;
        void                *ctx;
} vf_notifier_subscriber_t;

typedef struct {
        char                       id[VF_NOTIFIER_ID_MAX_LEN];
        vf_notifier_mode_t         mode;
        vf_notifier_subscriber_t   subscribers[VF_NOTIFIER_MAX_SUBSCRIBERS];
        uint32_t                   subscriber_count;
        pthread_mutex_t            mutex;
        int                        initialized;
} vf_notifier_t;

vf_err_t vf_notifier_init(vf_notifier_t *notifier, const char *id, vf_notifier_mode_t mode);
vf_err_t vf_notifier_deinit(vf_notifier_t *notifier);
vf_err_t vf_notifier_subscribe(vf_notifier_t *notifier, vf_notifier_event_t event,
                               vf_notifier_cb_t cb, void *ctx);
vf_err_t vf_notifier_publish(vf_notifier_t *notifier, vf_notifier_event_t event, void *data);
vf_err_t vf_notifier_broadcast(vf_notifier_t *notifier, vf_notifier_event_t event, void *data);
bool vf_notifier_is_sender(vf_notifier_mode_t mode);
bool vf_notifier_is_receiver(vf_notifier_mode_t mode);
const char *vf_notifier_event2str(vf_notifier_event_t event);

#endif /* VF_NOTIFIER_H */

