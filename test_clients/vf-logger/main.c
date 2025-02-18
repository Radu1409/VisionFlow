#include <stdio.h>

#include "vf-logger.h"

int main() {
    enable_console_log(1);

    log_info("Application initialization...\n");
    log_warning("Warning: Improper use of resources!\n");
    log_error("Error accessing the database!\n");
    log_debug("Debug details: function X is called.\n");
    //log_fatal("FATAL details: function X is called.\n");

    return 0;
}

