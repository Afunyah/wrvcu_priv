#pragma once

#include "constants.hpp"

namespace wrvcu {

enum class TSStates {
    Error,
    Idle,
    CloseContactors,
    WaitR2D,
    StartInverter,
    Buzzer,
    Driving
};

class TractiveSystem {
protected:
    TSStates state = TSStates::Idle;

    uint32_t contactorCloseStart = 0;
    uint32_t buzzerStart = 0;

    Mutex mutex;
    Task task;

    uint16_t InverterRequestedTorque = 0;

public:
    void init();
    void loop();

    bool sdcClosed();
    bool tsasPressed();
    bool startPressed();
    bool brakesOn();

    void setR2DLED(bool state);
    void setBuzzer(bool state);

    TSStates getState();
};
}