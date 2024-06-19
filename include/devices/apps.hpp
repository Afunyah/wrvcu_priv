#pragma once

#include "ADC.hpp"

namespace wrvcu {

class APPS {
protected:
    float angle_offset;
    float angle_range;

    ADC* adc;

    int adc_channel;

public:
    void init(ADC* adc_o, int adc_chan, float angle_offset_o, float angle_range_o);

    uint16_t read();

    float getVoltage();
    float getAngle();
    float getSaturatedFraction();
    float getProcessedAngle();

    float getRegenFraction();

    bool isConnected();
};

}