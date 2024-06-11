#include "arduino_freertos.h"
#include "rtos/rtos.hpp"
#include <can/AbstractCANController.hpp>
#include <can/CANController_T4.hpp>
#include <can/CANOpenHost.hpp>
#include <devices/inverter.hpp>

using namespace wrvcu;

static Task taskA;

static CANController_T4<CAN1> can1;
static Inverter inverter;

void test_inverter_task() {
    while (true) {
        auto byte = Serial.read();
        if (byte != -1) {
            if (byte == 's') {
                printf("Trying to start\n");
                inverter.start();
            } else if (byte == 't') {
                printf("Trying to stop\n");
                inverter.stop();
            } else if (byte >= '0' && byte < '9') {
                printf("Getting torque\n");
                int number = (int)(byte - '0');
                int torqueRequest = number * 100;

                if (torqueRequest < 1000)
                    inverter.sendTorque(torqueRequest);
            }
        }

        Task::delay(50);
    }
}

void test_inverter() {
    pinMode(LED_BUILTIN, OUTPUT);
    // digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);

    taskA.start(test_inverter_task, TASK_PRIORITY_DEFAULT, "Inv");

    can1.init(TASK_PRIORITY_DEFAULT + 3);
    // canOpen.init((&can1)); // init canopen main
    inverter.init((&can1), 1);
    // inverter.start();

    startScheduler();
}