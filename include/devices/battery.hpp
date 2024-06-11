#pragma once

#include "battery.h"
#include "can/CANOpenDevice.hpp"
#include "pins.hpp"
#include "rtos/mutex.hpp"

namespace wrvcu {

enum class ContactorStates {
    Init = BATTERY_BMS_BATT_STATUS_STATUS_INIT__MODE_CHOICE,
    Ready = BATTERY_BMS_BATT_STATUS_STATUS_READY__MODE_CHOICE,               // awaiting ignition
    AwaitActive = BATTERY_BMS_BATT_STATUS_STATUS_AWAIT__ACTIVE__MODE_CHOICE, // Precharging
    Active = BATTERY_BMS_BATT_STATUS_STATUS_ACTIVE__MODE_CHOICE,             // contactors closed
    Error = BATTERY_BMS_BATT_STATUS_STATUS_ERROR__MODE_CHOICE,
};

class Battery {
protected:
    AbstractCANController* can;

    Queue<CANMessage, 256> canQueue;

    Task task;

    Mutex mutex;
    bool enable = false;
    uint32_t last_tx = 0;

    void loop();

    void wake();

    void updateState(uint8_t status);

public:
    float maxDischargeCurrent = 0; // Amps
    float maxChargeCurrent = 0;    // Amps

    ContactorStates contactorState = ContactorStates::Init;
    float chargeRemaining = 0; // Ah
    float SoC = 0;             // %
    // float voltsSumOfCells = 0; // ignore

    float cellAvgTemp = 0; // deg C
    float cellMaxTemp = 0; // deg C
    float cellMinTemp = 0; // deg C

    float cellAvgVoltage = 0; // mV
    float cellMaxVoltage = 0; // mV
    float cellMinVoltage = 0; // mV

    float imdIsoRes = 0; // kOhms

    float terminalCurrent = 0; // Amps
    float packVoltage = 0;     // Volts
    float preFuseVoltage = 0;  // Volts
    float postFuseVoltage = 0; // Volts
    float power = 0;           // Watts

    void init(AbstractCANController* ican);

    void closeContactors();
    void openContactors();
};
}