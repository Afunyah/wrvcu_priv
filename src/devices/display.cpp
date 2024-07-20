#include "devices/display.hpp"
#include "car.hpp"

namespace wrvcu {
void Display::init() {
    mutex.init();
    task.start(
        [this] { updateDisplay(); }, DISPLAY_TASK_PRIORITY, "Display_Task");
}
void Display::writeVar_16Bit(byte PacketAddress, byte lByte, byte hByte) {
    Serial7.write(0xaa); // packet head
    Serial7.write(0x3d); // VP_N16 write command
    Serial7.write(0x00); // VP_N16 address
    Serial7.write(0x08);
    Serial7.write(0x00);
    Serial7.write(PacketAddress);
    Serial7.write(hByte); // VP_N16 data high byte
    Serial7.write(lByte); // VP_N16 data low byte
    Serial7.write(0xcc);  // packet tail
    Serial7.write(0x33);  // packet tail
    Serial7.write(0xc3);  // packet tail
    Serial7.write(0x3c);  // packet tail
}
void Display::writeVar_128Bit(byte PacketAddress1, byte PacketAddress2, String DisplayText) {
    Serial7.write(0xaa); // packet head
    Serial7.write(0x42); // VP_STR write command
    Serial7.write(0x00); // VP_STR address
    Serial7.write(0x00);
    Serial7.write(PacketAddress1);
    Serial7.write(PacketAddress2);
    Serial7.print(DisplayText);
    Serial7.write('\0'); // packet tail
    Serial7.write(0xcc); // packet tail
    Serial7.write(0x33); // packet tail
    Serial7.write(0xc3); // packet tail
    Serial7.write(0x3c); // packet tail
}

void Display::updateDisplay() {
    while (true) {
        // mutex.take();
        writeVar_16Bit(SOC_ADDRESS, lowByte((uint16_t)battery.SoC), highByte((uint16_t)battery.SoC));
        writeVar_16Bit(STATUS_ADDRESS, lowByte((uint16_t)ts.getState()), highByte((uint16_t)ts.getState()));
        writeVar_16Bit(REGEN_ACTIVE_ADDRESS, lowByte((uint16_t)ts.inRegenMode), highByte((uint16_t)ts.inRegenMode));
        writeVar_16Bit(CELL_MAX_TEMP_ADDRESS, lowByte((uint16_t)battery.cellMaxTemp), highByte((uint16_t)battery.cellMaxTemp));
        writeVar_16Bit(CELL_MAX_VOLTAGE_ADDRESS, lowByte((uint16_t)battery.cellMaxVoltage), highByte((uint16_t)battery.cellMaxVoltage));
        writeVar_16Bit(CELL_MIN_VOLTAGE_ADDRESS, lowByte((uint16_t)battery.cellMinVoltage), highByte((uint16_t)battery.cellMinVoltage));

        // float dataloggerDistance = datalogger.getDistanceInKM();
        // int raceProgress = map(std::clamp((int)(MAX_ENDUR_DIST_KM - dataloggerDistance), 0, MAX_ENDUR_DIST_KM), 0, MAX_ENDUR_DIST_KM, 0, 100);

        // writeVar_16Bit(DISTANCE_ADDRESS, lowByte((uint16_t)dataloggerDistance), highByte((uint16_t)dataloggerDistance));
        // writeVar_16Bit(RACE_PROGRESS_ADDRESS, lowByte((uint16_t)raceProgress), highByte((uint16_t)raceProgress));

        int inverter_rpm_dial = map(std::clamp((int)inverter.rpm, 0, MAX_RPM), 0, MAX_RPM, 0, 360);

        writeVar_16Bit(SPEED_ADDRESS, lowByte((uint16_t)inverter_rpm_dial), highByte((uint16_t)inverter_rpm_dial));
        writeVar_16Bit(SPEED_ADDRESS_2, lowByte((uint16_t)inverter.rpm), highByte((uint16_t)inverter.rpm));

        // VCU State Text
        String status_text;
        switch (ts.getState()) {
        case (TSStates::Idle):
            status_text = "IDLE";
            break;
        case (TSStates::CloseContactors):
            status_text = "CLOSE CONTACTORS";
            break;
        case (TSStates::StartInverter):
            status_text = "START INVERTER";
            break;
        case (TSStates::WaitR2D):
            status_text = "WAIT R2D";
            break;
        case (TSStates::Buzzer):
            status_text = "BUZZER";
            break;
        case (TSStates::Driving):
            status_text = "DRIVE";
            break;
        case (TSStates::Error):
            status_text = "ERROR";
            break;
        }
        writeVar_128Bit(STATE_ADDRESS_1, STATE_ADDRESS_2, status_text);

        if (ts.inRegenMode) {
            writeVar_128Bit(REGEN_TEXT_ADDRESS_1, REGEN_TEXT_ADDRESS_2, "Enabled");
        } else {
            writeVar_128Bit(REGEN_TEXT_ADDRESS_1, REGEN_TEXT_ADDRESS_2, "Disabled");
        }

        // SC Circuit Text
        if (ts.sdcClosed()) {
            writeVar_128Bit(SHUTDOWN_TEXT_ADDRESS_1, SHUTDOWN_TEXT_ADDRESS_2, "Closed");
        } else {
            writeVar_128Bit(SHUTDOWN_TEXT_ADDRESS_1, SHUTDOWN_TEXT_ADDRESS_2, "Open");
        }

        // mutex.give();
        Task::delay(10);
    }
}
}