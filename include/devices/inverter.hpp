#pragma once

#include "can/CANOpenDevice.hpp"
#include "rtos/mutex.hpp"

namespace wrvcu {

enum class InverterStates {
    Unknown,
    Error,
    Reset,
    PreOp,
    Op,
    Idle,
    Drive
};

class Inverter {
protected:
    CANOpenDevice device;
    uint8_t nodeID;

    Queue<PDOMessage, 256> pdoQueue;
    Queue<SDOMessage, 256> sdoQueue;

    Task task;

    Mutex mutex;
    bool enable = false;
    uint32_t last_tx = 0;

    void loop();

    void disable_pwm();
    void enable_pwm();

public:
    InverterStates state = InverterStates::Unknown;
    uint16_t errorCode = 0;
    uint16_t warningCode = 0;

    int32_t rpm;
    int16_t motorTemp;
    int16_t controllerTemp;

    int32_t polarityFactor = -1;

    void init(AbstractCANController* ican, uint8_t inodeID);

    /**
     * @brief Start the inverter.
     *
     */
    void start();

    /**
     * @brief Stop the inverter, sending it straight into PreOp.
     *
     */
    void stop();

    /**
     * @brief Send a torque command to the inverter.
     *
     * @param torque An integer from 0 to 1000
     */
    void sendTorque(int16_t torque);
};
}
