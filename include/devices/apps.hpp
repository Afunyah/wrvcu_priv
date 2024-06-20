#pragma once

#include "ADC.hpp"
#include <queue>
#include "constants.hpp"

namespace wrvcu {

class APPS {
protected:
    uint16_t window_total = 0;
    int window_index = 0;             // the index of the current reading
    uint16_t window_array[APPS_WINDOW];     // the window_array from the analog input

    std::queue<uint16_t> window_queue;

    bool useSmooth = true;

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