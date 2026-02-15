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

#include "stdint.h"

#include "vf-generic.h"
#include "vf-logger.h"
#include "vf-processing-unit.h"

// #include "c2c-unit.h"
#include "camera-unit.h"
#include "conversion-unit.h"
#include "display-unit.h"
#include "encoder-unit.h"

#define NOTIFY_PARENT_TIME              1

#define SEC_TO_MS(x)                    ((x) * (uint64_t)1000)
#define MS_TO_SEC(x)                    ((x) / (double)1000)
#define NS_TO_MS(x)                     ((x) / (uint64_t)1000000)

#define FPS_UNKNOWN                     -1

#define CALCULATION_FRAME_LIMIT         10
#define CALCULATE_FPS(frames, time)     ((frames) / (time))

typedef struct processing_stats {
        uint64_t start_time;
        uint64_t end_time;
        uint64_t total_processing_time;
        uint64_t processing_times[CALCULATION_FRAME_LIMIT];
        uint8_t  time_slot_idx;
} processing_stats_st_t;

__thread processing_stats_st_t unit_get_data_stats = {0};
__thread processing_stats_st_t unit_process_data_stats = {0};
__thread processing_stats_st_t unit_send_data_stats = {0};

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
vf_err_t set_unit_operations(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;

        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        switch (unit->type) {
        case UNIT_TYPE_CAMERA:
                rc = camera_unit_init_operations(&unit->operations);

                break;
        case UNIT_TYPE_DISPLAY:
                rc = display_unit_init_operations(&unit->operations);

                break;
        // case UNIT_TYPE_C2C:
        //         rc = c2c_unit_init_operations(&unit->operations);

        //         break;
        case UNIT_TYPE_ENCODER:
                rc = encoder_unit_init_operations(&unit->operations);

                break;
        case UNIT_TYPE_CONVERSION:
                rc = conversion_unit_init_operations(&unit->operations);

                break;
        default:
                log_error("Invalid unit: type: %d\n", unit->type);

                return VF_INVALID_PARAMETER;
        }

        log_debug("Unit (\'%s\') operation initialization finished with result: %s\n",
                  unit_type2str(unit->type), vf_err2str(rc));

        return rc;
}

static
vf_err_t init_unit(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;

        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        unit->state = UNIT_STATE_INITIALIZATION;

        rc = set_unit_operations(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to unit. Error: %s\n", vr_err2str(rc));

                return rc;
        }

        rc = unit->operations.init(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to initialize unit (\'%s\'). Error: %s\n", SAFE_STR(unit->name),
                           vf_err2str(rc));

                return rc;
        }

        unit->state = UNIT_STATE_INITIALIZED;

        log_info("Unit (\"%s\") initialization finished successfully\n", SAFE_STR(unit->name));

        return rc;
}

static
void deinit_unit(unit_st_t *unit)
{
        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return;
        }

        log_info("Unit (\'%s\') de-initialization started\n", SAFE_STR(unit->name));

        unit->state = UNIT_STATE_DEINITIALIZATION;

        unit->operations.deinit(unit);

        unit->state = UNIT_STATE_DEINITIALIZED;

        log_info("Unit (\'%s\') de-initialization finished\n", SAFE_STR(unit->name));

        return;
}

static
void unit_cleanup_handler(void *arg)
{
        unit_st_t *unit = NULL;

        unit = (unit_st_t *)arg;
        if (NULL == unit) {
            log_error("Invalid input: unit = %p\n", unit);

            return;
        }

        log_error("Clean-up handler called for unit: %s\n", unit->name);

        //vf_notifier_deinit(&unit->notifier);

        deinit_unit(unit);
}

static
vf_err_t get_timestamp_ms(uint64_t *msec)
{
        int err = 0;
        struct timespec time = {0};

        if (NULL == msec) {
                log_error("Invalid input: msec = %p\n", msec);

                return VF_INVALID_PARAMETER;
        }

        err = clock_gettime(CLOCK_MONOTONIC, &time);
        if (-1 == err) {
                log_error("Failed to get time. Error: %s\n", strerror(err));

                return VF_INVALID_PARAMETER;
        }

        *msec = SEC_TO_MS(time.tv_sec) + NS_TO_MS(time.tv_nsec);

        return VF_SUCCESS;
}

static
double calculate_processing_rate(processing_stats_st_t *stats)
{
        uint64_t processing_time = 0;

        if (NULL == stats) {
                log_error("Invalid input: stats = %p\n", stats);

                return FPS_UNKNOWN;
        }

        processing_time = stats->end_time - stats->start_time;

        /* A circular algorithm of FPS calculation is used (only the last N frames affect the
         * results)
         * Description:
         * 1) Removing old values from total time (zeros in first N iterations)
         * 2) Updating total processing time
         * 3) Calculating next index to use in 0...N range
         * */
        stats->total_processing_time -= stats->processing_times[stats->time_slot_idx];
        stats->processing_times[stats->time_slot_idx] = processing_time;
        stats->total_processing_time += processing_time;
        stats->time_slot_idx = (stats->time_slot_idx + 1) % CALCULATION_FRAME_LIMIT;

        return CALCULATE_FPS(CALCULATION_FRAME_LIMIT, MS_TO_SEC(stats->total_processing_time));
}

