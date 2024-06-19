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

    bool APPSDisconnectedError = false;
    bool brakeDisconnectedError = false;
    bool APPSPlausibilityError = false;
    bool brakePlausibilityError = false;
    bool hardBrakeError = false;

    float lastAPPSFraction = 0;

    void checkAPPSConnected();
    void checkAPPSPlausibility();
    void checkHardBrake();

    float getThrottleFraction();

    // void checkBrakesPlausibility();
    int getBrakePressure1();
    int getBrakePressure2();

    void checkBrakesConnected();

    

public:
    APPS APPS1;
    APPS APPS2;
    
    float getTorqueRequestFraction();
    float getBrakeRegenFraction();
    bool brakesOn();

    void checkBrakesPlausibility();

    bool isCriticalError();

    void init(ADC* adc);

};
}