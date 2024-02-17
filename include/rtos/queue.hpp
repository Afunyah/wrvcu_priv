#pragma once

#include <cstdint>
#include <memory>

namespace wrvcu {

template <typename T>
class Queue {
    std::shared_ptr<std::remove_pointer_t<QueueHandle_t>> queue;

public:
    Queue(uint32_t length) :
        queue(xQueueCreate(length, sizeof(T)), [](QueueHandle_t queue) { vQueueDelete(queue); }){};

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