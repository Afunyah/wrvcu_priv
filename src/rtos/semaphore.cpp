#include "rtos/semaphore.hpp"
#include "rtos/defs.hpp"

namespace wrvcu {

Semaphore::Semaphore(uint32_t max_count, uint32_t init_count) : // cppcheck-suppress misra-c2012-2.7; (False positive for unused parameter)
    sem(xQueueCreateCountingSemaphoreStatic(max_count, init_count, &semBuffer), [](SemaphoreHandle_t sem) { vSemaphoreDelete(sem); }) {
    if (isOnStack((void*)this)) {
        printf("WARNING: Static semaphore allocated on the stack! This WILL cause severe problems.\n");
        // configASSERT(0); // assert error
    } else { /* ok*/
    };
    if (sem.get() == NULL) {
        printf("Semaphore was not created!\n");
        configASSERT(sem);
    } else { // ok
    };
};

uint32_t Semaphore::get_count() {
    return uxSemaphoreGetCount(sem.get());
}

bool Semaphore::post() {
    return xSemaphoreGive(sem.get()) == pdTRUE;
}

bool Semaphore::wait(uint32_t timeout) {
    return xSemaphoreTake(sem.get(), pdMS_TO_TICKS(timeout));
}

} // namespace wrvcu