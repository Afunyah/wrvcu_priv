#pragma once
#include "can/CANOpenDevice.hpp"

#define VCU_NODE_ID 5

#define CAN_TASK_PRIORITY (TASK_PRIORITY_DEFAULT + 3)
#define INVERTER_TASK_PRIORITY (TASK_PRIORITY_DEFAULT + 2)
#define TRACTIVE_SYSTEM_TASK_PRIORITY (TASK_PRIORITY_DEFAULT + 2)
#define BATTERY_TASK_PRIORITY (TASK_PRIORITY_DEFAULT + 1)
#define CANOPEN_HOST_TASK_PRIORITY (TASK_PRIORITY_DEFAULT)

#define INVERTER_SEND_PERIOD 500
#define BMS_NMT_SEND_PERIOD 500

#define INVERTER_TPDO1 0x180
#define INVERTER_TPDO2 0x280
#define INVERTER_TPDO3 0x380
#define INVERTER_TPDO4 0x480

#define INVERTER_RPDO1 0x200

#define INVERTER_CONTROLWORD_INDEX 0x6040
#define INVERTER_CONTROLWORD_SUBINDEX 0x00

#define INVERTER_CW_DISABLE_PWM 6
#define INVERTER_CW_ENABLE_PWM 15

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Number between 0 and 1000
#define INVERTER_MAXMIMUM_TORQUE_REQUEST 100
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// TODO: These could go in dynamic configuration.
#define CONTACTOR_CLOSE_TIMEOUT 10000 // ms, the maximum time to wait for contactors to close
#define APPS_TIMEOUT 100              // ms, maximum APPS implausibility length

#define BRAKEPRESSURE1_THRESHOLD 600
#define BRAKEPRESSURE2_THRESHOLD 500
#define BRAKE_TIMEOUT 500

#define BUZZER_LENGTH 1500 // ms, how long the buzzer buzzes

#define INV_CW_INDEX 0x6040
#define INV_CW_SUBINDEX 0x00

#define ADC_FREQUENCY 1000000

#define APPS1_CHANNEL 0
#define APPS1_ANGLE_OFFSET 94
#define APPS1_ANGLE_RANGE -19

#define APPS2_CHANNEL 1
#define APPS2_ANGLE_OFFSET 170
#define APPS2_ANGLE_RANGE 19

#define APPS_IGNORE_FRACTION 0.2

#define APPS_LOW_VOLTAGE 0.18
#define APPS_HIGH_VOLTAGE 1.62

#define APPS_MAX_VOLTAGE 1.8
