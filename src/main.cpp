#include "arduino_freertos.h"
#include "rtos/rtos.hpp"

using namespace wrvcu;

void task1() {
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        Task::delay(500);

        digitalWrite(LED_BUILTIN, LOW);
        Task::delay(500);
    }
}

void task2() {
    while (true) {
        Serial.println("TICK");
        Task::delay(1000);

        Serial.println("TOCK");
        Task::delay(1000);
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Task(task1, "task1");
    Task(task2, "task2");

    startScheduler();
}

void loop() {}