#include "rtos/rtos.hpp"

namespace wrvcu {

void startScheduler() {
    Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    vTaskStartScheduler();

    Serial.println("Scheduler returned.");
    Serial.flush();
};

} // namespace wrvcu