#include "rtos/semaphore.hpp"
#include "rtos/defs.hpp"

namespace wrvcu {

void Semaphore::init(uint32_t max_count, uint32_t init_count) {
    if (isOnStack((void*)this)) {
        printf("WARNING: Static mutex allocated on the stack! This WILL cause severe problems.\n");
        configASSERT(0); // assert error
    } else {
        /* ok*/
    };

    if (sem == NULL)
        sem = xQueueCreateCountingSemaphoreStatic(max_count, init_count, &semBuffer);

    if (sem == NULL) {
        printf("Mutex was not created!\n");
        configASSERT(sem);
    } else { // ok
    };
}

uint32_t Semaphore::get_count() {
    return uxSemaphoreGetCount(sem);
}

bool Semaphore::post() {
    return xSemaphoreGive(sem.get()) == pdTRUE;
}

bool Semaphore::wait(uint32_t timeout) {
    return xSemaphoreTake(sem.get(), pdMS_TO_TICKS(timeout));
}

} // namespace wrvcu