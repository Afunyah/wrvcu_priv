#include "arduino_freertos.h"

// using namespace wrvcu;

void test_rtos();

extern unsigned long _estack; // stack top

bool is_on_stack(void* ptr) {
    /* MISRA deviation:
     * 11.4 and 11.6 are suppressed here, since a cast from pointers to ints is required here for the comparison.
     * We are trying to reason about the memory location of something, and compare it to another memory location,
     * so fundamentally this cast is required.
     */
    // cppcheck-suppress  misra-c2012-11.4
    // cppcheck-suppress  misra-c2012-11.6
    return ((long)ptr) < ((long)(&_estack));
}

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