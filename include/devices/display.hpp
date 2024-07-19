// 16-bit Variables
#define SOC_ADDRESS 0x02           // %
#define STATUS_ADDRESS 0x08        // Status Icon
#define REGEN_ACTIVE_ADDRESS 0x0A  // Regen Icon
#define CELL_MAX_TEMP_ADDRESS 0x0E // deg C 0-60
#define SPEED_ADDRESS 0x16         // Dial - scaled to 360
#define SPEED_ADDRESS_2 0x1A       // Raw RPM value

#define DISTANCE_ADDRESS 0x1C      // km
#define RACE_PROGRESS_ADDRESS 0x1E // %

// 128-bit String Variables
#define STATE_ADDRESS_1 0x00
#define STATE_ADDRESS_2 0x80
#define REGEN_TEXT_ADDRESS_1 0x01
#define REGEN_TEXT_ADDRESS_2 0x80
#define SHUTDOWN_TEXT_ADDRESS_1 0x02
#define SHUTDOWN_TEXT_ADDRESS_2 0x00

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