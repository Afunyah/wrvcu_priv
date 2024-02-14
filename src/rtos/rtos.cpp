#include "rtos/rtos.hpp"

namespace wrvcu {

// Most of this code is just a wrapper around FreeRTOS functions

void startScheduler() {
    Serial.printf("Booting FreeRTOS kernel %s.", tskKERNEL_VERSION_NUMBER);
    vTaskStartScheduler();
};

Task::Task(task_fn_t function, void* parameters, std::uint32_t prio, std::uint16_t stack_depth, const char* name) {
    // TODO: Use xTaskCreateStatic to statically allocate stack

    // create the task
    BaseType_t res = xTaskCreate(function, name, stack_depth, parameters, prio, &task);

    // Check that task was sucessfully created
    if (res == pdPASS) {
    } // OK
    else if (res == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
    }; // TODO: log an error
}

Task::Task(task_fn_t function, void* parameters, const char* name) :
    Task(function, parameters, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, name) {}

Task::Task(TaskHandle_t task) :
    task(task){};

Task Task::current() {
    return Task{ xTaskGetCurrentTaskHandle() };
}

void Task::remove() {
    vTaskDelete(task);
}

std::uint32_t Task::get_priority() {
    return uxTaskPriorityGet(task);
}

void Task::set_priority(std::uint32_t prio) {
    vTaskPrioritySet(task, prio);
}

std::uint32_t Task::get_state() {
    return eTaskGetState(task);
}

void Task::suspend() {
    vTaskSuspend(task);
}

void Task::resume() {
    vTaskResume(task);
}

const char* Task::get_name() {
    return pcTaskGetName(task);
}

void Task::notify() {
    xTaskNotifyGive(task);
}

// void Task::join() {
//     if (!task)
//         return;
//     task_notify_when_deleting(task, NULL, 1, E_NOTIFY_ACTION_INCR);
//     do {
//         vTaskGetInfo(task, &xTaskDetails, pdTRUE, E_TASK_STATE_INVALID);
//     } while (!task_notify_take(true, 20) && xTaskDetails.eCurrentState != E_TASK_STATE_DELETED);
// }

std::uint32_t Task::notify_take(bool clear_on_exit, std::uint32_t timeout) {
    return ulTaskNotifyTake(clear_on_exit, timeout);
}

void Task::notify_clear() {
    xTaskGenericNotifyStateClear(task, 0);
}

void Task::delay(const std::uint32_t milliseconds) {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

void Task::delay_until(std::uint32_t* const prev_time, const std::uint32_t delta) {
    vTaskDelayUntil(prev_time, pdMS_TO_TICKS(delta));
}

// std::uint32_t Task::get_count() {
//     return task_get_count();
// }

Mutex::Mutex() :
    // Define a creator and deleter for the underlying FreeRTOS Mutex
    // Use a lambda for the deleter because vSemaphoreDelete is a macro not a function.
    mutex(xSemaphoreCreateMutex(), [](SemaphoreHandle_t mutex) { vSemaphoreDelete(mutex); }) {}

bool Mutex::take() {
    return take(TIMEOUT_MAX);
}

bool Mutex::take(std::uint32_t timeout) {
    return xSemaphoreTake(mutex.get(), timeout);
}

bool Mutex::give() {
    return xSemaphoreGive(mutex.get());
}

Semaphore::Semaphore(uint32_t max_count, uint32_t init_count) :
    mutex(xQueueCreateCountingSemaphore(max_count, init_count), [](SemaphoreHandle_t mutex) { vSemaphoreDelete(mutex); }){};

} // namespace wrvcu