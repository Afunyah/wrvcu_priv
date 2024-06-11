#include "arduino_freertos.h"
#include "rtos/rtos.hpp"
#include <can/CANController_T4.hpp>

using namespace wrvcu;

static Task taskA;

static CANController_T4<CAN1> can1;
static CANController_T4<CAN3> can2;

static Queue<CANMessage, 256> canQueue;

void test_can_task() {
    can2.subscribe(0, &canQueue);
    while (true) {

        if (canQueue.size() > 0) {
            CANMessage rxMsg = canQueue.dequeue(0);
            Serial.print("CAN2 ");
            Serial.print("  ID: 0x");
            Serial.print(rxMsg.id, HEX);
            Serial.print("  EXT: ");
            Serial.print(rxMsg.flags.extended);
            Serial.print("  LEN: ");
            Serial.print(rxMsg.len);
            Serial.print(" DATA: ");
            for (uint8_t i = 0; i < 8; i++) {
                Serial.print(rxMsg.data[i]);
                Serial.print(" ");
            }
            Serial.print("  TS: ");
            Serial.println(rxMsg.timestamp);
        }

        CANMessage msg;
        msg.data[0] = 1;
        msg.data[1] = 2;
        msg.data[2] = 3;
        msg.data[3] = 4;
        msg.data[4] = 5;
        msg.data[5] = 6;
        msg.data[6] = 7;
        msg.data[7] = 8;
        can1.send(msg);

        Task::delay(10);
    }
}

void test_can() {
    pinMode(LED_BUILTIN, OUTPUT);
    // digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);

    taskA.start(test_can_task, TASK_PRIORITY_DEFAULT, "CAN");

    can1.init(TASK_PRIORITY_DEFAULT + 3);
    can2.init(TASK_PRIORITY_DEFAULT + 3);

    canQueue.init();

    startScheduler();
}