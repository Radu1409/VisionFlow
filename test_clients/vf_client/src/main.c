/**
 **************************************************************************************************
 *  @file           : main.c
 *  @brief          : VisionFlow Control Client
 **************************************************************************************************
 *  @author     Radu Purecel
 *
 *  @description:
 *  Command-line client for the VisionFlow control server. Connects to the
 *  Unix domain socket, sends a JSON command and prints the response.
 *
 *  Usage:
 *    vf_client --get_status
 *    vf_client --get_stats
 *    vf_client --stop
 *    vf_client --start
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define VF_CONTROL_SOCKET_PATH "/tmp/vf_control.sock"
#define VF_CLIENT_BUFFER_SIZE  2048U

static
void print_usage(const char *prog)
{
        fprintf(stderr, "Usage: %s <command>\n", prog);
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "  --get_status   Get service running status\n");
        fprintf(stderr, "  --get_stats    Get per-unit frame stats\n");
        fprintf(stderr, "  --stop         Stop the pipeline\n");
        fprintf(stderr, "  --start        Start the pipeline\n");
}

static
const char *flag_to_cmd(const char *flag)
{
        if (0 == strcmp(flag, "--get_status")) {
                return "{\"cmd\":\"get_status\"}\n";
        }

        if (0 == strcmp(flag, "--get_stats")) {
                return "{\"cmd\":\"get_stats\"}\n";
        }

        if (0 == strcmp(flag, "--stop")) {
                return "{\"cmd\":\"stop\"}\n";
        }

        if (0 == strcmp(flag, "--start")) {
                return "{\"cmd\":\"start\"}\n";
        }

        return NULL;
}

int main(int argc, char *argv[])
{
        struct sockaddr_un addr     = {0};
        char               buf[VF_CLIENT_BUFFER_SIZE] = {0};
        const char        *cmd     = NULL;
        int                fd      = -1;
        int                rc      = 0;
        ssize_t            written = 0;
        ssize_t            bytes   = 0;

        if (argc != 2) {
                print_usage(argv[0]);

                return EXIT_FAILURE;
        }

        cmd = flag_to_cmd(argv[1]);
        if (NULL == cmd) {
                fprintf(stderr, "Unknown command: '%s'\n", argv[1]);

                print_usage(argv[0]);

                return EXIT_FAILURE;
        }

        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) {
                fprintf(stderr, "Failed to create socket\n");

                return EXIT_FAILURE;
        }

        addr.sun_family = AF_UNIX;

        (void)strncpy(addr.sun_path, VF_CONTROL_SOCKET_PATH,
                      sizeof(addr.sun_path) - 1U);

        rc = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
        if (rc < 0) {
                fprintf(stderr, "Failed to connect to '%s' — is vf_service running?\n",
                        VF_CONTROL_SOCKET_PATH);

                (void)close(fd);

                return EXIT_FAILURE;
        }

        written = write(fd, cmd, strlen(cmd));
        if (written < 0) {
                fprintf(stderr, "Failed to send command\n");

                (void)close(fd);

                return EXIT_FAILURE;
        }

        bytes = read(fd, buf, sizeof(buf) - 1U);
        if (bytes <= 0) {
                fprintf(stderr, "Failed to read response\n");

                (void)close(fd);

                return EXIT_FAILURE;
        }

        buf[bytes] = '\0';

        printf("%s", buf);

        (void)close(fd);

        return EXIT_SUCCESS;
}

