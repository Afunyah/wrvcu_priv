#include "devices/inverter.hpp"
#include "constants.hpp"
#include "logging/log.hpp"
#include "rtos/task.hpp"

namespace wrvcu {

void Inverter::init(AbstractCANController* ican, uint8_t inodeID) {
    pdoQueue.init();
    sdoQueue.init();

    device.init(ican, inodeID, INVERTER_TASK_PRIORITY);
    device.addPDOQueue(&pdoQueue);
    device.addSDOQueue(&sdoQueue);

    device.subscribePDO(INVERTER_TPDO1);
    device.subscribePDO(INVERTER_TPDO2);
    device.subscribePDO(INVERTER_TPDO3);
    device.subscribePDO(INVERTER_TPDO4);

    mutex.init();

    task.start(
        [this] { loop(); }, INVERTER_TASK_PRIORITY, "Inverter_Task");
}

void Inverter::start() {
    mutex.take();
    enable = true;
    mutex.give();
}

void Inverter::stop() {
    // stop taken more seriously, send the CAN message immediately without waiting for mutex's or for tasks to wake up.
    device.sendNMT(NMTCommand::PreOperational);
    disable_pwm();
    state = InverterStates::PreOp;

    mutex.take();
    enable = false;
    mutex.give();
}

void Inverter::sendTorque(int16_t torque) {
    int sentTorque = torque * -1;
    uint8_t data[2] = { (uint8_t)(sentTorque & 0xff), (uint8_t)(sentTorque >> 8) };
    device.sendPDO(INVERTER_RPDO1, data);
};

void Inverter::loop() {
    while (true) {
        mutex.take();

        switch (state) {
        case (InverterStates::Unknown):
            if (Task::millis() > (last_tx + INVERTER_SEND_PERIOD)) {
                DEBUG("Inverter: Sending Reset from Unknown\n");
                device.sendNMT(NMTCommand::Reset);
                state = InverterStates::Reset;
                last_tx = Task::millis();
            }
            break;

        case (InverterStates::Reset):
            if (Task::millis() > (last_tx + INVERTER_SEND_PERIOD)) {
                DEBUG("Inverter: Sending PreOp from Reset\n");
                device.sendNMT(NMTCommand::PreOperational);
                state = InverterStates::PreOp;
                last_tx = Task::millis();
            }
            break;

        case (InverterStates::PreOp):
            if (enable) {
                DEBUG("Inverter: Sending Op\n");
                device.sendNMT(NMTCommand::Operational);
                state = InverterStates::Op;
                last_tx = Task::millis();
            }
            break;

        case (InverterStates::Op):
            if (Task::millis() > (last_tx + INVERTER_SEND_PERIOD)) {
                DEBUG("Inverter: Sending Disable PWM\n");
                disable_pwm();
                state = InverterStates::Idle;
                last_tx = Task::millis();
            }
            break;

        case (InverterStates::Idle):
            if (enable) {
                if (Task::millis() > (last_tx + INVERTER_SEND_PERIOD)) {
                    DEBUG("Inverter: Sending Enable\n");
                    enable_pwm();
                    state = InverterStates::Drive;
                    last_tx = Task::millis();
                }
            }
            break;

        case (InverterStates::Drive):
            if (!enable) {
                DEBUG("Inverter: Disabling PWM From Drive\n");
                disable_pwm();
                state = InverterStates::Idle;
            }
            break;

        case (InverterStates::Error):
            break;
        }

        // Control Word Read command
        // device.sendSDORead(INV_CW_INDEX, INV_CW_SUBINDEX);

        while (sdoQueue.size() > 0) {
            SDOMessage sdoMsg = sdoQueue.dequeue(0);

            // controlword message
            // if (sdoMsg.index == INVERTER_CONTROLWORD_INDEX && sdoMsg.subindex == INVERTER_CONTROLWORD_SUBINDEX) {

            //     if (sdoMsg.data[0] == INVERTER_CW_DISABLE_PWM && device.getNMTState() == NMTState::Operational) {
            //         state = InverterStates::Idle;
            //     } else if (sdoMsg.data[0] == INVERTER_CW_ENABLE_PWM && device.getNMTState() == NMTState::Operational) {
            //         state = InverterStates::Drive;
            //     }
            // }
        }

        while (pdoQueue.size() > 0) {
            PDOMessage pdoMsg = pdoQueue.dequeue(0);

            // Read Errors
            // if (pdoMsg.cobID == 0x180) {
            //     int newError = pdoMsg.data[4] | (pdoMsg.data[5] << 8);
            //     if (newError != errorCode){
            //         ERROR("Inverter: Got error code");
            //         printf("Error code: %x\n", newError);
            //         errorCode = newError;
            //     }
            // }
        }

        if (errorCode != 0) {
            state = InverterStates::Error;
        }

        mutex.give();

        Task::delay(10);
    }
}

void Inverter::disable_pwm() {
    uint8_t data[4] = { INVERTER_CW_DISABLE_PWM, 0, 0, 0 };
    device.sendSDOWrite(2, INV_CW_INDEX, INV_CW_SUBINDEX, data);
}

void Inverter::enable_pwm() {
    uint8_t data[4] = { INVERTER_CW_ENABLE_PWM, 0, 0, 0 };
    device.sendSDOWrite(2, INV_CW_INDEX, INV_CW_SUBINDEX, data);
}

}