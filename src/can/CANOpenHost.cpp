#include "can/CANOpenHost.hpp"

#include "constants.hpp"

namespace wrvcu {

void CANOpenHost::init(AbstractCANController* ican) {
    this->can = ican;

    task.start([this] { loop(); }, CANOPEN_HOST_TASK_PRIORITY, "CANOpen_Host_Task");
};

void CANOpenHost::loop() {
    while (true) {
        sendSync();
        sendHeartbeat();

        Task::delay(50);
    }
}

void CANOpenHost::sendSync() {
    CANMessage msg = {
        .id = 0x80,
        .len = 0
    };
    can->send(msg);
};

void CANOpenHost::sendHeartbeat() {
    CANMessage msg = {
        .id = 0x700 + VCU_NODE_ID,
        .len = 1
    };
    // Send VCU state, which is always operational
    msg.data[0] = { 0x01 };
    can->send(msg);
};

}