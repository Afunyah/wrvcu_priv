#pragma once

#include "rtos/rtos.hpp"
#include <cstdint>
#include <memory>

namespace wrvcu {

/**
 * @brief This is allocated statically, so MUST be in global scope. This MUST NOT be created inside a function.
 * A task executes within its own context with no coincidental dependency on other tasks within the system or the RTOS scheduler itself.
 * See the FreeRTOS website for more info (https://www.freertos.org/taskandcr.html).
 */
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
     * \param name
     *        A descriptive name for the task.  This is mainly used to facilitate
     *        debugging. The name may be up to 32 characters long.
     *
     */
    Task(task_fn_t function, void* parameters, uint32_t prio, const char* name);

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
     * \param name
     *        A descriptive name for the task.  This is mainly used to facilitate
     *        debugging. The name may be up to 32 characters long.
     *
     */
    template <class F>
    Task(F&& function, uint32_t prio, const char* name) : // cppcheck-suppress[misra-c2012-10.4,misra-c2012-2.7]; (False positive for unused parameter and arithemtic stuff)
                                                          // Wizardry from https://github.com/purduesigbots/pros/blob/1e7513d4f110d2eac625b6300dbbb8c086ab6c0c/include/pros/rtos.hpp#L107
        Task(                                             // Create task with a lambda that executes a function given through a parameter.
            [](void* parameters) {
                std::unique_ptr<std::function<void()>> ptr{ static_cast<std::function<void()>*>(parameters) };
                (*ptr)();
            },
            new std::function<void()>(std::forward<F>(function)), prio, name) {
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
    Task(F&& function, const char* name) : // cppcheck-suppress misra-c2012-2.7; (False positive for unused parameter)
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
     * @brief Get the time elapsed since the scheduler started in millis.
     *
     * @return uint32_t Milliseconds elapsed since the scheduler started.
     */
    static uint32_t millis();

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
    StackType_t stack[TASK_STACK_DEPTH_DEFAULT];
    StaticTask_t taskBuffer;
};

} // namespace wrvcu