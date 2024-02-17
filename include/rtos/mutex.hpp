#pragma once

#include <cstdint>
#include <memory>

namespace wrvcu {

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

} // namespace wrvcu