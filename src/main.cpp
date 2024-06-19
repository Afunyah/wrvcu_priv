#include "SD.h"
#include "arduino_freertos.h"
#include "can/CANController_T4.hpp"
#include "can/CANMessage.hpp"
#include "can/CANOpenHost.hpp"
#include "car.hpp"
#include "constants.hpp"
#include "pins.hpp"
#include "vcu_log.h"

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

static wrvcu::Task canLoggingTask;
void canLoggingLoop() {
    while (true) {
        vcu_log_vcu_log_t log_msg;

        log_msg.vcu_state = static_cast<int>(ts.getState());
        log_msg.scmon = ts.sdcClosed();
        log_msg.apps_disconnect = throttle.APPSDisconnectedError;
        log_msg.apps_plausibility = throttle.APPSPlausibilityError;
        log_msg.brake_disconnect = throttle.brakeDisconnectedError;
        log_msg.brake_plausibility = throttle.brakePlausibilityError;
        log_msg.brake_hard = throttle.hardBrakeError;
        log_msg.regen_active = ts.inRegenMode;
        log_msg.apps_raw = vcu_log_vcu_log_apps_raw_encode(throttle.getThrottleFraction() * INVERTER_MAXMIMUM_TORQUE_REQUEST);
        log_msg.brake_raw = throttle.getBrakePressure1();

        CANMessage msg;
        msg.id = VCU_LOG_VCU_LOG_FRAME_ID;
        msg.len = VCU_LOG_VCU_LOG_LENGTH;
        vcu_log_vcu_log_pack((uint8_t*)&msg.data, &log_msg, 8);

        can1.send(msg);

        wrvcu::Task::delay(100);
    }
}

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

    canLoggingTask.start([] { canLoggingLoop(); }, TASK_PRIORITY_DEFAULT - 3, "CAN_Logging_Task");

    startScheduler();

    // test_throttle_func();
}

void loop() {}
