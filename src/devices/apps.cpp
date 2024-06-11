#include "devices/apps.hpp"
#include "constants.hpp"
#include "pins.hpp"

namespace wrvcu {

void APPS::init(ADC* adc_o, int adc_channel, float angle_offset_o, float angle_range_o) {

    this->adc = adc_o;
    this->adc_channel = adc_channel;

    this->max_voltage = APPS_MAX_VOLTAGE;
    this->max_angle = 360;

    this->angle_offset = angle_offset_o;
    this->angle_range = angle_range_o;
    this->ignore_fraction = APPS_IGNORE_FRACTION;

    this->high_connect_voltage = APPS_HIGH_VOLTAGE;
    this->low_connect_voltage = APPS_LOW_VOLTAGE;
}

uint16_t APPS::read() {
    return adc->read(adc_channel);
}

float APPS::getVoltage() {
    return read() * max_voltage / 4096;
}

float APPS::getAngle() {
    return (getVoltage() - 0.1 * max_voltage) * max_angle / (0.8 * max_voltage);
}

float APPS::getFraction() {
    return (getAngle() - angle_offset - angle_range * ignore_fraction) / (angle_range);
}

float APPS::getSaturatedFraction() {
    float frac = getFraction();
    if (frac < 0) {
        frac = 0;
    }

    if (frac >= (1 - ignore_fraction)) {
        frac = (1 - ignore_fraction);
    }

    return (frac / (1 - ignore_fraction));
}

bool APPS::isConnected() {
    return (getVoltage() >= low_connect_voltage && getVoltage() <= high_connect_voltage);
}

}