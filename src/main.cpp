#include "arduino_freertos.h"

// using namespace wrvcu;

void test_rtos();
void test_can();

void test_inverter();

void setup() {
    Serial.begin(115200); // wait up to 2 seconds for serial connection

    if (CrashReport) {
        Serial.print(CrashReport);
        Serial.println();
        Serial.flush();
    }

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // test_rtos();
    // test_can();

    test_inverter();

    // test_logging();
    // test_can();
}

void loop() {
}