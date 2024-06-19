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
            if (battery.contactorState == ContactorStates::Active && sdcClosed() && /*brakesOn() &&*/ startPressed()) {
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
            // inverter.stop();
            break;
        }

        if (state == TSStates::Driving) {
            //     // if(throttle.isCriticalError()){
            //     //     state = TSStates::Error;
            InverterRequestedTorque = throttle.getTorqueRequestFraction() * INVERTER_MAXMIMUM_TORQUE_REQUEST;
            if (InverterRequestedTorque > INVERTER_MAXMIMUM_TORQUE_REQUEST) {
                // TODO: Fix for regen
                InverterRequestedTorque = INVERTER_MAXMIMUM_TORQUE_REQUEST;
            }
            inverter.sendTorque(InverterRequestedTorque);
        }

        mutex.give();

        Task::delay_until(&prev, 10);
    }
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

void TractiveSystem::setR2DLED(bool out) {
    digitalWrite(R2DLED_OUTPUT_PIN, (uint8_t)out);
}

void TractiveSystem::setBuzzer(bool out) {
    digitalWrite(BUZZER_SIGNAL_PIN, (uint8_t)out);
}

int TractiveSystem::getBrakePressure1() {
    return adc.read(BRAKEPRESSURE1_CHANNEL);
}

int TractiveSystem::getBrakePressure2() {
    return adc.read(BRAKEPRESSURE2_CHANNEL);
}

bool TractiveSystem::brakesOn() {
    if (getBrakePressure1() > BRAKEPRESSURE1_THRESHOLD || getBrakePressure2() > BRAKEPRESSURE2_THRESHOLD) {
        return true;
    } else {
        return false;
    }
}

bool TractiveSystem::checkBrakesPlausibility() {
    if (getBrakePressure1() > 2 * getBrakePressure2() + BRAKE_PLAUSIBILITY_FRACTION * ADC_RESOLUTION || getBrakePressure1() < 2 * getBrakePressure2() - BRAKE_PLAUSIBILITY_FRACTION * ADC_RESOLUTION) {
        return false;
    } else {
        return true;
    }
}

bool TractiveSystem::brakesConnected() {
    if (getBrakePressure1() > BRAKEPRESSURE1_LOW_ADC && getBrakePressure1() < BRAKEPRESSURE1_HIGH_ADC) {
        return true;
    } else {
        return false;
    }
}

TSStates TractiveSystem::getState() {
    return state;
}

}
