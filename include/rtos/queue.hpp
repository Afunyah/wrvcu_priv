#pragma once

#include <cstdint>
#include <memory>

namespace wrvcu {

/**
 * @brief A thread-safe queue. This is allocated statically, so MUST be in global scope. This MUST NOT be created inside a function.
 * Used to safely pass messages between threads, and the primary form of intertask communication.
 *
 * @tparam T The type of the queue items
 * @tparam LEN The length of the queue
 */
template <typename T, int LEN>
class Queue {
    std::shared_ptr<std::remove_pointer_t<QueueHandle_t>> queue;
    StaticQueue_t staticQueue;
    uint8_t queueStorageArea[LEN * sizeof(T)];

public:
    Queue() : // cppcheck-suppress misra-c2012-2.7; (False positive for unused parameter)
        queue(xQueueCreateStatic(LEN, sizeof(T), queueStorageArea, &staticQueue), [](QueueHandle_t queue) { vQueueDelete(queue); }){};

    /**
     * Posts an item to the end of a queue. The item is queued by copy, not by reference.
     * \param item
     *      A reference to the item that will be placed on the queue.
     * \param timeout
     *      Time to wait for space to become available. A timeout of 0 can be used to attempt to post without blocking. TIMEOUT_MAX can be used to block indefinitely.
     *
     * \return true if the item was enqueued, false otherwise
     */
    bool enqueue(T const& item, uint32_t timeout) {
        return xQueueSendToBack(queue.get(), &item, pdMS_TO_TICKS(timeout));
    }

    /**
     * Posts an item to the front of a queue. The item is queued by copy, not by reference.
     * \param item
     *      A reference to the item that will be placed on the queue.
     * \param timeout
     *      Time to wait for space to become available. A timeout of 0 can be used to attempt to post without blocking. TIMEOUT_MAX can be used to block indefinitely.
     *
     * \return true if the item was enqueued, false otherwise
     */
    bool prepend(T const& item, uint32_t timeout) {
        return xQueueSendToFront(queue.get(), &item, pdMS_TO_TICKS(timeout));
    }

    /**
     * Get an item from a queue without removing the item from the queue.
     * \param timeout
     *      Time to wait for space to become available. A timeout of 0 can be used to attempt to post without blocking. TIMEOUT_MAX can be used to block indefinitely.
     *
     * \return the received item
     */
    T peek(uint32_t timeout) {
        T item;
        xQueuePeek(queue.get(), &item, pdMS_TO_TICKS(timeout));
        return item;
    };

    /**
     * Get an item from the queue.
     * \param timeout
     *      Time to wait for space to become available. A timeout of 0 can be used to attempt to post without blocking. TIMEOUT_MAX can be used to block indefinitely.
     *
     * \return the received item
     */
    T dequeue(uint32_t timeout) {
        T item;
        xQueueReceive(queue.get(), &item, pdMS_TO_TICKS(timeout));
        return item;
    };

    /**
     * Get the number of items stored in the queue
     *
     * \return the number of items in the queue
     */
    uint32_t size() {
        return uxQueueMessagesWaiting(queue.get());
    };

    /**
     * Reset a queue to its original empty state.
     */
    void reset() {
        xQueueReset(queue.get());
    };
};

} // namespace wrvcu