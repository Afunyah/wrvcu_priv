#include "arduino_freertos.h"
#include "car.hpp"
#include "constants.hpp"
#include "devices/inverter.hpp"
#include "pins.hpp"
#include <can/AbstractCanController.hpp>
#include <can/CANController_T4.hpp>
#include <can/CANOpenHost.hpp>

// vehicle globals
namespace wrvcu {

CANController_T4<CAN1> can1;
std::shared_ptr<AbstractCANController> acan1((AbstractCANController*)&can1); // polymorphism

CANOpenHost canOpen;

Inverter inverter;
TractiveSystem ts;

}

using namespace wrvcu;

void test_rtos();
void test_can();
void test_inverter();

void test_inverter();

void setup() {
    Serial.begin(115200); // wait up to 2 seconds for serial connection

    if (CrashReport) {
        Serial.print(CrashReport);
        Serial.println();
        Serial.flush();
    }

    // ---------- setup pins ----------
    pinMode(BUZZER_SIGNAL_PIN, OUTPUT);
    pinMode(BMS_IGNITION_PIN, OUTPUT);
    pinMode(BRAKE_LIGHT_PIN, OUTPUT);

    pinMode(FANPWM_OUTPUT_PIN, OUTPUT);
    pinMode(R2DLED_OUTPUT_PIN, OUTPUT);
    pinMode(SP19_OUTPUT_PIN, OUTPUT);
    pinMode(SP18_OUTPUT_PIN, OUTPUT);

    pinMode(DEBUG_LED_A0, OUTPUT);
    pinMode(DEBUG_LED_A1, OUTPUT);
    pinMode(DEBUG_LED_A2, OUTPUT);

    pinMode(START_BUTTON_PIN, INPUT);
    pinMode(REGEN_BUTTON_PIN, INPUT);
    pinMode(TC_BUTTON_PIN, INPUT);
    pinMode(TSAS_PIN, INPUT);
    pinMode(SP15_PIN, INPUT);
    pinMode(SP14_PIN, INPUT);
    pinMode(HV_ACTIVE_PIN, INPUT);
    pinMode(SCMON_PIN, INPUT);

    // ------------------------------

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    can1.init(CAN_TASK_PRIORITY);

    canOpen.init(acan1); // init canopen main
    canOpen.sendSync();

    // test_rtos();
    // test_can();

    test_inverter();
}

void loop() {
}