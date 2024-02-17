#pragma once

#include "arduino_freertos.h"
#include "rtos/queue.hpp"
#include "semphr.h"
#include <cstdint>
#include <memory>


// Lots of inspiration taken from https://github.com/purduesigbots/pros
// Modified from https://github.com/purduesigbots/pros/blob/develop/src/rtos/rtos.cpp

// The highest priority that can be assigned to a task. Beware of deadlock.
#define TASK_PRIORITY_MAX 16

// The lowest priority that can be assigned to a task.
// This may cause severe performance problems and is generally not recommended.
#define TASK_PRIORITY_MIN 1

// The default task priority, which should be used for most tasks.
// Default tasks such as autonomous() inherit this priority.
#define TASK_PRIORITY_DEFAULT 8

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

class Task {
public:
    /**
     * Creates a new task and add it to the list of tasks that are ready to run.
     *
     * This function uses the following values of errno when an error state is
     * reached:
     * ENOMEM - The stack cannot be used as the TCB was not created.
     *
     * \param function
     *        Pointer to the task entry function
     * \param parameters
     *        Pointer to memory that will be used as a parameter for the task
     *        being created. This memory should not typically come from stack,
     *        but rather from dynamically (i.e., malloc'd) or statically
     *        allocated memory.
     * \param prio
     *        The priority at which the task should run.
     *        TASK_PRIO_DEFAULT plus/minus 1 or 2 is typically used.
     * \param stack_depth
     *        The number of words (i.e. 4 * stack_depth) available on the task's
     *        stack. TASK_STACK_DEPTH_DEFAULT is typically sufficienct.
     * \param name
     *        A descriptive name for the task.  This is mainly used to facilitate
     *        debugging. The name may be up to 32 characters long.
     *
     */
    Task(task_fn_t function, void* parameters, std::uint32_t prio, std::uint16_t stack_depth, const char* name);

    /**
     * Creates a new task and add it to the list of tasks that are ready to run.
     *
     * This function uses the following values of errno when an error state is
     * reached:
     * ENOMEM - The stack cannot be used as the TCB was not created.
     *
     * \param function
     *        Pointer to the task entry function
     * \param parameters
     *        Pointer to memory that will be used as a parameter for the task
     *        being created. This memory should not typically come from stack,
     *        but rather from dynamically (i.e., malloc'd) or statically
     *        allocated memory.
     * \param name
     *        A descriptive name for the task.  This is mainly used to facilitate
     *        debugging. The name may be up to 32 characters long.
     *
     */
    Task(task_fn_t function, void* parameters, const char* name);

    /**
     * Create a C++ task object from a task handle
     *
     * \param task
     *        A task handle from task_create() for which to create a wrvcu::Task
     *        object.
     */
    Task(TaskHandle_t task);

    /**
     * Creates a new task and add it to the list of tasks that are ready to run.
     *
     * This function uses the following values of errno when an error state is
     * reached:
     * ENOMEM - The stack cannot be used as the TCB was not created.
     *
     * \param function
     *        Callable object to use as entry function
     * \param prio
     *        The priority at which the task should run.
     *        TASK_PRIO_DEFAULT plus/minus 1 or 2 is typically used.
     * \param stack_depth
     *        The number of words (i.e. 4 * stack_depth) available on the task's
     *        stack. TASK_STACK_DEPTH_DEFAULT is typically sufficient.
     * \param name
     *        A descriptive name for the task.  This is mainly used to facilitate
     *        debugging. The name may be up to 32 characters long.
     *
     */
    template <class F>
    Task(F&& function, uint32_t prio, uint16_t stack_depth, const char* name) :
        // Wizardry from https://github.com/purduesigbots/pros/blob/1e7513d4f110d2eac625b6300dbbb8c086ab6c0c/include/pros/rtos.hpp#L107
        Task( // Create task with a lambda that executes a function given through a parameter.
            [](void* parameters) {
                std::unique_ptr<std::function<void()>> ptr{ static_cast<std::function<void()>*>(parameters) };
                (*ptr)();
            },
            new std::function<void()>(std::forward<F>(function)), prio, stack_depth, name) {
        static_assert(std::is_invocable_r_v<void, F>);
    }

    /**
     * Creates a new task and add it to the list of tasks that are ready to run.
     *
     * This function uses the following values of errno when an error state is
     * reached:
     * ENOMEM - The stack cannot be used as the TCB was not created.
     *
     * \param function
     *        Callable object to use as entry function
     * \param name
     *        A descriptive name for the task.  This is mainly used to facilitate
     *        debugging. The name may be up to 32 characters long.
     *
     */
    template <class F>
    Task(F&& function, const char* name) :
        Task(std::forward<F>(function), TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, name){};

