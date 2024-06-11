#pragma once

#include "apps.hpp"

namespace wrvcu {

class ThrottleManager {
protected:
    // APPS APPS1;
    // APPS APPS2;

    bool plausibilityTimerStarted = false;
    ulong currentAPPSMillis = 0;
    ulong prevAPPSMillis = 0;

    bool brakeTimerStarted = false;
    ulong currentBrakeMillis = 0;
    ulong prevBrakeMillis = 0;

    bool throttleError = false;
    bool brakeError = false;

    bool checkAPPSConnection();
    void checkAPPSPlausibility();
    void checkHardBrake();

    float getThrottleFraction();

public:
    APPS APPS1;
    APPS APPS2;
    
    float getTorqueRequestFraction();

    bool isCriticalError();

    void init(ADC* adc);

};
}