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
    QueueHandle_t queue;
    StaticQueue_t staticQueue;
    uint8_t queueStorageArea[LEN * sizeof(T)];

public:
    Queue() = default;

    /**
     * @brief Initialises the queue. This must be called before use.
     *
     */
    void init() {
        if (isOnStack((void*)this)) {
            printf("WARNING: Static queue allocated on the stack! This WILL cause severe problems.\n");
            configASSERT(0); // assert error
        } else {
            /* ok*/
        };

        queue = xQueueCreateStatic(LEN, sizeof(T), queueStorageArea, &staticQueue);

        if (queue == NULL) {
            printf("Queue was not created!\n");
            configASSERT(queue);
        } else { // ok
        };
    }

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
        return xQueueSendToBack(queue, &item, pdMS_TO_TICKS(timeout));
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
        return xQueueSendToFront(queue, &item, pdMS_TO_TICKS(timeout));
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
        xQueuePeek(queue, &item, pdMS_TO_TICKS(timeout));
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
        xQueueReceive(queue, &item, pdMS_TO_TICKS(timeout));
        return item;
    };

    /**
     * Get the number of items stored in the queue
     *
     * \return the number of items in the queue
     */
    uint32_t size() {
        return uxQueueMessagesWaiting(queue);
    };

    /**
     * Reset a queue to its original empty state.
     */
    void reset() {
        xQueueReset(queue);
    };
};

} // namespace wrvcu