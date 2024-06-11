#pragma once

#include "ADC.hpp"

namespace wrvcu {

class APPS {
protected:
    float max_voltage;
    float low_connect_voltage;
    float high_connect_voltage;

    float max_angle;

    float angle_offset;
    float angle_range;
    float ignore_fraction;

    ADC* adc;

    int adc_channel;

public:
    void init(ADC* adc_o, int adc_chan, float angle_offset_o, float angle_range_o);

    uint16_t read();

    float getVoltage();
    float getAngle();
    float getFraction();
    float getSaturatedFraction();

    bool isConnected();
};

}