/**
 **************************************************************************************************
 *  @file           : vf-notifier.c
 *  @brief          : VisionFlow Notifier API implementation
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Notifier API implementation. Provides intra-process event-based communication
 *  between pipeline units.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdbool.h>
#include <string.h>

#include "vf-error.h"
#include "vf-logger.h"
#include "vf-notifier.h"

#define MODULE_NAME "vf_notifier"

/* =========================================================================
 * Helpers
 * ========================================================================= */

bool vf_notifier_is_sender(vf_notifier_mode_t mode)
{
        return (VF_NOTIFIER_MODE_SENDER        == mode ||
                VF_NOTIFIER_MODE_BIDIRECTIONAL == mode);
}

bool vf_notifier_is_receiver(vf_notifier_mode_t mode)
{
        return (VF_NOTIFIER_MODE_RECEIVER      == mode ||
                VF_NOTIFIER_MODE_BIDIRECTIONAL == mode);
}

const char *vf_notifier_event2str(vf_notifier_event_t event)
{
        switch (event) {
                case VF_NOTIFIER_EVENT_FRAME_READY:
                        return VF_NOTIFIER_EVENT_FRAME_READY_STR;
                case VF_NOTIFIER_EVENT_FRAME_RECEIVED:
                        return VF_NOTIFIER_EVENT_FRAME_RECEIVED_STR;
                default:
                        return VF_NOTIFIER_EVENT_UNKNOWN_STR;
        }
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_notifier_init(vf_notifier_t *notifier, const char *id, vf_notifier_mode_t mode)
{
        int rc = 0;

        if (NULL == notifier || NULL == id) {
                log_err("Invalid input: notifier = %p, id = %p", (void *)notifier, (void *)id);

                return VF_INVALID_PARAMETER;
        }

        if (mode != VF_NOTIFIER_MODE_RECEIVER &&
            mode != VF_NOTIFIER_MODE_SENDER   &&
            mode != VF_NOTIFIER_MODE_BIDIRECTIONAL) {
                log_err("Invalid input: mode = %d", mode);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(notifier, 0, sizeof(vf_notifier_t));

        (void)snprintf(notifier->id, sizeof(notifier->id), "%s", id);

        notifier->mode = mode;
        notifier->subscriber_count = 0U;
        notifier->initialized = 0;

        rc = pthread_mutex_init(&notifier->mutex, NULL);
        if (0 != rc) {
                log_err("Failed to initialize mutex for notifier '%s'", notifier->id);

                return VF_SYNC_ERROR;
        }

        notifier->initialized = 1;

        log_info("Notifier '%s' initialized with mode=%d", notifier->id, notifier->mode);

        return VF_SUCCESS;
}

vf_err_t vf_notifier_deinit(vf_notifier_t *notifier)
{
        int rc = 0;

        if (NULL == notifier) {
                log_err("Invalid input: notifier = %p", (void *)notifier);

                return VF_INVALID_PARAMETER;
        }

        if (0 == notifier->initialized) {
                log_info("Notifier '%s' already deinitialized, skipping", notifier->id);

                return VF_SUCCESS;
        }

        rc = pthread_mutex_destroy(&notifier->mutex);
        if (0 != rc) {
                log_err("Failed to destroy mutex for notifier '%s'", notifier->id);
        }

        notifier->initialized = 0;
        notifier->subscriber_count = 0U;

        log_info("Notifier '%s' deinitialized", notifier->id);

        return VF_SUCCESS;
}

vf_err_t vf_notifier_subscribe(vf_notifier_t *notifier, vf_notifier_event_t event,
                               vf_notifier_cb_t cb, void *ctx)
{
        int ret = 0;
        vf_err_t err = VF_SUCCESS;

        if (NULL == notifier || NULL == cb) {
                log_err("Invalid input: notifier = %p, cb = %p",
                        (void *)notifier, (void *)cb);

                return VF_INVALID_PARAMETER;
        }

        if (0 == notifier->initialized) {
                log_err("Notifier not initialized");

                return VF_INIT_FAILED;
        }

        if (false == vf_notifier_is_receiver(notifier->mode)) {
                log_err("Notifier '%s' is not a receiver — cannot subscribe", notifier->id);

                return VF_INVALID_PARAMETER;
        }

        ret = pthread_mutex_lock(&notifier->mutex);
        if (ret != EOK) {
                log_err("Failed to lock notifier mutex.");

                return VF_SYNC_ERROR;
        }

        if (notifier->subscriber_count >= VF_NOTIFIER_MAX_SUBSCRIBERS) {
                log_err("Notifier '%s' has reached max subscribers (%u)",
                        notifier->id, VF_NOTIFIER_MAX_SUBSCRIBERS);

                err = VF_OOM;

                goto unlock;
        }

        notifier->subscribers[notifier->subscriber_count].event = event;
        notifier->subscribers[notifier->subscriber_count].cb = cb;
        notifier->subscribers[notifier->subscriber_count].ctx = ctx;
        notifier->subscriber_count++;

        log_info("Notifier '%s': subscriber added for event '%s' (total=%u)",
                 notifier->id,
                 vf_notifier_event2str(event),
                 notifier->subscriber_count);

unlock:
        ret = pthread_mutex_unlock(&notifier->mutex);
        if (ret != EOK) {
                log_err("Failed to unlock notifier mutex.");

                return VF_SYNC_ERROR;
        }

        return err;
}

vf_err_t vf_notifier_publish(vf_notifier_t *notifier, vf_notifier_event_t event,
                             void *data)
{
        uint32_t i = 0U;
        int ret = 0;
        vf_err_t err = VF_SUCCESS;

        if (NULL == notifier) {
                log_err("Invalid input: notifier = %p", (void *)notifier);

                return VF_INVALID_PARAMETER;
        }

        if (0 == notifier->initialized) {
                log_err("Notifier not initialized");

                return VF_INIT_FAILED;
        }

        if (false == vf_notifier_is_sender(notifier->mode)) {
                log_err("Notifier '%s' is not a sender — cannot publish", notifier->id);

                return VF_INVALID_PARAMETER;
        }

        ret = pthread_mutex_lock(&notifier->mutex);
        if (ret != EOK) {
                log_err("Failed to lock notifier mutex.");

                return VF_SYNC_ERROR;
        }

        if (0U == notifier->subscriber_count) {
                log_info("Notifier '%s': no subscribers for event '%s'",
                         notifier->id, vf_notifier_event2str(event));

                goto unlock;
        }

        for (i = 0U; i < notifier->subscriber_count; i++) {
                if (notifier->subscribers[i].event != event) {
                        continue;
                }

                log_dbg("Notifier '%s': publishing event '%s' to subscriber[%u]",
                        notifier->id, vf_notifier_event2str(event), i);

                notifier->subscribers[i].cb(event, data, notifier->subscribers[i].ctx);
        }

unlock:
        ret = pthread_mutex_unlock(&notifier->mutex);
        if (ret != EOK) {
                log_err("Failed to unlock notifier mutex.");

                return VF_SYNC_ERROR;
        }

        return err;
}

vf_err_t vf_notifier_broadcast(vf_notifier_t *notifier, vf_notifier_event_t event,
                               void *data)
{
        uint32_t i = 0U;
        int ret = 0;
        vf_err_t err = VF_SUCCESS;

        if (NULL == notifier) {
                log_err("Invalid input: notifier = %p", (void *)notifier);

                return VF_INVALID_PARAMETER;
        }

        if (0 == notifier->initialized) {
                log_err("Notifier not initialized");

                return VF_INIT_FAILED;
        }

        if (false == vf_notifier_is_sender(notifier->mode)) {
                log_err("Notifier '%s' is not a sender — cannot broadcast", notifier->id);

                return VF_INVALID_PARAMETER;
        }

        ret = pthread_mutex_lock(&notifier->mutex);
        if (ret != EOK) {
                log_err("Failed to lock notifier mutex.");

                return VF_SYNC_ERROR;
        }

        if (0U == notifier->subscriber_count) {
                log_info("Notifier '%s': no subscribers for broadcast event '%s'",
                         notifier->id, vf_notifier_event2str(event));

                goto unlock;
        }

        for (i = 0U; i < notifier->subscriber_count; i++) {
                log_dbg("Notifier '%s': broadcasting event '%s' to subscriber[%u]",
                        notifier->id, vf_notifier_event2str(event), i);

                notifier->subscribers[i].cb(event, data, notifier->subscribers[i].ctx);
        }

unlock:
        ret = pthread_mutex_unlock(&notifier->mutex);
        if (ret != EOK) {
                log_err("Failed to unlock notifier mutex.");

                return VF_SYNC_ERROR;
        }

        return err;
}

