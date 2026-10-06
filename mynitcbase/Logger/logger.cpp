#include "logger.h"

FILE *logFile = fopen(
    "/home/nitcbase/NITCbase/mynitcbase/Logger/log.txt",
    "w"
);

void initLogger() {
    if (logFile == NULL) {
        perror("Failed to open log file");
    }
    else{
      fprintf(logFile, "Starting NITCBase\n");

    }
}