static
vf_err_t unit_send_data(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;

        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        rc = get_timestamp_ms(&unit_send_data_stats.start_time);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain timestamp. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = unit->operations.send_data(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain data. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = get_timestamp_ms(&unit_send_data_stats.end_time);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain timestamp. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        unit->stats.avg_sent_fps = calculate_processing_rate(&unit_send_data_stats);
        unit->stats.sent_frames_cnt++;

        return rc;
}

static
vf_err_t unit_process_data(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;

        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        rc = get_timestamp_ms(&unit_process_data_stats.start_time);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain timestamp. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = unit->operations.process_data(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain data. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = get_timestamp_ms(&unit_process_data_stats.end_time);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain timestamp. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        unit->stats.avg_processed_fps = calculate_processing_rate(&unit_process_data_stats);

        return rc;
}

static
vf_err_t unit_get_data(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;

        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        rc = get_timestamp_ms(&unit_get_data_stats.start_time);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain timestamp. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = unit->operations.get_data(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain data. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = get_timestamp_ms(&unit_get_data_stats.end_time);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain timestamp. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        unit->stats.avg_obtained_fps = calculate_processing_rate(&unit_get_data_stats);
        unit->stats.obtained_frames_cnt++;

        return rc;
}

static
void print_unit_stats(unit_st_t *unit)
{
        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return;
        }

        log_debug("Unit '%s' obtained frames count: %ld\n", unit->name,
                unit->stats.obtained_frames_cnt);
        log_debug("Unit '%s' sent frames count: %ld\n", unit->name,
                unit->stats.sent_frames_cnt);

        log_debug("Unit '%s' average obtaining FPS: %.2f\n", unit->name,
                unit->stats.avg_obtained_fps);
        log_debug("Unit '%s' average processing FPS: %.2f\n", unit->name,
                unit->stats.avg_processed_fps);
        log_debug("Unit '%s' average sending FPS: %.2f\n", unit->name,
                unit->stats.avg_sent_fps);
}

static
vf_err_t unit_data_handling_routine(unit_st_t *unit)
{
        vf_err_t rc = VF_SUCCESS;

        rc = unit_get_data(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to obtain data. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = unit_process_data(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to process data. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        rc = unit_send_data(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to send data. Error: %s\n", vf_err2str(rc));

                return rc;
        }

        print_unit_stats(unit);

        return rc;
}

static
void *unit_task(void *arg)
{
        vf_err_t rc = VF_SUCCESS;
        unit_st_t *unit = NULL;

        unit = (unit_st_t *)arg;
        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return NULL;
        }

        pthread_cleanup_push(unit_cleanup_handler, unit);

        // rc = vf_notifier_init(&unit->notifier);
        // if (VF_SUCCESS != rc) {
        //         log_error("Failed to initialize notifier. Error: %s\n", vf_err2str(rc));

        //         goto stop_unit_task;
        // }

        rc = init_unit(unit);
        if (VF_SUCCESS != rc) {
                log_error("Failed to initialize unit. Error: %s\n", vf_err2str(rc));

                goto stop_unit_task;
        }

        unit->state = UNIT_STATE_RUNNING;

        while (UNIT_STATE_RUNNING == unit->state) {
                pthread_testcancel();

                rc = unit_data_handling_routine(unit);
                if (VF_SUCCESS != rc) {
                        log_error("Failed to proceed with data handling routine. Error: %s\n",
                                 vf_err2str(rc));

                        goto stop_unit_task;
                }
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
                log_error("Invalid input: unit = %p\n", unit);

                return VF_INVALID_PARAMETER;
        }

        log_info("Creating unit (\'%s\') with type: (\'%s\')\n", SAFE_STR(unit->name),
                 unit_type2str(unit->type));

        err = pthread_create(&unit->tid, NULL, unit_task, unit);
        if (EOK != err) {
                log_error("Failed to start unit task. Error: %s\n", strerror(err));

                return VF_INIT_FAILED;
        }

        log_debug("Unit (\"%s\") thread ID is: %d\n", SAFE_STR(unit->name), unit->tid);

        return rc;
}

void vf_destroy_unit(unit_st_t *unit)
{
        int err = EOK;

        if (NULL == unit) {
                log_error("Invalid input: unit = %p\n", unit);

                return;
        }

        log_info("Destroing unit (\'%s\') with tid\n", SAFE_STR(unit->name), unit->tid);

        err = pthread_cancel(unit->tid);
        if (EOK != err) {
                log_error("Failed to cancel thread: %d. Error: %s\n", unit->tid, strerror(err));

                return;
        }

        log_debug("Waiting joining of thread with tid: %d\n", unit->tid);

        err = pthread_join(unit->tid, NULL);
        if (EOK != err) {
                log_error("Failed to join unit thread: %d. Error: %s\n", unit->tid, strerror(err));
        }

        unit->state = UNIT_STATE_STOPPED;

        log_info("Unit (\'%s\') destroing finished\n", SAFE_STR(unit->name));

        return;
}

