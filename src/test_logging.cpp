#include "SD.h"
#include "arduino_freertos.h"
#include "logging/log.hpp"
#include "rtos/rtos.hpp"
// #include <MTP_Teensy.h>
#include <TimeLib.h>

using namespace wrvcu;

static Task taskA;
void test_logging_task() {
    while (true) {
        DEBUG("This is a test debug message");
        INFO("This is a test info message");
        WARN("This is a test warn message");
        ERROR("This is a test error message");

        Task::delay(1000);
    }
}

static Task taskB;
void test_mtp_task() {
    while (true) {
        // MTP.loop();

        Task::delay(50);
    }
}

void test_logging() {
    SD.begin(BUILTIN_SDCARD);
    loggingInit();

    // MTP.begin();
    // if (SD.mediaPresent()) {
    //     INFO("SD Card present");
    //     MTP.addFilesystem(SD, "SD");
    // } else {
    //     WARN("SD Card failed, or is not present.");
    // }

    setLogLevel(LogLocation::STDOUT, LogLevel::WARN);

    taskA.start(test_logging_task, TASK_PRIORITY_DEFAULT, "Log");
    // taskB.start(test_mtp_task, TASK_PRIORITY_DEFAULT - 4, "MTP");

    startScheduler();
}