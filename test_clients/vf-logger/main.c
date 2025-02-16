#include <stdio.h>

#include "vf-logger.h"

int main() {
    enable_console_log(1);

    LOG_MESSAGE(LOG_INFO, "Application initialization...");
    LOG_MESSAGE(LOG_WARNING, "Warning: Improper use of resources!");
    LOG_MESSAGE(LOG_ERROR, "Error accessing the database!");
    LOG_MESSAGE(LOG_DEBUG, "Debug details: function X is called.");
    LOG_MESSAGE(LOG_FATAL, "FATAL details: function X is called.");

    return 0;
}

