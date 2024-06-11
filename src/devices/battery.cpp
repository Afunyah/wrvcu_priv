#include "devices/battery.hpp"
#include "constants.hpp"
#include "rtos/task.hpp"

#define BATTERY_MAX_READS 15

namespace wrvcu {
void Battery::init(AbstractCANController* ican) {
    // Set initial state
    openContactors();

    canQueue.init();
    mutex.init();

    can = ican;

    can->subscribe(BATTERY_BMS_AVAIL_CURRENT_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_BMS_BATT_STATUS_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_BMS_CELL_STATUS_TEMPERATURES_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_BMS_CELL_STATUS_VOLTAGES_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_IMD_INFO_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_IVT_MSG_RESULT_I_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_IVT_MSG_RESULT_U1_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_IVT_MSG_RESULT_U2_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_IVT_MSG_RESULT_U3_FRAME_ID, &canQueue);
    can->subscribe(BATTERY_IVT_MSG_RESULT_W_FRAME_ID, &canQueue);

    // wake up BMS
    wake();

    task.start(
        [this] { loop(); }, BATTERY_TASK_PRIORITY, "Battery_Task");
}

void Battery::loop() {
    while (true) {
        int reads = 0;

        mutex.take();

        while (canQueue.size() > 0 && reads < BATTERY_MAX_READS) {
            reads++;

            CANMessage msg = canQueue.dequeue(TIMEOUT_MAX);

            switch (msg.id) {
            case BATTERY_BMS_AVAIL_CURRENT_FRAME_ID:
                battery_bms_avail_current_t current_msg;
                battery_bms_avail_current_unpack(&current_msg, (uint8_t*)&msg.data, 8);
                maxDischargeCurrent = battery_bms_avail_current_max_discharge_decode(current_msg.max_discharge);
                maxChargeCurrent = battery_bms_avail_current_max_charge_decode(current_msg.max_charge);
                break;
            case BATTERY_BMS_BATT_STATUS_FRAME_ID:
                battery_bms_batt_status_t status_msg;
                battery_bms_batt_status_unpack(&status_msg, (uint8_t*)&msg.data, 8);
                contactorState = static_cast<ContactorStates>(status_msg.status);
                chargeRemaining = battery_bms_batt_status_q_remain_nom_decode(status_msg.q_remain_nom);
                SoC = battery_bms_batt_status_so_c_decode(status_msg.so_c);
                break;
            case BATTERY_BMS_CELL_STATUS_TEMPERATURES_FRAME_ID:
                battery_bms_cell_status_temperatures_t cell_temp_msg;
                battery_bms_cell_status_temperatures_unpack(&cell_temp_msg, (uint8_t*)&msg.data, 8);
                cellAvgTemp = battery_bms_cell_status_temperatures_avg_cell_temp_decode(cell_temp_msg.avg_cell_temp);
                cellMinTemp = battery_bms_cell_status_temperatures_min_cell_temp_decode(cell_temp_msg.min_cell_temp);
                cellMaxTemp = battery_bms_cell_status_temperatures_max_cell_temp_decode(cell_temp_msg.max_cell_temp);
                break;
            case BATTERY_BMS_CELL_STATUS_VOLTAGES_FRAME_ID:
                battery_bms_cell_status_voltages_t cell_volt_msg;
                battery_bms_cell_status_voltages_unpack(&cell_volt_msg, (uint8_t*)&msg.data, 8);
                cellAvgVoltage = battery_bms_cell_status_voltages_avg_cell_voltage_decode(cell_volt_msg.avg_cell_voltage);
                cellMinVoltage = battery_bms_cell_status_voltages_min_cell_voltage_decode(cell_volt_msg.min_cell_voltage);
                cellMaxVoltage = battery_bms_cell_status_voltages_max_cell_voltage_decode(cell_volt_msg.max_cell_voltage);
                break;
            case BATTERY_IMD_INFO_FRAME_ID:
                battery_imd_info_t imd_msg;
                battery_imd_info_unpack(&imd_msg, (uint8_t*)&msg.data, 8);
                imdIsoRes = battery_imd_info_imd_r_iso_decode(imd_msg.imd_r_iso);
                break;
            case BATTERY_IVT_MSG_RESULT_I_FRAME_ID:
                battery_ivt_msg_result_i_t ivt_i_msg;
                battery_ivt_msg_result_i_unpack(&ivt_i_msg, (uint8_t*)&msg.data, 8);
                terminalCurrent = battery_ivt_msg_result_i_ivt_result_i_decode(ivt_i_msg.ivt_result_i);
                break;
            case BATTERY_IVT_MSG_RESULT_U1_FRAME_ID:
                battery_ivt_msg_result_u1_t ivt_u1_msg;
                battery_ivt_msg_result_u1_unpack(&ivt_u1_msg, (uint8_t*)&msg.data, 8);
                packVoltage = battery_ivt_msg_result_u1_ivt_result_u1_decode(ivt_u1_msg.ivt_result_u1);
                break;
            case BATTERY_IVT_MSG_RESULT_U2_FRAME_ID:
                battery_ivt_msg_result_u2_t ivt_u2_msg;
                battery_ivt_msg_result_u2_unpack(&ivt_u2_msg, (uint8_t*)&msg.data, 8);
                preFuseVoltage = battery_ivt_msg_result_u2_ivt_result_u2_decode(ivt_u2_msg.ivt_result_u2);
                break;
            case BATTERY_IVT_MSG_RESULT_U3_FRAME_ID:
                battery_ivt_msg_result_u3_t ivt_u3_msg;
                battery_ivt_msg_result_u3_unpack(&ivt_u3_msg, (uint8_t*)&msg.data, 8);
                postFuseVoltage = battery_ivt_msg_result_u3_ivt_result_u3_decode(ivt_u3_msg.ivt_result_u3);
                break;
            case BATTERY_IVT_MSG_RESULT_W_FRAME_ID:
                battery_ivt_msg_result_w_t ivt_w_msg;
                battery_ivt_msg_result_w_unpack(&ivt_w_msg, (uint8_t*)&msg.data, 8);
                power = battery_ivt_msg_result_w_ivt_result_w_decode(ivt_w_msg.ivt_result_w);
                break;
            }
        }

        mutex.give();

        // if (reads >= BATTERY_MAX_READS)
        //     WARN("Battery task max reads exceeded!");

        Task::delay(5);
    }
}

// void Battery::closeContactors(){
//     digitalWrite(BMS_IGN_SIGNAL_PIN, HIGH);
// }

// void Battery::openContactors(){
//     digitalWrite(BMS_IGN_SIGNAL_PIN, LOW);
// }

void Battery::wake() {
    // Based off n-BMS user manual section 5.5.1
    // Modified so the CAN ID has 3 bits dominant, 3 bits recessive, 3 bits dominant
    // 6us each at 500kbps, meets the 5us minimum of the standard
    // done by setting 3 bits in the id next to where the stuffing bit would go

    CANMessage msg{
        .id = 0x70,
        .len = 0
    };

    can->send(msg);
}

}