    /**
     * Sends a simple notification to task and increments the notification
     * counter.
     */
    void notify();

    /**
     * Waits for a notification to be nonzero.
     *
     * \param clear_on_exit
     *        If true (1), then the notification value is cleared.
     *        If false (0), then the notification value is decremented.
     * \param timeout
     *        Specifies the amount of time to be spent waiting for a notification
     *        to occur.
     *
     * \return The value of the task's notification value before it is decremented
     * or cleared
     */
    static std::uint32_t notify_take(bool clear_on_exit, std::uint32_t timeout);

    /**
     * Clears the notification for a task.
     *
     * \return False if there was not a notification waiting, true if there was
     */
    void notify_clear();

    /**
     * Suspends the specified task, making it ineligible to be scheduled.
     */
    void suspend();

    /**
     * Resumes the specified task, making it eligible to be scheduled.
     *
     * \param task
     *        The task to resume
     */
    void resume();

    /**
     * Removes the Task from the RTOS real time kernel's management. This task
     * will be removed from all ready, blocked, suspended and event lists.
     *
     * Memory dynamically allocated by the task is not automatically freed, and
     * should be freed before the task is deleted.
     */
    void remove();

    /**
     * Utilizes task notifications to wait until specified task is complete and deleted,
     * then continues to execute the program. Analogous to std::thread::join in C++.
     */
    // void join();

    /**
     * Get the currently running Task
     */
    static Task current();

    /**
     * Delays a task for a given number of milliseconds.
     *
     * This is not the best method to have a task execute code at predefined
     * intervals, as the delay time is measured from when the delay is requested.
     * To delay cyclically, use delay_until().
     *
     * \param milliseconds
     *        The number of milliseconds to wait
     */
    static void delay(const std::uint32_t milliseconds);

    /**
     * Delays a task until a specified time.  This function can be used by
     * periodic tasks to ensure a constant execution frequency.
     *
     * The task will be woken up at the time *prev_time + delta, and *prev_time
     * will be updated to reflect the time at which the task will unblock.
     *
     * \param prev_time
     *        A pointer to the location storing the setpoint time. This should
     *        typically be initialized to the return value from wrvcu::millis().
     * \param delta
     *        The number of milliseconds to wait
     */
    static void delay_until(std::uint32_t* const prev_time, const std::uint32_t delta);

    /**
     * Gets the priority of the specified task.
     *
     * \return The priority of the task
     */
    std::uint32_t get_priority();

    /**
     * Sets the priority of the specified task.
     *
     * If the specified task's state is available to be scheduled (e.g. not
     * blocked) and new priority is higher than the currently running task,
     * a context switch may occur.
     *
     * \param prio
     *        The new priority of the task
     */
    void set_priority(std::uint32_t prio);

    /**
     * Gets the name of the specified task.
     *
     * \return A pointer to the name of the task
     */
    const char* get_name();

    /**
     * Gets the state of the specified task.
     *
     * \return The state of the task
     */
    std::uint32_t get_state();

private:
    TaskHandle_t task;
};

class Mutex {
    std::shared_ptr<std::remove_pointer_t<SemaphoreHandle_t>> mutex;

public:
    Mutex();

    /**
     * Unlocks a mutex.
     *
     * \return True if the mutex was successfully returned, false otherwise.
     */
    bool give();

    /**
     * Takes and locks a mutex indefinetly.
     *
     * \return True if the mutex was successfully taken, false otherwise.
     */
    bool take();

    /**
     * Takes and locks a mutex, waiting for up to a certain number of milliseconds
     * before timing out.
     *
     * \param timeout
     *        Time to wait before the mutex becomes available. A timeout of 0 can
     *        be used to poll the mutex. TIMEOUT_MAX can be used to block
     *        indefinitely.
     *
     * \return True if the mutex was successfully taken, false otherwise.
     */
    bool take(std::uint32_t timeout);
};

class Semaphore {
    std::shared_ptr<std::remove_pointer_t<SemaphoreHandle_t>> sem;

public:
    Semaphore(uint32_t max_count, uint32_t init_count);

    /**
     * Returns the current value of the semaphore.
     * \return The current value of the semaphore
     */
    uint32_t get_count();

    /**
     * Increments a semaphore’s value.
     * \return True if the value was incremented, false otherwise.
     */
    bool post();

    /**
     * Waits for the semaphore’s value to be greater than 0. If the value is already greater than 0, this function immediately returns.
     * \param timeout
     *      Time to wait before the semaphore’s becomes available. A timeout of 0 can be used to poll the semaphore. TIMEOUT_MAX can be used to block indefinitely.
     * \return true if the semaphore was released. false if an error occurred.
     */
    bool wait(uint32_t timeout);
};
} // namespace wrvcu