/**
 **************************************************************************************************
 *  @file           : vf-control-handler.h
 *  @brief          : VisionFlow Control Handler Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Command handler for the VisionFlow control server. Parses incoming
 *  JSON commands, dispatches them to the appropriate service operation
 *  and serializes the response back to the client.
 *
 *  Supported commands:
 *    get_status — returns service running state and uptime
 *    get_stats  — returns per-unit frame count and latency averages
 *    stop       — triggers graceful pipeline shutdown
 *    start      — starts the pipeline if stopped
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CONTROL_HANDLER_H
#define VF_CONTROL_HANDLER_H

#include "vf-error.h"
#include "vf-service.h"

#define VF_CONTROL_BUFFER_SIZE      1024U
#define VF_CONTROL_CMD_MAX_LEN      64U
#define VF_CONTROL_RESPONSE_MAX_LEN 1024U

vf_err_t vf_control_handle_request(int client_fd, vf_service_t *svc);

#endif /* VF_CONTROL_HANDLER_H */

