#include "devices/apps.hpp"
#include "constants.hpp"
#include "pins.hpp"


namespace wrvcu {

void APPS::init(ADC* adc_o, int adc_channel, float angle_offset_o, float angle_range_o) {

    this->adc = adc_o;
    this->adc_channel = adc_channel;

    this->angle_offset = angle_offset_o;
    this->angle_range = angle_range_o;
 
}

uint16_t APPS::read() {
    return adc->read(adc_channel);
}

float APPS::getVoltage() {
    return read() * APPS_MAX_VOLTAGE / ADC_RESOLUTION;
}

float APPS::getAngle() {
    return (getVoltage() - APPS_START_FRACTION * APPS_MAX_VOLTAGE) * APPS_MAX_ANGLE / (abs(APPS_END_FRACTION - APPS_START_FRACTION) * APPS_MAX_VOLTAGE);
}

float APPS::getFraction() {
    return (getAngle() - angle_offset - angle_range * APPS_IGNORE_FRACTION) / (angle_range);
}

float APPS::getSaturatedFraction() {
    float frac = getFraction();
    if (frac < 0) {
        frac = 0;
    }

    if (frac >= (1 - APPS_IGNORE_FRACTION)) {
        frac = (1 - APPS_IGNORE_FRACTION);
    }

    return (frac / (1 - APPS_IGNORE_FRACTION));
}

bool APPS::isConnected() {
    float volts = getVoltage();
    return (volts > APPS_LOW_VOLTAGE && volts < APPS_HIGH_VOLTAGE);
}

}