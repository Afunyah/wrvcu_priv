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

    bool regenButtonHeld = false;

    Mutex mutex;
    Task task;

public:
    bool inRegenMode = false;

    void init();
    void loop();

    bool sdcClosed();
    bool tsasPressed();
    bool startPressed();
    bool checkRegenButtonState();

    void setR2DLED(bool state);
    void setBuzzer(bool state);
    void setBrakeLight(bool state);

    float calculateMaxRegenTorque();
    float calculateMaxDriveTorque();

    void DriveSequence();

    TSStates getState();

    void test_init();
    void test_loop();
};
}