#include "rtos/rtos.hpp"

namespace wrvcu {

void startScheduler() {
    Serial.printf("Booting FreeRTOS kernel %s\n", tskKERNEL_VERSION_NUMBER);
    vTaskStartScheduler();
};

} // namespace wrvcu