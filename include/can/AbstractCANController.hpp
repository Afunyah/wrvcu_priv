#pragma once

#include <can/CANMessage.hpp>
#include <map>
#include <memory>
#include <rtos/defs.hpp>
#include <rtos/queue.hpp>

namespace wrvcu {
class AbstractCANController {
protected:
    std::map<uint32_t, Queue<CANMessage, 256>*> _subscribers;

    /**
     * @brief Puts a received message into the right queue.
     *
     * @param message The message to post.
     */
    void post(CANMessage const& message) {
        if (_subscribers.find(message.id) == _subscribers.end())
            return; // no subscriber for this ID, so skip

        auto&& queue = _subscribers.at(message.id);
        queue->enqueue(message, TIMEOUT_MAX); // enqueue the message
    };

public:
    /**
     * @brief Start the CAN Controller.
     *
     */
    virtual void init(uint32_t task_priority) = 0;

    /**
     * @brief Set the controller baud rate
     *
     * @param baud_rate The new baud rate
     */
    virtual void set_baud_rate(const uint32_t baud_rate) = 0;

    /**
     * @brief Send a CAN Message
     *
     * @param message The message to send
     */
    virtual void send(CANMessage const& message) = 0;

    /**
     * @brief Subscribes to CAN messages with a given ID. Messages are put in the provided queue.
     *
     * @param id The ID to subscribe to.
     * @param queue The queue where new messages will be put.
     */
    void subscribe(uint16_t id, Queue<CANMessage, 256>* queue) {
        _subscribers.emplace(id, queue);
    };
};

}