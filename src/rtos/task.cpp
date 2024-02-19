#include "rtos/task.hpp"

namespace wrvcu {

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
    return ulTaskNotifyTake(clear_on_exit, pdMS_TO_TICKS(timeout));
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

uint32_t Task::millis() {
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

// std::uint32_t Task::get_count() {
//     return task_get_count();
// }

} // namespace wrvcu