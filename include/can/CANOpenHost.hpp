#pragma once

#include "can/AbstractCANController.hpp"
#include "rtos/task.hpp"

namespace wrvcu {

class CANOpenHost {
protected:
    AbstractCANController* can;

    Task task;

    void loop();

public:
    void init(AbstractCANController* ican);

    void sendSync();
    void sendHeartbeat();
    // void sendEmcy();
    // void sendTime();
};

}