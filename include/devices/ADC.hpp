#pragma once

#include "MAX22530.h"
#include "rtos/mutex.hpp"
#include <SPI.h>

namespace wrvcu {

class ADC {
public:
    ADC(uint8_t cspin, SPIClass* theSPI);
    void init(bool crc_enable);
    int read(int channel);

protected:
    MAX22530 adc;
    Mutex mutex;
};
}