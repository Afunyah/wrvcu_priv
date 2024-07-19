#include "devices/throttleManager.hpp"
#include "car.hpp"
#include "constants.hpp"
#include "logging/log.hpp"

namespace wrvcu {

void ThrottleManager::init(ADC* adc) {
    this->APPS1.init(adc, APPS1_CHANNEL, APPS1_ANGLE_OFFSET, APPS1_ANGLE_RANGE);
    this->APPS2.init(adc, APPS2_CHANNEL, APPS2_ANGLE_OFFSET, APPS2_ANGLE_RANGE);
}

void ThrottleManager::checkAPPSConnected() {
    if (!APPS1.isConnected() || !APPS2.isConnected()) {
        APPSDisconnectedError = true;
    } else {
        APPSDisconnectedError = false; // TEMPORARY RESET
    }
}

void ThrottleManager::checkAPPSPlausibility() {
    if ((APPS1.getSaturatedFraction() > APPS2.getSaturatedFraction() + APPS_PLAUSIBILITY_FRACTION) || (APPS1.getSaturatedFraction() < APPS2.getSaturatedFraction() - APPS_PLAUSIBILITY_FRACTION)) {
        currentAPPSMillis = Task::millis();

        if (!plausibilityTimerStarted) {
            prevAPPSMillis = currentAPPSMillis;
            plausibilityTimerStarted = true;
        }

        if (currentAPPSMillis - prevAPPSMillis >= APPS_TIMEOUT) {
            APPSPlausibilityError = true;
        }
    } else {
        if (plausibilityTimerStarted) {
            plausibilityTimerStarted = false;
            APPSPlausibilityError = false; // TEMPORARY RESET
        }
    }
}

void ThrottleManager::checkHardBrake() {
    float throttleFraction = getThrottleFraction();
    currentHardBrakeMillis = Task::millis();

    if (hardBrakeTimerStarted && (currentHardBrakeMillis - prevHardBrakeMillis >= BRAKE_TIMEOUT)) {
        hardBrakeError = true;
        hardBrakeTimerStarted = false;
    }

    if (brakesOn() && throttleFraction > HARD_BRAKE_APPS_LIMIT && !hardBrakeError) {
        if (!hardBrakeTimerStarted) {
            prevHardBrakeMillis = currentHardBrakeMillis;
            hardBrakeTimerStarted = true;
        }
    } else if (hardBrakeTimerStarted && !hardBrakeError && isHardBrakeReleased()) {
        hardBrakeTimerStarted = false; // Reset timer, no error is set here
    } else if (throttleFraction <= HARD_BRAKE_APPS_RELEASE && isHardBrakeReleased()) {
        hardBrakeError = false;        // Reset error
        hardBrakeTimerStarted = false; // Reset timer
    }
}

bool ThrottleManager::isCriticalError() {
    return APPSDisconnectedError || brakeDisconnectedError || APPSPlausibilityError || brakePlausibilityError;
}

float ThrottleManager::getThrottleFraction() {

    float APPSFraction = APPS1.getSaturatedFraction();
    float regenFraction = APPS1.getRegenFraction();
    if (APPSFraction > 0.00f) {
        return APPSFraction;
    } else {
        return regenFraction * MAX_APPS_REGEN_FRACTION * MAX_ACCELERATION_REGEN_FRACTION;
    }
}

float ThrottleManager::getTorqueRequestFraction() {
    checkAPPSConnected();   // Sets APPSDisconnected Error
    checkBrakesConnected(); // Sets BrakesDisconnected Error

    checkAPPSPlausibility();   // Sets APPSPlausibility Error
    checkBrakesPlausibility(); // Sets BrakesPlausibility Error

    checkHardBrake(); // Sets HardBrake Error

    if (APPSDisconnectedError) {
        ERROR("APPS Disconnected Error");
    }
    if (brakeDisconnectedError) {
        ERROR("Brake Disconnected Error");
    }
    if (APPSPlausibilityError) {
        ERROR("APPS Plausibility Error");
    }
    if (brakePlausibilityError) {
        ERROR("Brake Plausibility Error");
    }
    if (hardBrakeError) {
        ERROR("Hard Brake Error");
    }

    float return_val = 0.0;

    if (isCriticalError() || hardBrakeError) {
        return_val = 0.0;
    } else {
        float throttleFraction = getThrottleFraction();
        if (brakesOn()) {
            return_val = min(0.0, throttleFraction);
        } else {
            return_val = throttleFraction;
        }
    }

    return return_val;
}

void ThrottleManager::checkBrakesPlausibility() {
    uint16_t bp1_ints = getBrakePressure1();
    uint16_t bp2_ints = getBrakePressure2();

    float bp1 = (bp1_ints - BRAKEPRESSURE1_AVERAGE_START) / BRAKEPRESSURE1_RANGE;
    float bp2 = (bp2_ints - BRAKEPRESSURE2_AVERAGE_START) / BRAKEPRESSURE2_RANGE;

    if ((bp1 > bp2 + BRAKE_PLAUSIBILITY_FRACTION) || (bp1 < bp2 - BRAKE_PLAUSIBILITY_FRACTION)) {
        currentBrakeMillis = Task::millis();

        if (!brakePlausibilityTimerStarted) {
            prevBrakeMillis = currentBrakeMillis;
            brakePlausibilityTimerStarted = true;
        }

        if (currentBrakeMillis - prevBrakeMillis >= BRAKE_TIMEOUT) {
            brakePlausibilityError = true;
        }
    } else {
        if (brakePlausibilityTimerStarted) {
            brakePlausibilityTimerStarted = false;
            brakePlausibilityError = false; // TEMPORARY RESET
        }
    }
}

void ThrottleManager::checkBrakesConnected() {
    uint16_t val1 = getBrakePressure1();
    uint16_t val2 = getBrakePressure2();
    if (val1 < BRAKEPRESSURE1_LOW_ADC || val1 > BRAKEPRESSURE1_HIGH_ADC || val2 < BRAKEPRESSURE2_LOW_ADC || val2 > BRAKEPRESSURE2_HIGH_ADC) {
        brakeDisconnectedError = true;
    } else {
        brakeDisconnectedError = false; // TEMPORARY RESET
    }
}

float ThrottleManager::getBrakeRegenFraction() {
    float regen_range = ADC_RESOLUTION - BRAKEPRESSURE1_MIN_THRESHOLD;
    float regen_fraction = 0.0;

    if (!isCriticalError() && brakesOn() && !hardBrakeError) {
        regen_fraction = -1.0 * BRAKE_REGEN_SENSITIVITY * (getBrakePressure1() - BRAKEPRESSURE1_MIN_THRESHOLD) / regen_range;
    }

    regen_fraction = std::clamp(regen_fraction, -0.85f, 0.0f);
    return regen_fraction;
}

int ThrottleManager::getBrakePressure1() {
    uint16_t val = adc.read(BRAKEPRESSURE1_CHANNEL);

    if (BRAKE_USE_SMOOTH) {
        brake_1_window_total = brake_1_window_total - brake_1_window_array[brake_1_window_index]; // Remove earliest reading from sum
        brake_1_window_array[brake_1_window_index] = val;                                         // Add reading to the array
        brake_1_window_total = brake_1_window_total + brake_1_window_array[brake_1_window_index]; // Add reading to sum
        brake_1_window_index = brake_1_window_index + 1;                                          // go to the next position in the array

        if (brake_1_window_index >= BRAKE_WINDOW) {
            brake_1_window_index = 0;
        }
        uint16_t smooth_val = brake_1_window_total / BRAKE_WINDOW;
        val = smooth_val;
    }

    return val;
}

int ThrottleManager::getBrakePressure2() {
    return adc.read(BRAKEPRESSURE2_CHANNEL);
}

bool ThrottleManager::brakesOn() {
    return getBrakePressure1() > BRAKEPRESSURE1_MIN_THRESHOLD;
}

bool ThrottleManager::isHardBrakeReleased() {
    return getBrakePressure1() < HARDBRAKE_RELEASE_THRESHOLD;
}

}