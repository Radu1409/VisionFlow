/**
 **************************************************************************************************
 *  @file           : vf-core.c
 *  @brief          : VF main routine
 **************************************************************************************************
 *  @author     Radu-Ioan Purecel
 *
 *  @details:
 *    The main process initializes necessary VF resources and starts processing pipelines threads
 *
 *  @section  HISTORY
 *  v1.0  - First version
 *
 **************************************************************************************************
 **/

#include <signal.h>
#include <sys/procmgr.h>

#include "vf-logger.h"
#include "vf-parser.h"
#include "vf-pipeline-mgr.h"

pipeline_mgr_ctx_st_t *pipeline_mgr_ctx = NULL;

static
void handle_signal(int signal)
{
        log_info("Signal %d received\n", signal);
}

static
void deinit_service(pipeline_mgr_ctx_st_t *ctx)
{
        vf_destroy_pipelines(ctx);

        vf_log_deinit();
}

 int main(int argc, char *argv[])
 {
        vf_err_t rc = VF_SUCCESS;
        int err = EOK;

        // err = vf_log_init(APP_NAME);
        // if (VF_SUCCESS != err) {
        //         log_err("Failed to init logger. Error: %s\n", vf_err2str(err));

        //         return VF_INIT_FAILED;
        // }

        rc = vf_get_pipeline_mgr_ctx(&pipeline_mgr_ctx);
        if (VF_SUCCESS != rc) {
                log_err("Failed to obtain pipeline manager settings. Error: %s\n", vf_err2str(rc));

                goto vf_init_failed;
        }

        rc = vf_create_pipelines(pipeline_mgr_ctx);
        if (VF_SUCCESS != rc) {
                log_err("Failed to create pipelines. Error: %s\n", vf_err2str(rc));

                goto vf_init_failed;
        }

        err = procmgr_daemon(0, PROCMGR_DAEMON_NODEVNULL);
        if (-1 == err) {
                log_err("Failed to daemonize. Error: %s\n", strerror(errno));

                rc = VF_INIT_FAILED;

                goto vf_init_failed;
        }

        signal(SIGTERM, handle_signal);
        signal(SIGQUIT, handle_signal);
        signal(SIGINT, handle_signal);
        signal(SIGALRM, handle_signal);

        pause();

vf_init_failed:
        deinit_service(pipeline_mgr_ctx);

        return rc;
}

