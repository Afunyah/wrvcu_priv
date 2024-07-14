#include "TractiveSystem.hpp"
#include "arduino_freertos.h"
#include "constants.hpp"
#include "devices/throttleManager.hpp"
#include "pins.hpp"
#include "rtos/rtos.hpp"

using namespace wrvcu;

static Task taskA;
static TractiveSystem ts;
static ThrottleManager throttleT;
static ADC adc(ADC_CS, &SPI);

void test_throttle_task() {
    Task::delay(3000);

    ts.init();

    adc.init(false);

    throttleT.init(&adc);

    // Serial.println("start of task");

    while (true) {
        // Serial.print("APPS 1: ");
        // Serial.print(throttleT.APPS1.read());
        // Serial.print("\tAPPS 2: ");
        // Serial.println(throttleT.APPS2.read());
        // throttleT.APPS1.getSaturatedFraction();
        // Serial.println(throttleT.getBrakeRegenFraction());
        Serial.println(throttleT.getTorqueRequestFraction());

        // in tractivesystem.cpp
        // Serial.print("Brake 1: ");
        // Serial.print(ts.getBrakePressure1());
        // Serial.print("\tBrake 2: ");
        // Serial.println(ts.getBrakePressure2());
        Task::delay(10);
    }
}

void test_throttle_func() {
    pinMode(LED_BUILTIN, OUTPUT);
    // digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);

    taskA.start(test_throttle_task, TASK_PRIORITY_DEFAULT, "Throttle");

    startScheduler();
}