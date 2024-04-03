#pragma once

// Lots of inspiration taken from https://github.com/purduesigbots/pros
// Modified from https://github.com/purduesigbots/pros/blob/develop/src/rtos/rtos.cpp

// The highest priority that can be assigned to a task. Beware of deadlock.
#define TASK_PRIORITY_MAX configMAX_TASK_NAME_LEN

// The lowest priority that can be assigned to a task.
// This may cause severe performance problems and is generally not recommended.
#define TASK_PRIORITY_MIN 1

// The default task priority, which should be used for most tasks.
#define TASK_PRIORITY_DEFAULT 5

// The recommended stack size for a new task. This equates to 8192 bytes
#define TASK_STACK_DEPTH_DEFAULT 0x800

// The minimal stack size for a task.
#define TASK_STACK_DEPTH_MIN configMINIMAL_STACK_SIZE

// The maximum number of characters allowed in a task's name.
// #define TASK_NAME_MAX_LEN 32
#define TASK_NAME_MAX_LEN configMAX_TASK_NAME_LEN

// The maximum timeout value that can be given to, for instance, a mutex grab.
// #define TIMEOUT_MAX ((uint32_t)0xffffffffUL)
#define TIMEOUT_MAX portMAX_DELAY

namespace wrvcu {

typedef void* task_t;
typedef void (*task_fn_t)(void*);

/**
 * @brief Starts the RTOS scheduler
 *
 */
void startScheduler();

/**
 * @brief Checks whether a pointer is on the stack.
 *
 * @param ptr The pointer to check.
 * @return true The pointer is on the stack.
 * @return false The pointer is not on the stack.
 */
bool isOnStack(void* ptr);

} // namespace wrvcu