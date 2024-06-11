#pragma once

#include "MAX22530.h"
#include "rtos/mutex.hpp"
#include <SPI.h>

namespace wrvcu {

class ADC {
protected:
    MAX22530* adc;

    Mutex mutex;

public:
    void init(uint8_t cspin, SPIClass* theSPI, bool crc_enable);

    int read(int channel);
};
}