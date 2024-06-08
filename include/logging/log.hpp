#pragma once
#include "Arduino.h"

enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};

enum class LogLocation {
    STDOUT,
    FILE,
    RADIO
};

void setLogLevel(LogLocation location, LogLevel level);
LogLevel getLogLevel(LogLocation location);

void DEBUG(const char* message);
void DEBUG(const String message);
void INFO(const char* message);
void INFO(const String message);
void WARN(const char* message);
void WARN(const String message);
void ERROR(const char* message);
void ERROR(const String message);

void DEBUG(const char* module, const char* message);
void DEBUG(const String module, const String message);
void INFO(const char* module, const char* message);
void INFO(const String module, const String message);
void WARN(const char* module, const char* message);
void WARN(const String module, const String message);
void ERROR(const char* module, const char* message);
void ERROR(const String module, const String message);

void log(LogLevel level, const char* message);
void log(LogLevel level, const String message);

void log(LogLevel level, const char* module, const char* message);
void log(LogLevel level, const String module, const String message);

const char* logLevelAsString(LogLevel level);

void loggingInit();