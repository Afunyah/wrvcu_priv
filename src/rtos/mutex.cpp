#include "rtos/mutex.hpp"
#include "rtos/defs.hpp"

namespace wrvcu {

Mutex::Mutex() :
    // Define a creator and deleter for the underlying FreeRTOS Mutex
    // Use a lambda for the deleter because vSemaphoreDelete is a macro not a function.
    mutex(xSemaphoreCreateMutexStatic(&mutexBuffer), [](SemaphoreHandle_t mutex) { vSemaphoreDelete(mutex); }) {
    if (isOnStack((void*)this)) {
        printf("WARNING: Static mutex allocated on the stack! This WILL cause severe problems.\n");
        // configASSERT(0); // assert error
    } else { /* ok*/
    };
    if (mutex.get() == NULL) {
        printf("Mutex was not created!\n");
        configASSERT(mutex);
    } else { // ok
    };
}

bool Mutex::take() {
    return take(TIMEOUT_MAX);
}

bool Mutex::take(std::uint32_t timeout) {
    return xSemaphoreTake(mutex.get(), pdMS_TO_TICKS(timeout));
}

bool Mutex::give() {
    return xSemaphoreGive(mutex.get());
}

} // namespace wrvcu