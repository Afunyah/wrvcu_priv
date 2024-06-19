#include "TractiveSystem.hpp"
#include "car.hpp"
#include "logging/log.hpp"
#include "pins.hpp"

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
        if (inverter.state == InverterStates::Drive && (!sdcClosed() || battery.contactorState == ContactorStates::Error)) {
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
            if (state != TSStates::Idle || state != TSStates::Error) {
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
            if (battery.contactorState == ContactorStates::Active && sdcClosed() && brakesOn() && startPressed()) {
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
    if (throttle.isCriticalError()) {
        state = TSStates::Error;
    }

    // ADD RPM OR SPEED BUTTON CHECK

    checkRegenButtonState();
    int16_t InverterRequestedTorque = 0.0;

    float driveTorque = throttle.getTorqueRequestFraction();
    float brakeTorque = throttle.getBrakeRegenFraction();
    // float requestedTorque = 0.0;
    // if (inRegenMode) {
    //     requestedTorque = driveTorque + brakeTorque;
    // } else {
    //     requestedTorque = max(0, driveTorque);
    // }
    float requestedTorque = max(0, driveTorque);

    // InverterRequestedTorque = requestedTorque * INVERTER_MAXMIMUM_TORQUE_REQUEST;
    InverterRequestedTorque = requestedTorque * 200;



    // int16_t maxDriveTorqueRequest = calculateMaxDriveTorque() / INVERTER_NM_PER_UNIT; // Positive
    // int16_t maxRegenTorqueRequest = calculateMaxRegenTorque() / INVERTER_NM_PER_UNIT; // Negative

    // if (maxDriveTorqueRequest < 0.0 || maxRegenTorqueRequest > 0.0) {
    //     state = TSStates::Error;
    // }

    // if (inRegenMode /*&& inverterspeedinRPM>250*/) {
    //     InverterRequestedTorque = std::clamp(InverterRequestedTorque, maxRegenTorqueRequest, maxDriveTorqueRequest);
    // } else {
    //     InverterRequestedTorque = std::clamp(InverterRequestedTorque, (int16_t)0, maxDriveTorqueRequest);
    // }

    

    if (InverterRequestedTorque > INVERTER_MAXMIMUM_TORQUE_REQUEST) {
        InverterRequestedTorque = INVERTER_MAXMIMUM_TORQUE_REQUEST;
    }
    if (InverterRequestedTorque < INVERTER_MINIMUM_TORQUE_REQUEST) {
        InverterRequestedTorque = INVERTER_MINIMUM_TORQUE_REQUEST;
    }

    // Serial.println("-------------------------------------- ");
    // Serial.print("APPS Torque: ");
    // Serial.print(driveTorque);
    // Serial.print("\tBrake Torque: ");
    // Serial.println(brakeTorque);

    // Serial.print("Requested Torque: ");
    // Serial.print(requestedTorque);
    // Serial.print("\tTo Inverter: ");
    // Serial.println(InverterRequestedTorque);

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

    return pres;
}

void TractiveSystem::setR2DLED(bool out) {
    digitalWrite(R2DLED_OUTPUT_PIN, (uint8_t)out);
}

void TractiveSystem::setBuzzer(bool out) {
    digitalWrite(BUZZER_SIGNAL_PIN, (uint8_t)out);
}

TSStates TractiveSystem::getState() {
    return state;
}

float TractiveSystem::calculateMaxRegenTorque() {
    // float inverter_speed = inverter_rpm * RPM_TO_RADS_FACTOR;
    float inverter_speed = 0.0;

    float max_torque = -1.0 * BATTERY_VOLTAGE * battery.maxChargeCurrent / inverter_speed;

    return max_torque;
}

float TractiveSystem::calculateMaxDriveTorque() {
    // float inverter_speed = inverter_rpm * RPM_TO_RADS_FACTOR;
    float inverter_speed = 0.0;

    float max_torque = BATTERY_VOLTAGE * battery.maxDischargeCurrent / inverter_speed;

    return max_torque;
}

void TractiveSystem::test_init() {
    mutex.init();

    // initialize outputs to off
    setR2DLED(false);
    setBuzzer(false);

    task.start(
        [this] { test_loop(); }, TRACTIVE_SYSTEM_TASK_PRIORITY, "TractiveSystem_Task");
}

void TractiveSystem::test_loop() {
    while (true) {
        mutex.take();
        DriveSequence();
        // throttle.checkBrakesPlausibility();
        // Serial.print("APPS 1: ");
        // Serial.print(throttle.APPS1.read());
        // Serial.print("\tAPPS 2: ");
        // Serial.println(throttle.APPS2.read());
        // Serial.print("APPS 1: ");
        // Serial.print(throttle.APPS1.getSaturatedFraction());
        // Serial.print("\tAPPS 2: ");
        // Serial.println(throttle.APPS2.getSaturatedFraction());
        mutex.give();
        Task::delay(10);
    }
}

}
