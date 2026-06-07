/**
 **************************************************************************************************
 *  @file           : vf-control-server.h
 *  @brief          : VisionFlow Control Server Header
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Unix domain socket server for runtime control of the VisionFlow
 *  pipeline service. Accepts client connections, receives commands
 *  and dispatches them to the control handler for processing.
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#ifndef VF_CONTROL_SERVER_H
#define VF_CONTROL_SERVER_H

#include <pthread.h>
#include <stdatomic.h>

#include "vf-error.h"
#include "vf-service.h"

#define VF_CONTROL_SOCKET_PATH     "/tmp/vf_control.sock"
#define VF_CONTROL_MAX_CONNECTIONS 4

typedef struct {
        int              server_fd;
        pthread_t        thread;
        atomic_bool      running;
        vf_service_t    *svc;
        int              initialized;
} vf_control_server_t;

vf_err_t vf_control_server_init(vf_control_server_t *srv, vf_service_t *svc);
vf_err_t vf_control_server_start(vf_control_server_t *srv);
vf_err_t vf_control_server_stop(vf_control_server_t *srv);
void vf_control_server_deinit(vf_control_server_t *srv);

#endif /* VF_CONTROL_SERVER_H */

