#include "TractiveSystem.hpp"
#include "car.hpp"
#include "logging/log.hpp"
#include "pins.hpp"
#include <algorithm>

namespace wrvcu {

void TractiveSystem::init() {
    mutex.init();

    // initialize outputs to off
    setR2DLED(false);
    setBuzzer(false);

    task.start(
        [this] { loop(); }, TRACTIVE_SYSTEM_TASK_PRIORITY, "TractiveSystem_Task");
}

void TractiveSystem::loop() {
    uint32_t prev = Task::millis();
    while (true) {

        // If the SDC opens, and the inverter is running, we want to shut down the inverter immediately.
        if ((inverter.state == InverterStates::Drive) && (!sdcClosed() || battery.contactorState == ContactorStates::Error)) {
            // vPortEnterCritical();
            inverter.stop();
            // vPortExitCritical();
            ERROR("Inverter shut down due to SDC opening!");
        }

        if (battery.contactorState == ContactorStates::Error) {
            battery.openContactors();
            state = TSStates::Error;
        }

        if (!sdcClosed()) {
            battery.openContactors();
            if (state != TSStates::Error) {
                state = TSStates::Idle;
            }
        }

        if (state != TSStates::Driving) {
            setR2DLED(false);
        }

        mutex.take();

        switch (state) {
        case TSStates::Idle:
            if (battery.contactorState == ContactorStates::Ready && sdcClosed() && tsasPressed()) {
                battery.closeContactors();
                state = TSStates::CloseContactors;
                contactorCloseStart = millis(); // record when we request contactors to close
                INFO("Closing Contactors");
            }
            break;

        case TSStates::CloseContactors:
            if (battery.contactorState == ContactorStates::Active && sdcClosed()) {
                state = TSStates::WaitR2D;
                INFO("Waiting for R2D");
            } else if (!sdcClosed()) {
                battery.openContactors(); // set BMS back into 'ready' state
                state = TSStates::Idle;
                WARN("SDC opened while waiting for contactors to close.");
            } else if (Task::millis() > (contactorCloseStart + CONTACTOR_CLOSE_TIMEOUT)) {
                // timeout exceeded, so error.
                battery.openContactors();
                state = TSStates::Error;
                ERROR("Contactor close timeout exceeded.");
            }
            break;

        case TSStates::WaitR2D:
            if (battery.contactorState == ContactorStates::Active && sdcClosed() && throttle.brakesOn() && startPressed()) {
                inverter.start();
                state = TSStates::StartInverter;
                INFO("Starting Inverter");
            }
            break;

        case TSStates::StartInverter:
            if (inverter.state == InverterStates::Drive) {
                setBuzzer(true);
                state = TSStates::Buzzer;
                buzzerStart = Task::millis();

                INFO("Starting buzzer");
            } else if (inverter.state == InverterStates::Error || inverter.state == InverterStates::Unknown) {
                state = TSStates::Error;
                ERROR("Could not start inverter! Inverter entered error state");
            }
            break;

        case TSStates::Buzzer:
            if (Task::millis() > (buzzerStart + BUZZER_LENGTH)) {
                setBuzzer(false);
                state = TSStates::Driving;
                setR2DLED(true);
                INFO("Buzzer complete, entering drive");
            }
            break;

        case TSStates::Driving:
            if (inverter.state == InverterStates::Error || inverter.state == InverterStates::Unknown) {
                state = TSStates::Error;
                setR2DLED(false);
                ERROR("Inverter errored during drive!");
            };
            break;

        case TSStates::Error:
            inverter.stop();
            break;
        }

        if (state == TSStates::Driving) {
            DriveSequence();
        }

        mutex.give();

        Task::delay_until(&prev, 10);
    }
}

void TractiveSystem::DriveSequence() {
    // if (throttle.isCriticalError()) {
    //     state = TSStates::Error;
    // }

    // ADD RPM OR SPEED BUTTON CHECK

    // checkRegenButtonState(); // SHOULD NOT BE HERE
    int16_t InverterRequestedTorque = 0;

    float driveTorque = throttle.getTorqueRequestFraction();
    float brakeTorque = throttle.getBrakeRegenFraction();

    int16_t maxDriveTorqueRequest = calculateMaxDriveTorque() / INVERTER_NM_PER_UNIT; // Positive
    int16_t maxRegenTorqueRequest = calculateMaxRegenTorque() / INVERTER_NM_PER_UNIT; // Negative

    // if (maxDriveTorqueRequest < 0.0 || maxRegenTorqueRequest > 0.0) {
    //     ERROR("max drive or regen torque calculation was invalid!");
    //     state = TSStates::Error;
    // }

    float requestedTorque = 0.0;

    inRegenMode = false; // JUST FOR SAFETY JOSH

    if (inRegenMode) {
        float speedScale = 0.0f;
        if (inverter.rpm < REGEN_DERATE_RPM && inverter.rpm > REGEN_MIN_RPM) {
            speedScale = std::clamp(map(inverter.rpm, REGEN_MIN_RPM, REGEN_DERATE_RPM, 1, 0), 0L, 1L);
        } else if (inverter.rpm > REGEN_DERATE_RPM) {
            speedScale = 1.0;
        }

        // float imu_deceleration = imu.getLonAcceleration();
        // if (imu_deceleration > (-REGEN_IMU_MIN_DECEL / ACCEL_DUE_TO_GRAVITY)) {
        //     driveTorque = max(0.0f, driveTorque); // APPS CANNOT REGEN
        // }

        requestedTorque = driveTorque + brakeTorque;
        if (requestedTorque < 0.0) {
            // derate requested regen as we slow down
            requestedTorque *= speedScale;
        }

    } else {
        requestedTorque = max(0, driveTorque);
    }

    InverterRequestedTorque = requestedTorque * INVERTER_MAXMIMUM_TORQUE_REQUEST;

    // if (inRegenMode && inverter.rpm > REGEN_MIN_RPM) {
    //     InverterRequestedTorque = std::clamp(InverterRequestedTorque, maxRegenTorqueRequest, maxDriveTorqueRequest);
    // } else {
    //     InverterRequestedTorque = std::clamp(InverterRequestedTorque, (int16_t)0, maxDriveTorqueRequest);
    // }

    // Sanity checks (for safety)
    if (InverterRequestedTorque > INVERTER_MAXMIMUM_TORQUE_REQUEST) {
        InverterRequestedTorque = INVERTER_MAXMIMUM_TORQUE_REQUEST;
    }
    if (InverterRequestedTorque < INVERTER_MINIMUM_TORQUE_REQUEST) {
        InverterRequestedTorque = INVERTER_MINIMUM_TORQUE_REQUEST;
    }
    if (InverterRequestedTorque < 0 && (!inRegenMode || inverter.rpm < REGEN_MIN_RPM)) {
        InverterRequestedTorque = 0;
    }

    // if (InverterRequestedTorque < BRAKELIGHT_REGEN_THRESHOLD) {
    //     setBrakeLight(true);
    // } else {
    //     setBrakeLight(false);
    // }

    inverter.sendTorque(InverterRequestedTorque);
}

bool TractiveSystem::sdcClosed() {
    static bool lastSc = false;

    bool s = digitalRead(SCMON_PIN);
    if (s != lastSc) {
        if (s)
            INFO("SC Closed");
        else
            INFO("SC Open");
    }
    lastSc = s;
    return s;
}

bool TractiveSystem::tsasPressed() {
    return digitalRead(TSAS_PIN);
}

bool TractiveSystem::startPressed() {
    return digitalRead(START_BUTTON_PIN);
}

bool TractiveSystem::checkRegenButtonState() {
    bool pres = digitalRead(REGEN_BUTTON_PIN);
    if (pres) {
        regenButtonHeld = true;
    } else if (!pres && regenButtonHeld) {
        regenButtonHeld = false;
        inRegenMode = !inRegenMode;
    }

    // inRegenMode = false;
    return pres;
    // return false;
}

void TractiveSystem::setR2DLED(bool out) {
    digitalWrite(R2DLED_OUTPUT_PIN, (uint8_t)out);
}

void TractiveSystem::setBuzzer(bool out) {
    digitalWrite(BUZZER_SIGNAL_PIN, (uint8_t)out);
}

void TractiveSystem::setBrakeLight(bool out) {
    digitalWrite(BRAKE_LIGHT_PIN, (uint8_t)out);
}

TSStates TractiveSystem::getState() {
    return state;
}

float TractiveSystem::calculateMaxRegenTorque() {
    const float POWER_LIMITER_KP = 0.0; // 0.1
    const float POWER_LIMITER_KI = 0.0; // 0.15
    const float POWER_LIMITER_DT = 10;

    static float integral = 0.0;
    static float tLimit = 0.0;
    static uint32_t lastTime = 0;

    // Limit the rate at which this function is called - for consistent integration
    if ((Task::millis() - lastTime) < POWER_LIMITER_DT) {
        return tLimit;
    } else {
        lastTime = Task::millis();
    }

    float inverterSpeed = inverter.rpm * RPM_TO_RADS_FACTOR;
    float power = battery.packVoltage * battery.terminalCurrent; // positive current is regen
    float powerLimit = battery.packVoltage * battery.maxChargeCurrent;

    // Don't divide by 0 or a negative number!
    if (inverter.rpm < REGEN_MIN_RPM) {
        tLimit = 0.0;
    } else {
        // calculate maximum regen torque using power (p = wt)
        // "mechanical limiter"
        float tMech = powerLimit / inverterSpeed;

        // Feedback control loop, measuring actual power vs power limit
        // "electrical limiter"
        float tError = std::max(power - powerLimit, 0.0f) / inverterSpeed; // how much torque we are over by
        integral += (power - powerLimit) / inverterSpeed;                  // non-saturated error so integral can go down
        integral = std::max(integral, 0.0f);                               // integral can't go below 0, can't increase torque limit

        float tElec = -(POWER_LIMITER_KP * tError + POWER_LIMITER_KI * integral); // positive error means adjust limit backwards

        // clamp torque to maximum torque request
        tLimit = std::clamp(tMech + tElec, 0.0f, -(float)(INVERTER_MINIMUM_TORQUE_REQUEST * INVERTER_NM_PER_UNIT));
    }

    return -1.0 * tLimit;
}

float TractiveSystem::calculateMaxDriveTorque() {
    const float POWER_LIMITER_KP = 0.0; // 0.1
    const float POWER_LIMITER_KI = 0.0; // 0.15
    const float POWER_LIMITER_DT = 10;

    static float integral = 0.0;
    static float tLimit = INVERTER_MAXMIMUM_TORQUE_REQUEST * INVERTER_NM_PER_UNIT;
    static uint32_t lastTime = 0;

    // Limit the rate at which this function is called - for consistent integration
    if ((Task::millis() - lastTime) < POWER_LIMITER_DT) {
        return tLimit;
    } else {
        lastTime = Task::millis();
    }

    float inverterSpeed = inverter.rpm * RPM_TO_RADS_FACTOR;
    float power = battery.packVoltage * -battery.terminalCurrent; // negative current is drive
    float powerLimit = battery.packVoltage * battery.maxDischargeCurrent;

    // Don't divide by 0 or a negative number!
    if (inverter.rpm < 10) {
        tLimit = INVERTER_MAXMIMUM_TORQUE_REQUEST * INVERTER_NM_PER_UNIT; // return maximum torque if rpm < 10
    } else {

        // calculate maximum driving torque using power (p = wt)
        // "mechanical limiter"
        float tMech = powerLimit / inverterSpeed;

        // Feedback control loop, measuring actual power vs power limit
        // "electrical limiter"
        float tError = std::max(power - powerLimit, 0.0f) / inverterSpeed; // how much torque we are over by
        integral += (power - powerLimit) / inverterSpeed;                  // non-saturated error so integral can go down
        integral = std::max(integral, 0.0f);                               // integral can't go below 0, can't increase torque limit

        float tElec = -(POWER_LIMITER_KP * tError + POWER_LIMITER_KI * integral); // positive error means adjust limit backwards

        // clamp torque to maximum torque request
        tLimit = std::clamp(tMech + tElec, 0.0f, (float)(INVERTER_MAXMIMUM_TORQUE_REQUEST * INVERTER_NM_PER_UNIT));
    }

    return tLimit;
}

}
