#include "SD.h"
#include "arduino_freertos.h"
#include "can/CANController_T4.hpp"
#include "can/CANOpenHost.hpp"
#include "car.hpp"
#include "constants.hpp"
#include "pins.hpp"

// vehicle globals
namespace wrvcu {
TractiveSystem ts;
Inverter inverter;
Battery battery;
ThrottleManager throttle;

CANController_T4<CAN1> can1;
CANOpenHost canOpen;

ADC adc;

}

void test_throttle_func();
void test_rtos();
void test_can();
void test_inverter();

using namespace wrvcu;

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

    SD.begin(BUILTIN_SDCARD);

    setLogLevel(LogLocation::STDOUT, LogLevel::DEBUG);
    setLogLevel(LogLocation::FILE, LogLevel::DEBUG);
    setLogLevel(LogLocation::RADIO, LogLevel::DEBUG);

    // // ---------- start tasks ----------

    loggingInit();

    can1.init(TASK_PRIORITY_DEFAULT + 3);
    canOpen.init((&can1));
    inverter.init((&can1), 1);

    battery.init((&can1));

    adc.init(ADC_CS, &SPI, false);
    throttle.init(&adc);

    ts.init();

    startScheduler();

    // test_throttle_func();
}

void loop() {}
