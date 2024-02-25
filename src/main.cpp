#include "arduino_freertos.h"

// using namespace wrvcu;

void test_rtos();

void setup() {
    Serial.begin(115200); // wait up to 2 seconds for serial connection

    if (CrashReport) {
        Serial.print(CrashReport);
        Serial.println();
        Serial.flush();
    }

    test_rtos();
}

void loop() {}