#include "Logger/logger.h"
#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <cstring>
using namespace std;


int main(int argc, char *argv[]) {

    initLogger();

    Disk disk_run;
    StaticBuffer buffer;
    OpenRelTable cache;

    int ret = FrontendInterface::handleFrontend(argc, argv);

    fprintf(logFile, "Closing NITCBase\n");
    fflush(logFile);

    fclose(logFile);

    return ret;
}