#pragma once

#include <cstdint>
#include <memory>

namespace wrvcu {

/**
 * @brief A counting sempahore, for counting events, or for resource management.This is allocated statically, so MUST be in global scope. This MUST NOT be created inside a function.
 *
 */
class Semaphore {
    SemaphoreHandle_t sem = NULL;
    StaticSemaphore_t semBuffer;

public:
    Semaphore() = default;

    /**
     * @brief Initialise the semaphore. This must be done before using it.
     *
     */
    void init(uint32_t max_count, uint32_t init_count);

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