#include "rtos/mutex.hpp"
#include "rtos/defs.hpp"

namespace wrvcu {

void Mutex::init() {
    if (isOnStack((void*)this)) {
        printf("WARNING: Static mutex allocated on the stack! This WILL cause severe problems.\n");
        configASSERT(0); // assert error
    } else {
        /* ok*/
    };

    if (mutex == NULL)
        mutex = xSemaphoreCreateMutexStatic(&mutexBuffer);

    if (mutex == NULL) {
        printf("Mutex was not created!\n");
        configASSERT(mutex);
    } else { // ok
    };
}

bool Mutex::take() {
    return take(TIMEOUT_MAX);
}

bool Mutex::take(std::uint32_t timeout) {
    init();
    return xSemaphoreTake(mutex, pdMS_TO_TICKS(timeout));
}

bool Mutex::give() {
    init();
    return xSemaphoreGive(mutex);
}

} // namespace wrvcu