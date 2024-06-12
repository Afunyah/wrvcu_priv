#include "logging/log.hpp"
#include "SD.h"
#include <rtos/queue.hpp>
#include <rtos/task.hpp>

#define LOGGING_TASK_PRIORITY (TASK_PRIORITY_DEFAULT - 3)

static LogLevel _levels[] = {
    LogLevel::INFO,  // STDOUT
    LogLevel::DEBUG, // FILE
    LogLevel::WARN   // RADIO
};

const char LEVEL_DEBUG[] = "DEBUG";
const char LEVEL_INFO[] = "INFO";
const char LEVEL_WARN[] = "WARNING";
const char LEVEL_ERROR[] = "ERROR";

const char* const LOG_LEVEL_STRINGS[] = {
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
};

struct LogMessage {
    LogLevel level;
    const char* module;
    const char* message;
};

static wrvcu::Task loggingTask;
static wrvcu::Queue<LogMessage, 512> loggingQueue;
static File logFile;

void setLogLevel(LogLocation location, LogLevel level) {
    _levels[static_cast<int>(location)] = level;
}

LogLevel getLogLevel(LogLocation location) {
    return _levels[static_cast<int>(location)];
}

void DEBUG(const char* message) {
    log(LogLevel::DEBUG, message);
}

void DEBUG(const String message) {
    log(LogLevel::DEBUG, message);
}
void INFO(const char* message) {
    log(LogLevel::INFO, message);
}

void INFO(const String message) {
    log(LogLevel::INFO, message);
}
void WARN(const char* message) {
    log(LogLevel::WARN, message);
}

void WARN(const String message) {
    log(LogLevel::WARN, message);
}
void ERROR(const char* message) {
    log(LogLevel::ERROR, message);
}

void ERROR(const String message) {
    log(LogLevel::ERROR, message);
}

void DEBUG(const char* module, const char* message) {
    log(LogLevel::DEBUG, module, message);
}

void DEBUG(const String module, const String message) {
    log(LogLevel::DEBUG, module, message);
}
void INFO(const char* module, const char* message) {
    log(LogLevel::INFO, module, message);
}

void INFO(const String module, const String message) {
    log(LogLevel::INFO, module, message);
}
void WARN(const char* module, const char* message) {
    log(LogLevel::WARN, module, message);
}

void WARN(const String module, const String message) {
    log(LogLevel::WARN, module, message);
}
void ERROR(const char* module, const char* message) {
    log(LogLevel::ERROR, module, message);
}

void ERROR(const String module, const String message) {
    log(LogLevel::ERROR, module, message);
}

void log(LogLevel level, const char* message) {
    log(level, "", message);
}

void log(LogLevel level, const String message) {
    log(level, "", message);
}

/**
 * @brief The underlying function that receives all log messages.
 * Forwards to other functions that logs to various locations.
 */
void log(LogLevel level, const char* module, const char* message) {
    LogMessage msg = {
        .level = level,
        .module = module,
        .message = message
    };

    loggingQueue.enqueue(msg, 0);
}

void log(LogLevel level, const String module, const String message) {
    log(level, module.c_str(), message.c_str());
}

const char* logLevelAsString(LogLevel level) {
    return LOG_LEVEL_STRINGS[static_cast<int>(level)];
}

// ------------------ Raw logging functions ------------------

void writeLogToFlash(LogLevel level, const char* module, const char* message) {
    if (logFile) {
        logFile.print(F("["));

        logFile.print(logLevelAsString(level));

        logFile.print(F("] "));

        if (strlen(module) > 0) {
            logFile.print(F(": "));
            logFile.print(module);
            logFile.print(F(" "));
        }

        logFile.println(message);
        logFile.flush();
    }
}

/**
 * @brief Write logging messages to serial stdout.
 */
void writeLogToSerial(LogLevel level, const char* module, const char* message) {
    Serial.print(F("["));

    Serial.print(logLevelAsString(level));

    Serial.print(F("] "));

    if (strlen(module) > 0) {
        Serial.print(F(": "));
        Serial.print(module);
        Serial.print(F(" "));
    }

    Serial.println(message);
}

// ------------------ Setup and run ------------------

void loggingLoop() {
    while (true) {
        LogMessage msg = loggingQueue.dequeue(TIMEOUT_MAX);

        if (msg.level >= getLogLevel(LogLocation::STDOUT)) {
            writeLogToSerial(msg.level, msg.module, msg.message);
        }

        if (msg.level >= getLogLevel(LogLocation::FILE)) {
            // writeLogToFlash(msg.level, msg.module, msg.message);
        }

        if (msg.level >= getLogLevel(LogLocation::RADIO)) {
            // writeLogToRadio(level, module, message);
        }

        wrvcu::Task::delay(0);
    }
}

void loggingInit() {
    loggingQueue.init();

    // TODO: rename with date

    // If a log file exists, rename it to log<x>, where x is the smallest number available
    if (SD.exists("log.txt")) {
        int i = 1;
        char newName[] = "log1.txt";

        while (SD.exists(newName) && i < 1000) {
            i++;
            newName[3] = i + '0';
        }

        SD.rename("log.txt", newName);
    }

    logFile = SD.open("log.txt", FILE_WRITE);

    loggingTask.start([] { loggingLoop(); }, LOGGING_TASK_PRIORITY, "Logging_Task");
}
