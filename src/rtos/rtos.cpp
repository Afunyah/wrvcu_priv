#include "rtos/rtos.hpp"

extern unsigned long _estack; // stack top

namespace wrvcu {

void startScheduler() {
    Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    vTaskStartScheduler();

    Serial.println("Scheduler returned.");
    Serial.flush();
};

bool isOnStack(void* ptr) {
    /* MISRA deviation:
     * 11.4 and 11.6 are suppressed here, since a cast from pointers to ints is required here for the comparison.
     * We are trying to reason about the memory location of something, and compare it to another memory location,
     * so fundamentally this cast is required.
     */
    // cppcheck-suppress  misra-c2012-11.4
    // cppcheck-suppress  misra-c2012-11.6
    return ((long)ptr) < ((long)(&_estack));
}

} // namespace wrvcu