#pragma once

#include <FlexCAN_T4.h>
#include <can/AbstractCANController.hpp>
#include <rtos/rtos.hpp>

namespace wrvcu {

// templated class, so needs to be defined in the header :(
template <CAN_DEV_TABLE BUS>
class CANController_T4 : public AbstractCANController {

    FlexCAN_T4<BUS, RX_SIZE_256, TX_SIZE_16> can;
    Task task;
    const int MAX_READS = 15; // max reads without waiting
    Mutex mutex;

    /**
     * @brief The function run in the controller's task. This listens to CAN messages, and puts them in the correct queues.
     *
     */
    void loop() {
        while (true) {
            int reads = 0;
            CAN_message_t msg;

            mutex.take();

            // read a burst of messages to increase throughput
            // but limit max reads in one burst to prevent hogging
            // - especially important as a high priority task
            while (can.read(msg) && reads < MAX_READS) {
                reads++;

                // convert message types
                CANMessage wr_msg{
                    .id = msg.id,
                    .len = msg.len,
                    .timestamp = msg.timestamp,

                    .flags = {
                              .extended = msg.flags.extended,
                              .remote = msg.flags.remote,
                              .overrun = msg.flags.overrun,
                              .reserved = msg.flags.reserved}
                };
                memcpy(wr_msg.data, msg.buf, sizeof(wr_msg.data));

                // put into the right queue
                post(wr_msg);
            }
            mutex.give();

            if (reads >= MAX_READS)
                printf("Too many CAN messages! Max reads exceeded.\n"); // TODO: Error logging?

            Task::delay(5);
        };
    }

public:
    /**
     * @brief Start the CAN Controller.
     *
     */
    void init(uint32_t task_priority) override {
        can.begin();
        can.setBaudRate(500000);

        mutex.init();

        task.start(
            [this] {
                loop();
            },

            task_priority, "CAN_Task");
    };

    /**
     * @brief Set the controller baud rate
     *
     * @param baud_rate The new baud rate
     */
    void set_baud_rate(uint32_t baud_rate) override {
        can.setBaudRate(baud_rate);
    };

    /**
     * @brief Send a CAN Message
     *
     * @param message The message to send
     */
    void send(CANMessage const& message) override {
        CAN_message_t msg{
            .id = message.id,
            .flags = {
                      .extended = message.flags.extended,
                      .remote = message.flags.remote,
                      .overrun = msg.flags.overrun,
                      .reserved = msg.flags.reserved},

            .len = message.len,
        };
        memcpy(msg.buf, message.data, sizeof(msg.buf));

        mutex.take();
        can.write(msg);
        mutex.give();
    };
};

}