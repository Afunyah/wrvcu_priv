#include "arduino_freertos.h"
#include <can/CANController_T4.hpp>

using namespace wrvcu;

// using namespace wrvcu;

void test_rtos();
void test_can();

void setup() {
    Serial.begin(115200); // wait up to 2 seconds for serial connection

    if (CrashReport) {
        Serial.print(CrashReport);
        Serial.println();
        Serial.flush();
    }

    pinMode(LED_BUILTIN, OUTPUT);

    // test_rtos();
    test_can();
}

void loop() {
}