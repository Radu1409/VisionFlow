/**
 **************************************************************************************************
 *  @file           : vf-control-server.c
 *  @brief          : VisionFlow Control Server
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

#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "vf-control-handler.h"
#include "vf-control-server.h"
#include "vf-error.h"
#include "vf-logger.h"

#define MODULE_NAME "vf_control_server"

static
void *vf_control_server_thread_fn(void *arg)
{
        vf_control_server_t *srv = NULL;
        int client_fd = -1;
        vf_err_t err = VF_SUCCESS;

        if (NULL == arg) {
                log_err("Invalid input: arg = %p", (void *)arg);

                return NULL;
        }

        srv = (vf_control_server_t *)arg;

        log_info("Control server thread started — listening on '%s'",
                 VF_CONTROL_SOCKET_PATH);

        while (atomic_load(&srv->running)) {
                client_fd = accept(srv->server_fd, NULL, NULL);
                if (client_fd < 0) {
                        if (!atomic_load(&srv->running)) {
                                break;
                        }

                        log_err("Failed to accept connection");

                        continue;
                }

                log_info("Control server: client connected");

                err = vf_control_handle_request(client_fd, srv->svc);
                if (VF_SUCCESS != err) {
                        log_err("Failed to handle request: %s", vf_err2str(err));
                }

                (void)close(client_fd);

                log_info("Control server: client disconnected");
        }

        log_info("Control server thread stopped");

        return NULL;
}

vf_err_t vf_control_server_init(vf_control_server_t *srv, vf_service_t *svc)
{
        struct sockaddr_un addr = {0};
        int rc = 0;

        if ((NULL == srv) || (NULL == svc)) {
                log_err("Invalid params: srv=%p svc=%p",
                        (void *)srv, (void *)svc);

                return VF_INVALID_PARAMETER;
        }

        (void)memset(srv, 0, sizeof(*srv));

        srv->server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (srv->server_fd < 0) {
                log_err("Failed to create Unix socket");

                return VF_SYNC_ERROR;
        }

        rc = unlink(VF_CONTROL_SOCKET_PATH);
        if (EOK != rc) {
                if (ENOENT != errno) {
                        log_err("Failed to remove existing socket '%s': %s",
                                VF_CONTROL_SOCKET_PATH, strerror(errno));
                }
        }

        addr.sun_family = AF_UNIX;

        (void)strncpy(addr.sun_path, VF_CONTROL_SOCKET_PATH,
                      sizeof(addr.sun_path) - 1U);

        rc = bind(srv->server_fd, (struct sockaddr *)&addr, sizeof(addr));
        if (rc < 0) {
                log_err("Failed to bind Unix socket to '%s'",
                        VF_CONTROL_SOCKET_PATH);

                (void)close(srv->server_fd);

                srv->server_fd = -1;

                return VF_SYNC_ERROR;
        }

        rc = listen(srv->server_fd, VF_CONTROL_MAX_CONNECTIONS);
        if (rc < 0) {
                log_err("Failed to listen on Unix socket");

                (void)close(srv->server_fd);

                srv->server_fd = -1;

                return VF_SYNC_ERROR;
        }

        srv->svc = svc;
        srv->initialized = 1;

        log_info("Control server initialized: socket='%s'",
                 VF_CONTROL_SOCKET_PATH);

        return VF_SUCCESS;
}

vf_err_t vf_control_server_start(vf_control_server_t *srv)
{
        int rc = 0;

        if (NULL == srv) {
                log_err("Invalid input: srv = %p", (void *)srv);

                return VF_INVALID_PARAMETER;
        }

        if (0 == srv->initialized) {
                log_err("Control server not initialized");

                return VF_INIT_FAILED;
        }

        atomic_store(&srv->running, true);

        rc = pthread_create(&srv->thread, NULL,
                            vf_control_server_thread_fn, srv);
        if (EOK != 0) {
                log_err("Failed to create control server thread. Error: %d", rc);

                atomic_store(&srv->running, false);

                return VF_SYNC_ERROR;
        }

        log_info("Control server started");

        return VF_SUCCESS;
}

vf_err_t vf_control_server_stop(vf_control_server_t *srv)
{
        int rc = 0;

        if (NULL == srv) {
                log_err("Invalid input: srv = %p", (void *)srv);

                return VF_INVALID_PARAMETER;
        }

        if (0 == srv->initialized) {
                log_err("Control server not initialized");

                return VF_INIT_FAILED;
        }

        atomic_store(&srv->running, false);

        if (srv->server_fd >= 0) {
                (void)shutdown(srv->server_fd, SHUT_RDWR);
        }

        rc = pthread_join(srv->thread, NULL);
        if (EOK != 0) {
                log_err("Failed to join control server thread. Error: %d", rc);

                return VF_SYNC_ERROR;
        }

        log_info("Control server stopped");

        return VF_SUCCESS;
}

void vf_control_server_deinit(vf_control_server_t *srv)
{
        int rc = 0;

        if (NULL == srv) {
                log_err("Invalid input: srv = %p", (void *)srv);

                return;
        }

        if (0 == srv->initialized) {
                log_err("Control server not initialized");

                return;
        }

        if (srv->server_fd >= 0) {
                (void)close(srv->server_fd);

                srv->server_fd = -1;
        }

        rc = unlink(VF_CONTROL_SOCKET_PATH);
        if (EOK != rc) {
                if (ENOENT != errno) {
                        log_err("Failed to remove existing socket '%s': %s",
                                VF_CONTROL_SOCKET_PATH, strerror(errno));
                }
        }

        (void)memset(srv, 0, sizeof(*srv));

        log_info("Control server deinitialized");
}

