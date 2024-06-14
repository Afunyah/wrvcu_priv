#include "arduino_freertos.h"
#include "constants.hpp"
#include "pins.hpp"
#include "rtos/rtos.hpp"
#include "TractiveSystem.hpp"
#include "devices/throttleManager.hpp"
#include "car.hpp"


using namespace wrvcu;

static Task taskA;
// static TractiveSystem ts;
static ThrottleManager throttleT;

void test_throttle_task() {
    Task::delay(3000);

    ts.init();

    ADC adc;
    adc.init(ADC_CS, &SPI, false);

    throttleT.init(&adc);

    Serial.println("start of task");

    while (true) {
        Serial.print("APPS 1: ");
        Serial.print(throttleT.APPS1.getSaturatedFraction());
        Serial.print("\tAPPS 2: ");
        Serial.println(throttleT.APPS2.getSaturatedFraction());
        throttleT.getTorqueRequestFraction();
        Task::delay(200);
    }
}

void test_throttle_func() {
    pinMode(LED_BUILTIN, OUTPUT);
    // digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);

    taskA.start(test_throttle_task, TASK_PRIORITY_DEFAULT, "Throttle");

    startScheduler();
}