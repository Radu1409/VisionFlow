/**
 **************************************************************************************************
 *  @file           : vf-processing-unit.c
 *  @brief          : VF processing unit routine
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *      Processing unit operating API
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include "vf-generic.h"
#include "vf-logger.h"
#include "vf-processing-unit.h"

#define NOTIFY_PARENT_TIME      1

static
const char *unit_type2str(unit_type_t type)
{
        switch (type) {
        case UNIT_TYPE_CAMERA:
                return UNIT_TYPE_CAMERA_STR;
        case UNIT_TYPE_DISPLAY:
                return UNIT_TYPE_DISPLAY_STR;
        case UNIT_TYPE_ENCODER:
                return UNIT_TYPE_ENCODER_STR;
        case UNIT_TYPE_CONVERSION:
                return UNIT_TYPE_CONVERSION_STR;
        default:
                return UNIT_TYPE_INVALID_STR;
        }
}

static
vf_err_t init_unit(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;

        if (NULL == unit) {
                log_err("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        unit->state = UNIT_STATE_INITIALIZATION;

        log_info("Unit (\"%s\") initialization finished successfully\n", SAFE_STR(unit->name));

        return rc;
}

static
void deinit_unit(unit_st_t *unit)
{
        if (NULL == unit) {
                log_err("Invalid input: unit = %p\n", unit);

                return;
        }

        log_info("Unit (\"%s\") de-initialization started\n", SAFE_STR(unit->name));

        unit->state = UNIT_STATE_DEINITIALIZATION;

        log_info("Unit (\"%s\") de-initialization finished\n", SAFE_STR(unit->name));

        return;
}

static
void unit_cleanup_handler(void *arg)
{
        unit_st_t *unit = NULL;

        unit = (unit_st_t *)arg;
        if (NULL == unit) {
            log_err("Invalid input: unit = %p\n", unit);

            return;
        }

        log_err("Clean-up handler called for unit: %s\n", unit->name);

        //vf_notifier_deinit(&unit->notifier);

        deinit_unit(unit);
}

static
void *unit_task(void *arg)
{
        vf_err_t rc = VF_SUCCESS;
        unit_st_t *unit = NULL;

        unit = (unit_st_t *)arg;
        if (NULL == unit) {
                log_err("Invalid input: unit = %p\n", unit);

                return NULL;
        }

        pthread_cleanup_push(unit_cleanup_handler, unit);

        // rc = vf_notifier_init(&unit->notifier);
        // if (VF_SUCCESS != rc) {
        //         log_err("Failed to initialize notifier. Error: %s\n", vf_err2str(rc));

        //         goto stop_unit_task;
        // }

        rc = init_unit(unit);
        if (VF_SUCCESS != rc) {
                log_err("Failed to initialize unit. Error: %s\n", vf_err2str(rc));

                goto stop_unit_task;
        }

        unit->state = UNIT_STATE_RUNNING;

        while (UNIT_STATE_RUNNING == unit->state) {
                pthread_testcancel();
        }

stop_unit_task:
        pthread_cleanup_pop(1);

        alarm(NOTIFY_PARENT_TIME);

        pthread_exit(NULL);
}

vf_err_t vf_create_unit(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;
        int err = EOK;

        if (NULL == unit) {
                log_err("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        log_info("Creating unit (\"%s\") with type: (\"%s\")\n", SAFE_STR(unit->name),
                 unit_type2str(unit->type));

        err = pthread_create(&unit->tid, NULL, unit_task, unit);
        if (EOK != err) {
                log_err("Failed to start unit task. Error: %s\n", strerror(err));

                return VF_INIT_FAILED;
        }

        log_dbg("Unit (\"%s\") thread ID is: %d\n", SAFE_STR(unit->name), unit->tid);

        return rc;
}

void vf_destroy_unit(unit_st_t *unit)
{
        int err = EOK;

        if (NULL == unit) {
                log_err("Invalid input: unit = %p\n", unit);

                return;
        }

        log_info("Destroing unit (\"%s\") with tid\n", SAFE_STR(unit->name), unit->tid);

        err = pthread_cancel(unit->tid);
        if (EOK != err) {
                log_err("Failed to cancel thread: %d. Error: %s\n", unit->tid, strerror(err));

                return;
        }

        log_dbg("Waiting joining of thread with tid: %d\n", unit->tid);

        err = pthread_join(unit->tid, NULL);
        if (EOK != err) {
                log_err("Failed to join unit thread: %d. Error: %s\n", unit->tid, strerror(err));
        }

        unit->state = UNIT_STATE_STOPPED;

        log_info("Unit (\"%s\") destroing finished\n", SAFE_STR(unit->name));

        return;
}

