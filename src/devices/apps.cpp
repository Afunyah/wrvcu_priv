#include "devices/apps.hpp"
#include "constants.hpp"
#include "pins.hpp"

namespace wrvcu {

void APPS::init(ADC* adc_o, int adc_channel, float angle_offset_o, float angle_range_o) {

    this->adc = adc_o;
    this->adc_channel = adc_channel;

    this->angle_offset = angle_offset_o;
    this->angle_range = angle_range_o;

    if (APPS_USE_SMOOTH) {
        // for (int i = 0; i < APPS_WINDOW; i++) {
        //     window_array[i]= 0;
        // }
        // for (int i = 0; i < APPS_WINDOW; i++) {
        //     read();
        // }
    }
}

uint16_t APPS::read() {
    uint16_t val = adc->read(adc_channel);

    if (APPS_USE_SMOOTH) {
        window_total = window_total - window_array[window_index]; // Remove earliest reading from sum
        window_array[window_index] = val;                         // Add reading to the array
        window_total = window_total + window_array[window_index]; // Add reading to sum
        window_index = window_index + 1;                          // go to the next position in the array

        if (window_index >= APPS_WINDOW) {
            window_index = 0;
        }
        // window_total -= window_queue.front();
        // window_total += val;
        // window_queue.pop();
        // window_queue.push(val);

        uint16_t smooth_val = window_total / APPS_WINDOW;
        // Serial.print("True:");
        // Serial.print(val);
        // Serial.print(",");
        // Serial.print("Smooth:");
        // Serial.println(smooth_val);

        val = smooth_val;
    }

    return val;
}

float APPS::getVoltage() {
    return read() * APPS_MAX_VOLTAGE / ADC_RESOLUTION;
}

float APPS::getAngle() {
    return (getVoltage() - APPS_START_FRACTION * APPS_MAX_VOLTAGE) * APPS_MAX_ANGLE / (abs(APPS_END_FRACTION - APPS_START_FRACTION) * APPS_MAX_VOLTAGE);
}

float APPS::getProcessedAngle() {
    return (getAngle() - angle_offset - angle_range * APPS_IGNORE_FRACTION);
}

float APPS::getRegenFraction() {
    float frac_val = getProcessedAngle() / (angle_range * APPS_IGNORE_FRACTION);
    return (std::clamp(frac_val, -1.0f, 0.0f));
}

float APPS::getSaturatedFraction() {
    float frac_val = getProcessedAngle() / (angle_range);
    return (std::clamp(frac_val, 0.0f, (float)(1.0f - APPS_IGNORE_FRACTION)) / (1 - APPS_IGNORE_FRACTION));
}

bool APPS::isConnected() {
    float volts = getVoltage();
    return (volts > APPS_LOW_VOLTAGE && volts < APPS_HIGH_VOLTAGE);
}

}