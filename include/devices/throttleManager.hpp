#pragma once

#include "apps.hpp"

namespace wrvcu {

class ThrottleManager {
protected:
    bool plausibilityTimerStarted = false;
    ulong currentAPPSMillis = 0;
    ulong prevAPPSMillis = 0;

    bool hardBrakeTimerStarted = false;
    ulong currentHardBrakeMillis = 0;
    ulong prevHardBrakeMillis = 0;

    bool brakePlausibilityTimerStarted = false;
    ulong currentBrakeMillis = 0;
    ulong prevBrakeMillis = 0;

    float lastAPPSFraction = 0;

    uint16_t brake_1_window_total = 0;
    int brake_1_window_index = 0;               // the index of the current reading
    uint16_t brake_1_window_array[APPS_WINDOW]; // the window_array from the analog input

    void checkAPPSConnected();
    void checkAPPSPlausibility();
    void checkHardBrake();
    bool isHardBrakeReleased();

    // void checkBrakesPlausibility();

    void checkBrakesConnected();

public:
    APPS APPS1;
    APPS APPS2;

    bool APPSDisconnectedError = false;
    bool brakeDisconnectedError = false;
    bool APPSPlausibilityError = false;
    bool brakePlausibilityError = false;
    bool hardBrakeError = false;

    float getTorqueRequestFraction();
    float getBrakeRegenFraction();
    bool brakesOn();

    float getThrottleFraction();
    int getBrakePressure1();
    int getBrakePressure2();

    void checkBrakesPlausibility();

    bool isCriticalError();

    void init(ADC* adc);
};
}