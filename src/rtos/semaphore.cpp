#include "rtos/semaphore.hpp"

namespace wrvcu {

Semaphore::Semaphore(uint32_t max_count, uint32_t init_count) : // cppcheck-suppress misra-c2012-2.7; (False positive for unused parameter)
    sem(xQueueCreateCountingSemaphore(max_count, init_count), [](SemaphoreHandle_t sem) { vSemaphoreDelete(sem); }){};

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