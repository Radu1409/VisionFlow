/**
 **************************************************************************************************
 *  @file           : vf-control-handler.c
 *  @brief          : VisionFlow Control Handler
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Command handler for the VisionFlow control server. Parses incoming
 *  JSON commands, dispatches them to the appropriate service operation
 *  and serializes the response back to the client.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "vf-control-handler.h"
#include "vf-error.h"
#include "vf-logger.h"
#include "vf-service.h"

#define VF_CONTROL_UNIT_BUF_SIZE   128U
#define VF_CONTROL_UNITS_JSON_SIZE 768U

#define MODULE_NAME                "vf_control_handler"

/* =========================================================================
 * Internal helpers
 * ========================================================================= */

static
void send_response(int client_fd, const char *response)
{
        ssize_t written = 0;
        size_t len = 0;

        if ((client_fd < 0) || (NULL == response)) {
                log_err("Invalid params: client_fd=%d response=%p",
                        client_fd, (void *)response);

                return;
        }

        len = strlen(response);
        written = write(client_fd, response, len);
        if (written < 0) {
                log_err("Failed to write response to client");
        }
}

static
void send_error(int client_fd, const char *message)
{
        char response[VF_CONTROL_RESPONSE_MAX_LEN] = {0};

        if (NULL == message) {
                log_err("Invalid param: message=%p", (void *)message);

                return;
        }

        (void)snprintf(response, sizeof(response),
                       "{\"status\":\"error\",\"message\":\"%s\"}\n",
                       message);

        send_response(client_fd, response);
}

static
void handle_get_status(int client_fd, vf_service_t *svc)
{
        char response[VF_CONTROL_RESPONSE_MAX_LEN] = {0};

        if (NULL == svc) {
                log_err("Invalid param: svc=%p", (void *)svc);

                return;
        }

        (void)snprintf(response, sizeof(response),
                       "{\"status\":\"ok\","
                       "\"data\":{\"service\":\"%s\"}}\n",
                       svc->initialized ? "running" : "stopped");

        send_response(client_fd, response);

        log_info("Control handler: get_status served");
}

static
void handle_get_stats(int client_fd, vf_service_t *svc)
{
        char response[VF_CONTROL_RESPONSE_MAX_LEN] = {0};
        char units_json[VF_CONTROL_UNITS_JSON_SIZE] = {0};
        char unit_buf[VF_CONTROL_UNIT_BUF_SIZE] = {0};
        uint32_t i = 0U;
        uint64_t avg_get = 0U;
        uint64_t avg_process = 0U;
        uint64_t avg_send = 0U;
        const uint32_t unit_count = 4U;

        if (NULL == svc) {
                log_err("Invalid param: svc=%p", (void *)svc);

                return;
        }

        const vf_unit_t *units[] = {
                &svc->camera_unit,
                &svc->sp_unit,
                &svc->conv_unit,
                &svc->file_out_unit,
        };

        for (i = 0U; i < unit_count; i++) {
                const vf_unit_t *unit = units[i];
                const vf_unit_stats_t *stats = &unit->stats;

                avg_get = 0U;
                avg_process = 0U;
                avg_send = 0U;

                if (stats->frames_processed > 0U) {
                        avg_get = stats->total_get_data_ms / stats->frames_processed;
                        avg_process = stats->total_process_data_ms / stats->frames_processed;
                        avg_send = stats->total_send_data_ms / stats->frames_processed;
                }

                (void)snprintf(unit_buf, sizeof(unit_buf),
                               "%s{"
                               "\"name\":\"%s\","
                               "\"frames\":%u,"
                               "\"avg_get_ms\":%llu,"
                               "\"avg_process_ms\":%llu,"
                               "\"avg_send_ms\":%llu"
                               "}",
                               (i > 0U) ? "," : "",
                               unit->name ? unit->name : "unknown",
                               stats->frames_processed,
                               (unsigned long long)avg_get,
                               (unsigned long long)avg_process,
                               (unsigned long long)avg_send);

                (void)strncat(units_json, unit_buf,
                              sizeof(units_json) - strlen(units_json) - 1U);
        }

        (void)snprintf(response, sizeof(response),
                       "{\"status\":\"ok\","
                       "\"data\":{\"units\":[%s]}}\n",
                       units_json);

        send_response(client_fd, response);

        log_info("Control handler: get_stats served");
}

static
void handle_stop(int client_fd, vf_service_t *svc)
{
        vf_err_t err = VF_SUCCESS;

        if (NULL == svc) {
                log_err("Invalid param: svc=%p", (void *)svc);

                return;
        }

        err = vf_service_stop(svc);
        if (VF_SUCCESS != err) {
                log_err("Control handler: failed to stop service: %s",
                        vf_err2str(err));

                send_error(client_fd, "failed to stop service");

                return;
        }

        send_response(client_fd,
                      "{\"status\":\"ok\","
                      "\"message\":\"service stopped\"}\n");

        log_info("Control handler: stop served");
}

static
void handle_start(int client_fd, vf_service_t *svc)
{
        vf_err_t err = VF_SUCCESS;

        if (NULL == svc) {
                log_err("Invalid param: svc=%p", (void *)svc);

                return;
        }

        err = vf_service_start(svc);
        if (VF_SUCCESS != err) {
                log_err("Control handler: failed to start service: %s",
                        vf_err2str(err));

                send_error(client_fd, "failed to start service");

                return;
        }

        send_response(client_fd,
                      "{\"status\":\"ok\","
                      "\"message\":\"service started\"}\n");

        log_info("Control handler: start served");
}

/* =========================================================================
 * Public API
 * ========================================================================= */

vf_err_t vf_control_handle_request(int client_fd, vf_service_t *svc)
{
        char buf[VF_CONTROL_BUFFER_SIZE] = {0};
        ssize_t bytes_read = 0;

        if ((client_fd < 0) || (NULL == svc)) {
                log_err("Invalid params: client_fd=%d svc=%p",
                        client_fd, (void *)svc);

                return VF_INVALID_PARAMETER;
        }

        bytes_read = read(client_fd, buf, sizeof(buf) - 1U);
        if (bytes_read <= 0) {
                log_err("Failed to read request from client");

                return VF_FILE_ERR;
        }

        buf[bytes_read] = '\0';

        log_info("Control handler: received request: '%s'", buf);

        if (NULL != strstr(buf, "\"cmd\":\"get_status\"")) {
                handle_get_status(client_fd, svc);
        } else if (NULL != strstr(buf, "\"cmd\":\"get_stats\"")) {
                handle_get_stats(client_fd, svc);
        } else if (NULL != strstr(buf, "\"cmd\":\"stop\"")) {
                handle_stop(client_fd, svc);
        } else if (NULL != strstr(buf, "\"cmd\":\"start\"")) {
                handle_start(client_fd, svc);
        } else {
                log_err("Control handler: unknown command: '%s'", buf);

                send_error(client_fd, "unknown command");
        }

        return VF_SUCCESS;
}

