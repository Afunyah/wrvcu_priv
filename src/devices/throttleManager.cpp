#include "devices/throttleManager.hpp"
#include "car.hpp"
#include "constants.hpp"

namespace wrvcu {

void ThrottleManager::init(ADC* adc) {
    this->APPS1.init(adc, APPS1_CHANNEL, APPS1_ANGLE_OFFSET, APPS1_ANGLE_RANGE);
    this->APPS2.init(adc, APPS2_CHANNEL, APPS2_ANGLE_OFFSET, APPS2_ANGLE_RANGE);
}

bool ThrottleManager::checkAPPSConnection() {
    return APPS1.isConnected() && APPS2.isConnected();
}

void ThrottleManager::checkAPPSPlausibility() {
    if ((APPS1.getSaturatedFraction() > APPS2.getSaturatedFraction() + 0.2) || (APPS1.getSaturatedFraction() < APPS2.getSaturatedFraction() - 0.2) || throttleError) {
        currentAPPSMillis = Task::millis();

        if (!plausibilityTimerStarted) {
            prevAPPSMillis = currentAPPSMillis;
            plausibilityTimerStarted = true;
        }

        if (currentAPPSMillis - prevAPPSMillis >= APPS_TIMEOUT) {
            throttleError = true;
        }
    } else {
        plausibilityTimerStarted = false;
        throttleError = false;
    }
}

// float ThrottleManager::getThrottleFraction() {
//     if (throttleError || brakeError || ts.brakesOn()) {
//         return 0;
//     } else {
//         return APPS1.getSaturatedFraction();
//     }
// }

float ThrottleManager::getThrottleFraction() {
    return APPS1.getSaturatedFraction();
}

bool ThrottleManager::isCriticalError() {
    return throttleError && checkAPPSConnection();
}

void ThrottleManager::checkHardBrake() {

    if (ts.brakesOn() && getThrottleFraction() > 0.25 && !brakeError) {
        currentBrakeMillis = Task::millis();

        if (!brakeTimerStarted) {
            prevBrakeMillis = currentBrakeMillis;
            brakeTimerStarted = true;
        }

        if (currentBrakeMillis - prevBrakeMillis >= BRAKE_TIMEOUT) {
            brakeError = true;
            brakeTimerStarted = false;
        }
    } else if (brakeTimerStarted && !brakeError) {
        brakeTimerStarted = false;
    }
}

float ThrottleManager::getTorqueRequestFraction() {
    // checkAPPSConnection();
    // checkAPPSPlausibility();
    // checkHardBrake();
    return getThrottleFraction();
}

}