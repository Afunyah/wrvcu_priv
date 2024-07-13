// 16-bit Variables
#define SOC_ADDRESS 0x02
#define POWER_ADDRESS 0x04
#define REGEN_POWER_ADDRESS 0x06 // Regen Power
#define STATUS_ADDRESS 0x08      // Status Icon
#define REGEN_ACTIVE_ADDRESS 0x0A
#define REGEN_COLOUR_ADDRESS 0x0C
#define CELL_MAX_TEMP_ADDRESS 0x0E
#define POST_FUSE_VOLTAGE_ADDRESS 0x10
#define TERMINAL_CURRENT_ADDRESS 0x12
#define SHUTDOWN_COLOUR_ADDRESS 0x14
#define SPEED_ADDRESS 0x16
#define IMD_FAULT_ADDRESS 0x00
#define BMS_FAULT_ADDRESS 0x18

// 128-bit String Variables
#define LOG_ADDRESS_1 0x01
#define LOG_ADDRESS_2 0x00
#define STATE_ADDRESS_1 0x00
#define STATE_ADDRESS_2 0x80
#define REGEN_TEXT_ADDRESS_1 0x00
#define REGEN_TEXT_ADDRESS_2 0x0C
#define SHUTDOWN_TEXT_ADDRESS_1 0x00
#define SHUTDOWN_TEXT_ADDRESS_2 0x14

#pragma once

#include "rtos/mutex.hpp"
#include "rtos/task.hpp"

namespace wrvcu {

class Display {
protected:
    Task task;
    Mutex mutex;
    void writeVar_16Bit(byte PacketAddress, byte lByte, byte hByte);
    void writeVar_128Bit(byte PacketAddress1, byte PacketAddress2, String DisplayText);

public:
    void init();
    void updateDisplay();
};
}