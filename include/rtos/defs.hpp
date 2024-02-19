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

// The recommended stack size for a new task. This equates to 32,768 bytes
#define TASK_STACK_DEPTH_DEFAULT 0x2000

// The minimal stack size for a task. This equates to 2048 bytes
#define TASK_STACK_DEPTH_MIN 0x200

// The maximum number of characters allowed in a task's name.
// #define TASK_NAME_MAX_LEN 32
#define TASK_NAME_MAX_LEN configMAX_TASK_NAME_LEN

// The maximum timeout value that can be given to, for instance, a mutex grab.
// #define TIMEOUT_MAX ((uint32_t)0xffffffffUL)
#define TIMEOUT_MAX portMAX_DELAY

namespace wrvcu {

typedef void* task_t;
typedef void (*task_fn_t)(void*);

void startScheduler();

} // namespace wrvcu